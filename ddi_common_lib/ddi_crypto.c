/******************************************************************************
 * (c) Copyright 2020 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/

#include "ddi_crypto.h"

// Basic XOR data cipher
// Supports position-independent XOR operation
void ddi_xor_cipher(volatile uint8_t *s, uint32_t len)
{
   char xor_key = 'c';
   for (uint32_t i = 0; i < len; i++)
   {
      s[i] = (s[i] ^ (xor_key));
   }
}

