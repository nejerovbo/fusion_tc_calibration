/*****************************************************************************
 * (c) Copyright 2020 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
/*  ddi_seq_utils.c
 *  Created by Johana Lehrer on 2020-01-30
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
//#include <math.h>   // compile with -lm
#include <limits.h>
#include <inttypes.h>
#include "ddi_seq_utils.h"
#include "ddi_defines.h"
#include "ddi_ntime.h"

#define DEBUG
#include "ddi_debug.h"

double seq_get_event_time(seq_event_t *key)
{
  return ddi_ntime_to_time(&key->ntime);
}

seq_event_t *seq_get_next_event(seq_t *seq, uint32_t *ptrack_idx)
{
  uint32_t tn, next_tn;
  seq_track_t *track;
  seq_event_t *next, *event;

  next = 0;
  next_tn = 0;
  track = seq->track;

  for (tn = 0; tn < seq->track_count; tn++)
  {
    if (track->idx < track->event_count)
    {
      event = &track->event[track->idx];
      if (!next || (ddi_ntime_diff_ns(&next->ntime, &event->ntime) > 0))
      {
        next = event;
        next_tn = tn;
      }
    }
    track++;
  }

  *ptrack_idx = next_tn;
  return next;
}

seq_event_t *seq_find_event(seq_event_t *key, seq_event_t *list, uint32_t imax)
{ 
  uint32_t imin, imid;
  double dt, tgt, min, mid, max;
  seq_event_t *event;
  
  if (imax <= 1)
    return list;
  
  imin = 0;
  imax--;
  tgt = seq_get_event_time(key);
  
  for (;;)
  { 
    imid = imin + ((imax - imin) / 2);
    min = seq_get_event_time(&list[imin]);
    max = seq_get_event_time(&list[imax]);
    mid = seq_get_event_time(&list[imid]);
    dt = mid - tgt;

    VLOG("min[%u]:%1.9f max[%u]:%1.9f : mid[%u]:%1.9f %c tgt:%1.9f : dt:%1.9f\n",
      imin, min, imax, max, imid, mid, ((dt == 0) ? '=' : (dt < 0) ? '<' : '>'), tgt, dt);

    if ((dt == 0) || ((imax - imin) <= 1))
    { 
      if (dt < -0.5)
        event = &list[imax];
      else
        event = &list[imid];
      break;
    }
    else if (dt > 0)
    { 
      imax = imid;
    }
    else
    { 
      imin = imid;
    }
  }

  return event;
}

// seq t0        t1     t2     t3     t4
//      |      '  |      |      |      |
//      +------'--+------+------+------+--> t
//             '
//             |<- seq position
// Tnow ------>|    (Tnow: cur systime)
// Tseq |----->|    (Tseq: cur seq ntime: relative to t0)
//      |<-----|    (T0 = Tnow - Tseq: systime at t0)
//
//      |-------->| (Tevent: time of event: relative to t0)
//
//          -->|  |<-- dt: Tevent - Tseq

int seq_proc(seq_t *seq, double tstop)
{
  int status = OK;
  uint64_t sec, nsec;
  uint32_t tn;
  int64_t dt;
  ntime_t T0, Tseq, Tevent;
  seq_event_t *event;
  struct timespec deadline, remaining;

  Tseq = seq->ntime;
  ddi_ntime_get_systime(&T0);
  ddi_ntime_sub_ns(&T0, Tseq.sec, Tseq.ns);

  event = seq_get_next_event(seq, &tn);
  if (!event)
    return NOT_OK;
  Tevent = event->ntime;

  for (;;)
  {
    dt = ddi_ntime_diff_ns(&Tseq, &Tevent);
    if (dt >= 0)
    {
      printf("DT:%"PRId64" late\n", dt);
      seq->track[tn].func(seq, tn, event);
      seq->track[tn].idx++;

      event = seq_get_next_event(seq, &tn);
      if (!event || (seq_get_event_time(event) > tstop))
      {
        printf("done 1\n");
        break;
      }

      Tevent = event->ntime;
      continue;
    }

// The following code block is experimental code to optimize CPU usage during long intervals to the next deadline event time
// by sleeping the sequence thread. The intent here is to predict when to schedule to wake up so that if the OS scheduler
// can wake up consistantly within, e.g. 22 milliseconds, then we should be safe to sleep for longer durations as long as we
// schedule to wake up before 22 msec and then spin until the exact event time.

// Tseq                                  Tevent
// |                                     |
// |  okay to sleep     |   |            |
// |  until here: ----->|   |            |
// |                    |   |<-- 22ms ---| wake_latency : worst-case wake time
// |    scheduler       |   |            |
// |    uncertainty: -->|   |<--         |
// |                    |                |
// |<- calc Tdeadline ->|                |
// |                    |                |
// |  sleep for periods |    spinloop    |
// |  ( dt > 22ms )     |  until Tevent  |
// |                    |                |
// |<------------------>|<-------------->|
// |                                     |
// |---------------- dt ---------------->| dt : positive delta time in nsec from Tseq to Tevent

#if 1
    dt = -dt;
    //printf("DT:%ld early\n", dt);
    if (dt > (seq->wake_latency + seq->scheduler_uncertainty))
    {
      dt -= seq->wake_latency;
      sec = dt / NSEC_PER_SEC;
      nsec = dt % NSEC_PER_SEC;
      deadline.tv_sec = sec;
      deadline.tv_nsec = nsec;
      remaining.tv_sec = 0;
      remaining.tv_nsec = 0;

      //printf("sleep:%ld sec %ld ns\n", deadline.tv_sec, deadline.tv_nsec);
      status = clock_nanosleep(NTIME_CLOCK_ID, 0/*TIMER_ABSTIME*/, &deadline, &remaining);
      if (status != 0)
        break;

      if (remaining.tv_nsec || remaining.tv_sec)
      {
        VLOG("remaining:%"PRId64".%09ld\n", remaining.tv_sec, remaining.tv_nsec);
      }
      //printf("status:%d\n", status);
    }
