#include "uavsim/config.hpp"
#include <nlohmann/json.hpp>
#include <picosha2/picosha2.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>

namespace uavsim {
namespace {
using Json = nlohmann::json;
constexpr std::size_t max_config_bytes = 1024 * 1024;
void keys(const Json& object, std::initializer_list<const char*> expected) {
    if (!object.is_object()) throw std::invalid_argument("configuration field must be an object");
    std::set<std::string> allowed;
    for (auto key : expected) {
        allowed.emplace(key);
        if (!object.contains(key)) throw std::invalid_argument(std::string("missing configuration field: ") + key);
    }
    for (const auto& item : object.items())
        if (!allowed.contains(item.key())) throw std::invalid_argument("unknown configuration field: " + item.key());
}
std::string string_value(const Json& value) {
    if (!value.is_string()) throw std::invalid_argument("configuration text field must be a string");
    auto text = value.get<std::string>();
    if (text.empty() || text.size() > 4096 || text.find_first_of("\r\n\t") != std::string::npos)
        throw std::invalid_argument("configuration text must be nonempty, single-line and at most 4096 bytes");
    return text;
}
double number(const Json& value) {
    if (!value.is_number()) throw std::invalid_argument("configuration numeric field must be a number");
    const double result = value.get<double>();
    if (!std::isfinite(result)) throw std::invalid_argument("configuration number must be finite");
    return result;
}
Vec3 vector3(const Json& value) {
    if (!value.is_array() || value.size() != 3) throw std::invalid_argument("configuration vector requires exactly three numbers");
    return {number(value[0]), number(value[1]), number(value[2])};
}
std::vector<std::string> strings(const Json& value) {
    if (!value.is_array()) throw std::invalid_argument("provenance/assumptions must be arrays of strings");
    std::vector<std::string> result;
    for (const auto& item : value) result.push_back(string_value(item));
    return result;
}
}
LoadedVehicle parse_vehicle(std::string_view text) {
    if (text.empty() || text.size() > max_config_bytes) throw std::invalid_argument("vehicle JSON must contain 1..1048576 bytes");
    std::vector<std::set<std::string>> object_keys;
    const auto root = Json::parse(text, [&object_keys](int, Json::parse_event_t event, Json& parsed) {
        if (event == Json::parse_event_t::object_start) object_keys.emplace_back();
        if (event == Json::parse_event_t::key && !object_keys.back().insert(parsed.get<std::string>()).second)
            throw std::invalid_argument("duplicate JSON object key");
        if (event == Json::parse_event_t::object_end) object_keys.pop_back();
        return true;
    });
    keys(root, {"schema_version", "id", "name", "category", "revision", "physics_backend", "mass_kg",
        "inertia_kg_m2", "center_of_mass_m", "rotors", "drag_area_coeff_m2", "angular_drag", "controls",
        "asset_reference", "provenance", "assumptions", "validation_status"});
    if (!root.at("schema_version").is_number_integer() || root.at("schema_version") != 1)
        throw std::invalid_argument("unsupported vehicle schema_version");
    LoadedVehicle loaded;
    auto& config = loaded.vehicle;
    config.id = string_value(root.at("id"));
    if (config.id.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789-_") != std::string::npos)
        throw std::invalid_argument("vehicle id must use lowercase ASCII letters, digits, dash or underscore");
    config.name = string_value(root.at("name"));
    config.revision = string_value(root.at("revision"));
    loaded.category = string_value(root.at("category"));
    loaded.physics_backend = string_value(root.at("physics_backend"));
    if (loaded.category != "quadrotor" || loaded.physics_backend != "uavsim-rk4")
        throw std::invalid_argument("this backend supports only quadrotor / uavsim-rk4");
    config.mass_kg = number(root.at("mass_kg"));
    const auto& inertia = root.at("inertia_kg_m2");
    if (!inertia.is_array() || inertia.size() != 3) throw std::invalid_argument("inertia requires a 3x3 matrix");
    for (int row = 0; row < 3; ++row) config.inertia.row(row) = vector3(inertia[row]).transpose();
    config.center_of_mass_m = vector3(root.at("center_of_mass_m"));
    const auto& rotors = root.at("rotors");
    if (!rotors.is_array() || rotors.size() != 4) throw std::invalid_argument("quadrotor requires exactly four rotors");
    for (std::size_t i = 0; i < 4; ++i) {
        const auto& source = rotors[i];
        keys(source, {"position_m", "spin", "k_thrust", "k_torque", "max_speed_radps", "time_constant_s"});
        auto& rotor = config.rotors[i];
        rotor.position_m = vector3(source.at("position_m"));
        if (!source.at("spin").is_number_integer() || (source.at("spin") != 1 && source.at("spin") != -1))
            throw std::invalid_argument("rotor spin must be integer -1 or 1");
        rotor.spin = source.at("spin").get<int>();
        rotor.k_thrust = number(source.at("k_thrust"));
        rotor.k_torque = number(source.at("k_torque"));
        rotor.max_speed_radps = number(source.at("max_speed_radps"));
        rotor.time_constant_s = number(source.at("time_constant_s"));
    }
    config.drag_area_coeff = vector3(root.at("drag_area_coeff_m2"));
    config.angular_drag = number(root.at("angular_drag"));
    const auto& controls = root.at("controls");
    keys(controls, {"max_tilt_rad", "max_rate_radps", "max_torque_nm"});
    config.max_tilt_rad = number(controls.at("max_tilt_rad"));
    config.max_rate_radps = number(controls.at("max_rate_radps"));
    config.max_torque_nm = number(controls.at("max_torque_nm"));
    loaded.asset_reference = string_value(root.at("asset_reference"));
    loaded.validation_status = string_value(root.at("validation_status"));
    if (loaded.validation_status != "engineering_estimate" && loaded.validation_status != "simulation_tested")
        throw std::invalid_argument("validation_status must be engineering_estimate or simulation_tested");
    const auto& provenance = root.at("provenance");
    keys(provenance, {"official", "measured", "engineering_estimates", "unknown"});
    std::size_t provenance_count = 0;
    for (const auto& entry : provenance.items()) provenance_count += strings(entry.value()).size();
    if (provenance_count == 0) throw std::invalid_argument("parameter provenance cannot be empty");
    loaded.assumptions = strings(root.at("assumptions"));
    if (loaded.assumptions.empty()) throw std::invalid_argument("at least one modeling assumption is required");
    validate_config(config);
    loaded.source_json = std::string(text);
    loaded.sha256 = picosha2::hash256_hex_string(loaded.source_json);
    return loaded;
}
LoadedVehicle load_vehicle(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("cannot open vehicle configuration");
    std::string text;
    char block[4096];
    while (input.read(block, sizeof(block)) || input.gcount()) {
        text.append(block, static_cast<std::size_t>(input.gcount()));
        if (text.size() > max_config_bytes) throw std::invalid_argument("vehicle configuration exceeds 1 MiB");
    }
    if (input.bad()) throw std::runtime_error("vehicle configuration read failed");
    auto loaded = parse_vehicle(text);
    std::clog << "INFO vehicle_loaded id=" << loaded.vehicle.id << " revision=" << loaded.vehicle.revision << '\n';
    return loaded;
}
} // namespace uavsim
