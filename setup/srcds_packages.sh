#!/bin/bash
#These are a few packages you need to install on your live server machine to load this extension.
#If you are running a server from a GSP, you may not be able to install these.
sudo apt-get update
sudo apt-get upgrade -y
sudo apt-get install libunwind-dev:i386 libbsd-dev:i386 -y
