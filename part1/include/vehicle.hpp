#ifndef VEHICLE_HPP
#define VEHICLE_HPP

#include <string>
#include <vector>
#include <chrono>

class Vehicle {
protected:
    std::string name;
private:
    float battery_level;
    std::string status;
    std::vector<std::string> flight_log;

protected:
    std::string get_current_timestamp() const;

public:
    Vehicle(const std::string& v_name, float initial_battery);
    virtual ~Vehicle() = default;

    virtual std::string get_info() const = 0;
    
    void drain_battery(float amount);
    void charge_battery(float amount, int duration_seconds);
    bool is_critical() const;
    
    void set_status(const std::string& new_status);
    void add_log(const std::string& entry);
    
    std::string get_flight_log() const;
    std::string get_name() const { return name; }
    float get_battery_level() const { return battery_level; }
    std::string get_status() const { return status; }
};

#endif