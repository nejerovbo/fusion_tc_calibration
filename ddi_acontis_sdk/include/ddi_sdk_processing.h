/******************************************************************************
 * (c) Copyright 2019-2021 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/

#ifndef DDI_SDK_PROCESSING_H
#define DDI_SDK_PROCESSING_H

#ifdef __cplusplus
extern "C" {
#endif

/** compute_scaled_sin
 * Returns a scaled integer 
 *
 * @param angle the angle into the sin function
 * @param phase offset into the sin function
 * @return ddi_status_t
 * @see ddi_status_t
 */
 uint16_t compute_scaled_sin(float angle, float phase);

/** compare_ain_aout
 * Compares two signed analog numbers.  The two 
 * numbers be within AIN_AOUT_COMPARE_BAND units of each others
 *
 * @param ain_value the ain value 
 * @param compare_value the compare value
 * @return ddi_status_t
 * @see ddi_status_t
 */
 uint16_t compare_ain_aout (int16_t ain_value, int16_t compare_value);

#ifdef __cplusplus
}
#endif

#endif // DDI_SDK_PROCESSING_H

