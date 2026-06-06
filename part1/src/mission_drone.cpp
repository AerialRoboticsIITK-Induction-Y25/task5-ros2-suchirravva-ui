#include "mission_drone.hpp"
#include <iostream>

MissionDrone::MissionDrone(const std::string& v_name, float initial_battery, float max_alt, float spd, const std::string& m_name)
    : Drone(v_name, initial_battery, max_alt, spd), mission_name(m_name), current_waypoint_index(0) {}

void MissionDrone::add_waypoint(float x, float y, float z) {
    waypoints.push_back(std::make_tuple(x, y, z));
}

std::tuple<float, float, float> MissionDrone::next_waypoint() {
    if (mission_complete()) {
        return std::make_tuple(0.0f, 0.0f, 0.0f);
    }
    auto wp = waypoints[current_waypoint_index];
    drain_battery(1.5f);
    visited_waypoints.push_back({wp, get_current_timestamp()});
    add_log("Reached Waypoint " + std::to_string(current_waypoint_index + 1));
    current_waypoint_index++;
    return wp;
}

void MissionDrone::skip_waypoint(const std::string& reason) {
    if (!mission_complete()) {
        add_log("Skipped waypoint " + std::to_string(current_waypoint_index + 1) + ". Reason: " + reason);
        current_waypoint_index++;
    }
}

bool MissionDrone::mission_complete() const {
    return current_waypoint_index >= static_cast<int>(waypoints.size());
}

std::string MissionDrone::mission_summary() const {
    std::string summary = "Mission summary for " + mission_name + ":\n";
    summary += "Waypoints visited: " + std::to_string(visited_waypoints.size()) + "/" + std::to_string(waypoints.size()) + "\n";
    return summary;
}

std::string MissionDrone::get_info() const {
    return "[MissionDrone] Name: " + get_name() + " | Mission: " + mission_name +
           " | Progress: " + std::to_string(current_waypoint_index) + "/" + std::to_string(waypoints.size());
}