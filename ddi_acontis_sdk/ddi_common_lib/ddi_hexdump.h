/*****************************************************************************
 * (c) Copyright 2019 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
/*  ddi_hexdump.h
 *  Created by Johana Lehrer on 2019-07-10
 */

#ifndef DDI_HEXDUMP_H
#define DDI_HEXDUMP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** simple_hexdump
 @brief Prints a simple binary hexdump of a memory buffer with no ascii formatting.
 * @param data Memory to hexdump
 * @param len size of the data to hexdump
 */
void simple_hexdump(const void *data, size_t len);

/** hexdump
 * @brief Prints a binary hexdump of a memory buffer.
 * When buf param is non-null it must point to enough memory to receive the formatted hexdump
 * including the address+hex+ascii display.
 * @param addr Logical start address of hexdump
 * @param data Memory to hexdump
 * @param len size of the data to hexdump
 * @param buf If non-null hexdump is formatted into this buffer; otherwise hexdump to stdout
 * @return the number of chars printed
 */
int hexdump(uint32_t addr, const void *data, size_t len, char *buf, int width);

#ifdef __cplusplus
}
#endif

#endif // DDI_HEXDUMP_H
