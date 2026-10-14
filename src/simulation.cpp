#include "uavsim/simulation.hpp"
#include <Eigen/Cholesky>
#include <Eigen/LU>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace uavsim {
namespace {
using ODE = Eigen::Matrix<double, 13, 1>;
void require(bool condition, const char* message) {
    if (!condition) {
        std::clog << "ERROR: " << message << '\n';
        throw std::invalid_argument(message);
    }
}
bool finite(double value) { return std::isfinite(value); }
void validate_inertia(const Mat3& inertia) {
    require(inertia.allFinite(), "Inertia must be finite");
    require(inertia.isApprox(inertia.transpose(), 1e-12), "Inertia must be symmetric");
    Eigen::LLT<Mat3> decomposition(inertia);
    require(decomposition.info() == Eigen::Success, "Inertia must be positive definite");
    const Vec3 diagonal = decomposition.matrixL().toDenseMatrix().diagonal();
    require(diagonal.minCoeff() > 1e-8, "Inertia is numerically singular");
}
ODE pack(const State& state) {
    ODE result;
    result.segment<3>(0) = state.position_m;
    result.segment<3>(3) = state.velocity_mps;
    result.segment<4>(6) = state.orientation.coeffs();
    result.segment<3>(10) = state.angular_velocity_radps;
    return result;
}
State unpack(const ODE& vector) {
    State state;
    state.position_m = vector.segment<3>(0);
    state.velocity_mps = vector.segment<3>(3);
    state.orientation.coeffs() = vector.segment<4>(6);
    state.angular_velocity_radps = vector.segment<3>(10);
    return state;
}
ODE derivative(const State& state, double mass, const Mat3& inertia, const Mat3& inverse_inertia,
               const Vec3& body_force, const Vec3& world_force,
               const Vec3& torque, const Vec3& gravity) {
    require(state.orientation.coeffs().allFinite() && state.orientation.norm() > 1e-12,
            "Invalid quaternion during integration");
    ODE result;
    result.segment<3>(0) = state.velocity_mps;
    result.segment<3>(3) = (state.orientation.normalized() * body_force + world_force) / mass + gravity;
    const Vec3& omega = state.angular_velocity_radps;
    const Quat rate = state.orientation * Quat(0, omega.x(), omega.y(), omega.z());
    result.segment<4>(6) = 0.5 * rate.coeffs();
    result.segment<3>(10) = inverse_inertia * (torque - omega.cross(inertia * omega));
    return result;
}
template<class Function>
State rk4(const State& state, double dt, Function&& function) {
    const ODE initial = pack(state);
    const ODE k1 = function(state, 0.0);
    const ODE k2 = function(unpack(initial + dt * 0.5 * k1), dt * 0.5);
    const ODE k3 = function(unpack(initial + dt * 0.5 * k2), dt * 0.5);
    const ODE k4 = function(unpack(initial + dt * k3), dt);
    State result = unpack(initial + dt / 6.0 * (k1 + 2.0 * k2 + 2.0 * k3 + k4));
    require(result.orientation.coeffs().allFinite() && result.orientation.norm() > 1e-12,
            "Invalid integrated quaternion");
    result.orientation.normalize();
    validate_state(result);
    return result;
}
Eigen::Matrix4d allocation_matrix(const VehicleConfig& config) {
    Eigen::Matrix4d matrix;
    for (std::size_t i = 0; i < config.rotors.size(); ++i) {
        const Rotor& rotor = config.rotors[i];
        const Vec3 arm = rotor.position_m - config.center_of_mass_m;
        matrix.col(static_cast<Eigen::Index>(i)) << rotor.k_thrust,
            arm.y() * rotor.k_thrust, -arm.x() * rotor.k_thrust,
            static_cast<double>(rotor.spin) * rotor.k_torque;
    }
    return matrix;
}
double motor_speed(const Rotor& rotor, double initial, double target, double elapsed) {
    return target + (initial - target) * std::exp(-elapsed / rotor.time_constant_s);
}
void validate_environment(const Environment& environment) {
    require(finite(environment.gravity_mps2) && environment.gravity_mps2 >= 0,
            "Gravity must be finite and nonnegative");
    require(finite(environment.air_density) && environment.air_density >= 0,
            "Air density must be finite and nonnegative");
    require(environment.wind_mps.allFinite() && finite(environment.ground_height_m),
            "Environment must be finite");
}
void validate_input(const Input& input) {
    require(finite(input.roll) && std::abs(input.roll) <= 1 &&
            finite(input.pitch) && std::abs(input.pitch) <= 1 &&
            finite(input.yaw) && std::abs(input.yaw) <= 1 &&
            finite(input.collective) && input.collective >= 0 && input.collective <= 1 &&
            finite(input.altitude_m), "Control inputs are outside their finite valid ranges");
    require(input.mode == FlightMode::Rate || input.mode == FlightMode::Attitude ||
            input.mode == FlightMode::AltitudeHold, "Unsupported flight mode");
}
} // namespace

