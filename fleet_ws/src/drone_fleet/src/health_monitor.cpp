#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include <map>
#include <deque>
#include <sstream>
#include <iomanip>
#include <iostream>

struct HealthMetrics {
    std::deque<std::pair<rclcpp::Time, float>> samples;
    float current_drain_rate{0.0f};
    float last_known_battery{100.0f};
};

class HealthMonitor : public rclcpp::Node {
private:
    std::map<std::string, HealthMetrics> metrics_map_;
    std::vector<rclcpp::Subscription<std_msgs::msg::String>::SharedPtr> telemetry_subs_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr warning_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr summary_pub_;
    rclcpp::TimerBase::SharedPtr evaluation_timer_;

public:
    HealthMonitor() : Node("health_monitor") {
        std::vector<std::string> drones = {"Alpha", "Beta", "Gamma"};

        warning_pub_ = this->create_publisher<std_msgs::msg::String>("/fleet/health_warning", 10);
        summary_pub_ = this->create_publisher<std_msgs::msg::String>("/fleet/health_summary", 10);

        for (const auto& name : drones) {
            telemetry_subs_.push_back(this->create_subscription<std_msgs::msg::String>(
                "/drone/" + name + "/telemetry", 10,
                [this, name](const std_msgs::msg::String::SharedPtr msg) { this->process_telemetry(name, msg->data); }));
        }

        evaluation_timer_ = this->create_wall_timer(std::chrono::seconds(10), std::bind(&HealthMonitor::evaluate_and_report, this));
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

    void process_telemetry(const std::string& name, const std::string& data) {
        std::string bat_str = extract_value(data, "battery", false);
        if (bat_str.empty()) return;
        float bat = std::stof(bat_str);

        auto& metric = metrics_map_[name];
        metric.last_known_battery = bat;
        rclcpp::Time now = this->now();

        metric.samples.push_back({now, bat});
        if (metric.samples.size() > 10) {
            metric.samples.pop_front();
        }

        if (metric.samples.size() >= 2) {
            auto first = metric.samples.front();
            auto last = metric.samples.back();
            double dt = (last.first - first.first).seconds();
            if (dt > 0.1) {
                metric.current_drain_rate = (first.second - last.second) / static_cast<float>(dt);
                if (metric.current_drain_rate > 1.5f) {
                    auto warn = std_msgs::msg::String();
                    warn.data = "CRITICAL METRIC EXCEEDED: " + name + " consumption leak rate evaluated at " + std::to_string(metric.current_drain_rate) + "/s";
                    warning_pub_->publish(warn);
                }
            }
        }
    }

    void evaluate_and_report() {
        std::cout << "\n======================= HEALTH DIAGNOSTICS TIMELINE =======================\n";
        printf("%-10s | %-15s | %-18s | %-18s\n", "Drone", "Drain Rate/s", "Est. to Critical", "Est. to Depletion");
        std::cout << "---------------------------------------------------------------------------\n";

        std::stringstream summary_json;
        summary_json << "{";

        for (auto it = metrics_map_.begin(); it != metrics_map_.end(); ++it) {
            std::string name = it->first;
            auto metric = it->second;

            float rate = metric.current_drain_rate;
            float bat = metric.last_known_battery;

            std::string est_crit = "INF";
            std::string est_depl = "INF";

            if (rate > 0.001f) {
                if (bat > 20.0f) est_crit = std::to_string(static_cast<int>((bat - 20.0f) / rate)) + "s";
                else est_crit = "0s";
                est_depl = std::to_string(static_cast<int>(bat / rate)) + "s";
            }

            printf("%-10s | %-15.3f | %-18s | %-18s\n", name.c_str(), rate, est_crit.c_str(), est_depl.c_str());

            summary_json << "\"" << name << "\":{\"drain_rate\":" << rate << "}";
            if (std::next(it) != metrics_map_.end()) summary_json << ",";
        }
        summary_json << "}";
        std::cout << "===========================================================================\n" << std::endl;

        auto summary_msg = std_msgs::msg::String();
        summary_msg.data = summary_json.str();
        summary_pub_->publish(summary_msg);
    }
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<HealthMonitor>());
    rclcpp::shutdown();
    return 0;
}