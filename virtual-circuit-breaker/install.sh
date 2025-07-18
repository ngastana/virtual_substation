#!/bin/sh

RED='\033[0;31m'
NC='\033[0m' # No Color


parse_git_branch() {
     git branch 2> /dev/null | sed -e '/^[^*]/d' -e 's/* //'
}

if [ $# -lt 1 ]; then 
	echo "illegal number of parameters"
	echo "Usage: install_ied.sh <arg> [cont_name]"
	echo "${RED} arg = 0 only build, arg = 1 build and run , arg = 2 only run ${NC}"
	echo "${RED} cont_name, name given to the container, default vxcbr ${NC}"
	exit 1
fi

cont_name=vxcbr

if [ $# -eq 2 ]; then
cont_name=$2
fi

image_name="virtual-xcbr-ied-"$(parse_git_branch)
echo "Docker image to use ${image_name}"

docker stop ${cont_name}
docker image prune -f

build_ied (){
	docker stop ${cont_name}
	docker rmi ${image_name}
	docker build -t ${image_name} . --build-arg WITH_GPIO=false
}

run_ied(){

	docker create --privileged -v /sys/class/gpio/:/sys/class/gpio \
		--rm -t --env INTERFACE=eth0 --hostname virtual-xcbr-ied \
		--network virtual_process_bus --name ${cont_name} ${image_name}
	docker start -i ${cont_name}
}

if [ $1 -eq 0 ]; then
	build_ied
fi

if [ $1 -eq 1 ]; then
	build_ied
	run_ied
fi

if [ $1 -eq 2 ]; then
	run_ied
fi




