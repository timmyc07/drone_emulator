#include "uavsim/config.hpp"
#include <nlohmann/json.hpp>
#include <picosha2/picosha2.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {
using Json = nlohmann::json;
void require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
template<class Callable> void rejects(Callable action, const char* message) {
    bool rejected = false;
    try { action(); } catch (const std::exception&) { rejected = true; }
    require(rejected, message);
}
}
int main(int argc, char** argv) {
    try {
        require(argc == 2, "supply reference configuration filename");
        const auto loaded = uavsim::load_vehicle(argv[1]);
        require(loaded.vehicle.mass_kg == 1.5, "reference mass");
        require(!loaded.assumptions.empty() && loaded.sha256.size() == 64, "provenance/hash");
        require(picosha2::hash256_hex_string(std::string("abc")) ==
            "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", "SHA256 known vector");
        auto valid = Json::parse(loaded.source_json);
        auto mutate_reject = [&](auto mutate, const char* message) {
            auto candidate = valid; mutate(candidate);
            rejects([&] { uavsim::parse_vehicle(candidate.dump()); },message);
        };
        mutate_reject([](Json& j) { j["mass_kg"] = -1; }, "negative mass accepted");
        mutate_reject([](Json& j) { j["mass_kg"] = true; }, "boolean mass accepted");
        mutate_reject([](Json& j) { j["mass_kg"] = "NaN"; }, "string NaN accepted");
        mutate_reject([](Json& j) { j["mass_kg"] = nullptr; }, "null mass accepted");
        mutate_reject([](Json& j) { j["inertia_kg_m2"][0][0] = -0.01; }, "indefinite inertia accepted");
        mutate_reject([](Json& j) { j["inertia_kg_m2"][0][1] = 0.1; }, "asymmetric inertia accepted");
        mutate_reject([](Json& j) { j["inertia_kg_m2"][0] = Json::array({1,2}); }, "bad matrix shape accepted");
        mutate_reject([](Json& j) { j["rotors"].erase(0); }, "three rotors accepted");
        mutate_reject([](Json& j) { j["rotors"][0]["spin"] = 0; }, "invalid spin accepted");
        mutate_reject([](Json& j) { j["rotors"][0]["spin"] = 1.0; }, "fractional spin type accepted");
        mutate_reject([](Json& j) { j["rotors"][0]["k_thrust"] = 0; }, "zero thrust coefficient accepted");
        mutate_reject([](Json& j) { j["rotors"][0]["time_constant_s"] = -1; }, "negative motor lag accepted");
        mutate_reject([](Json& j) { j["rotors"][0]["typo"] = 3; }, "unknown rotor field accepted");
        mutate_reject([](Json& j) { j["controls"]["max_tilt_rad"] = -1; }, "invalid limits accepted");
        mutate_reject([](Json& j) { j["provenance"]["engineering_estimates"] = Json::array(); j["provenance"]["unknown"] = Json::array(); }, "empty provenance accepted");
        mutate_reject([](Json& j) { j["provenance"]["measured"] = Json::array({7}); }, "nonnumeric provenance type accepted");
        mutate_reject([](Json& j) { j["assumptions"] = Json::array(); }, "empty assumptions accepted");
        mutate_reject([](Json& j) { j["physics_backend"] = "unknown"; }, "unsupported backend accepted");
        mutate_reject([](Json& j) { j["schema_version"] = 2; }, "unsupported schema accepted");
        mutate_reject([](Json& j) { j["id"] = "bad,id"; }, "CSV-unsafe ID accepted");
        mutate_reject([](Json& j) { j["extra"] = 1; }, "unknown field accepted");
        mutate_reject([](Json& j) { j.erase("provenance"); }, "missing field accepted");
        rejects([] { uavsim::parse_vehicle("{broken"); },"malformed JSON accepted");
        rejects([] { uavsim::parse_vehicle("{\"mass_kg\":1e999}"); },"infinite numeric literal accepted");
        rejects([] { uavsim::parse_vehicle("{\"schema_version\":1,\"schema_version\":1}"); },"duplicate key accepted");
        rejects([] { uavsim::parse_vehicle(std::string(1048577,' ')); },"oversized configuration accepted");
        const auto repeated = uavsim::parse_vehicle(loaded.source_json);
        require(repeated.sha256 == loaded.sha256, "same source hash is unstable");
        const auto whitespace = uavsim::parse_vehicle(loaded.source_json + "\n");
        require(whitespace.sha256 != loaded.sha256, "hash must identify exact input bytes");
        std::cout << "PASS strict vehicle schema, provenance, invalid physical data and SHA256\n";
        return 0;
    } catch (const std::exception& error) { std::cerr << "ERROR config tests: " << error.what() << '\n'; return 1; }
}
