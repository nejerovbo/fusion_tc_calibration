#!/bin/bash
#**************************************************************************
#(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
#Unpublished copyright. All rights reserved. Contains proprietary and
#confidential trade secrets belonging to DDI. Disclosure or release without
#prior written authorization of DDI is prohibited.
#*************************************************************************/

# Installation script for the DDI ECAT Master SDK

RED="\e[31m"
GREEN="\e[32m"
YELLOW="\e[33m"
CLEAR="\e[0m"
INSTALLATION_SUCCESS=0

# Print confirmation dialog
echo
echo -e "${GREEN}*****This installer script will install the DDI ECAT Master SDK${CLEAR}"
echo -e "${GREEN}*****libraries to /lib/ and include files to /usr/include/${CLEAR}"
echo
read -p "Do you wish to continue(Y/N)? " input_value

# Check for root-user
if [ "$EUID" -ne 0 ]
  then echo -e "${YELLOW}Please execute this installer script as superuser${CLEAR}"
  exit 1
fi

# Display a fail message and exit
fail_installer_msg () {
  echo -e "${RED} $1 ${CLEAR}"
  exit 1
}

# If user enters 'Y' then proceeed with installation
if [ "$input_value" == "Y" ]; then
  echo -e "${GREEN}Proceeding with installation${CLEAR}"
  # Installs the include/ directory to /usr/include/
  cp -rf include/* /usr/include/
  if [ $? -ne 0 ]; then 
    fail_installer_msg "Installation failed while installing headers"
  fi
  # Installs the include/ directory to /lib64/
  cp -rf lib/* /lib64/
  if [ $? -ne 0 ]; then 
    fail_installer_msg "Installation failed while installing libraries"
  fi
  # Display success message and exit
  echo
  echo -e "${GREEN}DDI ECAT Master SDK installation success${CLEAR}"
  exit 0;
else
  echo
  echo -e "${YELLOW}DDI ECAT Master SDK installation aborted${CLEAR}"
  exit 0;
fi
