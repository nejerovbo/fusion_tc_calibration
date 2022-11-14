/**************************************************************************
(c) Copyright 2021 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef DDI_EM_TRANSLATE_H
#define DDI_EM_TRANSLATE_H

#include "ddi_em_api.h"

/** translate_ddi_acontis_err_code
 @brief Translate from acontis->ddi error code. This is used to handle failures from the Acontis subsytem
 @param em_handle The EtherCAT master handle
 @param acontis_code The Acontis error code to be translated into DDI error code
 @return ddi_em_result The translated DDI code
 @see ddi_em_result
 */
ddi_em_result translate_ddi_acontis_err_code (ddi_em_handle em_handle, uint32_t acontis_code);
/** translate_acontis_ddi_event_code
 @brief Translate from acontis to ddi notification code. This is used to handle events from the Acontis subsystem
 @param em_handle The EtherCAT master handle
 @param es_handle The EtherCAT slave handle
 @param acontis_code The Acontis event code to be translated into DDI event code
 @param event The DDI EM event corresponding to the Acontis code
 @return ddi_em_result The translated DDI code
 @see ddi_em_result
 */
ddi_em_result translate_acontis_ddi_event_code (ddi_em_handle em_handle, ddi_es_handle es_handle , uint32_t acontis_code, ddi_em_event *event);
/** translate_ddi_acontis_event_code
 @brief Translate from acontis to ddi notification code. This is used for setting event handling masks where the DDI->Acontis mapping is needed
 @param ddi_notify_code The EtherCAT master handle
 @param acontis_code The Acontis code
 @return ddi_em_result The translated DDI code
 @see ddi_em_result
 */
ddi_em_result translate_ddi_acontis_event_code (ddi_em_event_type ddi_notify_code, uint32_t *acontis_code);

#endif //__DDI_EM_TRANSLATE_H__
