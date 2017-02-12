#!/bin/bash

cd '/sys/devices/platform/bone_capemgr/'
sudo sh -c 'echo BB-UART4 > slots'
sudo sh -c 'echo BB-UART5 > slots'
cd '/root'
sudo sh -c './SailbotBrain'

#i) Create a script called mystartup.sh in /etc/init.d/ directory(login as root)
# vi /etc/init.d/mystartup.sh

#ii) Add commands to this script one by one:
#!/bin/bash
#echo “Setting up customized environment…”
#fortune

#iii) Setup executable permission on script:
# chmod +x /etc/init.d/mystartup.sh

#iv)Make sure this script get executed every time Debian Linux system boot up/comes up:
# update-rc.d mystartup.sh defaults 100
