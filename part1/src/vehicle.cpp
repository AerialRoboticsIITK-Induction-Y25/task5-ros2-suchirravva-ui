#include "vehicle.hpp"
#include "drone_exceptions.hpp"
#include <sstream>
#include <iomanip>

Vehicle::Vehicle(const std::string& v_name, float initial_battery)
    : name(v_name), battery_level(initial_battery), status("idle") {
    if (battery_level > 100.0f) battery_level = 100.0f;
    if (battery_level < 0.0f) battery_level = 0.0f;
}

std::string Vehicle::get_current_timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    return ss.str();
}

void Vehicle::drain_battery(float amount) {
    if (battery_level <= 0.0f) {
        throw BatteryDepletedError(name + " has no remaining energy!");
    }
    battery_level -= amount;
    if (battery_level < 0.0f) battery_level = 0.0f;
}

void Vehicle::charge_battery(float amount, int duration_seconds) {
    if (status != "charging") {
        throw InvalidStateError(name + " must be in 'charging' state to top up!");
    }
    battery_level += amount;
    if (battery_level > 100.0f) battery_level = 100.0f;
    add_log("Charged for " + std::to_string(duration_seconds) + "s. Level: " + std::to_string(battery_level));
}

bool Vehicle::is_critical() const {
    return battery_level < 20.0f;
}

void Vehicle::set_status(const std::string& new_status) {
    if (new_status != "idle" && new_status != "flying" && new_status != "charging") {
        throw InvalidStateError("State '" + new_status + "' is invalid.");
    }
    status = new_status;
    add_log("Status changed to: " + status);
}

void Vehicle::add_log(const std::string& entry) {
    flight_log.push_back("[" + get_current_timestamp() + "] " + entry);
}

std::string Vehicle::get_flight_log() const {
    std::string summary;
    for (const auto& log : flight_log) {
        summary += log + "\n";
    }
    return summary;
}