# Rafmod [sigsegv/rafra MvM]
A patched-up clone of https://github.com/rafradek/sigsegv-mvm that anyone can compile and run on their machine.

**Please note that this extension only works on Linux!**

# Build Instructions
*(Note: These instructions are intended for **Debian/Ubuntu/Linux Mint**.)*

Precompiled binaries are available in the [Releases](https://github.com/ramirez-tf2/Rafmod/releases) section. If you wish to build the extension yourself, follow these instructions:

1. Clone this repository (either by git or by downloading the zip).

2. Navigate to this repository's folder on a bash shell and run the following commands:

```
$ cd setup

$ sh ambuild_setup.sh             #NOTE: Skip this step if you already installed ambuild
                                  #(but read the .sh files and change the sdkfolder paths!)
$ sh additional_packages.sh

$ sh build.sh

$ cd ../build/package
```

3. Merge & copy everything in here (in `./build/package`) into your server's `/tf/` folder.

4. Copy `srcds_packages.sh` from the `/setup` folder to somewhere in your server and run it. When it's finished, delete the .sh file.

5. Start your server. Run `find sig_` on your srcds console to print out all the available cvars. Set the cvars of the features you want to enable in your server.cfg.

# Troubleshooting
These steps created a working binary on a blank Linux Mint 20 64-bit installation. If you have trouble compiling the extension on your machine, try setting up a fresh virtual machine and following the exact steps outlined here to compile the extension.

A loading error can occur if you compiled the extension against the wrong version of Sourcemod or Metamod. Make sure that your copies of the Sourcemod and Metamod source code (the "sourcemod" and "mmsource-1.10" folders in ~/sdkfolder) match the same versions your server is running! If not, then switch the branch in the SM/MM folders to the correct versions and re-run build.sh again. (The version number is in the "product.version" file.)

As of writing, compiling against Sourcemod 1.10-dev and Metamod 1.11-dev on a server running SM 1.10 and MM 1.11 works properly.
