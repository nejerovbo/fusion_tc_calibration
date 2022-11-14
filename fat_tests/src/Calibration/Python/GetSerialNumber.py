#*****************************************************************************
#* (c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
#* Unpublished copyright. All rights reserved. Contains proprietary and
#* confidential trade secrets belonging to DDI. Disclosure or release without
#* prior written authorization of DDI is prohibited.
#*****************************************************************************/

# created by Teresa Faasolo & Zuola Guoerluoti
# Python 3?

from datetime import datetime
import time

serial_number_list = []

def prompt_print():
    print("Enter '1' if this is an AIN card.")
    print("Enter '2' if this is an AOUT card.")
    print("Enter 'x' if there are no more cards.")    
    
while True:
    prompt_print()
    
    user_input = raw_input("Please enter your selection: ")
    if user_input == '1':
        serial_number = raw_input("Please enter the serial number of your card: ")
        # add input check here
        serial_number += "_ai_"
        for i in serial_number_list:
            if i == serial_number:
                print("This card has already been scanned. Please scan a new card: ")
            else:
                serial_number_list.append(serial_number)
    if user_input == '2':
        serial_number = raw_input("Please enter the serial number of your card: ")
        # add input check here
        serial_number += "_ao_"
        for i in serial_number_list:
            if i == serial_number:
                print("This card has already been scanned. Please scan a new card: ")
            else:
                serial_number_list.append(serial_number)
    if user_input == 'x':
        break

# create a .txt file here
file = open('/home/ddi-zuola/code/fusion_automated_test/fat_tests/serial_numbers_list.txt', 'w')

# file = open('/media/vault/Shaun/Python_texts/serial_numbers_list.txt', 'w')

now = datetime.now()
now = now.strftime("%m-%d-%Y_%H:%M:%S")
for x in serial_number_list:
    x += now
    file.write(x + '\n') # or whatever character that is accepted by c++

file.close()
