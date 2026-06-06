#include "autonomous_drone.hpp"
#include <cmath>

AutonomousDrone::AutonomousDrone(const std::string& v_name, float initial_battery, float max_alt, float spd, const std::string& m_name, std::tuple<float, float, float> home)
    : MissionDrone(v_name, initial_battery, max_alt, spd, m_name), ai_mode("manual"), home_position(home) {}

void AutonomousDrone::set_ai_mode(const std::string& mode) {
    ai_mode = mode;
    add_log("AI Mode altered to: " + ai_mode);
    if (ai_mode == "return_home") {
        waypoints.insert(waypoints.begin() + current_waypoint_index, home_position);
        add_log("Return Home route injected into flight path.");
    }
}

void AutonomousDrone::detect_obstacle(std::tuple<float, float, float> position, const std::string& severity) {
    std::string entry = "Obstacle detected at depth position. Severity: " + severity;
    obstacle_log.push_back("[" + get_current_timestamp() + "] " + entry);
    add_log(entry);

    if (severity == "high") {
        emergency_stop();
    }
}

std::vector<std::tuple<float, float, float>> AutonomousDrone::auto_replan(const std::vector<std::tuple<float, float, float>>& obstacles) {
    add_log("Executing spatial path replanning avoidance strategy.");
    std::vector<std::tuple<float, float, float>> safe_route;
    for (const auto& wp : waypoints) {
        bool unsafe = false;
        for (const auto& obs : obstacles) {
            float dist = std::sqrt(std::pow(std::get<0>(wp) - std::get<0>(obs), 2) +
                                   std::pow(std::get<1>(wp) - std::get<1>(obs), 2) +
                                   std::pow(std::get<2>(wp) - std::get<2>(obs), 2));
            if (dist < 5.0f) {
                unsafe = true;
                break;
            }
        }
        if (!unsafe) {
            safe_route.push_back(wp);
        }
    }
    return safe_route;
}

std::string AutonomousDrone::get_info() const {
    return "[AutonomousDrone] Name: " + get_name() + " | AI Mode: " + ai_mode + " | Battery: " + std::to_string(get_battery_level()) + "%";
}