/*****************************************************************************
 * (c) Copyright 2020 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
/*  ddi_ntime.h
 *  Created by Johana Lehrer on 2020-01-30
 */

#ifndef DDI_NTIME_H
#define DDI_NTIME_H

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SEC_PER_MIN   (60L)
#define SEC_PER_HR    (60L * SEC_PER_MIN)
#define SEC_PER_DAY   (24L * SEC_PER_HR)
#define MSEC_PER_SEC  1000L
#define USEC_PER_MSEC 1000L
#define NSEC_PER_USEC 1000L
#define NSEC_PER_MSEC 1000000L
#define USEC_PER_SEC  1000000L
#define NSEC_PER_SEC  1000000000L

#ifdef WIN32
#define CLOCK_MONOTONIC 1
#define TIMER_ABSTIME   1
#endif

#define NTIME_CLOCK_ID    CLOCK_MONOTONIC

//! Time structure in nanoseconds
typedef struct
{
  uint32_t sec;  /**< seconds up to 136.103027100141155 years */
  uint32_t ns;   /**< nanoseconds 0 - 999999999 **/
} ntime_t;

// System uptime clock turnover epochs: can be used to for ddi_ntime_set()
// ms counter turnover times    d   h   m   s   ms
#define UTIME_MS_MAX_SIGNED    24854, 23, 59, 59, 999999999  // time when ms counter reaches 0x7FFFFFFF
#define UTIME_MS_MAX_UNSIGNED  49709, 23, 59, 59, 999999999  // time when ms counter reaches 0xFFFFFFFF

void ddi_ntime_get_systime(ntime_t *t);

uint64_t ddi_ntime_sleep_ns(ntime_t *ntime);

/** ddi_ntime_set
 Sets a ntime_t struct to an absolute time value

 @param day Days since startup (0 - 49710)
 @param hr Hours since startup (0 - 23)
 @param m Minutes since startup (0 - 59)
 @param s Seconds since startup (0 - 59)
 @param ns Nanoseconds since startup (0 - 999999999)
*/
void ddi_ntime_set(ntime_t *tm, uint32_t day, uint32_t hr, uint32_t m, uint32_t s, uint32_t ns);

/** ddi_ntime_diff_ns

 Calculates the difference in nanoseconds between two ntime_t values.

 This function returns a signed value so that t0 can be >= t1 or t0 can be <= t1

 @param t0 the lesser utime_t value
 @param t1 the greater utime_t value
 @returns the difference between t0 and t1 in nanoseconds.
*/
int64_t ddi_ntime_diff_ns(ntime_t *t0, ntime_t *t1);

/** ddi_ntime_add_ns
 Adds a number of nanoseconds to the ntime_t value.

 @param t0 the ntime_t value which is incremented by the sec & ns param.
 @param sec the number of seconds to add.
 @param ns the number of nanoseconds to add.
*/
void ddi_ntime_add_ns(ntime_t *t0, uint32_t sec, uint64_t ns);

/** ddi_ntime_sub_ns
 Subtracts a number of nanoseconds from the ntime_t value.

 @param t0 the ntime_t value which is decremented by the sec & ns param.
 @param sec the number of seconds to subtract.
 @param ns the number of nanoseconds to subtract.
*/
void ddi_ntime_sub_ns(ntime_t *t0, uint32_t sec, uint64_t ns);

/** ddi_ntime_print
 Pretty formatted sprintf
*/
size_t ddi_ntime_print(char *buf, ntime_t *tm);

/** ddi_ntime_to_time
 Returns the ntime value as a double precision floating point.
 The value returned is in seconds. (e.g. 1.000000501 seconds)
 */
double ddi_ntime_to_time(ntime_t *ntime);

#ifdef __cplusplus
}
#endif

#endif /* DDI_NTIME_H */

