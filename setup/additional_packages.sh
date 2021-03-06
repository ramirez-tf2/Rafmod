#!/bin/bash
#These are a few more packages you need to install on your system to compile this extension.
#Your TF2 server box also needs some of these packages to run this extension. See srcds_packages.sh for more details.
sudo apt-get update
sudo apt-get upgrade -y
sudo apt-get install nasm libunwind-dev:i386 libiberty-dev:i386 libelf-dev libboost-dev libbsd-dev:i386 make -y
