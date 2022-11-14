/*****************************************************************************
 * (c) Copyright 2020 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
/*  ddi_ntime.c
 *  Created by Johana Lehrer on 2020-01-30
 */

#include <stdio.h>
#include "ddi_ntime.h"

#ifdef WIN32
#define NTIME_EPOCH 0//1601lu
#else
#define NTIME_EPOCH 0//1970lu
#endif

#ifdef WIN32
#include <Windows.h>

LARGE_INTEGER getFILETIMEoffset()
{
  SYSTEMTIME s;
  FILETIME f;
  LARGE_INTEGER t;

  s.wYear = 1970;
  s.wMonth = 1;
  s.wDay = 1;
  s.wHour = 0;
  s.wMinute = 0;
  s.wSecond = 0;
  s.wMilliseconds = 0;

  SystemTimeToFileTime(&s, &f);

  t.QuadPart = f.dwHighDateTime;
  t.QuadPart <<= 32;
  t.QuadPart |= f.dwLowDateTime;

  return t;
}

/** clock_gettime
 implementation from:
 https://stackoverflow.com/questions/5404277/porting-clock-gettime-to-windows
*/
int clock_gettime(int X, struct timespec *tv)
{
  LARGE_INTEGER           now;
  static LARGE_INTEGER    START_TIME;
  static int              initialized = 0;
  static BOOL             USE_PERFORMANCE_COUNTER = 0;
  static LARGE_INTEGER    PERFORMANCE_FREQUENCY;
  uint64_t time_ns;

  if (!initialized)
  {
    // initialize START_TIME and FREQUENCY_TO_MICROSECONDS
    initialized = 1;
    USE_PERFORMANCE_COUNTER = QueryPerformanceFrequency(&PERFORMANCE_FREQUENCY);
    if (USE_PERFORMANCE_COUNTER)
    {
      QueryPerformanceCounter(&START_TIME);
    }
    else
    {
      START_TIME.QuadPart = 0;//((LONGLONG)SYSTEM_TIME_UTC.dwHighDateTime << 32) | SYSTEM_TIME_UTC.dwLowDateTime;
//      START_TIME = getFILETIMEoffset();
//      FREQUENCY_TO_MICROSECONDS = 10.0;
    }
  }

  if (USE_PERFORMANCE_COUNTER)
  {
    QueryPerformanceCounter(&now);
    time_ns = now.QuadPart - START_TIME.QuadPart;
    time_ns *= 1000000llu;
    time_ns /= PERFORMANCE_FREQUENCY.QuadPart;
  }
  else
  {
    FILETIME f;
    GetSystemTimePreciseAsFileTime(&f);
    now.QuadPart = 0;//((LONGLONG)f.dwHighDateTime << 32) | f.dwLowDateTime;
    //now.QuadPart *= 100000lu; // 100ns to nsec
  }

  tv->tv_sec = (time_t)time_ns / 1000000lu;
  tv->tv_nsec = (time_ns % 1000000lu) * 1000;
//  ddi_ntime_add_ns(&t, tv->tv_sec + year_in_sec, tv->tv_nsec);
//  tv->tv_sec = t.sec;
//  tv->tv_nsec = t.ns;

  return 0;
}

/** clock_nanosleep
  If flags is 0, then the value specified in request is interpreted as an interval
  relative to the current value of the clock specified by clock_id.

  If flags is TIMER_ABSTIME, then request is interpreted as an absolute time
  as measured by the clock, clock_id.
*/
int clock_nanosleep(int clock_id, int flags, const struct timespec *request, struct timespec *remain)
{
  LONGLONG ns;
  HANDLE timer;
  LARGE_INTEGER li;

  if (!(timer = CreateWaitableTimer(NULL, TRUE, NULL)))
    return -1;

  // SetWaitableTimer resolution is in 100 nanosecond units
  ns = request->tv_nsec + (request->tv_sec * 1000000000) / 100;

  // Positive values indicate absolute time
  // Negative values indicate relative time
  if (flags == 0)
    ns = -ns;

  li.QuadPart = ns;
  if (!SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE))
  {
    CloseHandle(timer);
    return -1;
  }

  WaitForSingleObject(timer, INFINITE);
  CloseHandle(timer);

  // Slept without problems
  return 0;
}
#endif

void ddi_ntime_get_systime(ntime_t *t)
{
  struct timespec ts;

  clock_gettime(NTIME_CLOCK_ID, &ts);
  t->sec = (uint32_t)(ts.tv_sec);
  t->ns = ts.tv_nsec;
}

