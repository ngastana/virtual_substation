#!/bin/sh
docker run --privileged -v /sys/class/gpio/:/sys/class/gpio --rm -p 102:102 -it --env INTERFACE=eth0 --hostname virtual-prot-ied --network sv_network --name vied virtual-prot-ied-tt
