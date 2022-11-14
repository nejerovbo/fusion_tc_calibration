# DDI ECAT Master SDK Library
Repo for the DDI ECAT Master SDK library

# OS dependencies
64-bit Linux

# GCC dependencies
C++ 11 or greater is required in the current version of the DDI ECAT Master SDK

# Cmake
A mimumum version of 3.3.2 of cmake is required

## Checkout source code
### Checkout the required source code with the --recursive flag
```shell
git clone git@github.com:ddi-lib/ddi_ecat_master_lib.git --recursive
```
## Install prequisites (if needed)

## Conan
### Install the latest version of conan (if needed) and setup the PATH environment variable
```shell
sudo apt install python3-pip
pip3 install conan
echo "PATH=$PATH:$HOME/.local/bin" >> ~/.bashrc
source ~/.bashrc
conan profile new default --detect
conan profile update settings.compiler.libcxx=libstdc++11 default
```

## Link conan to the DDI repository
To use the DDI repository, you will need to add it as a conan remote repo:
```
conan remote add ddi-dev  https://ddi.jfrog.io/artifactory/api/conan/ddi-conan-dev-local
conan user username -r ddi-dev -p Password
```

Add the cacert.pem file from https://github.com/ddi-build/conan_config/blob/main/config/cacert.pem into the ~/.conan directory

## Cmake
### Install the latest version of cmake (if needed)
Install cmake with sudo apt-get 
```shell
sudo apt update
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
sudo apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main"
sudo apt update
sudo apt install cmake
```

### Important: If you are receiving build errors
> :warning: **If you are using GCC compiler >= 5.1,** Conan will set the compiler.libcxx to the old ABI for backwards compatibility. In the context of this getting started example, this is a bad choice though: Recent gcc versions will compile the example by default with the new ABI and linking will fail without further customization of your cmake configuration. You can avoid this with the following commands:
```shell
conan profile new default --detect  # Generates default profile detecting GCC and sets old ABI
conan profile update settings.compiler.libcxx=libstdc++11 default  # Sets libcxx to C++11 ABI
```

### Build
Building is currently only supported on Linux
```shell
./build_lin64.sh
```
