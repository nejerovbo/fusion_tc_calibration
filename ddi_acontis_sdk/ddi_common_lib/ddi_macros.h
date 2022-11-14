/******************************************************************************
 * (c) Copyright 2018-2020 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
/*
 * ddi_macros.h
 *
 *  Created on: Aug 21, 2018
 *      Author: jlehrer
 *
 */

#ifndef ddi_macros_h
#define ddi_macros_h

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

/** ARRAY32 macros ******************************************************************
 The following ARRAY32 macros are used to set and get bitfields in an array.
 They can be used in the case where you want to pack many bitfields into an array
 and use the smallest array size to do so.

 These macros work with any size bitfield from 1 to 32 bits;
 and are most efficient when 32 can be evenly divisible by the bitfield size
 and as the bitfield size decreases.

 Example usage:
 #define BITSIZE       4
 #define FIELDCOUNT    64

 uint32_t array[ARRAY32_SIZE(BITSIZE, FIELDCOUNT)] = {0};
 uint32_t value = 42;
 uint32_t index = 54;

 array[ARRAY32_INDEX(BITSIZE, index)] = ARRAY32_SET_ENTRY(array, BITSIZE, index, value);

 uint32_t the_value = ARRAY32_GET_ENTRY(array, BITSIZE, index);
 */


/** ARRAY32_SHIFT(entrysize,index)
 Returns the left-shift for the bitfield entry within the array index it is contained in.

 @param entrysize The number of bits of each bitfield entry in the array.
 @param index The index of the bitfield entry to access.
 @returns The low-order bit number of the bitfield within the array.
 */
#define ARRAY32_SHIFT(entrysize,index) (((entrysize)*(index))%((entrysize)*(32/(entrysize))))

/** ARRAY32_INDEX(entrysize,index)
 Returns the array index containing the bitfield entry.

 @param entrysize The number of bits of each bitfield entry. (!) must be in the range from 1 to 32
 @param index The index of the bitfield entry in the array.
 @returns The array index which contains the field.
 */
#define ARRAY32_INDEX(entrysize,index) ((index)/(32/(entrysize)))

/** ARRAY32_SIZE(entrysize,count)
 Returns the array size needed to contain 'count' number of bitfield entries each with 'entrysize' number of bits:

 @param entrysize The number of bits of each bitfield entry in the array. (!) must be in the range from 1 to 32
 @param count The number of bitfield entries to store in the array.
 @returns The array size needed for 'count' number of fields of size 'entrysize'.
 */
#define ARRAY32_SIZE(entrysize,count) ((count + ((32/(entrysize))-1))/(32/(entrysize)))

/** ARRAY32_MASK(entrysize,index)
 Returns the bitmask of the bitfield within the array:
 For example: given entrysize=3; index=26; this macro would return 0x001c0000 (i.e. the bitmask for the array element containing the bitfield)

 @param entrysize The number of bits of each bitfield entry in the array. (!) must be in the range from 1 to 32
 @param index The index of the bitfield entry to access.
 @returns The bitmask for the bitfield within the array.
 */
#define ARRAY32_MASK(entrysize,index) ((0xffffffff>>(32-(entrysize))) << ARRAY32_SHIFT((entrysize),(index)))

/** ARRAY32_VALUE(array,entrysize,index,value)
 Returns the left-shifted bitfield 'value' of the 'index'ed register of 'entrysize' bits within the 32-bit 'array':

 @param array The array containing the bitfield.
 @param entrysize The number of bits of each bitfield entry in the array. (!) must be in the range from 1 to 32
 @param index The index of the bitfield entry to access.
 @returns The value of the bitfield within the array.
 */
#define ARRAY32_VALUE(array,entrysize,index,value) (ARRAY32_MASK(entrysize,index) & (value<<ARRAY32_SHIFT(entrysize,index)))

/** ARRAY32_CONST(array,entrysize,index,hilo,_const)
 Returns the left-shifted bitfield entry constant value of the 'index'ed entry within the 32-bit 'array':

 @param array The array containing the bitfield.
 @param entrysize The number of bits of each bitfield entry in the array. (!) must be in the range from 1 to 32
 @param index The index of the bitfield entry to access.
 @returns The value of the bitfield within the array.
 */
#define ARRAY32_CONST(array,entrysize,index,hilo,_const) (ARRAY32_MASK(entrysize,index) & (REG32_VALUE(hilo,hilo##_const)<<ARRAY32_SHIFT(entrysize,index)))

/** ARRAY32_CLR_ENTRY(array,entrysize,index)
 Clears the entry in the array.

 @param array The array containing the entry to set.
 @param entrysize The number of bits of each entry in the array. (!) must be in the range from 1 to 32
 @param index The index of the entry to access.
 @returns The value of the array entry containing the bitfield after the new bitfield value is set.
 */
#define ARRAY32_CLR_ENTRY(array,entrysize,index) (array[ARRAY32_INDEX(entrysize, index)] = \
                    (array)[ARRAY32_INDEX(entrysize,index)] & ~ARRAY32_MASK(entrysize,index))

