#!/bin/sh
docker rmi virtual-prot-ied-tt
docker build -t virtual-prot-ied-tt . --build-arg WITH_GPIO=false
docker run --privileged -v /sys/class/gpio/:/sys/class/gpio --rm -p 102:102 -it --env INTERFACE=eth0 --hostname virtual-prot-ied --network sv_network --name vied virtual-prot-ied-tt
