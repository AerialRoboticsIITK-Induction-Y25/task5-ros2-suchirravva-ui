#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "mission_drone.hpp"
#include <sstream>
#include <iomanip>

class DroneNode : public rclcpp::Node {
private:
    std::unique_ptr<MissionDrone> drone_impl_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr status_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr alert_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr completed_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr telemetry_pub_;
    
    rclcpp::TimerBase::SharedPtr timer_1s_;
    rclcpp::TimerBase::SharedPtr timer_2s_;
    int tick_count_{0};

public:
    DroneNode() : Node("drone_node") {
        this->declare_parameter<std::string>("drone_name", "Alpha");
        this->declare_parameter<double>("initial_battery", 100.0);
        this->declare_parameter<std::string>("mission_name", "Alpha_Mission");

        std::string name = this->get_parameter("drone_name").as_string();
        double bat = this->get_parameter("initial_battery").as_double();
        std::string mission = this->get_parameter("mission_name").as_string();

        drone_impl_ = std::make_unique<MissionDrone>(name, static_cast<float>(bat), 150.0f, 4.5f, mission);
        for(int i = 1; i <= 5; ++i) {
            drone_impl_->add_waypoint(i * 10.0f, i * 15.0f, 20.0f);
        }
        drone_impl_->take_off(25.0f);

        status_pub_    = this->create_publisher<std_msgs::msg::String>("/drone/" + name + "/status", 10);
        alert_pub_     = this->create_publisher<std_msgs::msg::String>("/drone/" + name + "/alert", 10);
        completed_pub_ = this->create_publisher<std_msgs::msg::String>("/drone/" + name + "/mission_complete", 10);
        telemetry_pub_ = this->create_publisher<std_msgs::msg::String>("/drone/" + name + "/telemetry", 10);

        timer_1s_ = this->create_wall_timer(std::chrono::seconds(1), std::bind(&DroneNode::on_tick_1s, this));
        timer_2s_ = this->create_wall_timer(std::chrono::seconds(2), std::bind(&DroneNode::on_tick_2s, this));
    }

private:
    void on_tick_1s() {
        if(drone_impl_->get_status() == "idle") return;

        drone_impl_->drain_battery(0.5f);
        tick_count_++;

        if(tick_count_ % 3 == 0) {
            if(!drone_impl_->mission_complete()) {
                drone_impl_->next_waypoint();
            }
        }

        std::stringstream ss;
        ss << "name:" << drone_impl_->get_name()
           << "|battery:" << std::fixed << std::setprecision(1) << drone_impl_->get_battery_level()
           << "|altitude:" << std::fixed << std::setprecision(1) << drone_impl_->get_altitude()
           << "|status:" << drone_impl_->get_status()
           << "|waypoint:" << drone_impl_->get_current_waypoint_index() << "/" << drone_impl_->get_total_waypoints()
           << "|speed:" << std::fixed << std::setprecision(1) << drone_impl_->get_speed();

        auto msg = std_msgs::msg::String();
        msg.data = ss.str();
        status_pub_->publish(msg);

        if(drone_impl_->is_critical()) {
            auto alert_msg = std_msgs::msg::String();
            alert_msg.data = "[" + drone_impl_->get_name() + "] CRITICAL VOLTAGE DRAIN WARNING!";
            alert_pub_->publish(alert_msg);
            drone_impl_->land();
            RCLCPP_WARN(this->get_logger(), "Critical energy levels hit! Landing initiated.");
        }

        if(drone_impl_->mission_complete()) {
            auto comp_msg = std_msgs::msg::String();
            comp_msg.data = drone_impl_->get_name() + " mission execution path target achieved.";
            completed_pub_->publish(comp_msg);
            
            drone_impl_->land();
            drone_impl_->charge_battery(100.0f - drone_impl_->get_battery_level(), 0); 
            drone_impl_->take_off(25.0f);
        }
    }

    void on_tick_2s() {
        std::stringstream json;
        json << "{\"name\":\"" << drone_impl_->get_name() 
             << "\",\"battery\":" << std::fixed << std::setprecision(1) << drone_impl_->get_battery_level()
             << ",\"altitude\":" << std::fixed << std::setprecision(1) << drone_impl_->get_altitude()
             << ",\"status\":\"" << drone_impl_->get_status() 
             << "\",\"waypoint\":" << drone_impl_->get_current_waypoint_index() << "}";

        auto msg = std_msgs::msg::String();
        msg.data = json.str();
        telemetry_pub_->publish(msg);
    }
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<DroneNode>());
    rclcpp::shutdown();
    return 0;
}