/** ARRAY32_SET_ENTRY(array,entrysize,index,value)
 Sets value of the 'index'ed entry in the array.

 @param value The new value to set
 @param array The array containing the entry to set.
 @param entrysize The number of bits of each entry in the array. (!) must be in the range from 1 to 32
 @param index The index of the entry to access.
 @returns The value of the array entry containing the bitfield after the new bitfield value is set.
 */
#define ARRAY32_SET_ENTRY(array,entrysize,index,value) array[ARRAY32_INDEX(entrysize, index)] = \
                  (((array)[ARRAY32_INDEX(entrysize,index)] & ~ARRAY32_MASK(entrysize,index)) | \
   ((((uint32_t)(value)) << ARRAY32_SHIFT(entrysize,index)) & ARRAY32_MASK(entrysize,index)))

/** ARRAY32_GET_ENTRY(array,entrysize,index)
 Returns the right-shifted value of the 'index'ed entry in the array.

 @param array The array containing the entry to get.
 @param entrysize The number of bits of each entry in the array. (!) must be in the range from 1 to 32
 @param index The index of the entry to access.
 @returns The value of the entry within the array.
 */
#define ARRAY32_GET_ENTRY(array,entrysize,index) ((array[ARRAY32_INDEX(entrysize,index)] & ARRAY32_MASK(entrysize,index)) >>ARRAY32_SHIFT(entrysize,index))

/** ARRAY32_SET_VALUE(array,entrysize,index,hilo,value)
 Sets the bitfield value within the 'index'ed entry in the array.

 @param array The array containing the entry.
 @param entrysize The number of bits of each entry in the array. (!) must be in the range from 1 to 32
 @param index The index of the entry to access.
 @param value The value to set into the entry bitfield.
 @returns The value of the array element containing the entry after the new value is set.
 */
#define ARRAY32_SET_VALUE(array,entrysize,index,hilo,value) array[ARRAY32_INDEX(entrysize, index)] = \
          REG32_SET_VALUE(array[ARRAY32_INDEX(entrysize, index)],hilo,value)

/** ARRAY32_SET_CONST(array,entrysize,index,hilo,_const)
 Sets the bitfield constant value within the 'index'ed entry in the array.

 @param array The array containing the entry.
 @param entrysize The number of bits of each bitfield entry in the array. (!) must be in the range from 1 to 32
 @param index The index of the entry to access.
 @returns The value of the array element containing the entry after the new value is set.
 */