VehicleConfig reference_quadrotor() {
    VehicleConfig config;
    const std::array<Vec3, 4> positions = {Vec3(0.2, 0.2, 0), Vec3(-0.2, 0.2, 0),
                                         Vec3(-0.2, -0.2, 0), Vec3(0.2, -0.2, 0)};
    for (std::size_t i = 0; i < 4; ++i) {
        config.rotors[i].position_m = positions[i];
        config.rotors[i].spin = i % 2 == 0 ? 1 : -1;
    }
    return config;
}
void validate_config(const VehicleConfig& config) {
    require(!config.id.empty() && !config.name.empty() && !config.revision.empty(),
            "Vehicle identity fields must be nonempty");
    require(finite(config.mass_kg) && config.mass_kg > 1e-8, "Mass must be positive and finite");
    validate_inertia(config.inertia);
    require(config.center_of_mass_m.allFinite() && config.drag_area_coeff.allFinite() &&
            config.drag_area_coeff.minCoeff() >= 0, "Invalid center of mass or drag coefficients");
    require(finite(config.angular_drag) && config.angular_drag >= 0 &&
            finite(config.max_tilt_rad) && config.max_tilt_rad > 0 && config.max_tilt_rad < 1.4 &&
            finite(config.max_rate_radps) && config.max_rate_radps > 0 &&
            finite(config.max_torque_nm) && config.max_torque_nm > 0, "Invalid controller or angular drag limits");
    for (const Rotor& rotor : config.rotors) {
        require(rotor.position_m.allFinite() && (rotor.spin == 1 || rotor.spin == -1) &&
                finite(rotor.k_thrust) && rotor.k_thrust > 0 && finite(rotor.k_torque) && rotor.k_torque > 0 &&
                finite(rotor.max_speed_radps) && rotor.max_speed_radps > 0 &&
                finite(rotor.time_constant_s) && rotor.time_constant_s > 0,
                "Invalid rotor parameters");
        require(finite(rotor.k_thrust * rotor.max_speed_radps * rotor.max_speed_radps) &&
                finite(rotor.k_torque * rotor.max_speed_radps * rotor.max_speed_radps),
                "Rotor force or torque exceeds numerical range");
    }
    const Eigen::Matrix4d matrix = allocation_matrix(config);
    require(matrix.allFinite() && matrix.fullPivLu().isInvertible(), "Rotor allocation must have full rank");
}
void validate_state(const State& state) {
    require(state.position_m.allFinite() && state.velocity_mps.allFinite() &&
            state.angular_velocity_radps.allFinite() && state.orientation.coeffs().allFinite(),
            "Simulation state must be finite");
    require(std::abs(state.orientation.norm() - 1.0) <= 1e-6, "Orientation must be a unit quaternion");
}
const char* flight_mode_name(FlightMode mode) {
    switch (mode) {
    case FlightMode::Rate: return "rate";
    case FlightMode::Attitude: return "attitude";
    case FlightMode::AltitudeHold: return "altitude_hold";
    }
    return "invalid";
}

