#include "uavsim/config.hpp"
#include "uavsim/version.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <stdexcept>

namespace {
using Json = nlohmann::json;
using namespace uavsim;
constexpr double dt = 0.004;
constexpr std::uint64_t max_steps = 900000;
constexpr double replay_tolerance = 1e-9;
Json vector_json(const Vec3& v) { return Json::array({v.x(), v.y(), v.z()}); }
Json state_json(const State& s) {
    return {{"position_m", vector_json(s.position_m)}, {"velocity_mps", vector_json(s.velocity_mps)},
        {"orientation_wxyz", Json::array({s.orientation.w(), s.orientation.x(), s.orientation.y(), s.orientation.z()})},
        {"angular_velocity_radps", vector_json(s.angular_velocity_radps)}};
}
double numeric(const Json& value) {
    if (!value.is_number()) throw std::invalid_argument("recording field must be numeric");
    const double n = value.get<double>();
    if (!std::isfinite(n)) throw std::invalid_argument("recording field must be finite");
    return n;
}
Vec3 vector_from_json(const Json& value) {
    if (!value.is_array() || value.size() != 3) throw std::invalid_argument("invalid recorded vector");
    return {numeric(value[0]), numeric(value[1]), numeric(value[2])};
}
State state_from_json(const Json& value) {
    State s;
    s.position_m = vector_from_json(value.at("position_m"));
    s.velocity_mps = vector_from_json(value.at("velocity_mps"));
    s.angular_velocity_radps = vector_from_json(value.at("angular_velocity_radps"));
    const auto& q = value.at("orientation_wxyz");
    if (!q.is_array() || q.size() != 4) throw std::invalid_argument("invalid recorded quaternion");
    s.orientation = Quat(numeric(q[0]), numeric(q[1]), numeric(q[2]), numeric(q[3]));
    validate_state(s);
    return s;
}
double state_error(const State& a, const State& b) {
    return std::max({(a.position_m-b.position_m).norm(), (a.velocity_mps-b.velocity_mps).norm(),
        (a.angular_velocity_radps-b.angular_velocity_radps).norm(), (a.orientation.coeffs()-b.orientation.coeffs()).norm()});
}
Json input_json(const Input& input) {
    return {{"armed",input.armed},{"mode",flight_mode_name(input.mode)}, {"roll",input.roll},
        {"pitch",input.pitch},{"yaw",input.yaw},{"collective",input.collective},{"altitude_m",input.altitude_m}};
}
Input input_from_json(const Json& value) {
    Input result;
    if (!value.at("armed").is_boolean()) throw std::invalid_argument("recorded armed field must be boolean");
    result.armed = value.at("armed").get<bool>();
    const auto mode = value.at("mode").get<std::string>();
    if (mode == flight_mode_name(FlightMode::Rate)) result.mode = FlightMode::Rate;
    else if (mode == flight_mode_name(FlightMode::Attitude)) result.mode = FlightMode::Attitude;
    else if (mode == flight_mode_name(FlightMode::AltitudeHold)) result.mode = FlightMode::AltitudeHold;
    else throw std::invalid_argument("unsupported recorded flight mode");
    result.roll = numeric(value.at("roll"));
    result.pitch = numeric(value.at("pitch"));
    result.yaw = numeric(value.at("yaw"));
    result.collective = numeric(value.at("collective"));
    result.altitude_m = numeric(value.at("altitude_m"));
    return result;
}
Json read_json(const std::filesystem::path& file, std::uintmax_t limit = 1048576) {
    if (std::filesystem::file_size(file) > limit) throw std::invalid_argument("session JSON exceeds size limit");
    std::ifstream stream(file);
    if (!stream) throw std::runtime_error("cannot open session JSON");
    return Json::parse(stream);
}
void write_json(const std::filesystem::path& file, const Json& data) {
    std::ofstream stream(file);
    stream << data.dump(2) << '\n';
    stream.close();
    if (!stream) throw std::runtime_error("session metadata write failed");
}
class Recorder {
public:
    Recorder(const std::filesystem::path& output, const LoadedVehicle& loaded) : directory(output) {
        if (std::filesystem::exists(output)) throw std::invalid_argument("output directory already exists; use a new session directory");
        if (!std::filesystem::create_directories(output)) throw std::runtime_error("cannot create session directory");
        states.open(output / "telemetry.csv");
        inputs.open(output / "inputs.jsonl");
        std::ofstream config(output / "vehicle.json", std::ios::binary);
        config << loaded.source_json;
        config.close();
        if (!states || !inputs || !config) throw std::runtime_error("cannot create session files");
        states << std::setprecision(17)
            << "time_s,step,aircraft_id,position_e_m,position_n_m,position_u_m,velocity_e_mps,velocity_n_mps,velocity_u_mps,"
            << "qw,qx,qy,qz,body_p_radps,body_q_radps,body_r_radps,"
            << "motor_0,motor_1,motor_2,motor_3,rotor_0_radps,rotor_1_radps,rotor_2_radps,rotor_3_radps,"
            << "thrust_0_n,thrust_1_n,thrust_2_n,thrust_3_n,mode,armed,grounded,wind_e_mps,wind_n_mps,wind_u_mps\n";
    }
    void state(const Snapshot& s, const Simulation& simulation) {
        states << s.time << ',' << s.step << ',' << simulation.config().id;
        for (int i = 0; i < 3; ++i) states << ',' << s.state.position_m[i];
        for (int i = 0; i < 3; ++i) states << ',' << s.state.velocity_mps[i];
        states << ',' << s.state.orientation.w() << ',' << s.state.orientation.x()
               << ',' << s.state.orientation.y() << ',' << s.state.orientation.z();
        for (int i = 0; i < 3; ++i) states << ',' << s.state.angular_velocity_radps[i];
        for (auto v : s.motor_commands) states << ',' << v;
        for (auto v : s.rotor_speeds) states << ',' << v;
        for (auto v : s.thrusts) states << ',' << v;
        states << ',' << flight_mode_name(s.mode) << ',' << s.armed << ',' << s.grounded;
        for (int i = 0; i < 3; ++i) states << ',' << simulation.environment().wind_mps[i];
        states << '\n';
        if (!states) throw std::runtime_error("telemetry write failed");
    }
    void record(const Input& input, const Snapshot& after) {
        inputs << Json({{"step",after.step},{"input",input_json(input)},{"state",state_json(after.state)}}).dump() << '\n';
        if (!inputs) throw std::runtime_error("input recording write failed");
    }
    void finish(const Json& metadata) {
        states.close(); inputs.close();
        if (!states || !inputs) throw std::runtime_error("session file close failed");
        write_json(directory / "metadata.json", metadata);
    }
private:
    std::filesystem::path directory;
    std::ofstream states, inputs;
};
void usage() {
    std::cout << "UAVSim headless " << UAVSIM_VERSION_STRING << "\n"
        << "  uavsim_headless validate --vehicle FILE\n"
        << "  uavsim_headless hover --vehicle FILE --duration SECONDS --output NEW_DIRECTORY\n"
        << "  uavsim_headless scenario --scenario takeoff-land --vehicle FILE --duration SECONDS --output NEW_DIRECTORY\n"
        << "  uavsim_headless replay --session DIRECTORY --vehicle FILE --output NEW_DIRECTORY\n"
        << "Recorded-input replay compares the trajectory at each 0.004 s step; this is not state playback.\n";
}
}
int main(int argc, char** argv) {
    try {
        if (argc == 2 && std::string(argv[1]) == "--help") { usage(); return 0; }
        if (argc < 2) { usage(); return 2; }
        const std::string command = argv[1];
        if (command != "validate" && command != "hover" && command != "scenario" && command != "replay")
            throw std::invalid_argument("unknown command; use --help");
        std::map<std::string,std::string> options;
        for (int i = 2; i < argc; i += 2) {
            if (i + 1 >= argc) throw std::invalid_argument("option requires a value");
            std::string key = argv[i];
            const bool allowed = key == "--vehicle" || (command != "validate" && key == "--output")
                || ((command == "hover" || command == "scenario") && key == "--duration")
                || (command == "scenario" && key == "--scenario") || (command == "replay" && key == "--session");
            if (!allowed || !options.emplace(key,argv[i+1]).second) throw std::invalid_argument("unknown or duplicate option: " + key);
        }
        auto required = [&](const std::string& key) -> std::string {
            if (!options.contains(key) || options.at(key).empty()) throw std::invalid_argument("missing option: " + key);
            return options.at(key);
        };
        const auto loaded = load_vehicle(required("--vehicle"));
        if (command == "validate") {
            std::cout << "PASS vehicle=" << loaded.vehicle.id << " sha256=" << loaded.sha256 << '\n';
            return 0;
        }
        Simulation simulation(loaded.vehicle);
        State initial;
        std::array<double,4> initial_rotors{};
        std::uint64_t steps = 0;
        std::string scenario = command;
        std::ifstream replay_inputs;
        if (command == "replay") {
            const std::filesystem::path session = required("--session");
            const auto metadata = read_json(session / "metadata.json");
            if (metadata.at("session_schema") != 1 || metadata.at("simulator_version") != UAVSIM_VERSION_STRING
                || metadata.at("configuration_sha256") != loaded.sha256 || numeric(metadata.at("timestep_s")) != dt
                || metadata.at("world_frame") != "ENU" || metadata.at("body_frame") != "FLU")
                throw std::invalid_argument("recording version/configuration/timestep/frame is incompatible");
            if (!metadata.at("steps").is_number_unsigned()) throw std::invalid_argument("recording steps must be an unsigned integer");
            steps = metadata.at("steps").get<std::uint64_t>();
            if (steps == 0 || steps > max_steps) throw std::invalid_argument("recording step count outside supported limits");
            initial = state_from_json(metadata.at("initial_state"));
            const auto& speeds = metadata.at("initial_rotor_speeds_radps");
            if (!speeds.is_array() || speeds.size() != 4) throw std::invalid_argument("invalid initial rotor speeds");
            for (std::size_t i=0;i<4;++i) initial_rotors[i] = numeric(speeds[i]);
            replay_inputs.open(session / "inputs.jsonl");
            if (!replay_inputs) throw std::runtime_error("cannot open recorded inputs");
            scenario = "input-replay";
        } else {
            const auto duration_text = required("--duration");
            std::size_t consumed = 0;
            const double duration = std::stod(duration_text, &consumed);
            if (consumed != duration_text.size() || !std::isfinite(duration) || duration < dt || duration > 3600)
                throw std::invalid_argument("duration must be finite and between 0.004 and 3600 seconds");
            steps = static_cast<std::uint64_t>(std::ceil(duration / dt));
            if (command == "hover") { initial.position_m.z() = 2.0; initial_rotors = simulation.hover_rotor_speeds(); }
            else {
                scenario = required("--scenario");
                if (scenario != "takeoff-land" || duration < 20) throw std::invalid_argument("supported scenario is takeoff-land with duration at least 20 s");
            }
        }
        simulation.reset(initial, initial_rotors);
        Recorder recorder(required("--output"), loaded);
        recorder.state(simulation.snapshot(), simulation);
        double max_altitude_error = 0, max_replay_error = 0, max_quaternion_error = 0;
        double peak_altitude = initial.position_m.z();
        std::clog << "INFO session_started scenario=" << scenario << " steps=" << steps << '\n';
        for (std::uint64_t i = 0; i < steps; ++i) {
            Input input;
            State expected;
            if (command == "replay") {
                std::array<char,65537> line{};
                if (!replay_inputs.getline(line.data(),static_cast<std::streamsize>(line.size())))
                    throw std::invalid_argument("recording is truncated or input record exceeds 64 KiB");
                const auto count = replay_inputs.gcount() - (replay_inputs.eof() ? 0 : 1);
                const auto end = line.begin() + count;
                if (std::find(line.begin(),end,'\0') != end)
                    throw std::invalid_argument("recording contains an embedded NUL byte");
                const auto record = Json::parse(line.begin(),end);
                if (!record.at("step").is_number_unsigned() || record.at("step").get<std::uint64_t>() != i+1)
                    throw std::invalid_argument("recorded input step order is invalid");
                input = input_from_json(record.at("input"));
                expected = state_from_json(record.at("state"));
            } else {
                input.armed = true;
                input.mode = FlightMode::AltitudeHold;
                input.altitude_m = (command == "hover" || i < steps/2) ? 2.0 : 0.0;
            }
            simulation.set_input(input);
            simulation.step();
            const auto snapshot = simulation.snapshot();
            peak_altitude = std::max(peak_altitude,snapshot.state.position_m.z());
            max_quaternion_error = std::max(max_quaternion_error, std::abs(snapshot.state.orientation.norm()-1));
            if (command == "hover") max_altitude_error = std::max(max_altitude_error,std::abs(snapshot.state.position_m.z()-2.0));
            if (command == "replay") {
                max_replay_error = std::max(max_replay_error,state_error(snapshot.state,expected));
                if (max_replay_error > replay_tolerance) throw std::runtime_error("input replay trajectory exceeds 1e-9 tolerance");
            }
            recorder.state(snapshot, simulation);
            recorder.record(input, snapshot);
        }
        if (command == "replay") {
            std::string extra;
            if (std::getline(replay_inputs,extra) || replay_inputs.bad()) throw std::invalid_argument("recording contains unexpected additional input records or read error");
        }
        const auto final = simulation.snapshot();
        const bool hover_pass = command != "hover" || max_altitude_error < 0.2;
        const bool landing_pass = command != "scenario" || (peak_altitude >= 1.8 && final.grounded && final.state.velocity_mps.norm() < 0.2);
        Json metadata = {{"session_schema",1},{"simulator_version",UAVSIM_VERSION_STRING},{"aircraft_id",loaded.vehicle.id},
            {"configuration_sha256",loaded.sha256},{"scenario",scenario},{"timestep_s",dt},{"steps",steps},
            {"duration_s",final.time},{"world_frame","ENU"},{"body_frame","FLU"},{"units","SI"},
            {"quaternion_convention","Hamilton wxyz, body-to-world"},{"initial_state",state_json(initial)},
            {"initial_rotor_speeds_radps",initial_rotors},{"final_state",state_json(final.state)},
            {"max_quaternion_norm_error",max_quaternion_error},{"max_hover_altitude_error_m",command == "hover" ? Json(max_altitude_error) : Json(nullptr)},
            {"peak_altitude_m",peak_altitude},{"final_grounded",final.grounded},
            {"max_replay_state_error",command == "replay" ? Json(max_replay_error) : Json(nullptr)},{"replay_tolerance",replay_tolerance},
            {"replay_type",command == "replay" ? "recorded-input re-execution" : "input recording with expected trajectory"},
            {"result",hover_pass && landing_pass && max_quaternion_error < 1e-6 ? "PASS" : "FAIL"}};
        recorder.finish(metadata);
        if (!hover_pass || !landing_pass || max_quaternion_error >= 1e-6) throw std::runtime_error("scenario acceptance failed; inspect session metadata");
        std::cout << "PASS scenario=" << scenario << " steps=" << steps << " time_s=" << final.time
            << " hover_error_m=" << max_altitude_error << " replay_error=" << max_replay_error << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "ERROR " << error.what() << '\n';
        return 1;
    }
}
