/******************************************************************************
 * (c) Copyright 2019-2021 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/

#ifndef DDI_SDK_DISPLAY_H
#define DDI_SDK_DISPLAY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void ddi_display_clear_UI(void);
void ddi_display_update_UI_DO(int, int,uint16_t);
void ddi_display_update_UI_DI(int, int,uint16_t);
void ddi_display_update_UI_XY(int , int );
void ddi_display_update_UI_Result(int,int,uint16_t);
void ddi_display_update_UI_EC(int ,int ,uint32_t );
void ddi_display_update_UI_AI(int ,int ,uint16_t);
void ddi_display_update_UI_AO(int ,int ,uint16_t);
void ddi_display_update_UI_TC(int ,int ,uint16_t);
void ddi_display_update_UI_CJ(int ,int ,uint16_t);
void ddi_display_update_UI_AI_EC(int ,int ,uint32_t);

#ifdef __cplusplus
}
#endif

#endif // DDI_SDK_DISPLAY_H

