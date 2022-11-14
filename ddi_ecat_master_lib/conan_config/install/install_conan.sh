#!/bin/bash
# Installs conan on Debian distributions only
# Do not run this script as sudo
if [ "$EUID" -eq 0 ]
  then echo "Please run as a normal user"
  exit
fi
sudo apt install python3-pip
sudo pip3 install conan
# Sets up path correctly
echo 'export PATH=$PATH:~/.local/bin' >> ~/.bashrc
export PATH=$PATH:~/.local/bin
echo $PATH
conan --version
conan profile new default --detect  # Generates default profile detecting GCC and sets old ABI
conan profile update settings.compiler.libcxx=libstdc++11 default  # Sets libcxx to C++11 ABI
