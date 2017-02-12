#!/bin/bash

cd '/sys/devices/bone_capemgr.9/'
sudo sh -c 'echo BB-UART4 > slots'
sudo sh -c 'echo BB-UART5 > slots'
