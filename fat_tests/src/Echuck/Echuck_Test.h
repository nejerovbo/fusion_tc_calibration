#ifndef ECHUCK_H
#define ECHUCK_H

#include "EnvironmentProvider.h"
#include <stdint.h>

// Color defines
#define RED       "\033[0;91m"
#define GREEN     "\033[1;92m"
#define YELLOW    "\033[1;93m"
#define ORANGE    "\033[0;93m"
#define BLUE      "\033[1;94m"
#define VIOLET    "\033[0;95m"
#define CYAN      "\033[0;96m"
#define CLEAR     "\033[0m"

// Printf with coordinates followed by a clear line
#define PRINTF_GOTO_XY(x,y) do{ printf("\033[%d;%dH", y, x); printf("\33[2K"); } while(0)

// Dos clear screen command
#define clrscr()  printf("\e[1;1H\e[2J")

// Clear the current line
#define CLEARLINE() printf("\33[2K")

class EchuckCommand : public DDICommand {
public:
    using DDICommand::DDICommand;
    virtual string GetTestFilter() { return "*Echuck*"; }
    virtual string GetDescription() { return "Echuck Test(s)"; }
};

typedef struct{
  uint8_t digital_out[8];   // toggle between 0 and 0xff
  uint16_t analog_out[4];   // toggle between 8000 ( -10 ) and 7fff ( 10 ) (signed)      
} echuck_out_t;

typedef struct{
  uint16_t analog_in[8];    // print out the reads
  uint8_t digital_in[7];    // print out the reads
} echuck_in_t;

float ddi_fusion_convert_hex_to_volts (uint16_t input_in_hex);
uint16_t ddi_fusion_convert_volts_to_hex (float input_in_volts);

#endif
