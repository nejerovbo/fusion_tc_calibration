/*****************************************************************************
 * (c) Copyright 2020-2021 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
/*  seq_test.c
 *  Created by Johana Lehrer on 2020-02-07
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ddi_ntime.h"
#include "ddi_defines.h"
#include "ddi_seq_utils.h"

int ddi_log_level = 3;
#define NSEC 1lu
#define USEC 1000lu
#define MSEC 1000000lu
#define MINS 60lu
#define HRS  360lu

void test_func(seq_t *seq, uint32_t event_index, seq_event_t *event);

seq_event_t track1_events[] =
{
  {{   0,    1 * NSEC},    "test1 started"},
  {{   1,  125 * USEC},    "run test 1"},
  {{   3,  500 * MSEC},    "run test 2"},
  {{   4,    0 * MSEC},    "bug 1"},
  {{   5,    0 * MSEC},    "bug 2"},
  {{   5,   (1 * MSEC) + (999 * USEC)},    "piig 1"},
  {{   5,   (2 * MSEC) + (  1 * USEC)},    "piig 2"},
  {{   5,   (2 * MSEC) + (  2 * USEC)},    "piig 3"},
  {{   6,    0 * MSEC},    "bug 3"},
  {{   7,    0 * MSEC},    "bug 4"},
  {{   8,    0 * MSEC},    "bug 5"},
  {{  10,  999 * USEC},    "run test 3"},
  {{  10,    1 * MSEC},    "run test 4"},
  {{  59,  100 * MSEC},    "fifty-nine seconds"},
  {{  60,  100 * USEC},    "sixty seconds"},
  {{ 359,  999 * MSEC},    "three-fifty-nine"},
  {{ 360,    0 * MSEC},    "three-sixty"},
  {{ 360,  100 * MSEC},    "run test 5"},
  {{3559,  999 * MSEC},    "run test 6"},
  {{3600,    0 * MSEC},    "run test 7"},
};

seq_event_t track2_events[] =
{
  {{   0,    0lu * NSEC},    "test2 started"},
  {{   0,  124lu * USEC},    "hi i'm a chuu"},
  {{   1,  500lu * MSEC},    "miku matsu 1"},
  {{   3,  500lu * MSEC},    "miku matsu 2"},
  {{   4,    0lu * MSEC},    "wurm 1"},
  {{   5,    0lu * MSEC},    "wurm 2"},
  {{   6,    0lu * MSEC},    "wurm 3"},
  {{   7,    0lu * MSEC},    "wurm 4"},
  {{   8,    0lu * MSEC},    "wurm 5"},
  {{  11,    0lu * MSEC},    "kutska 1"},
  {{  11,    1lu * MSEC},    "kutska 2"},
  {{  11,   10lu * MSEC},    "kutska 3"},
  {{  11,   30lu * MSEC},    "kutska 4"},
  {{  11,   55lu * MSEC},    "kutska 5"},
  {{  80,  500lu * MSEC},    "miku matsu 3"},
  {{3559,  500lu * MSEC},    "miku matsu 4"},
  {{3559,  998lu * MSEC},    "miku matsu 5"},
  {{3559,  999lu * MSEC},    "miku matsu 6"},
};

seq_track_t test_tracks[] =
{
  {0, ARRAY_ELEMENTS(track1_events), track1_events, test_func},
  {0, ARRAY_ELEMENTS(track2_events), track2_events, test_func},
};

void test_find(void)
{
  seq_event_t key, *event;
  char *cmd, tstr[512];

  key.ntime.sec = 6;
  //key.ntime.ns  = (501lu * MSEC);
  //key.ntime.ns  = (499lu * MSEC);
  key.ntime.ns  = ((500lu * MSEC) - (0lu * NSEC));
  key.data = "key";

  event = seq_find_event(&key, track2_events, ARRAY_ELEMENTS(track2_events));

  printf("result:%s\n", (!event) ? "ERR" : (char *)event->data);
  if (event)
  {
    cmd = (char *)event->data;
    ddi_ntime_print(tstr, &event->ntime);
    printf("%s %s\n", tstr, cmd);
  }
}

void test_func(seq_t *seq, uint32_t track_index, seq_event_t *event)
{
  char *cmd, tstr[512];

//  seq_get_systime(&Tnow);
//  ddi_ntime_sub_ns(&Tnow, T0.sec, T0.ns);
//  ddi_ntime_print(tstr, &Tnow);
//  printf("%s :", tstr); 

  cmd = (char *)event->data;
  ddi_ntime_print(tstr, &event->ntime);
  printf("[%d] %s %s\n", track_index, tstr, cmd);
}

void seq_test(void)
{
  seq_t seq;

  ddi_ntime_set(&seq.ntime, 0, 0, 0, 0, 0);
  seq.track_count = ARRAY_ELEMENTS(test_tracks);
  seq.track = test_tracks;

  seq_set_index(&seq, 0);

  if (seq_run(&seq) != OK)
  {
    printf("sequence did not complete\n");
  }
}

/** seq_iter_test
 Test seq_get_next_event iterates through all events
 */
int seq_iter_test(void)
{
  seq_t seq;
  uint32_t tn;
  seq_event_t *event;

  ddi_ntime_set(&seq.ntime, 0, 0, 0, 0, 0);
  seq.track_count = ARRAY_ELEMENTS(test_tracks);
  seq.track = test_tracks;

  for (;;)
  {
    event = seq_get_next_event(&seq, &tn);
    if (!event)
      break;

    seq.track[tn].func(&seq, tn, event);
    seq.track[tn].idx++;
  }
  printf("done\n");
  return OK;
}

int seq_stop_test(void)
{
#if 0 // test seq_get_next_event stops after Tseq
  ddi_ntime_add_ns(&Tseq, 1, NSEC_PER_MSEC * 500);
  seq->ntime = Tseq;
  printf("%u.%09u start\n", seq->ntime.sec, seq->ntime.ns);
  for (;;)
  {  
    event = seq_get_next_event(seq, &tn);
    if (!event)
      break;

    printf("%u.%09u : ", seq->ntime.sec, seq->ntime.ns);
    seq->track[tn].func(seq, tn, event);
    seq->track[tn].idx++;
  }
  return OK;
#endif

  return OK; 
}

int main(int argc, const char *argv[])
{
  test_find();
  seq_iter_test();
  seq_test();
  return 0;
}

