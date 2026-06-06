#include "drone.hpp"
#include "drone_exceptions.hpp"

Drone::Drone(const std::string& v_name, float initial_battery, float max_alt, float spd)
    : Vehicle(v_name, initial_battery), altitude(0.0f), max_altitude(max_alt), speed(spd) {}

void Drone::take_off(float target_altitude) {
    if (target_altitude > max_altitude) {
        throw AltitudeError("Requested altitude " + std::to_string(target_altitude) + "m exceeds ceiling limit.");
    }
    set_status("flying");
    altitude = target_altitude;
    add_log("Took off to altitude: " + std::to_string(altitude) + "m");
}

void Drone::land() {
    altitude = 0.0f;
    set_status("idle");
    add_log("Safely landed.");
}

void Drone::emergency_stop() {
    add_log("EMERGENCY STOP TRIGGERED!");
    drain_battery(30.0f);
    land();
}

std::string Drone::get_info() const {
    return "[Drone] Name: " + get_name() + " | Battery: " + std::to_string(get_battery_level()) +
           "% | Status: " + get_status() + " | Alt: " + std::to_string(altitude) + "m";
}