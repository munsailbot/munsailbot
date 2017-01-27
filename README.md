# munsailbot
This repository contains all code for the Memorial University (MUN) SailBOT team.
SailBOT is an annual competition in which teams compete in various events with autonomous sailboats.

## Overview
###BeagleCode
Contains two subprojects, libBeagleUtil and SailbotBrain. Both target
the Beaglebone Black (BBB), at present requiring ARM GCC 4.7 (gnueabi**hf**).

libBeagleUtil is largely unfinished and the only working code is related to serial IO.
In the future this can (and should) be merged with SailbotBrain.

SailbotBrain is an application intended to be started at boot on the BBB, and executes
the autonomous sailing algorithm when activated by the controller.

###HardwareController
Arduino-based code for the hand-held controller. Missing proper build system at present.

###MotorController
Arduino-based code for the motor controller board.

###RTuinOS
[RTuinOS](https://github.com/PeterVranken/RTuinOS) is a real-time operating system targeting the ATmega2560.

The sailbot application lives in RTuinOS/code/applications/sailbot. RTuinOS ships with its own makefile and doesn't
require CMake - see RTuinOS/readMe.txt and the output of RTuinOS make for build instructions.

###Simulator
Unfinished Java simulator for the sailbot

###Testing
Various python scripts used to unit-test methods and algorithms prior to implementation in C++ on the BBB.

## General build instructions
Most of the code is meant to be built with CMake and make. CMake is a cross-platform utility for generating makefiles
and IDE projects. The following instructions assume CMake is available and used from a Linux-based OS. It should also work
the same on Windows and OS X, but you will need to ensure that it can find the appropriate compilers.

There is a guide [here](http://jkuhlm.bplaced.net/hellobone/) for cross-compiling for the BBB from Windows.
It also discusses building with make and Eclipse. CMake may be configured to generate an Eclipse project if you wish.

To generate a *nix makefile for Linux/MacOS, the usual procedure is
```
$> cd munsailbot/<subproject>
$> mkdir build
$> cd build
$> cmake ../ -G "Unix Makefiles"
```
This creates a build/ directory for all intermediate and final output. This directory should **not** be added to the repo.
It isn't actually necessary, but it helps to keep build output separate from the rest of the repo.

Provided CMake can find everything, this will generate a Makefile for the project.
You can then compile with
```
$> cd ../
$> make
```

For arduino-based projects, this will generate a .hex file which can be uploaded with the upload.sh script found
in the root directory of the project (*nix only). Depending on your system this script may need to be adjusted.

For the BBB, an executable targeting ARM will be compiled. This must be copied to the BBB via SSHFS.
The makefile for the BBB will look for the compiler arm-linux-gnueabihf-gcc-4.7. This is GCC 4.7 targeting ARM with hard-float.
Your system likely doesn't ship with this compiler, it can be installed with most package managers on Linux easily.

On Ubuntu, for example
```
sudo apt-get install gcc-4.7-arm-linux-gnueabihf
```
## VM Build

On OSX or Windows, download and install Vagrant and Virtualbox

https://www.vagrantup.com/
https://www.virtualbox.org/wiki/Downloads

```
vagrant init munsailbot/base; vagrant up --provider virtualbox; vagrant ssh
```

This will install and launch a headless minimal build of Ubuntu with all project dependencies which can be used to easily compile and share files between the host and VM. Any changes made in ~/vagrant of the Ubuntu VM is mirrored in the host OS.

## SSH

__Windows & OSX Users__ Follow steps 1 & 2 seen here http://beagleboard.org/getting-started and then navigate to 192.168.7.2 in your browser. If a Beaglebone starter page appears, you can SSH to the Beaglebone by entering ```ssh root@192.168.7.2``` in your terminal, or using an SSH-ready software like Cyberduck or WinSCP.

__Linux Users__ ```ssh root@192.168.7.2``` with Beaglebone connected via USB.
