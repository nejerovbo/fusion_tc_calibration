/*****************************************************************************
 * (c) Copyright 2020 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
/*  ddi_str_utils.h
 *  Created by Johana Lehrer on 2020-01-18
 */

#ifndef _DDI_STR_UTILS_H
#define _DDI_STR_UTILS_H

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** scan_to
 @brief Scans through string 'p' until the first instance of the character 'c' or NULL is found.
 @param p Pointer to a NULL terminated C-String to scan
 @param c The character to match
 @return A pointer to the first instance of 'c' or NULL within the string.
 */
char *scan_to(const char *p, char c);

/** skip_spaces
 @brief Scans through string 'p' until the first non-whitespace character is found.
 @param p Pointer to a NULL terminated C-String to scan
 @return A pointer to the next non-whitespace char or NULL within the string.
 */
char *skip_spaces(const char *p);

/** next_line
 @brief Scans through string 'p' until the first character of the next line is found.
 Multiple newlines and/or return characters at the end of the line are skipped.
 @param p Pointer to a NULL terminated C-String to scan
 @return A pointer to the next line or NULL within the string.
 */
char *next_line(const char *p);

/** scan_to_eol
 @brief Scan through string 'p' until the first instance of a newline '\n'; return '\r' or NULL is found.
 @param p Pointer to a NULL terminated C-String to scan
 @return A pointer to the first instance of '\n', '\r' or NULL within the string.
 */
char *scan_to_eol(const char *p);

/** atox
 @brief Converts an ascii character 'a' to its hexadecimal value.
 The character must be 'A-F', 'a-f' or '0-9' otherwise the result is undetermined.
 */
uint8_t atox(char a);

/** append_char
 @brief Appends the character 'c' to end of the C-string and stes the next character to a NULL termination.
 The memory containing the string 'str' must be long enough to add one character beyond the original (strlen + 1).
 The resulting string will be one byte longer.
 @param str Pointer to a NULL terminated C-String within a memory buffer at least one byte longer.
 @param c The character to append
 */
void append_char(char *str, char c);

/** basename
 @brief returns a pointer to the filename string without the leading path
 */
#ifndef __cplusplus
char *basename(const char *path);
#endif

/** suffix
 @brief returns a pointer to the dot suffix of a filename
 */
char *suffix(const char *filename);

/** vmemcpy
 @brief implements a volatile memcpy
 */
void vmemcpy (volatile void *dest, volatile void *src, int len );

/** vstrncpy
 @brief implements a volatile strncpy
 */
volatile char* vstrncpy(volatile char* dest, volatile const char* src, size_t num);

#ifdef __cplusplus
}
#endif

#endif // _DDI_STR_UTILS_H