Pid::Pid(double kp, double ki, double kd, double min_output, double max_output)
    : kp_(kp), ki_(ki), kd_(kd), min_(min_output), max_(max_output) {
    require(finite(kp) && finite(ki) && finite(kd) && kp >= 0 && ki >= 0 && kd >= 0 &&
            finite(min_output) && finite(max_output) && min_output < max_output, "Invalid PID settings");
}
double Pid::update(double error, double dt) {
    require(finite(error) && finite(dt) && dt > 0, "Invalid PID input or timestep");
    const double derivative_term = initialized_ ? kd_ * (error - last_error_) / dt : 0;
    const double candidate = integral_ + ki_ * error * dt;
    const double candidate_output = kp_ * error + candidate + derivative_term;
    require(finite(candidate_output), "PID numeric overflow");
    // Conditional integration prevents saturation from accumulating further windup.
    if ((candidate_output <= max_ || error < 0) && (candidate_output >= min_ || error > 0))
        integral_ = std::clamp(candidate, min_, max_);
    last_error_ = error;
    initialized_ = true;
    return std::clamp(kp_ * error + integral_ + derivative_term, min_, max_);
}
void Pid::reset() { integral_ = 0; last_error_ = 0; initialized_ = false; }

State integrate_rigid_body(const State& state, double mass, const Mat3& inertia,
                          const Vec3& force_body, const Vec3& force_world,
                          const Vec3& torque_body, const Vec3& gravity, double dt) {
    validate_state(state);
    validate_inertia(inertia);
    require(finite(mass) && mass > 1e-8 && finite(dt) && dt > 0 && dt <= 0.1,
            "Invalid mass or integration timestep");
    require(force_body.allFinite() && force_world.allFinite() && torque_body.allFinite() && gravity.allFinite(),
            "Forces and gravity must be finite");
    const Mat3 inverse = inertia.inverse();
    return rk4(state, dt, [&](const State& stage, double) {
        return derivative(stage, mass, inertia, inverse, force_body, force_world, torque_body, gravity);
    });
}

