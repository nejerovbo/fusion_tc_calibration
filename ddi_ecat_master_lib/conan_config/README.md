# Conan
This is a default configuration repository for conan configurations.  What is conan? Conan is a binary repository manager for C++ provided by the company jfrog.  What is jfrog?

Jfrog is a company that has create artifactory.  What is artifactory?  Artifactory is a remote binary manager.
Artifactory - Universal Artifact Repository Manager - JFrog

Artifactory is used to manage build artifacts like libraries, dll’s, executables, containers and much much more!

Artifactory has several toolchain specific clients to accommodate artifacts generated from C++/C, java, nodejs, python…
For C++, the client is called conan.

You can log into our instance at:
https://ddi.jfrog.io/ui/login/


# Setup

## Install prequisites (if needed)

## Conan
### Install the latest version of conan (if needed)
```shell
sudo apt install python3-pip
pip3 install conan
Add PATH="$PATH:/home/<username>/.local/bin/" to the /home/<user>/.bashrc
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

## Link to our repository
To use the repository, you will need to add it to conan locally.
conan remote add ddi-dev  https://ddi.jfrog.io/artifactory/api/conan/ddi-conan-dev-local

Add the cacert.pem file into the ~/.conan directory
Copy the profiles directory into the ~/.conan directory

We wil create a script to perform this in the future!

# Installing Conan

# Using
## Profiles
There are currently two profiles in the profiles directory.  Based on your use case, specify which profile you would like to use.

msvc_x86 - Microsoft Visual C++ 32 bit 
msvc_x64 - Microsoft Visual C++ 64 bit 
lin64 - Linux 64 bit

You should copy the conan_config/profiles/ contents to your ~/.conan/profiles/ directory.

To specify a version you can use the -pr parameter.
```
conan install . -pr lin64
```

There is a conanfile.txt example in the example directory to see how to consume our internal libaries.

# Notes

I have currently added a windoze 64 bit and linux 64 bit libraries as a starting point.

Note that the structure for the libraries is the following:
lib/ folder contains the libraries 
include/ folder contains the header files

Names of the libraries only includes the name of the library and does not contain the version information:

Example: ddi_md5.lib
