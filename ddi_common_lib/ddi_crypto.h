/******************************************************************************
 * (c) Copyright 2020 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** ddi_xor_cipher
 @brief Performs an in-place xor cipher at the data pointed to by s for len bytes
 @param s The source virtual address
 @param len The length in bytes
 */
void ddi_xor_cipher(volatile uint8_t *s, uint32_t len);

#ifdef __cplusplus
}
#endif