Simulation::Simulation(VehicleConfig config, Environment environment, double dt)
    : config_(std::move(config)), environment_(std::move(environment)), dt_(dt),
      rate_pid_{Pid(0.18, 0.035, 0.002, -config_.max_torque_nm, config_.max_torque_nm),
                Pid(0.18, 0.035, 0.002, -config_.max_torque_nm, config_.max_torque_nm),
                Pid(0.25, 0.04, 0.002, -config_.max_torque_nm, config_.max_torque_nm)},
      altitude_pid_(3.0, 0.25, 2.5, -4.0, 4.0) {
    validate_config(config_);
    validate_environment(environment_);
    require(finite(dt_) && dt_ > 0 && dt_ <= 0.02, "Simulation timestep must be in (0, 0.02] seconds");
    allocation_inverse_ = allocation_matrix(config_).inverse();
    State initial;
    if (environment_.ground_enabled) initial.position_m.z() = environment_.ground_height_m;
    reset(initial);
    std::clog << "INFO: initialized simulation vehicle=" << config_.id << " dt=" << dt_ << '\n';
}
void Simulation::reset(const State& state, const std::array<double, 4>& rotor_speeds) {
    validate_state(state);
    for (std::size_t i = 0; i < 4; ++i)
        require(finite(rotor_speeds[i]) && rotor_speeds[i] >= 0 && rotor_speeds[i] <= config_.rotors[i].max_speed_radps,
                "Initial rotor speed outside valid range");
    require(!environment_.ground_enabled || state.position_m.z() >= environment_.ground_height_m,
            "Initial state is below ground");
    Snapshot next;
    next.state = state;
    next.state.orientation.normalize();
    next.rotor_speeds = rotor_speeds;
    for (std::size_t i = 0; i < 4; ++i)
        next.thrusts[i] = config_.rotors[i].k_thrust * rotor_speeds[i] * rotor_speeds[i];
    next.grounded = environment_.ground_enabled && state.position_m.z() <= environment_.ground_height_m;
    snapshot_ = next;
    input_ = Input{};
    paused_ = false;
    for (Pid& pid : rate_pid_) pid.reset();
    altitude_pid_.reset();
    std::clog << "INFO: simulation reset\n";
}
void Simulation::set_input(const Input& input) {
    validate_input(input);
    if (input.armed != input_.armed || input.mode != input_.mode) {
        for (Pid& pid : rate_pid_) pid.reset();
        altitude_pid_.reset();
        std::clog << "INFO: controls armed=" << input.armed << " mode=" << flight_mode_name(input.mode) << '\n';
    }
    input_ = input;
    snapshot_.armed = input.armed;
    snapshot_.mode = input.mode;
}
void Simulation::set_environment(const Environment& environment) {
    validate_environment(environment);
    environment_ = environment;
}
void Simulation::set_paused(bool paused) {
    if (paused_ != paused) std::clog << "INFO: simulation paused=" << paused << '\n';
    paused_ = paused;
}
std::array<double, 4> Simulation::allocate(double thrust_n, const Vec3& torque_nm) const {
    require(finite(thrust_n) && thrust_n >= 0 && torque_nm.allFinite(), "Invalid requested rotor wrench");
    double maximum_thrust = 0;
    for (const Rotor& rotor : config_.rotors)
        maximum_thrust += rotor.k_thrust * rotor.max_speed_radps * rotor.max_speed_radps;
    const double collective = std::clamp(thrust_n, 0.0, maximum_thrust);
    const auto squared_for = [&](double torque_scale) -> Eigen::Vector4d {
        const Eigen::Vector4d requested(collective, torque_scale * torque_nm.x(),
                                        torque_scale * torque_nm.y(), torque_scale * torque_nm.z());
        return (allocation_inverse_ * requested).eval();
    };
    const auto feasible = [&](const Eigen::Vector4d& squared) {
        if (!squared.allFinite()) return false;
        for (std::size_t i = 0; i < config_.rotors.size(); ++i) {
            const double maximum_squared = config_.rotors[i].max_speed_radps * config_.rotors[i].max_speed_radps;
            const double tolerance = maximum_squared * 1e-12;
            if (squared[static_cast<Eigen::Index>(i)] < -tolerance ||
                squared[static_cast<Eigen::Index>(i)] > maximum_squared + tolerance)
                return false;
        }
        return true;
    };
    // PX4-style control allocation gives collective authority priority. The remaining
    // attitude wrench is desaturated uniformly instead of clipping individual motors.
    double low = 0.0, high = 1.0;
    for (int iteration = 0; iteration < 48; ++iteration) {
        const double middle = (low + high) / 2.0;
        if (feasible(squared_for(middle))) low = middle;
        else high = middle;
    }
    const Eigen::Vector4d squared = squared_for(low);
    // `low` starts at zero torque, which is feasible for the clamped collective.
    // Round-off at an exact motor limit is handled by the final physical clamp.
    std::array<double, 4> commands{};
    for (std::size_t i = 0; i < 4; ++i) {
        const double maximum_squared = config_.rotors[i].max_speed_radps * config_.rotors[i].max_speed_radps;
        const double speed = std::sqrt(std::clamp(squared[static_cast<Eigen::Index>(i)], 0.0, maximum_squared));
        commands[i] = std::clamp(speed / config_.rotors[i].max_speed_radps, 0.0, 1.0);
    }
    return commands;
}
std::array<double, 4> Simulation::hover_rotor_speeds() const {
    const Eigen::Vector4d wrench(config_.mass_kg * environment_.gravity_mps2, 0, 0, 0);
    const Eigen::Vector4d squared = allocation_inverse_ * wrench;
    std::array<double, 4> speeds{};
    for (std::size_t i = 0; i < 4; ++i) {
        require(squared[static_cast<Eigen::Index>(i)] >= 0, "Vehicle cannot produce hover equilibrium");
        speeds[i] = std::sqrt(squared[static_cast<Eigen::Index>(i)]);
        require(speeds[i] < config_.rotors[i].max_speed_radps, "Vehicle has insufficient hover thrust reserve");
    }
    return speeds;
}
void Simulation::step() {
    if (paused_) return;
    // Stage everything, including controller memory, so a rejected step cannot corrupt the session.
    Snapshot next = snapshot_;
    auto controllers = rate_pid_;
    auto altitude = altitude_pid_;
    std::array<double, 4> commands{};
    const State& current = snapshot_.state;
    if (input_.armed) {
        Vec3 rate_target(input_.roll, input_.pitch, input_.yaw);
        rate_target *= config_.max_rate_radps;
        if (input_.mode != FlightMode::Rate) {
            const Mat3 rotation = current.orientation.toRotationMatrix();
            const double yaw = std::atan2(rotation(1, 0), rotation(0, 0));
            const Quat desired = Eigen::AngleAxisd(yaw, Vec3::UnitZ()) *
                Eigen::AngleAxisd(input_.pitch * config_.max_tilt_rad, Vec3::UnitY()) *
                Eigen::AngleAxisd(input_.roll * config_.max_tilt_rad, Vec3::UnitX());
            Quat error = current.orientation.conjugate() * desired;
            if (error.w() < 0) error.coeffs() *= -1;
            rate_target = 8.0 * error.vec();
            rate_target.z() = input_.yaw * config_.max_rate_radps;
            rate_target = rate_target.cwiseMax(-config_.max_rate_radps).cwiseMin(config_.max_rate_radps);
        }
        Vec3 torque;
        for (Eigen::Index i = 0; i < 3; ++i)
            torque[i] = controllers[static_cast<std::size_t>(i)].update(rate_target[i] - current.angular_velocity_radps[i], dt_);
        double maximum_thrust = 0;
        for (const Rotor& rotor : config_.rotors)
            maximum_thrust += rotor.k_thrust * rotor.max_speed_radps * rotor.max_speed_radps;
        double thrust = input_.collective * maximum_thrust;
        if (input_.mode == FlightMode::AltitudeHold) {
            const double acceleration = altitude.update(input_.altitude_m - current.position_m.z(), dt_);
            const double vertical_fraction = (current.orientation * Vec3::UnitZ()).z();
            thrust = std::clamp(config_.mass_kg * (environment_.gravity_mps2 + acceleration) /
                                std::max(0.3, vertical_fraction), 0.0, maximum_thrust);
        }
        commands = allocate(thrust, torque);
    }
    const Mat3 inverse_inertia = config_.inertia.inverse();
    next.state = rk4(current, dt_, [&](const State& stage, double elapsed) {
        Vec3 force = Vec3::Zero();
        Vec3 torque = Vec3::Zero();
        for (std::size_t i = 0; i < 4; ++i) {
            const Rotor& rotor = config_.rotors[i];
            const double speed = motor_speed(rotor, snapshot_.rotor_speeds[i], commands[i] * rotor.max_speed_radps, elapsed);
            const Vec3 rotor_force(0, 0, rotor.k_thrust * speed * speed);
            force += rotor_force;
            torque += (rotor.position_m - config_.center_of_mass_m).cross(rotor_force);
            torque.z() += static_cast<double>(rotor.spin) * rotor.k_torque * speed * speed;
        }
        const Vec3 relative = stage.orientation.normalized().conjugate() * (stage.velocity_mps - environment_.wind_mps);
        force -= (0.5 * environment_.air_density * config_.drag_area_coeff.array() *
                  relative.array().abs() * relative.array()).matrix();
        torque -= config_.angular_drag * stage.angular_velocity_radps;
        return derivative(stage, config_.mass_kg, config_.inertia, inverse_inertia, force, Vec3::Zero(), torque,
                          Vec3(0, 0, -environment_.gravity_mps2));
    });
    next.grounded = false;
    if (environment_.ground_enabled && next.state.position_m.z() <= environment_.ground_height_m) {
        // Flat, inelastic point contact; orientation stays physical. Upgrade for landing gear contact geometry.
        next.state.position_m.z() = environment_.ground_height_m;
        next.state.velocity_mps.z() = std::max(0.0, next.state.velocity_mps.z());
        next.state.velocity_mps.head<2>() *= std::exp(-8.0 * dt_);
        next.state.angular_velocity_radps *= std::exp(-8.0 * dt_);
        next.grounded = true;
    }
    for (std::size_t i = 0; i < 4; ++i) {
        const Rotor& rotor = config_.rotors[i];
        next.rotor_speeds[i] = motor_speed(rotor, snapshot_.rotor_speeds[i], commands[i] * rotor.max_speed_radps, dt_);
        next.thrusts[i] = rotor.k_thrust * next.rotor_speeds[i] * next.rotor_speeds[i];
        require(finite(next.rotor_speeds[i]) && finite(next.thrusts[i]), "Invalid integrated rotor state");
    }
    next.motor_commands = commands;
    ++next.step;
    next.time = static_cast<double>(next.step) * dt_;
    validate_state(next.state);
    require(finite(next.time), "Simulation clock overflow");
    snapshot_ = next;
    rate_pid_ = controllers;
    altitude_pid_ = altitude;
}
} // namespace uavsim
