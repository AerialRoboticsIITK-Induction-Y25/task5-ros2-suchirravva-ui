#!/bin/bash
IMAGE_NAME="ariitk_drone_fleet_jazzy:latest"

echo "Verifying local repository structural map..."
if [ ! -d "src/drone_fleet" ]; then
    echo "ERROR: Execute this workspace run runner profile directly inside the 'fleet_ws/' directory!"
    exit 1
fi

echo "Building production-grade multi-stage Docker layer targeted for ROS 2 Jazzy..."
docker build -f Dockerfile -t $IMAGE_NAME ..

echo "Launching virtual fleet monitor infrastructure system..."
# Bind-mount structural workspace files to mirror modifications without execution rebuilds
docker run -it --rm \
    --net=host \
    -e ROS_DOMAIN_ID=42 \
    -v "$(pwd)/src/drone_fleet:/fleet_ws/src/drone_fleet:ro" \
    $IMAGE_NAME