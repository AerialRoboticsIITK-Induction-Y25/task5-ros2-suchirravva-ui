cmake_minimum_required(VERSION 3.22)
project(DroneFleetOOP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

include_directories(include)

add_executable(drone_fleet_oop
    src/vehicle.cpp
    src/drone.cpp
    src/mission_drone.cpp
    src/autonomous_drone.cpp
    src/main.cpp
)