#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <array>
#include <cstdint>
#include <string>

namespace uavsim {
using Vec3 = Eigen::Vector3d;
using Mat3 = Eigen::Matrix3d;
using Quat = Eigen::Quaterniond;

// ENU world, FLU body, SI. Hamilton q rotates body to world; qdot = q*(0,omega)/2.
struct State {
    Vec3 position_m = Vec3::Zero();
    Vec3 velocity_mps = Vec3::Zero();
    Quat orientation = Quat::Identity();
    Vec3 angular_velocity_radps = Vec3::Zero();
};
struct Rotor {
    Vec3 position_m = Vec3::Zero();
    int spin = 1; // Sign of the reaction torque on the body about +Z.
    double k_thrust = 1e-5;
    double k_torque = 1.5e-7;
    double max_speed_radps = 1000.0;
    double time_constant_s = 0.04;
};
struct VehicleConfig {
    std::string id = "generic-quadrotor-x01";
    std::string name = "Generic Quadrotor X-01";
    std::string revision = "1";
    double mass_kg = 1.5;
    Mat3 inertia = Vec3(0.025, 0.025, 0.045).asDiagonal(); // About center of mass.
    Vec3 center_of_mass_m = Vec3::Zero();
    std::array<Rotor, 4> rotors{};
    Vec3 drag_area_coeff = Vec3(0.08, 0.08, 0.12); // Cd*A in m^2, body axes.
    double angular_drag = 0.01;
    double max_tilt_rad = 0.5;
    double max_rate_radps = 2.0;
    double max_torque_nm = 1.0;
};
VehicleConfig reference_quadrotor();
void validate_config(const VehicleConfig& config);
void validate_state(const State& state);

struct Environment {
    double gravity_mps2 = 9.80665;
    double air_density = 1.225;
    Vec3 wind_mps = Vec3::Zero();
    double ground_height_m = 0.0;
    bool ground_enabled = true;
};
enum class FlightMode { Rate, Attitude, AltitudeHold };
const char* flight_mode_name(FlightMode mode);
struct Input {
    bool armed = false;
    FlightMode mode = FlightMode::Attitude;
    double roll = 0.0;
    double pitch = 0.0;
    double yaw = 0.0;
    double collective = 0.5;
    double altitude_m = 1.0;
};
struct Snapshot {
    State state{};
    double time = 0.0;
    std::uint64_t step = 0;
    std::array<double, 4> motor_commands{}; // Normalized rotor speed targets.
    std::array<double, 4> rotor_speeds{};
    std::array<double, 4> thrusts{};
    FlightMode mode = FlightMode::Attitude;
    bool armed = false;
    bool grounded = true;
};

class Pid {
public:
    Pid(double kp, double ki, double kd, double min_output, double max_output);
    double update(double error, double dt);
    void reset();
    double integral() const { return integral_; }
private:
    double kp_, ki_, kd_, min_, max_, integral_ = 0, last_error_ = 0;
    bool initialized_ = false;
};

// Forces and torque are held constant for one step; gravity is a world vector.
State integrate_rigid_body(const State& state, double mass, const Mat3& inertia,
    const Vec3& force_body, const Vec3& force_world, const Vec3& torque_body,
    const Vec3& gravity, double dt);

class Simulation {
public:
    explicit Simulation(VehicleConfig config = reference_quadrotor(),
                        Environment environment = {}, double dt = 0.004);
    void reset(const State& state = {}, const std::array<double, 4>& rotor_speeds = {});
    void set_input(const Input& input);
    void set_paused(bool paused);
    bool paused() const { return paused_; }
    void step();
    Snapshot snapshot() const { return snapshot_; }
    double timestep() const { return dt_; }
    const VehicleConfig& config() const { return config_; }
    const Environment& environment() const { return environment_; }
    std::array<double, 4> allocate(double thrust_n, const Vec3& torque_nm) const;
    std::array<double, 4> hover_rotor_speeds() const;
private:
    VehicleConfig config_;
    Environment environment_;
    double dt_;
    bool paused_ = false;
    Input input_{};
    Snapshot snapshot_{};
    Eigen::Matrix4d allocation_inverse_;
    std::array<Pid, 3> rate_pid_;
    Pid altitude_pid_;
};
} // namespace uavsim
