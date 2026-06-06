FROM ros:jazzy AS builder
SHELL ["/bin/bash", "-c"]

WORKDIR /build_ws

COPY src ./src
COPY ../part1 /part1

RUN apt-get update && apt-get install -y \
    python3-colcon-common-extensions \
    && rm -rf /var/lib/apt/lists/* \
    && source /opt/ros/jazzy/setup.bash \
    && colcon build --cmake-args -DCMAKE_BUILD_TYPE=Release

FROM ros:jazzy-ros-core AS runtime
WORKDIR /fleet_ws

COPY --from=builder /build_ws/install ./install

RUN echo "source /opt/ros/jazzy/setup.bash" >> ~/.bashrc \
    && echo "source /fleet_ws/install/setup.bash" >> ~/.bashrc

ENTRYPOINT ["/bin/bash", "-c", "source /opt/ros/jazzy/setup.bash && source /fleet_ws/install/setup.bash && ros2 launch drone_fleet fleet.launch.py"]