#include "uavsim/config.hpp"
#include "uavsim/version.hpp"
#include <emscripten/bind.h>
#include <nlohmann/json.hpp>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace {
using namespace uavsim;
using Json = nlohmann::json;
std::unique_ptr<Simulation> simulation;
std::string configuration_sha256;
std::string error;

Simulation& session() {
    if (!simulation) throw std::logic_error("Load a vehicle before starting the simulation");
    return *simulation;
}
template<class Function> bool checked(Function&& function) {
    try {
        function();
        error.clear();
        return true;
    } catch (const std::exception& exception) {
        error = exception.what();
        std::clog << "ERROR: web bridge: " << error << '\n';
        return false;
    }
}
bool load_vehicle_text(const std::string& text) {
    return checked([&] {
        auto loaded = parse_vehicle(text);
        auto next = std::make_unique<Simulation>(loaded.vehicle);
        configuration_sha256 = std::move(loaded.sha256);
        simulation = std::move(next);
        std::clog << "INFO: web vehicle loaded id=" << simulation->config().id << '\n';
    });
}
bool set_input(bool armed, double mode, double roll, double pitch, double yaw,
               double collective, double altitude_m) {
    return checked([&] {
        if (!std::isfinite(mode) || mode != std::floor(mode) || mode < 0 || mode > 2)
            throw std::invalid_argument("Flight mode must be 0, 1 or 2");
        Input input;
        input.armed = armed;
        input.mode = static_cast<FlightMode>(static_cast<int>(mode));
        input.roll = roll;
        input.pitch = pitch;
        input.yaw = yaw;
        input.collective = collective;
        input.altitude_m = altitude_m;
        session().set_input(input);
    });
}
bool advance(double steps) {
    return checked([&] {
        if (!std::isfinite(steps) || steps != std::floor(steps) || steps < 0 || steps > 64)
            throw std::invalid_argument("Advance requires an integer from 0 to 64");
        auto& sim = session();
        for (int i = 0; i < static_cast<int>(steps); ++i) sim.step();
    });
}
bool reset() { return checked([] { session().reset(); }); }
bool set_paused(bool paused) { return checked([&] { session().set_paused(paused); }); }
Json vector_json(const Vec3& value) { return Json::array({value.x(), value.y(), value.z()}); }
std::string snapshot() {
    std::string result;
    checked([&] {
        const auto& sim = session();
        const auto state = sim.snapshot();
        const auto& q = state.state.orientation;
        result = Json{
            {"position_m", vector_json(state.state.position_m)},
            {"velocity_mps", vector_json(state.state.velocity_mps)},
            {"orientation_wxyz", Json::array({q.w(), q.x(), q.y(), q.z()})},
            {"angular_velocity_radps", vector_json(state.state.angular_velocity_radps)},
            {"time", state.time}, {"step", state.step}, {"armed", state.armed},
            {"grounded", state.grounded}, {"mode", flight_mode_name(state.mode)},
            {"paused", sim.paused()}, {"motor_commands", state.motor_commands},
            {"rotor_speeds", state.rotor_speeds}, {"thrusts", state.thrusts},
            {"vehicle_id", sim.config().id}, {"config_sha256", configuration_sha256},
            {"core_version", UAVSIM_VERSION_STRING}, {"timestep_s", sim.timestep()}
        }.dump();
    });
    return result;
}
std::string last_error() { return error; }
}

EMSCRIPTEN_BINDINGS(uavsim_web) {
    emscripten::function("loadVehicle", &load_vehicle_text);
    emscripten::function("setInput", &set_input);
    emscripten::function("advance", &advance);
    emscripten::function("reset", &reset);
    emscripten::function("setPaused", &set_paused);
    emscripten::function("snapshot", &snapshot);
    emscripten::function("lastError", &last_error);
}
