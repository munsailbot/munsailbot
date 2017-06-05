sudo apt-get install cmake g++ git gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf
sudo apt-get install lib32ncurses5-dev
echo 'PS1="MUNSB # "' >> ~/.bashrc
echo 'export CC="/usr/bin/arm-linux-gnueabihf-gcc-4.8"' >> ~/.bashrc
echo 'export CXX="/usr/bin/arm-linux-gnueabihf-g++-4.8"' >> ~/.bashrc
echo 'alias buildsb="cd /home/vagrant/git/munsailbot/BeagleCode/SailbotBrain/; rm -rf build; mkdir build; cd build; cmake ../ -G 'Unix Makefiles'; make;"' >> ~/.bashrc
sudo mkdir /usr/local/boost
sudo fallocate -l 4G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile
sudo swapon -s
echo '/swapfile   none    swap    sw    0   0' >> /etc/fstab
wget -O boost_1_64_0.tar.gz http://sourceforge.net/projects/boost/files/boost/1.64.0/boost_1_64_0.tar.gz/download
tar xzvf boost_1_64_0.tar.gz
cd boost_1_64_0/
./bootstrap.sh --prefix=/usr/local/boost
n=`cat /proc/cpuinfo | grep "cpu cores" | uniq | awk '{print $NF}'`
sudo ./b2 toolset=gcc-arm --with-filesystem --with-system -j $n install