#endif
    ddi_ntime_get_systime(&Tseq);
    //printf("now:%d %d T0:%d %d\n", Tseq.sec, Tseq.ns, T0.sec, T0.ns);
    ddi_ntime_sub_ns(&Tseq, T0.sec, T0.ns);
    //printf("sub\n");
    seq->ntime = Tseq;
  }
  return status;
}

int seq_run(seq_t *seq)
{
  uint32_t i;
  double max, tevt;
  seq_track_t *track;
  seq_event_t *event;

  max = 0.0;
  track = seq->track;
  for (i = 0; i < seq->track_count; i++)
  {
    event = &track->event[track->event_count - 1];
    tevt = seq_get_event_time(event);
    if (tevt > max)
      max = tevt;
    track++;
  }
  return seq_proc(seq, max);
}

int seq_stop(seq_t *seq)
{
  return OK;
}

int seq_run_to_index(seq_t *seq, uint32_t index)
{
/*
  uint32_t i, imax, ievt;
  double max;

  track = seq->track;
  for (i = 0; i < seq->track_count; i++)
  {
    event = &track->event[track->event_count - 1];
    tevt = seq_get_event_time(event);
    if (tevt > max)
      max = tevt;
    track++;
  }
  return seq_proc(seq, max);
*/
  return OK;
}

int seq_run_to_ntime(seq_t *seq, ntime_t *ntime)
{
  return seq_proc(seq, ddi_ntime_to_time(ntime));
}

int seq_set_index(seq_t *seq, uint32_t index)
{
  uint32_t i;
  seq_track_t *track;

  track = seq->track;
  for (i = 0; i < seq->track_count; i++)
  {
    if (track->event_count > index) {
      track->idx = index;
    } else {
      track->idx = track->event_count - 1;
    }
    track++;
  }
  return OK;
}

int seq_set_ntime(seq_t *seq, ntime_t *ntime)
{

  return OK;
}