double ddi_ntime_to_time(ntime_t *ntime)
{
  return ((double)ntime->sec) + (((double)ntime->ns) / 1000000000.0);
}

uint64_t ddi_ntime_sleep_ns(ntime_t *ntime)
{
  uint64_t rem = 0;
  struct timespec deadline, remaining;
  deadline.tv_sec = ntime->sec;
  deadline.tv_nsec = ntime->ns;
  if (clock_nanosleep(NTIME_CLOCK_ID, TIMER_ABSTIME, &deadline, &remaining) != 0)
    rem = (NSEC_PER_SEC * remaining.tv_sec) + remaining.tv_nsec;
  return rem;
}

void ddi_ntime_set(ntime_t *tm, uint32_t day, uint32_t hr, uint32_t m, uint32_t s, uint32_t ns)
{
  tm->sec = (SEC_PER_DAY * day) + (SEC_PER_HR * hr) + (SEC_PER_MIN * m) + s;
  tm->ns = ns;
}

int64_t ddi_ntime_diff_ns(ntime_t *t0, ntime_t *t1)
{
  int64_t dt_sec, dt_ns, dt;
  dt_sec = ((int64_t)t0->sec) - t1->sec;
  if (dt_sec == 0)
  {
    // |---------|--'XXXX'--|------>
    //              t0   t1
    //              |<----|   negative (t0 is earlier than t1)
    //
    //              t1   t0
    //              |---->|   positive (t0 is later than t1)

    dt_ns = ((int64_t)t0->ns) - t1->ns;
    return dt_ns;
  }

  if (dt_sec < 0) // negative (t0 is earlier than t1)
  {
    // |-----'XXXX|XXXX'----|
    //       t0       t1
    //       |<--------|

    dt_ns = ((NSEC_PER_SEC - (int64_t)t0->ns) + t1->ns);
    dt = ((dt_sec + 1) * NSEC_PER_SEC) - dt_ns;
  }
  else // positive (t0 is later than t1)
  {
    // |-----'XXXX|XXXX'----|
    //       t1       t0
    //       |-------->|

    dt_ns = ((NSEC_PER_SEC - (int64_t)t1->ns) + t0->ns);
    dt = ((dt_sec - 1) * NSEC_PER_SEC) + dt_ns;
  }
  return dt;
}

void ddi_ntime_add_ns(ntime_t *t0, uint32_t sec, uint64_t ns)
{
  int64_t nsec;

  t0->sec += sec;
  nsec = (int64_t)ns + t0->ns;
  while (nsec >= NSEC_PER_SEC)
  {
    nsec -= NSEC_PER_SEC;
    t0->sec++;
  }
  t0->ns = (uint32_t)nsec;
}

void ddi_ntime_sub_ns(ntime_t *t0, uint32_t sec, uint64_t ns)
{
  int64_t nsec;

  // check if result would be negative: the minumum normalized time is 0
  if ((t0->sec < sec) || ((t0->sec == sec) && (t0->ns <= ns)))
  {
    t0->sec = 0;
    t0->ns = 0;
    return;
  }

  // this algorithm is from linux kernel timespec_sub and set_normalized_timespec
  sec = t0->sec - sec;
  nsec = (int64_t)t0->ns - (int64_t)ns;
  while (nsec >= NSEC_PER_SEC)
  {
    nsec -= NSEC_PER_SEC;
    ++sec; // carry
  }
  while (nsec < 0)
  {
    nsec += NSEC_PER_SEC;
    --sec; // borrow
  }
  t0->sec = sec;
  t0->ns = (uint32_t)nsec;
}

size_t ddi_ntime_print(char *buf, ntime_t *tm)
{
  uint32_t yr, day, hr, min, sec;
  sec = tm->sec;
  day =  (sec / SEC_PER_DAY) % SEC_PER_DAY;
  sec -= (SEC_PER_DAY * day);
  hr =   (sec / SEC_PER_HR) % SEC_PER_HR;
  sec -= (SEC_PER_HR * hr);
  min =  (sec / SEC_PER_MIN) % SEC_PER_MIN;
  sec -= (SEC_PER_MIN * min);
  yr = NTIME_EPOCH + (uint32_t)((double)day / 365.24l);
  day = (uint32_t)((double)day / 365.24l);

  return sprintf(buf, "%d %2d %02d:%02d:%02d.%09d", yr, day, hr, min, sec, tm->ns);
}

