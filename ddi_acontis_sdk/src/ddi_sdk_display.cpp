/******************************************************************************
 * (c) Copyright 2019-2021 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include "ddi_sdk_display.h"


void ddi_display_clear_UI()
{
// clear screen
    printf("\033[H\033[J");
}

void ddi_display_update_UI_DO(int x,int y,uint16_t value)
{
    printf("\033[%d;%dH %04X",x,y,value);
}

void ddi_display_update_UI_XY(int x, int y)
{
    printf("\033[%d;%dH",x,y);
}
void ddi_display_update_UI_EC(int x,int y,uint32_t value)
{
    printf("\033[%d;%dH %08X",x,y,value);
}
void ddi_display_update_UI_AI_EC(int x,int y,uint32_t value)
{
    printf("\033[%d;%dH %01X",x,y,value);
}

void ddi_display_update_UI_CJ(int x,int y,uint16_t value)
{
    double degrees_c;
    //voltage = (float)value * .0003051859;
    degrees_c = (double)value / 32.767f;
    printf("\033[%d;%dH AI - %04X  Deg (C) - %7.2f",x,y,value,degrees_c);
}

void ddi_display_update_UI_TC(int x,int y,uint16_t value)
{
    double voltage;
    double mv_reading;
    voltage = (value - 22767.0 );
    mv_reading = voltage/32767.0f;
    printf("\033[%d;%dH AI - %04X  mV - %7.6f",x,y,value,mv_reading);
}

void ddi_display_update_UI_AO(int x,int y,uint16_t value)
{
    double voltage;
    if ( value & 0x8000)
      voltage = 0;
    else 
      //voltage = (float)value * .0003051859;
      voltage = (double)value / 3276.7f;
    printf("\033[%d;%dH %04X   V - %7.6f",x,y,value,voltage);

}

void ddi_display_update_UI_AI(int x,int y,uint16_t value)
{
    double voltage;
    
    //voltage = (float)value * .0003051859;
    voltage = (double)(int16_t)value / 3276.7f;
    printf("\033[%d;%dH %04X %7.6f",x,y,value,voltage);

}
void ddi_display_update_UI_DI(int x,int y,uint16_t value)
{
    printf("\033[%d;%dH %04X",x,y,value);

}
void ddi_display_update_UI_Result(int x,int y,uint16_t value)
{
    printf("\033[%d;%dH %04X",x,y,value);
}
