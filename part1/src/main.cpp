#include "drone_exceptions.hpp"
#include "vehicle.hpp"
#include "drone.hpp"
#include "mission_drone.hpp"
#include "autonomous_drone.hpp"

#include <iostream>
#include <vector>

int main() {
    Drone drone("Alpha", 95.0f, 50.0f, 5.5f);
    MissionDrone m_drone("Beta", 85.0f, 100.0f, 12.0f, "Survey_Project");
    AutonomousDrone auto_drone("Gamma", 90.0f, 120.0f, 15.0f, "Border_Patrol", std::make_tuple(0.0f, 0.0f, 0.0f));

    auto_drone.add_waypoint(10.0f, 12.0f, 15.0f);
    auto_drone.add_waypoint(20.0f, -5.0f, 25.0f);
    auto_drone.add_waypoint(40.0f, 30.0f, 10.0f);

    std::vector<Vehicle*> fleet = { &drone, &m_drone, &auto_drone };
    std::cout << "--- Fleet Polymorphism Check ---" << std::endl;
    for (const auto* vehicle : fleet) {
        std::cout << vehicle->get_info() << std::endl;
    }

    // Checking Encapsulation Protection (Explain via comment)
    // The next line cannot be compiled because battery_level is private.
    // auto_drone.battery_level = 100.0f;

    std::cout << "\n--- Exception Handling Operational Trial Run ---" << std::endl;
    try {
        drone.take_off(200.0f);
    } catch (const AltitudeError& e) {
        std::cout << "Caught Expected Error: " << e.what() << std::endl;
    }

    try {
        drone.charge_battery(10.0f, 5);
    } catch (const InvalidStateError& e) {
        std::cout << "Caught Expected Error: " << e.what() << std::endl;
    }

    std::cout << "\n--- Full Autonomous Mission Profile Simulating Threat Mitigation ---" << std::endl;
    auto_drone.set_ai_mode("auto");
    auto_drone.take_off(30.0f);
    auto_drone.next_waypoint();
    
    std::cout << "[SIMULATION] High-severity structural constraint hazard intersection observed!" << std::endl;
    auto_drone.detect_obstacle(std::make_tuple(20.0f, -4.0f, 24.5f), "high");

    std::cout << "\n--- Post-Incident Drone Flight Log Diagnostic Summary ---" << std::endl;
    std::cout << auto_drone.get_flight_log() << std::endl;
    std::cout << auto_drone.mission_summary() << std::endl;

    return 0;
}