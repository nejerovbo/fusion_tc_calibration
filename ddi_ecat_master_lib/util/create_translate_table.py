#/**************************************************************************
#(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
#Unpublished copyright. All rights reserved. Contains proprietary and
#confidential trade secrets belonging to DDI. Disclosure or release without
#prior written authorization of DDI is prohibited.
#**************************************************************************/

# This file generates an acontis to DDI translation table for both errors and notifications

preamble = """/**************************************************************************\n\
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.\n\
Unpublished copyright. All rights reserved. Contains proprietary and\n\
confidential trade secrets belonging to DDI. Disclosure or release without\n\
prior written authorization of DDI is prohibited.\n\
**************************************************************************/\n\
\n\
#include <stdio.h>\n\
#include <stdlib.h>\n\
#include <stdarg.h>\n\
#include <time.h>\n\
#include <signal.h>\n\
#include <AtEthercat.h>\n\
#include "EcError.h"\n\
#include "EcInterfaceCommon.h"\n\
#include "ddi_em_api.h"\n\
\n\
// Provides a mapping from Acontis codes and strings to DDI codes and strings\n\
\n\
typedef struct {\n\
  uint32_t    acontis_code;\n\
  const char* acontis_string;\n\
  uint32_t    ddi_code;\n\
  const char* ddi_string;\n\
} acontis_ddi_mapping_obj;\n\
\n\
#define ACONTIS_ERR(d) EC_E_##d, EC_SZTXT_E_##d\n\
#define ACONTIS_NOTIFY(d) EC_NOTIFY_##d, EC_SZTXT_NOTIFY_##d\n\
"""

# Generate the error table
def generate_error_table (source_file, dest_file):
  output = open(dest_file,'wt')
  output.write(preamble + "\n")
  output.write("acontis_ddi_mapping_obj g_acontis_ddi_err_table []  = {\n");
  with open(source_file,'rt') as input:
    for line in input.readlines():
      # Skip error codes that don't have a matching text field
      if ( '_LAST' in line or '_FIRST' in line or 'ERROR2' in line or 'EC_E_PTS_IS_RUNNING' in line or 'SZTXT' in line):
        continue
      # Use the ACONTIS_ERR macro to represent the actonis error codes in one table entry
      # For the most part, the Acontis errors have the format EC_E_ for numeric codes and EC_SZTXT_E_ for the string
      # E.g. EC_E_NOTSUPPORTED and EC_SZTXT_E_NOTSUPPORTED
      if ('EC_E' in line):
        if ('define' in line):
          output.write("  { ")
          error_line = line.split()
          error_text = error_line[1].replace('EC_E_','ACONTIS_ERR(')
          print(error_text)
          strlen = len(error_text)
          output.write(error_text)
          output.write(')')
          output.write(',')
          # Align table column so the columns are in order
          for count in range(0, 44-strlen):
            output.write(' ')
          # Generate default tmp dayt
          output.write(' 0, "txt"},\n')
  output.write("\n}\n")

def generate_notify_table (source_file, dest_file):
  output = open(dest_file,'at')
  output.write(preamble + "\n")
  output.write("acontis_ddi_mapping_obj g_acontis_ddi_notify_table []  = {\n");
  with open(source_file,'rt') as input:
    for line in input.readlines():
      # Skip the SZTXT entries
      if ('SZTXT' in line):
        continue
      # Use the NOTIFY macro to represent the actonis error codes in one table entry
      # For the most part, the Acontis errors have the format EC_NOTIFY_ for numeric codes and EC_SZTXT_NOTIFY_ for the string
      # E.g. EC_NOTIFY_SLAVE_PRESENCE and EC_SZTXT_NOTIFY_SLAVE_PRESENCE
      if ('EC_NOTIFY' in line):
        if ('define' in line):
          print(line.split)
          output.write("  { ")
          notify_line = line.split()
          notify_text = notify_line[1].replace('EC_NOTIFY_','ACONTIS_NOTIFY(')
          print(notify_text)
          strlen = len(notify_text)
          output.write(notify_text)
          output.write(')')
          output.write(',')
          for count in range(0, 44-strlen):
            output.write(' ')
          output.write(' 0, "txt"},\n')
  output.write("\n}\n")

def __main__():
  # Generate an error mapping table to a temporary file
  generate_error_table("acontis_lib/SDK/INC/EcError.h","translate_tmp.cpp")
  # Generate an notification mapping table to a temporary file
  generate_notify_table("acontis_lib/SDK/INC/EcInterfaceCommon.h","translate_tmp.cpp")

# Call the main function
__main__()
