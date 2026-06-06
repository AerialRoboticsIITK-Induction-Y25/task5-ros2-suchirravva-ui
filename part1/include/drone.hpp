#ifndef DRONE_HPP
#define DRONE_HPP

#include "vehicle.hpp"

class Drone : public Vehicle {
protected:
    float altitude;
    float max_altitude;
private:
    float speed;

public:
    Drone(const std::string& v_name, float initial_battery, float max_alt, float spd);
    
    void take_off(float target_altitude);
    void land();
    void emergency_stop();
    
    std::string get_info() const override;
    float get_altitude() const { return altitude; }
    float get_speed() const { return speed; }
};

#endif