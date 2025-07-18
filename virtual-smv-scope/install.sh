#!/bin/bash

RED='\033[0;31m'
NC='\033[0m' # No Color

parse_git_branch() {
     #git branch 2> /dev/null | sed -e '/^[^*]/d' -e 's/* \(.*\)/(\1)/'
     git branch 2> /dev/null | sed -e '/^[^*]/d' -e 's/* //'
}

if [ $# -ne 1 ]; then 
	echo "illegal number of parameters"
	echo "Usage: install_ied.sh <arg>"
	echo "${RED} arg = 0 only build, arg = 1 build and run , arg = 2 only run ${NC}"
	exit 1
fi

image_name="virtual-smv-scope-"$(parse_git_branch)
echo "Docker image to use ${image_name}"

docker stop vscope
docker image prune -f

build_ied (){
	docker rmi ${image_name}
	docker build -t ${image_name} .
}

run_ied(){
	 docker create --privileged  \
                --rm -p 50010:5000 -t --hostname virtual-smv-scope \
                --name vscope ${image_name}
        #docker network connect sv_network vscope
        docker start -i vscope
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




