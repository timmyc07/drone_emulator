#pragma once
#include "uavsim/simulation.hpp"
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace uavsim {
struct LoadedVehicle {
    VehicleConfig vehicle;
    std::string category;
    std::string physics_backend;
    std::string asset_reference;
    std::string validation_status;
    std::vector<std::string> assumptions;
    std::string source_json;
    std::string sha256;
};
// Schema version 1 is strict: unknown/missing keys, duplicate keys and nonfinite values fail.
LoadedVehicle parse_vehicle(std::string_view text);
LoadedVehicle load_vehicle(const std::filesystem::path& path);
} // namespace uavsim
