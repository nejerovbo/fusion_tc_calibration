/*****************************************************************************
 * (c) Copyright 2020 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
/*  ddi_mem_utils.h
 *  Created by Johana Lehrer on 2020-01-18
 */

#ifndef _DDI_MEM_UTILS_H
#define _DDI_MEM_UTILS_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/** zalloc
 @brief Allocates zero-filled memory.
 */
void *zalloc(size_t size);

/** blockzero
 @brief Zero-fills a block of memory, using the specified 8-bit, 16-bit or 32-bit width access operations.
 @param mem The base-address of the memory to zero-fill
 @param len The length in bytes of the memory.
 @param width Specifies 1 = 8-bit, 2 = 16-bit or 4 = 32-bit width access operations
 */
void blockzero(void *mem, int len, int width);

/** blockcopy
 @brief Copies a block of memory, using the specified 8-bit, 16-bit or 32-bit width access operations.
 @param dst The base-address of the destination memory
 @param src The base-address of the source memory
 @param len The length in bytes of the memory.
 @param width Specifies 1 = 8-bit, 2 = 16-bit or 4 = 32-bit width access operations
 */
void blockcopy(void *dst, void *src, int len, int width);

/** block_endian_swap
 @brief Performs a 16-bit or 32-bit in-place endian swap
 This function performs a 16-bit or 32-bit endian swap based on the size of the width argument
 Width 1 = No operation
 Width 2 = 16-bit endian conversion is performed
 Width 4 = 32-bit endian conversion is performed
 To date, the 32-bit conversion routine from little-endian to big-endian has been tested.
 @param src The base-address of the source memory
 @param len The length in bytes of the memory.
 @param width Specifies 1 = 8-bit, 2 = 16-bit or 4 = 32-bit width access operations
 */
void block_endian_swap(void *src, uint32_t offset, int len, int width);

/** readfile
 @brief Reads a file into memory. The memory is allocated to hold the file data.
 Convience function for "fopen,seek,rewind,malloc,fread,fclose"
 @param filename The file to read into memory
 @param data Pointer to a void * which is set to the newly allocated memory containing the file contents.
 @return The size of the file and the allocated memory in bytes.
 */
size_t readfile(const char *filename, void **data);

/** readfile_offset
 @brief Reads a file into memory with a given offset. The memory is allocated to hold the file data.
 Convience function for "fopen,seek,rewind,malloc,fread,fclose" with an offset into the file data.
 @param filename The file to read into memory
 @oaram offset The offset into the file to start reading from
 @param data Pointer to a void * which is set to the newly allocated memory containing the file contents.
 @return The size of the file and the allocated memory in bytes.
 */
size_t readfile_offset(const char *filename, int offset, void **pdata);

/** writefile
 @brief Writes memory to a file.
 Convience function for "fopen,fwrite,fclose"
 @param filename The file to write.
 @param data The address of the memory to write.
 @param len The size of the memory to write to the file.
 @return The number of bytes written.
 */
size_t writefile(const char *filename, const void *data, size_t len);

/** write_mem_to_file
 @brief Writes memory to a file, using the specified 8-bit, 16-bit or 32-bit width access operations.
 The memory is first blockcopied to the data buffer using the width access operations. Then the buffered
 data is written to the file.
 @param filename The file to write.
 @param mem The base-address of the memory to write to the file.
 @param data A user allocated buffer which is used to blockcopy the memory using the specified width access operations.
 @param len The size of the memory to write to the file.
 @param width Specifies 1 = 8-bit, 2 = 16-bit or 4 = 32-bit width access operations
 @return The number of bytes written.
 */
int write_mem_to_file(const char *filename, uint8_t *mem, uint8_t *data, uint32_t len, uint32_t width);

/** read_file_to_mem
 @brief Reads a file into memory, using the specified 8-bit, 16-bit or 32-bit width access operations.
 The file is first read into the 'data' buffer then it is blockcopied to the memory using the width access operations.
 If the file length is less then 'len' then the remaining memory is unchanged. If the file length exceeds len
 then the memory will contain the truncated file contents.
 @param filename The file to read.
 @param mem The base-address of the memory which receives the read file contents.
 @param data A user allocated buffer which is used to blockcopy the memory using the specified width access operations.
 @param len The size of the memory to read.
 @param width Specifies 1 = 8-bit, 2 = 16-bit or 4 = 32-bit width access operations
 @return The number of bytes read.
 */
int read_file_to_mem(const char *filename, uint8_t *mem, uint8_t *data, uint32_t len, uint32_t width);



#ifdef __cplusplus
}
#endif

#endif // _DDI_MEM_UTILS_H

