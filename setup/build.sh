#!/bin/bash

#This script will compile the extension and produce a binary.
#Make sure to change SDK_LOCATION below if you did not run ambuild_setup.sh.
#
#IMPORTANT NOTE: Make sure your sourcemod and sourcemm-1.10 folders point to the correct branch of the version that your server is running. Compiling against the wrong version of SM or MM may make the extension fail to load!

#User-customizable settings:
SDK_LOCATION=~/sdkfolder   		#Set the location where Sourcemod, Metamod, and the hl2sdk files are at

################################
# DO NOT EDIT BELOW THIS LINE! #
################################

#Move back up to the main repository folder
cd ..

#Create the build folder, deleting it in case it already exists:
rm -r build
mkdir build
cd build

#Set our compiler to gcc/g++, then run the configure.py:
CC=gcc CXX=g++ python3 ../configure.py --hl2sdk-root="$SDK_LOCATION" --sm-path="$SDK_LOCATION/sourcemod" --mms-path="$SDK_LOCATION/mmsource-1.10" --enable-optimize --enable-debug

#Then start the build:
ambuild

#Inside the addons/sourcemod/extensions folder, create a blank .autoload file so that the extension will always load with Sourcemod:
touch package/addons/sourcemod/extensions/sigsegv.autoload

#Done!
echo Completed running the build process.
