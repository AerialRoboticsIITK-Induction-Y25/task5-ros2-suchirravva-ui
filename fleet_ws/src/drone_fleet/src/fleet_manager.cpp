#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_srvs/srv/trigger.hpp"
#include <map>
#include <iostream>

struct FleetTelemetry {
    std::string name{"?"};
    float battery{0.0f};
    float altitude{0.0f};
    std::string status{"unknown"};
    int waypoint{0};
};

class FleetManager : public rclcpp::Node {
private:
    std::map<std::string, FleetTelemetry> fleet_registry_;
    std::vector<rclcpp::Subscription<std_msgs::msg::String>::SharedPtr> subs_;
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr report_srv_;
    rclcpp::TimerBase::SharedPtr report_timer_;

public:
    FleetManager() : Node("fleet_manager") {
        std::vector<std::string> drones = {"Alpha", "Beta", "Gamma"};
        
        for (const auto& name : drones) {
            fleet_registry_[name].name = name;

            subs_.push_back(this->create_subscription<std_msgs::msg::String>(
                "/drone/" + name + "/telemetry", 10,
                [this, name](const std_msgs::msg::String::SharedPtr msg) { this->parse_telemetry_json(msg->data); }));

            subs_.push_back(this->create_subscription<std_msgs::msg::String>(
                "/drone/" + name + "/alert", 10,
                [this](const std_msgs::msg::String::SharedPtr msg) {
                    RCLCPP_WARN(this->get_logger(), "ALERT INGESTION DETECTED: %s", msg->data.c_str());
                }));
        }

        report_srv_ = this->create_service<std_srvs::srv::Trigger>(
            "/fleet/status_report",
            std::bind(&FleetManager::handle_report_service, this, std::placeholders::_1, std::placeholders::_2));

        report_timer_ = this->create_wall_timer(std::chrono::seconds(5), std::bind(&FleetManager::print_report_table, this));
    }

private:
    std::string extract_value(const std::string& json, const std::string& key, bool is_string) {
        size_t pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return "";
        size_t start = json.find(":", pos);
        if (start == std::string::npos) return "";
        start++;

        if (is_string) {
            size_t open_quote = json.find("\"", start);
            size_t close_quote = json.find("\"", open_quote + 1);
            return json.substr(open_quote + 1, close_quote - open_quote - 1);
        } else {
            size_t end = json.find_first_of(",}", start);
            return json.substr(start, end - start);
        }
    }

    void parse_telemetry_json(const std::string& raw_data) {
        std::string name = extract_value(raw_data, "name", true);
        if (name.empty()) return;

        fleet_registry_[name].name = name;
        fleet_registry_[name].status = extract_value(raw_data, "status", true);
        
        std::string bat_str = extract_value(raw_data, "battery", false);
        std::string alt_str = extract_value(raw_data, "altitude", false);
        std::string wp_str  = extract_value(raw_data, "waypoint", false);

        if (!bat_str.empty()) fleet_registry_[name].battery = std::stof(bat_str);
        if (!alt_str.empty()) fleet_registry_[name].altitude = std::stof(alt_str);
        if (!wp_str.empty())  fleet_registry_[name].waypoint = std::stoi(wp_str);
    }

    void print_report_table() {
        std::cout << "\n======================= FLEET REPORT PROFILES (JAZZY) =======================\n";
        printf("%-10s | %-12s | %-10s | %-10s | %-10s\n", "Drone", "Battery (%)", "Altitude", "Waypoint", "Status");
        std::cout << "---------------------------------------------------------------------\n";
        for (const auto& [name, telemetry] : fleet_registry_) {
            printf("%-10s | %-12.1f | %-10.1f | %-10d | %-10s\n",
                   telemetry.name.c_str(), telemetry.battery, telemetry.altitude, telemetry.waypoint, telemetry.status.c_str());
        }
        std::cout << "=====================================================================\n" << std::endl;
    }

    void handle_report_service(const std::shared_ptr<std_srvs::srv::Trigger::Request>,
                               std::shared_ptr<std_srvs::srv::Trigger::Response> res) {
        print_report_table();
        res->success = true;
        res->message = "Fleet status matrix printed successfully down terminal link pipeline.";
    }
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<FleetManager>());
    rclcpp::shutdown();
    return 0;
}