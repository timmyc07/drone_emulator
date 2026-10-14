#include "uavsim/simulation.hpp"
#include <Eigen/LU>
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

using namespace uavsim;
namespace {
void check(bool result, const std::string& message) {
    if (!result) throw std::runtime_error(message);
}
void near(double actual, double expected, double tolerance, const std::string& message) {
    check(std::isfinite(actual) && std::abs(actual - expected) <= tolerance,
          message + " actual=" + std::to_string(actual) + " expected=" + std::to_string(expected));
}
template<class Function> void rejects(Function function) {
    bool rejected = false;
    try { function(); } catch (const std::invalid_argument&) { rejected = true; }
    check(rejected, "invalid input must be rejected");
}
Environment airborne() { Environment e; e.ground_enabled = false; return e; }
Input hold(double height = 1.0) {
    Input input;
    input.armed = true;
    input.mode = FlightMode::AltitudeHold;
    input.altitude_m = height;
    return input;
}
void mathematics() {
    const Vec3 x = Vec3::UnitX(), y = Vec3::UnitY(), z = Vec3::UnitZ();
    check(x.cross(y).isApprox(z), "FLU cross product");
    const double pi = std::acos(-1.0);
    const Quat yaw(Eigen::AngleAxisd(pi / 2, z));
    check((yaw * x).isApprox(y, 1e-12), "body to ENU rotation");
    const Quat roll(Eigen::AngleAxisd(pi / 2, x));
    check(((yaw * roll) * y).isApprox(z, 1e-12), "right operand rotates first");
    check((yaw.conjugate() * (yaw * x)).isApprox(x), "inverse coordinate transform");
}
void free_fall_and_force() {
    State state;
    const Mat3 inertia = Mat3::Identity();
    for (int i = 0; i < 250; ++i)
        state = integrate_rigid_body(state, 1.5, inertia, Vec3::Zero(), Vec3::Zero(), Vec3::Zero(), Vec3(0, 0, -9.80665), 0.004);
    near(state.position_m.z(), -9.80665 / 2, 1e-10, "one-second free fall position");
    near(state.velocity_mps.z(), -9.80665, 1e-10, "one-second free fall velocity");
    state = State{};
    state.orientation = Quat(Eigen::AngleAxisd(std::acos(-1.0) / 2, Vec3::UnitZ()));
    for (int i = 0; i < 250; ++i)
        state = integrate_rigid_body(state, 2.0, inertia, Vec3(2, 0, 0), Vec3(0, 2, 0), Vec3::Zero(), Vec3::Zero(), 0.004);
    near(state.position_m.y(), 1.0, 1e-10, "rotated body plus world force");
    near(state.position_m.x(), 0.0, 1e-10, "rotated force axis");
}
void full_inertia() {
    Mat3 inertia;
    inertia << 2, .2, .1, .2, 3, .3, .1, .3, 4;
    State state;
    state.angular_velocity_radps = Vec3(0.3, -0.2, 0.4);
    const Vec3 torque(0.7, -0.4, 0.2);
    const Vec3 expected = inertia.inverse() * (torque - state.angular_velocity_radps.cross(inertia * state.angular_velocity_radps));
    const State next = integrate_rigid_body(state, 1, inertia, Vec3::Zero(), Vec3::Zero(), torque, Vec3::Zero(), 1e-6);
    check(((next.angular_velocity_radps - state.angular_velocity_radps) / 1e-6 - expected).norm() < 1e-7,
          "nondiagonal inertia gyroscopic equation");
    const double energy = state.angular_velocity_radps.dot(inertia * state.angular_velocity_radps);
    for (int i = 0; i < 1000; ++i)
        state = integrate_rigid_body(state, 1, inertia, Vec3::Zero(), Vec3::Zero(), Vec3::Zero(), Vec3::Zero(), .004);
    near(state.angular_velocity_radps.dot(inertia * state.angular_velocity_radps), energy, 1e-10,
         "torque-free rotational energy conservation");
}
void convergence() {
    auto error = [](double dt) {
        State state;
        state.angular_velocity_radps = Vec3(2, -3, 4);
        const Vec3 omega = state.angular_velocity_radps;
        for (int i = 0; i < static_cast<int>(std::round(1.0 / dt)); ++i)
            state = integrate_rigid_body(state, 1, Mat3::Identity(), Vec3::Zero(), Vec3::Zero(), Vec3::Zero(), Vec3::Zero(), dt);
        const Quat exact(Eigen::AngleAxisd(omega.norm(), omega.normalized()));
        near(state.orientation.norm(), 1.0, 1e-12, "quaternion normalization");
        return (state.orientation.coeffs() - exact.coeffs()).norm();
    };
    const double coarse = error(.02), medium = error(.01), fine = error(.005);
    check(coarse > medium * 14 && medium > fine * 14, "RK4 fourth-order convergence");
    std::cout << "RK4 errors=" << coarse << ',' << medium << ',' << fine << '\n';
}
void configuration_validation() {
    auto config = reference_quadrotor();
    validate_config(config);
    config.mass_kg = 0; rejects([&] { validate_config(config); });
    config = reference_quadrotor(); config.inertia(1, 0) = .1; rejects([&] { validate_config(config); });
    config = reference_quadrotor(); config.inertia(2, 2) = -1; rejects([&] { validate_config(config); });
    config = reference_quadrotor(); config.rotors[0].time_constant_s = 0; rejects([&] { validate_config(config); });
    config = reference_quadrotor(); for (auto& rotor : config.rotors) rotor.spin = 1;
    rejects([&] { validate_config(config); });
    config = reference_quadrotor(); config.drag_area_coeff.x() = std::numeric_limits<double>::quiet_NaN();
    rejects([&] { validate_config(config); });
    rejects([] { Simulation sim(reference_quadrotor(), {}, 0); });
    rejects([] { Environment e; e.wind_mps.x() = std::numeric_limits<double>::infinity(); Simulation sim(reference_quadrotor(), e); });
}
void mixing_and_motors() {
    Simulation sim(reference_quadrotor(), airborne());
    for (int axis = 0; axis < 3; ++axis) {
        Vec3 torque = Vec3::Zero(); torque[axis] = 0.1;
        const auto commands = sim.allocate(15, torque);
        Vec3 actual_torque = Vec3::Zero(); double thrust = 0;
        for (std::size_t i = 0; i < 4; ++i) {
            const Rotor& rotor = sim.config().rotors[i];
            const double speed = commands[i] * rotor.max_speed_radps;
            const Vec3 force(0, 0, rotor.k_thrust * speed * speed);
            thrust += force.z();
            actual_torque += (rotor.position_m - sim.config().center_of_mass_m).cross(force);
            actual_torque.z() += rotor.spin * rotor.k_torque * speed * speed;
        }
        near(thrust, 15, 1e-10, "mixer collective");
        check(actual_torque.isApprox(torque, 1e-10), "mixer torque signs");
    }
    for (double command : sim.allocate(1e6, Vec3(100, -100, 100)))
        check(command >= 0 && command <= 1, "motor command saturation");
    Input input; input.armed = true; input.mode = FlightMode::Rate; input.collective = .25;
    sim.set_input(input);
    for (int i = 0; i < 10; ++i) sim.step();
    const auto snapshot = sim.snapshot();
    const double expected = 500 * (1 - std::exp(-1.0));
    near(snapshot.rotor_speeds[0], expected, 1e-9, "exact first order motor lag");
    near(snapshot.thrusts[0], 1e-5 * expected * expected, 1e-10, "rotor thrust law");
    input.armed = false; sim.set_input(input); sim.step();
    near(sim.snapshot().rotor_speeds[0], expected * std::exp(-.1), 1e-9, "disarming physical spin down");
    for (double command : sim.snapshot().motor_commands) near(command, 0, 0, "disarmed target");
    const auto speeds = sim.hover_rotor_speeds();
    double weight = 0;
    for (std::size_t i = 0; i < 4; ++i) weight += sim.config().rotors[i].k_thrust * speeds[i] * speeds[i];
    near(weight, sim.config().mass_kg * sim.environment().gravity_mps2, 1e-10, "hover thrust equilibrium");
}
void pid() {
    Pid controller(1, 1, .1, -1, 1);
    for (int i = 0; i < 1000; ++i) near(controller.update(10, .004), 1, 1e-12, "PID saturation");
    near(controller.integral(), 0, 1e-12, "PID anti-windup");
    controller.reset(); near(controller.update(0, .004), 0, 1e-12, "PID reset clears derivative history");
    controller.update(.1, .1); check(controller.integral() > 0, "PID accumulates unsaturated error");
    controller.reset(); near(controller.integral(), 0, 0, "PID integral reset");
    rejects([&] { controller.update(0, 0); });
}
void coupled_motor_convergence() {
    auto error = [](double dt) {
        auto config = reference_quadrotor();
        config.drag_area_coeff.setZero();
        Simulation sim(config, airborne(), dt);
        Input input; input.armed = true; input.mode = FlightMode::Rate; input.collective = .25;
        sim.set_input(input);
        for (int i = 0; i < static_cast<int>(std::round(1.0 / dt)); ++i) sim.step();
        const double tau = .04, t = 1, acceleration = 4 * 1e-5 * 500 * 500 / 1.5;
        const double exact = acceleration * (t * t / 2 - 1.5 * tau * t +
            2 * tau * tau * (1 - std::exp(-t / tau)) - tau * tau / 4 * (1 - std::exp(-2 * t / tau))) - 9.80665 / 2;
        return std::abs(sim.snapshot().state.position_m.z() - exact);
    };
    const double coarse = error(.02), medium = error(.01), fine = error(.005);
    check(coarse > medium * 12 && medium > fine * 14, "coupled motor and rigid body RK4 convergence");
    std::cout << "Coupled motor position errors=" << coarse << ',' << medium << ',' << fine << '\n';
}
void hover_and_recovery() {
    Simulation sim(reference_quadrotor(), airborne());
    State initial; initial.position_m.z() = 1.05;
    sim.reset(initial, sim.hover_rotor_speeds());
    sim.set_input(hold(1));
    double max_error = 0;
    for (int i = 0; i < 7500; ++i) {
        sim.step();
        max_error = std::max(max_error, std::abs(sim.snapshot().state.position_m.z() - 1));
        near(sim.snapshot().state.orientation.norm(), 1, 1e-6, "hover quaternion norm");
    }
    check(max_error < .2, "30 second near-equilibrium hover maximum altitude error");
    near(sim.snapshot().time, 30, 1e-12, "fixed simulation clock");
    std::cout << "30s hover maximum altitude error=" << max_error << '\n';
    initial.position_m.z() = 3;
    initial.orientation = Quat(Eigen::AngleAxisd(.3, Vec3::UnitX()));
    sim.reset(initial, sim.hover_rotor_speeds()); sim.set_input(hold(3));
    for (int i = 0; i < 2500; ++i) sim.step();
    check(sim.snapshot().state.orientation.angularDistance(Quat::Identity()) < .03, "attitude recovery");
    near(sim.snapshot().state.position_m.z(), 3, .1, "altitude hold after disturbance");
    sim.set_input(hold(4));
    for (int i = 0; i < 4000; ++i) sim.step();
    near(sim.snapshot().state.position_m.z(), 4, .1, "altitude target tracking");
}
void runtime() {
    Simulation a, b;
    Input input = hold(); a.set_input(input); b.set_input(input);
    for (int i = 0; i < 1000; ++i) { a.step(); b.step(); }
    check(a.snapshot().state.position_m == b.snapshot().state.position_m, "deterministic position");
    check(a.snapshot().state.orientation.coeffs() == b.snapshot().state.orientation.coeffs(), "deterministic attitude");
    check(a.snapshot().state.position_m.z() > .8, "takeoff from zero rotor speed");
    const Snapshot previous = a.snapshot();
    a.set_paused(true); for (int i = 0; i < 10; ++i) a.step();
    check(a.snapshot().step == previous.step && a.snapshot().time == previous.time, "pause clock");
    a.set_paused(false); a.step(); check(a.snapshot().step == previous.step + 1, "resume");
    Input invalid = input; invalid.roll = std::numeric_limits<double>::quiet_NaN();
    rejects([&] { a.set_input(invalid); });
    const auto before_reset = a.snapshot();
    State invalid_state; invalid_state.orientation.coeffs().setZero();
    rejects([&] { a.reset(invalid_state); });
    check(a.snapshot().step == before_reset.step, "invalid reset is atomic");
    a.reset();
    near(a.snapshot().time, 0, 0, "reset time"); check(!a.snapshot().armed, "reset disarms");
    a.step(); near(a.snapshot().state.position_m.z(), 0, 0, "ground contact");
    near(a.snapshot().state.velocity_mps.z(), 0, 0, "inelastic ground response");
    Environment wind = airborne(); wind.wind_mps.x() = 5;
    Simulation windy(reference_quadrotor(), wind); windy.step();
    check(windy.snapshot().state.velocity_mps.x() > 0, "wind drag follows relative air velocity");
    Environment changed = windy.environment(); changed.wind_mps = Vec3(-2, 3, 0); changed.air_density = 1.0;
    windy.set_environment(changed);
    check(windy.environment().wind_mps == changed.wind_mps && windy.environment().air_density == 1.0, "runtime environment update");
    const Environment valid_environment = windy.environment();
    changed.wind_mps.x() = std::numeric_limits<double>::infinity();
    rejects([&] { windy.set_environment(changed); });
    check(windy.environment().wind_mps == valid_environment.wind_mps, "invalid environment update is atomic");
    State extreme; extreme.velocity_mps.x() = 1e200;
    windy.reset(extreme);
    const Snapshot valid = windy.snapshot();
    rejects([&] { windy.step(); });
    check(windy.snapshot().state.velocity_mps == valid.state.velocity_mps && windy.snapshot().step == valid.step,
          "failed integration preserves state and clock");
    Environment raised; raised.ground_height_m = 10;
    Simulation on_hill(reference_quadrotor(), raised);
    near(on_hill.snapshot().state.position_m.z(), 10, 0, "initial ground elevation");
}
} // namespace
int main() {
    int failed = 0;
    const std::pair<const char*, std::function<void()>> tests[] = {
        {"mathematics", mathematics}, {"free_fall_and_force", free_fall_and_force},
        {"full_inertia", full_inertia}, {"rk4_convergence", convergence},
        {"configuration_validation", configuration_validation}, {"mixing_and_motors", mixing_and_motors},
        {"pid", pid}, {"coupled_motor_convergence", coupled_motor_convergence},
        {"hover_and_recovery", hover_and_recovery}, {"runtime", runtime}};
    for (const auto& [name, test] : tests) {
        try { test(); std::cout << "PASS " << name << '\n'; }
        catch (const std::exception& exception) { ++failed; std::cerr << "FAIL " << name << ": " << exception.what() << '\n'; }
    }
    return failed == 0 ? 0 : 1;
}
