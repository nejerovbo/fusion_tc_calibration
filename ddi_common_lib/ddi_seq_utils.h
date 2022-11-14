/*****************************************************************************
 * (c) Copyright 2020 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
/*  ddi_seq_utils.h
 *  Created by Johana Lehrer on 2020-01-30
 */

#ifndef _DDI_SEQ_UTILS_H
#define _DDI_SEQ_UTILS_H

#include <stdint.h>
#include "ddi_ntime.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct seq_event_t seq_event_t;
typedef struct seq_t seq_t;

/** seq_event_func_t
 Prototype of the sequence event callback function.
 */
typedef void (seq_event_func_t)(seq_t *seq, uint32_t track_index, seq_event_t *event);

struct seq_event_t
{
  ntime_t       ntime;              // time of event
  void          *data;              // opaque event data
};

typedef struct
{
  uint32_t          idx;            // current event index
  uint32_t          event_count;    // number of elements in event array
  seq_event_t       *event;         // event array
  seq_event_func_t  *func;          // function called when each event occurs
} seq_track_t;

struct seq_t
{
  ntime_t           ntime;          // current sequence time
  uint32_t          wake_latency;
  uint32_t          scheduler_uncertainty;
  uint32_t          track_count;    // number of elements in track array
  seq_track_t       *track;         // track array
};

/** seq_run
 * Runs the sequence from the current position
 */
int seq_run(seq_t *seq);

/** stop_run
 * Stops the sequence
 */
int seq_stop(seq_t *seq);

/** seq_run_to_index
 * Runs the sequence from the current position until the index
 */
int seq_run_to_index(seq_t *seq, uint32_t index);

/** seq_run_to_ntime
 * Runs the sequence from the current position until the ntime
 */
int seq_run_to_ntime(seq_t *seq, ntime_t *ntime);

/** seq_set_index
 * Sets the sequence position to the index
 */
int seq_set_index(seq_t *seq, uint32_t index);

/** seq_set_ntime
 * Sets the sequence position to the ntime
 */
int seq_set_ntime(seq_t *seq, ntime_t *ntime);

/** seq_get_next_event
 * Gets the next event after the current position
 */
seq_event_t *seq_get_next_event(seq_t *seq, uint32_t *ptrack_idx);

/** seq_find_event
 * Finds the event closest to the key
 */
seq_event_t *seq_find_event(seq_event_t *key, seq_event_t *list, uint32_t imax);

/** seq_get_systime
 * Sets ntime to the current system time.
 */
void seq_get_systime(ntime_t *t);

/** seq_get_event_time
 * Gets the time of the event
 */
double seq_get_event_time(seq_event_t *key);

#ifdef __cplusplus
}
#endif

#endif // _DDI_SEQ_UTILS_H