#define ARRAY32_SET_CONST(array,entrysize,index,hilo,_const) array[ARRAY32_INDEX(entrysize, index)] = \
          REG32_SET_VALUE(array[ARRAY32_INDEX(entrysize, index)],hilo,hilo##_const)




/** BIT32 macros ******************************************************************
 The following BIT32 macros all take a 'hilo' parameter in the form of hi:lo where:
  hi is the high-order bit index of the bitfield.
  lo is the low-order bit index of the bitfield.

 For example:
 #define MY_REGISTER_BITFIELD_A             31:20
 #define MY_REGISTER_BITFIELD_B             19:16
 #define MY_REGISTER_BITFIELD_B_VAL1        0x3
 #define MY_REGISTER_BITFIELD_B_VAL2        0x9
 #define MY_REGISTER_BITFIELD_B_VAL3        0xd
 #define MY_REGISTER_BITFIELD_C             15:10
 #define MY_REGISTER_BITFIELD_D             9:8
 #define MY_REGISTER_BITFIELD_E             7:1
 #define MY_REGISTER_BITFIELD_F             0:0

 BIT32_SHIFT(MY_REGISTER_BITFIELD_A)                             returns 20
 BIT32_SIZE(MY_REGISTER_BITFIELD_B)                              returns 4
 BIT32_MASK(MY_REGISTER_BITFIELD_A)                              returns 0x00000FFF
 BIT32_MASK(MY_REGISTER_BITFIELD_B)                              returns 0x0000000F
 BIT32_SHIFT_MASK(MY_REGISTER_BITFIELD_C)                        returns 0x0000FC00
 BIT32_SHIFT_MASK(MY_REGISTER_BITFIELD_B)                        returns 0x000F0000
 BIT32_VALUE(MY_REGISTER_BITFIELD_A, 0x12)                       returns 0x01200000
 BIT32_CONST(MY_REGISTER_BITFIELD_B,_VAL2)                       returns 0x00090000
 BIT32_CLR_FIELD(0xFFFFFFFF, MY_REGISTER_BITFIELD_B)             returns 0xFFF0FFFF
 BIT32_SET_VALUE(0xFFFFFFFF, MY_REGISTER_BITFIELD_A, 0x123)      returns 0x123FFFFF
 BIT32_SET_CONST(0xFFFFFFFF, MY_REGISTER_BITFIELD_B,_VAL2)       returns 0xFFF9FFFF
 BIT32_GET_FIELD(0x98765432, MY_REGISTER_BITFIELD_A)             returns 0x987
 BIT32_MASK(MY_REGISTER_BITFIELD_A);                             returns 0x00000fff
*/

/** BIT32_SIZE(hilo)
 Returns the size of the bitfield

 @param hilo bitfield definition
 @returns the bitfield size in bits
*/
#define BIT32_SIZE(hilo)                    ((1?hilo)-(0?hilo)+1)

/** BIT32_SHIFT(hilo)
 Returns the left-shift value of the bitfield

 @param hilo bitfield definition
 @returns the bitfield left-shift value
*/
#define BIT32_SHIFT(hilo)                   (0?hilo)

#define BIT32_SHIFT_IDX(hilo,i)             (0?hilo+(BIT32_SIZE(hilo)*i))

/** BIT32_MASK(hilo)
 Returns the right-shifted bitfield mask with bits of the field set to 1's

 @param hilo bitfield definition
 @returns the bitfield mask
*/
#define BIT32_MASK(hilo)                    (0xffffffff>>(32-BIT32_SIZE(hilo)))

/** BIT32_SHIFT_MASK(hilo)
 Returns the left-shifted bitfield mask with bits of the field set to 1's

 @param hilo bitfield definition
 @returns the left-shifted bitfield mask
*/
#define BIT32_SHIFT_MASK(hilo)              (BIT32_MASK(hilo)<<BIT32_SHIFT(hilo))
#define BIT32_SHIFT_MASK_IDX(hilo,i)        (BIT32_MASK(hilo)<<BIT32_SHIFT_IDX(hilo,i))

/** BIT32_VALUE(hilo,value)
 Returns the value left-shifted into the bitfield position.

 @param hilo bitfield definition
 @param value The bitfield value to set
 @returns the left-shifted bitfield value
*/
#define BIT32_VALUE(hilo,v)                 ((((uint32_t)(v))&BIT32_MASK(hilo))<<BIT32_SHIFT(hilo))
#define BIT32_VALUE_IDX(hilo,v,i)           ((((uint32_t)(v))&BIT32_MASK(hilo))<<BIT32_SHIFT_IDX(hilo,i))

/** BIT32_CONST(hilo,value)
 Returns the constant value left-shifted into the bitfield position.

 @param hilo bitfield definition
 @param value The bitfield constant definition
 @returns the register value with the new bitfield value set
*/
#define BIT32_CONST(hilo,c)                 (((hilo##c)&BIT32_MASK(hilo))<<BIT32_SHIFT(hilo))
#define BIT32_CONST_IDX(hilo,c,i)           (((hilo##c)&BIT32_MASK(hilo))<<BIT32_SHIFT_IDX(hilo,i))


/** BIT32_CLR_FIELD(reg,hilo)
 Returns the value of 'reg' with the bitfield value cleared.

 @param hilo bitfield definition
 @param reg The register value
 @returns the register value with the bitfield cleared to zero
*/
#define BIT32_CLR_FIELD(hilo,reg)           ((reg)&~BIT32_SHIFT_MASK(hilo))
#define BIT32_CLR_FIELD_IDX(hilo,i,reg)     ((reg)&~BIT32_SHIFT_MASK_IDX(hilo,i))

/** BIT32_GET_FIELD(hilo,reg)
 Returns the right-shifted value of a register bitfield

 @param hilo bitfield definition
 @param reg The register value
 @returns the register bitfield value
 */
#define BIT32_GET_FIELD(hilo,reg)           ((((uint32_t)(reg))>>BIT32_SHIFT(hilo))&BIT32_MASK(hilo))
#define BIT32_GET_FIELD_IDX(hilo,i,reg)     ((((uint32_t)(reg))>>BIT32_SHIFT_IDX(hilo,i))&BIT32_MASK(hilo))

#define BIT32_GET_VALUE                     BIT32_GET_FIELD

/** BIT32_SET_FIELD(hilo,reg,value)
 Returns the value of 'reg' with the new bitfield value set.

 @param hilo The bitfield definition
 @param value The new value to set
 @param reg The register value
 @returns the register value with the new bitfield value set
*/
#define BIT32_SET_FIELD(hilo,v,reg)         (BIT32_CLR_FIELD(hilo,reg)|BIT32_VALUE(hilo,v))
#define BIT32_SET_FIELD_IDX(hilo,v,i,reg)   (BIT32_CLR_FIELD_IDX(hilo,i,reg)|BIT32_VALUE_IDX(hilo,v,i))

#define BIT32_SET_VALUE                     BIT32_SET_FIELD
#define BIT32_SET_VALUE_IDX                 BIT32_SET_FIELD_IDX

/** BIT32_SET_CONST(hilo,reg,_const)
 Returns the value of 'reg' with the new bitfield constant set

 @param hilo bitfield definition
 @param reg The register value
 @param _const The new constant value to set
 @returns the bitfield constant value
*/
#define BIT32_SET_CONST(hilo,c,reg)         (BIT32_CLR_FIELD(hilo,reg)|BIT32_VALUE(hilo,hilo##c))
#define BIT32_SET_CONST_IDX(hilo,c,i,reg)   (BIT32_CLR_FIELD_IDX(hilo,i,reg)|BIT32_VALUE_IDX(hilo,i,hilo##c))




/** REG32 macros ******************************************************************
 The following REG32 macros all take 'd', 'r', 'f' parameters.
 The const REG32 macros take an additional 'c' parameter and
 The value REG32 macros take an additional 'v' parameter.
 The index REG32 macros take an additional 'i' parameter.

 The 'd', 'r', 'f' parameters expand into the following register definitions:
  d       : device base address
  d##r    : device register offset
  d##r##f : device register field in the form of hi:lo where:
  hi is the high-order bit index of the bitfield.
  lo is the low-order bit index of the bitfield.

 Macros with the 'c' parameter expand into the value of a field constant definition:
  d##r##f##c : device register field constant

 Macros with the 'v' parameter expand into:
  d##r##f : device register field; and set/get the field value

  usage:
  #define MYDEV                              0x43d00000
  #define MYDEV_A_REG                            0x0000
  #define MYDEV_A_REG_FIELD1                      31:20
  #define MYDEV_A_REG_FIELD1_ENUM1                   13
  #define MYDEV_A_REG_FIELD1_ENUM2                    7
  #define MYDEV_A_REG_FIELD1_ENUM3                   44

  #define MYDEV_A_REG_FIELD2                      19:16
  #define MYDEV_A_REG_FIELD2_ENUM1                  0xF
  #define MYDEV_A_REG_FIELD2_ENUM2                  0xA
  #define MYDEV_A_REG_FIELD2_ENUM3                  0xC
  #define MYDEV_A_REG_FIELD2_ENUM4                  0xE

  #define MYDEV_A_REG_FIELD3                       15:0
  #define MYDEV_A_REG_FIELD3_ENUM1               0xBEEF
  #define MYDEV_A_REG_FIELD3_ENUM2               0xCA7
  #define MYDEV_A_REG_FIELD3_ENUM3               0xF00D

  #define MYDEV_B_REG                            0x0004
  #define MYDEV_B_REG_FIELD1                      31:20
*/

/** REG32_SIZE(d,r,f)
 Returns the size of the bitfield

 @param d device definition
 @param r register definition
 @param f bitfield definition
 @returns the bitfield size in bits
*/
#define REG32_SIZE(d,r,f)                   BIT32_SIZE(d##r##f)

/** REG32_SHIFT(d,r,f)
 Returns the left-shift value of the bitfield

 @param d device definition
 @param r register definition
 @param f bitfield definition
 @returns the bitfield left-shift value
*/
#define REG32_SHIFT(d,r,f)                  BIT32_SHIFT(d##r##f)
#define REG32_SHIFT_IDX(d,r,f,i)            BIT32_SHIFT_IDX(d##r##f,i)

/** REG32_MASK(d,r,f)
 Returns the right-shifted bitfield mask with bits of the field set to 1's

 @param d device definition
 @param r register definition
 @param f bitfield definition
 @returns the bitfield mask
*/
#define REG32_MASK(d,r,f)                   BIT32_MASK(d##r##f)

/** REG32_SHIFT_MASK(d,r,f)
 Returns the left-shifted bitfield mask with bits of the field set to 1's

 @param d device definition
 @param r register definition
 @param f bitfield definition
 @returns the left-shifted bitfield mask
*/
#define REG32_SHIFT_MASK(d,r,f)             BIT32_SHIFT_MASK(d##r##f)
#define REG32_SHIFT_MASK_IDX(d,r,f,i)       BIT32_SHIFT_MASK(d##r##f,i)

/** REG32_VALUE(d,r,f,v)
 Returns the value left-shifted into the bitfield position.

 @param d device definition
 @param r register definition
 @param f bitfield definition
 @param v The bitfield value to set
 @returns the left-shifted bitfield value
*/
#define REG32_VALUE(d,r,f,v)                BIT32_VALUE(d##r##f,v)

/** REG32_VALUE_IDX(d,r,f,v,i)
 Returns the value 'v' left-shifted into the bitfield position
 of an indexed "repeated" bitfield. e.g. given the register bitfield:
 [31..28|27..24|23..20|19..16|15..12|11..8|7..4|3..0]
 specifying index 'i' = 3 would return the value 'v' left-shifted
 into bits[15..12] which is the 3rd zero-based index of the repeating bitfield.

 @param d device definition
 @param r register definition
 @param f bitfield definition
 @param v The bitfield value to set
 @param i The bitfield index to set
 @returns the left-shifted bitfield value
 */
#define REG32_VALUE_IDX(d,r,f,v,i)          BIT32_VALUE_IDX(d##r##f,v,i)

/** REG32_CONST(d,r,f,c)
 Returns the constant value left-shifted into the bitfield position.

 @param d device definition
 @param r register definition
 @param f bitfield definition
 @param c The bitfield constant definition
 @returns the register value with the new bitfield value set
*/
#define REG32_CONST(d,r,f,c)                (((d##r##f##c)&BIT32_MASK(d##r##f))<<BIT32_SHIFT(d##r##f))

/** REG32_CONST_IDX(d,r,f,v,i)
 Returns the constant 'c' left-shifted into the bitfield position
 of an indexed "repeated" bitfield. e.g. given the register bitfield:
 [31..28|27..24|23..20|19..16|15..12|11..8|7..4|3..0]
 specifying index 'i' = 3 would return the constant 'c' left-shifted
 into bits[15..12] which is the 3rd zero-based index of the repeating bitfield.

 @param d device definition
 @param r register definition
 @param f bitfield definition
 @param c The bitfield constant definition
 @param i The bitfield index to set
 @returns the left-shifted bitfield value
 */
#define REG32_CONST_IDX(d,r,f,c,i)          (((d##r##f##c)&BIT32_MASK(d##r##f))<<BIT32_SHIFT_IDX(d##r##f,i))


/** REG32_CLR_FIELD(d,r,f,reg)
 Returns the value of 'reg' with the bitfield value cleared.

 @param d device definition
 @param r register definition
 @param f bitfield definition
 @param reg The register value
 @returns the register value with the bitfield cleared to zero
*/
#define REG32_CLR_FIELD(d,r,f,reg)          BIT32_CLR_FIELD(d##r##f,reg)

/** REG32_CLR_FIELD_IDX(d,r,f,i,reg)
 Returns the value of 'reg' with the field cleared at the position 'i'
 of an indexed "repeated" bitfield. e.g. given the register bitfield:
 [31..28|27..24|23..20|19..16|15..12|11..8|7..4|3..0]
 specifying index 'i' = 3 would return the value of 'reg' with bits[15..12] cleared.

 @param d device definition
 @param r register definition
 @param f bitfield definition
 @param i The bitfield index to clear
 @param reg The register value
 @returns the register value with the indexed bitfield cleared to zero
 */
#define REG32_CLR_FIELD_IDX(d,r,f,i,reg)    BIT32_CLR_FIELD_IDX(d##r##f,i,reg)

/** REG32_GET_FIELD(d,r,f,reg)
 Returns the right-shifted value of a register bitfield

 @param d device definition
 @param r register definition
 @param f bitfield definition
 @param reg The register value
 @returns the register bitfield value
 */
#define REG32_GET_FIELD(d,r,f,reg)          BIT32_GET_FIELD(d##r##f,reg)

/** REG32_GET_FIELD_IDX(d,r,f,i,reg)
 Returns the right-shifted value of a field at the position 'i'
 of an indexed "repeated" bitfield. e.g. given the register bitfield:
 [31..28|27..24|23..20|19..16|15..12|11..8|7..4|3..0]
 specifying index 'i' = 3 would return the value of bits[15..12].

 @param d device definition
 @param r register definition
 @param f bitfield definition
 @param i The bitfield index to get
 @param reg The register value
 @returns the indexed bitfield register value
 */
#define REG32_GET_FIELD_IDX(d,r,f,i,reg)    BIT32_GET_FIELD_IDX(d##r##f,i,reg)

#define REG32_GET_VALUE                     REG32_GET_FIELD

/** REG32_SET_VALUE(d,r,f,v,reg)
 Returns the value of 'reg' with the new bitfield value set.

 @param d device definition
 @param r register definition
 @param f bitfield definition
 @param v The new value to set
 @param reg The register value
 @returns the register value with the new bitfield value set
*/
#define REG32_SET_FIELD(d,r,f,v,reg)        BIT32_SET_VALUE(d##r##f,v,reg)

/** REG32_SET_FIELD_IDX(d,r,f,v,i,reg)
 Returns the value of 'reg' with the new value 'v' set into a field at the position 'i'
 of an indexed "repeated" bitfield. e.g. given the register bitfield:
 [31..28|27..24|23..20|19..16|15..12|11..8|7..4|3..0]
 specifying index 'i' = 3 would return the value of 'reg' with bits[15..12] set to the value 'v'.

 @param d device definition
 @param r register definition
 @param f bitfield definition
 @param v The new value to set
 @param i The bitfield index to set
 @param reg The register value
 @returns the register value with the new indexed bitfield value set
 */
#define REG32_SET_FIELD_IDX(d,r,f,v,i,reg)  BIT32_SET_VALUE_IDX(d##r##f,v,i,reg)

/** REG32_SET_CONST(d,r,f,c,reg)
 Returns the value of 'reg' with the new bitfield constant set

 @param d device definition
 @param r register definition
 @param f bitfield definition
 @param c The new constant value to set
 @param reg The register value
 @returns the bitfield constant value
*/
#define REG32_SET_CONST(d,r,f,c,reg)        (BIT32_CLR_FIELD(d##r##f,reg)|BIT32_VALUE(d##r##f,d##r##f##c))

/** REG32_SET_CONST_IDX(d,r,f,c,i,reg)
 Returns the value of 'reg' with the new constant value 'c' set into a field at the position 'i'
 of an indexed "repeated" bitfield. e.g. given the register bitfield:
 [31..28|27..24|23..20|19..16|15..12|11..8|7..4|3..0]
 specifying index 'i' = 3 would return the value of 'reg' with bits[15..12] set to the constant value 'c'.

 @param d device definition
 @param r register definition
 @param f bitfield definition
 @param c The new constant value to set
 @param i The bitfield index to set
 @param reg The register value
 @returns the register value with the new indexed bitfield value set
 */
#define REG32_SET_CONST_IDX(d,r,f,c,i,reg)  (BIT32_CLR_FIELD_IDX(d##r##f,i,reg)|BIT32_VALUE_IDX(d##r##f,d##r##f##c, i))





/** REG32 READ/WRITE macros ******************************************************************
 This section of REG32 macros are used for reading and writing device registers
 The previous BIT32 and REG32 macros manipulated data in memory. These macros access the device
 and perform various bit manipulations.

 Some devices have several instances of their register-address-map repeated at an offset interval.
 The REG32_IDX_READ_* macros provide indexing to the desired instance. To do so the device register
 map definitions must provide a DEV_IDX(i) definition; where DEV is the device base address
 and DEV_IDX(i) expands to the device instance at index 'i'.

 Some devices have registers, or blocks of registers, which are repeated at an offset interval.
 The REG32_READ_IDX_* macros provide indexing the desired instance of the register or block.
 To do so the device register map definitions must provide a DEV_REG_IDX(i) definition;
 where DEV_REG expands to the device register base offset and DEV_REG_IDX(i) expands to the
 register offset at index 'i'.

 Some devices have registers with bitfields which are repeated at a fixed interval.
 The REG32_*_IDX macros provide the bitfield manipulation functions to access indexed bitfields
 within a register. If read-modify-write operations are needed then perform a REG32_READ, then
 perform any indexed bitfield operations in memory; and finally REG32_WRITE the newly formatted
 register data back to the device.

 To add all of the possible combinations of indexed DEV/REG/FIELD macros may become overwhelming.
 We can always add more as needed.
*/

/** REG32_WRITE(dev,reg,val)
 Writes the value 'val' to the 32-bit register 'reg' offset of device with the base address 'dev'
 @param dev device base address
 @param reg register offset
 @param val value to write
*/
#define REG32_WRITE(d,r,v)                do{ *(volatile uint32_t *)(((uint8_t *)(d))+(d##r)) = (v); }while(0)

/** REG32_WRITE_FIELD(dev,reg,field,val)
 Writes the value 'val' to a bit-field 'field' of a 32-bit register at offset 'reg' of a device with the base address 'dev'
 @param dev device base address
 @param reg register offset
 @param field hi:lo bitfield definition
 @param val value to write
*/
#define REG32_WRITE_FIELD(d,r,f,v)        do{ volatile uint32_t *a = (volatile uint32_t *)(((uint8_t *)(d))+(d##r)); *a = BIT32_SET_VALUE(d##r##f, v, *a); }while(0)

/** REG32_WRITE_CONST(dev,reg,field,c)
 Writes the constant definition value 'c' to a bit-field 'field' of a 32-bit register at offset 'reg' of a device with the base address 'dev'
 @param dev device base address
 @param reg register offset
 @param field hi:lo bitfield definition
 @param c constant definition value to write
*/
#define REG32_WRITE_CONST(d,r,f,c)        do{ volatile uint32_t *a = (volatile uint32_t *)(((uint8_t *)(d))+(d##r)); *a = BIT32_SET_VALUE(d##r##f, d##r##f##c, *a); }while(0)

/** REG32_WRITE_IDX(d,r,ri,v)
 Writes the value 'v' to the 32-bit indexed register 'r' at index 'ri'
 of an indexed register.
 e.g. given a device with an array of  32-bit registers REG2 at offset 0x0080:
 DEVICE               0x10000000
 DEVICE_REG0          0x0000
 DEVICE_REG1          0x0040
 DEVICE_REG2          0x0080
 DEVICE_REG2_IDX(ri) (0x0080 + ((ri)*4))
 DEVICE_REG3          0x0100
 speciying 'r' = REG2; 'ri' = 1 would write the value 'v' to the address 0x100000c0
 which is REG2[1]

 @param d device base address
 @param r register offset
 @param ri register index
 @param v value to write
*/
#define REG32_WRITE_IDX(d,r,ri,v)         do{ *((volatile uint32_t *)(((uint8_t *)(d))+(d##r##_IDX(ri)))) = (v); }while(0)

/** REG32_WRITE_IDX_FIELD
 Writes a bitfield in an indexed register
 Used when a device has an array of registers - each with the same bitfields
 @param d device base address
 @param r register offset
 @param f bitfield hi:lo definition
 @param ri register index
 @param v value to write
*/
#define REG32_WRITE_IDX_FIELD(d,r,f,ri,v)        do{ volatile uint32_t *a = ((volatile uint32_t *)(((uint8_t *)(d))+(d##r##_IDX(ri)))); *a = BIT32_SET_VALUE(d##r##f, v, *a); }while(0)

/** REG32_WRITE_IDX_FIELD_IDX(d,r,f,ri,fi,v)
 Writes the value 'v' to the register 'r' field 'f' and field index 'fi' of a register field array
 @param d device base address
 @param r register offset
 @param f bitfield hi:lo definition
 @param fi the bitfield array index
 @param ri register index
 @param v value to write
 */
#define REG32_WRITE_IDX_FIELD_IDX(d,r,f,ri,fi,v) do{ volatile uint32_t *a = ((volatile uint32_t *)(((uint8_t *)(d))+(d##r##_IDX(ri)))); *a = BIT32_SET_VALUE(d##r##f##_IDX(fi), v, *a); }while(0)


/** REG32_IDX_WRITE(d,di,r,v)
 Writes the value 'v' to the 32-bit register 'r' offset of an indexed "repeated" device
 array with the base address 'd' at device index 'di'
 e.g. given four instances of a device with 16 x 32-bit registers:
                 [31..0][31..0][31..0][31..0]
 0x1000 device0: [reg 3][reg 2][reg 1][reg 0]
 0x1040 device1: [reg 3][reg 2][reg 1][reg 0]
 0x1080 device2: [reg 3][reg 2][reg 1][reg 0]
 0x10c0 device3: [reg 3][reg 2][reg 1][reg 0]

 @param d device base address of the device array
 @param di device instance
 @param r register offset
 @param v value to write
*/
#define REG32_IDX_WRITE(d,di,r,v)         do{ *(volatile uint32_t *)(((uint8_t *)(d##_IDX(di)))+(d##r)) = (v); }while(0)
#define REG32_IDX_WRITE_FIELD(d,di,r,f,v) do{ volatile uint32_t *a = (volatile uint32_t *)(((uint8_t *)(d##_IDX(di)))+(d##r)); *a = BIT32_SET_VALUE(d##r##f, v, *a); }while(0)
#define REG32_IDX_WRITE_CONST(d,di,r,f,c) do{ volatile uint32_t *a = (volatile uint32_t *)(((uint8_t *)(d##_IDX(di)))+(d##r)); *a = BIT32_SET_VALUE(d##r##f, d##r##f##c, *a); }while(0)

/** REG32_IDX_WRITE_IDX(d,di,r,ri,v)
 Writes the value 'v' to the device 'd' at instance 'di' and register 'r' at index 'ri'
 This is a combination of REG32_WRITE_IDX for an array of devices and REG32_IDX_WRITE for a register array within each device.

 @param d device base address of the first device instance
 @param di device instance
 @param r register offset
 @param ri register index
 @param v value to write
*/
#define REG32_IDX_WRITE_IDX(d,di,r,ri,v)  do{ *((volatile uint32_t *)(((uint8_t *)(d##_IDX(di)))+(d##r##_IDX(ri)))) = (v); }while(0)

/** REG32_READ(dev,reg)
 Reads the value of the 32-bit register 'reg' offset of device with the base address 'dev'

 @param dev device base address
 @param reg register offset
*/
#define REG32_READ(d,r)                   (*(volatile uint32_t *)(((uint8_t *)(d))+(d##r)))

/** REG32_READ_FIELD(dev,reg,field)
 Reads the value of a bit-field 'field' in a 32-bit register at offset 'reg' of a device with the base address 'dev'

 @param dev device base address
 @param reg register offset
*/
#define REG32_READ_FIELD(d,r,f)           BIT32_GET_FIELD(d##r##f, (*(volatile uint32_t *)(((uint8_t *)(d))+(d##r))) )

/** REG32_READ_IDX(d,r,ri)
 Reads a register 'r' from device 'd' at register index 'ri'

 @param d device base address
 @param r register offset
 @param ri register index
 */
#define REG32_READ_IDX(d,r,ri)            (*(volatile uint32_t *)(((uint8_t *)(d))+(d##r##_IDX(ri))))

/** REG32_READ_IDX_FIELD(d,r,f,ri)
 Reads the field 'f' of a register 'r' from device 'd' at register index 'ri'
 Use this macro when a device has an array of registers with bitfield hi:lo definitions

 @param d device base address
 @param r register offset
 @param f register field
 @param ri register index
 */
#define REG32_READ_IDX_FIELD(d,r,f,ri)        BIT32_GET_FIELD(d##r##f,           *((volatile uint32_t *)(((uint8_t *)(d))+(d##r##_IDX(ri)))))

/** REG32_READ_IDX_FIELD_IDX(d,r,f,ri,fi)
 Reads the field 'f' of a register 'r' from device 'd' at register index 'ri' and field index 'fi
 Use this macro when a device has an array of registers with a bitfield array hi:lo definition

 @param d device base address
 @param r register offset
 @param f register field
 @param ri register index
 */
#define REG32_READ_IDX_FIELD_IDX(d,r,f,ri,fi) BIT32_GET_VALUE(d##r##f##_IDX(fi), *((volatile uint32_t *)(((uint8_t *)(d))+(d##r##_IDX(ri)))))

/** REG32_IDX_READ(d,di,r)
 Reads a register 'r' from a device array with a base address 'd' and device instance 'di'

 @param d device base address
 @param di device instance
 @param r register offset
 */
#define REG32_IDX_READ(d,di,r)            (*(volatile uint32_t *)(((uint8_t *)(d##_IDX(di)))+(d##r)))

/** REG32_IDX_READ_FIELD(d,di,r,f)
 Reads the field 'f' of a register 'r' from device 'd' array at instance 'di'

 @param d device base address
 @param di device instance
 @param r register offset
 @param f register field
 */
#define REG32_IDX_READ_FIELD(d,di,r,f)    BIT32_GET_FIELD(d##r##f, (*(volatile uint32_t *)(((uint8_t *)(d##_IDX(di)))+(d##r))) )

/** REG32_IDX_READ_IDX(d,di,r,ri)
 Reads the value of the device 'd' at instance 'di' and register 'r' at index 'ri'
 This is a combination of REG32_READ_IDX for an array of devices and REG32_IDX_READ for a register array within each device.

 @param d device base address of the first device instance
 @param di device instance
 @param r register offset
 @param ri register index
 @return the right-shifted value of the register
*/
#define REG32_IDX_READ_IDX(d,di,r,ri)     (*(volatile uint32_t *)(((uint8_t *)(d##_IDX(di)))+(d##r##_IDX(ri))))




/** REG16 READ/WRITE macros ******************************************************************
 This section of REG16 macros are used for reading and writing 16-bit device registers.
 They perform identitical functions as the REG32 macros but use 16-bit data accessors and
 can access registers aligned to 16-bit boundaries - without throwing an alignment exception.
*/

/** REG16_WRITE(dev,reg,val)
 Writes the value 'val' to the 16-bit register 'reg' offset of device with the base address 'dev'
 @param dev device base address
 @param reg register offset
 @param val value to write
*/
#define REG16_WRITE(d,r,v)                do{ *(volatile uint16_t *)(((uint8_t *)(d))+(d##r)) = (v); }while(0)
#define REG16_IDX_WRITE(d,di,r,v)         do{ *(volatile uint16_t *)(((uint8_t *)(d##_IDX(di)))+(d##r)) = (v); }while(0)

#define REG16_WRITE_IDX(d,r,ri,v)         do{ *((volatile uint16_t *)(((uint8_t *)(d))+(d##r##_IDX(ri)))) = (v); }while(0)
#define REG16_IDX_WRITE_IDX(d,di,r,ri,v)  do{ *((volatile uint16_t *)(((uint8_t *)(d##_IDX(di)))+(d##r##_IDX(ri)))) = (v); }while(0)

/** REG16_READ(dev,reg)
 Reads the value of the 16-bit register 'reg' offset of device with the base address 'dev'
 @param dev device base address
 @param reg register offset
*/
#define REG16_READ(d,r)                   (*(volatile uint16_t *)(((uint8_t *)(d))+(d##r)))
#define REG16_IDX_READ(d,di,r)            (*(volatile uint16_t *)(((uint8_t *)(d##_IDX(di)))+(d##r)))

#define REG16_READ_IDX(d,r,ri)            (*(volatile uint16_t *)(((uint8_t *)(d))+(d##r##_IDX(ri))))
#define REG16_IDX_READ_IDX(d,di,r,ri)     (*(volatile uint16_t *)(((uint8_t *)(d##_IDX(di)))+(d##r##_IDX(ri))))

#define REG16_IDX_READ_BLOCK_IDX(d,di,r,bi,ri)     (*(volatile uint16_t *)(((uint8_t *)(d##_IDX(di)))+(d##r##_IDX(bi)+(ri))))

/** REG16_WRITE_FIELD(dev,reg,field,val)
 Writes the value 'val' to a bit-field 'field' of a 16-bit register at offset 'reg' of a device with the base address 'dev'
 @param dev device base address
 @param reg register offset
 @param field hi:lo bitfield definition
 @param val value to write
*/
#define REG16_WRITE_FIELD(d,r,f,v)        do{ volatile uint16_t *a = (volatile uint16_t *)(((uint8_t *)(d))+(d##r)); *a = BIT32_SET_VALUE(d##r##f, v, *a); }while(0)
#define REG16_IDX_WRITE_FIELD(d,di,r,f,v) do{ volatile uint16_t *a = (volatile uint16_t *)(((uint8_t *)(d##_IDX(di)))+(d##r)); *a = BIT32_SET_VALUE(d##r##f, v, *a); }while(0)
#define REG16_IDX_WRITE_FIELD_IDX(d,di,r,f,fi,v) do{ volatile uint16_t *a = (volatile uint16_t *)(((uint8_t *)(d##_IDX(di)))+(d##r)); *a = BIT32_SET_VALUE(d##r##f##_IDX(fi), v, *a); }while(0)
#define REG16_IDX_WRITE_FIELD_IDX_CONST(d,di,r,f,c,fi) do{ volatile uint16_t *a = (volatile uint16_t *)(((uint8_t *)(d##_IDX(di)))+(d##r)); *a = BIT32_SET_VALUE(d##r##f##_IDX(fi), d##r##f##c, *a); }while(0)

/** REG16_READ_FIELD(dev,reg,field)
 Reads the value of a bit-field 'field' in a 16-bit register at offset 'reg' of a device with the base address 'dev'
 @param dev device base address
 @param reg register offset
*/
#define REG16_READ_FIELD(d,r,f)           BIT32_GET_FIELD(d##r##f, (*(volatile uint16_t *)(((uint8_t *)(d))+(d##r))) )
#define REG16_IDX_READ_FIELD(d,di,r,f)    BIT32_GET_FIELD(d##r##f, (*(volatile uint16_t *)(((uint8_t *)(d##_IDX(di)))+(d##r))) )

/** REG16_WRITE_CONST(dev,reg,field,c)
 Writes the constant definition value 'c' to a bit-field 'field' of a 16-bit register at offset 'reg' of a device with the base address 'dev'
 @param dev device base address
 @param reg register offset
 @param field hi:lo bitfield definition
 @param c constant definition value to write
*/
#define REG16_WRITE_CONST(d,r,f,c)        do{ volatile uint16_t *a = (volatile uint16_t *)(((uint8_t *)(d))+(d##r)); *a = BIT32_SET_VALUE(d##r##f, d##r##f##c, *a); }while(0)
#define REG16_IDX_WRITE_CONST(d,di,r,f,c) do{ volatile uint16_t *a = (volatile uint16_t *)(((uint8_t *)(d##_IDX(di)))+(d##r)); *a = BIT32_SET_VALUE(d##r##f, d##r##f##c, *a); }while(0)

#endif // ddi_macros_h

