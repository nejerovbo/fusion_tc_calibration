# Sample Test Apps
Repo for the Fusion Automated Tester.  Intended to provide a single repository with shared code structure for all fusion (and beyond) tests

# OS dependencies
64-bit Linux

# GCC dependencies
C++ 17 is required in the current version of fusion automated tester.  This limits the accessibility on Ubuntu 16.04, where C++ 17 is not available through sudo apt-get

## Checkout source code
### Checkout the required source code with the --recursive flag
```shell
git clone git@github.com:ddi-sqa/fusion_automated_test.git --recursive
```
## Install prequisites (if needed)

## Conan
### Install the latest version of conan (if needed) and setup the PATH environment variable
```shell
sudo apt install python3-pip
pip3 install conan
# Set PATH=$PATH:$HOME/.local/bin in the ~/.bashrc file manually
source ~/.bashrc
conan profile new default --detect
conan profile update settings.compiler.libcxx=libstdc++11 default
```

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
Building is currently only supported on Linux (tested on Ubuntu 18.04.5)
```shell
cd fat_tests
./build.sh
```

### Test
Run the tests:
```shell
sudo ./run.sh
```
