#!/bin/bash

#This script will install everything needed to set your system up to compile Sourcemod extensions on Linux.
#We are not responsible for any undesired modifications that may happen to your system from running this script.
#Read through it in its entirety and ensure that you are okay with every command before running it.

#User-customizable settings:
SM_BRANCH=1.10-dev         		#Set the SM branch name to clone (leave blank to clone the unstable branch)
SDK_LOCATION=~/sdkfolder   		#Set the location to store the Sourcemod, Metamod, and hl2sdk files at

################################
# DO NOT EDIT BELOW THIS LINE! #
################################

#[1] Update OS & packages:
echo Please type your sudo password if requested.
sudo apt-get update
sudo apt-get upgrade -y

#[2] Install the necessary extra packages (git, python3 setuptools module, clang, g++multilib):
sudo apt-get install git python3-setuptools clang g++-multilib -y

#[3] Create our SDK folder location and change to it:
mkdir $SDK_LOCATION
cd $SDK_LOCATION

#[4] Clone the SM repository:
if [ -z "${SM_BRANCH}" ]; then
    git clone https://github.com/alliedmodders/sourcemod --recursive
else
    git clone https://github.com/alliedmodders/sourcemod --recursive --branch $SM_BRANCH
fi

#[5] HACK: As of writing (this may change in the future), checkout-deps.sh uses "python" instead of "python3",
#but the newest versions of Ubuntu dropped "python" completely. Rather than arse the user to modify checkout-deps.sh,
#we will check if "python" exists, and if not, then we'll temporarily symlink "python3" to "python" and then delete
#the symlink before we exit. Ideally, AlliedModders will use python3 directly and we can cut this part out.
temp_python=0
if ! [ -x "$(command -v python)" ]; then
    sudo ln -s /usr/bin/python3 /usr/bin/python
    temp_python=1
fi

#[6] Run the checkout:
bash sourcemod/tools/checkout-deps.sh

#[7] HACK: Delete the temporary symlink we made to avoid leaving a permanent and possibly undesired change to the system:
if [ $temp_python -eq 1 ]; then
    sudo rm /usr/bin/python
fi

#[8] Install AMBuild:
cd ambuild
sudo python3 setup.py install

#[9] Done!
echo You are all set up to install Sourcemod extensions!


