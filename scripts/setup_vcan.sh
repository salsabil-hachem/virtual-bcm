#!/usr/bin/env bash
# Cree le reseau CAN virtuel vcan0 (a relancer apres chaque redemarrage du PC).
set -e
sudo modprobe vcan
if ! ip link show vcan0 > /dev/null 2>&1; then
    sudo ip link add dev vcan0 type vcan
fi
sudo ip link set up vcan0
echo "vcan0 est pret."
