
#include <stdio.h>
#include <string.h>
#include "ddi_queue.h"
#include "ddi_atomic.h"

void ddi_queue_init(ddi_queue_t *q, uint32_t depth)
{
  q->depth = depth;
  q->idx_in = depth - 1;
  q->idx_out = depth - 1;
  q->count = 0;
}

int ddi_queue_full(ddi_queue_t *q)
{
  return (q->count == q->depth) ? 1 : 0;
}

int ddi_queue_empty(ddi_queue_t *q)
{
  return (q->count == 0) ? 1 : 0;
}

int ddi_queue_available(ddi_queue_t *q)
{
  return ((int)q->depth - (int)q->count);
}

int ddi_queue_out(ddi_queue_t *q)
{
  if (q->count == 0) // empty
    return -1;
  return q->idx_out;
}

int ddi_queue_in(ddi_queue_t *q)
{
  if (q->count == q->depth) // full
    return -1;
  return (int)q->idx_in;
}

int ddi_enqueue(ddi_queue_t *q)
{
  int idx;
  union {
    ddi_queue_t  i;
    uint32_t     u;
  } desired;
  union {
    ddi_queue_t  i;
    uint32_t     u;
  } expected;

  if (q->count == q->depth) // full
    return -1;

  for (;;)
  {
    expected.i = *q;
    desired.u = expected.u;
    desired.i.idx_in = (expected.i.idx_in + 1) % q->depth;
    desired.i.count = expected.i.count + 1;
    if (ddi_atomic_compare_exchange(q, &expected.u, desired.u))
    {
      return desired.i.idx_in;
    }

    if (expected.i.count == q->depth) // full
      return -1;
  }

  return idx;
}

int ddi_dequeue(ddi_queue_t *q)
{
  int idx;
  union {
    ddi_queue_t  i;
    uint32_t     u;
  } desired;
  union {
    ddi_queue_t  i;
    uint32_t     u;
  } expected;

  if (q->count == 0) // empty
    return -1;

  for (;;)
  {
    expected.i = *q;
    desired.u = expected.u;
    desired.i.idx_out = (expected.i.idx_out + 1) % q->depth;
    desired.i.count = expected.i.count - 1;

    if (ddi_atomic_compare_exchange(q, &expected.u, desired.u))
    {
      return desired.i.idx_out;
    }

    if (expected.i.count == 0) // empty
      return -1;
  }
  return idx;
}

void ddi_queue_print(ddi_queue_t *q)
{
  printf("queue count:%d depth:%d in:%d out:%d %s\n", q->count, q->depth, q->idx_in, q->idx_out,
      (q->count == 0) ? "empty" : (q->count == q->depth) ? "full" : "");

  for (int i = 0; i < q->depth; i++)
    printf("%c", ((q->idx_in == q->idx_out) && (q->idx_out == i)) ? 'X' : (q->idx_in == i) ? 'I' : (q->idx_out == i) ? 'O' : '.');
  printf("\n");
}

uint8_t *ddi_message_queue_in(ddi_message_queue_t *q)
{
  int idx = ddi_queue_in(&q->q);
  if (idx >= 0)
  {
    return q->messages + (q->size * idx);
  }
  return NULL;
}

uint8_t *ddi_message_queue_out(ddi_message_queue_t *q)
{
  int idx = ddi_queue_out(&q->q);
  if (idx >= 0)
  {
    return q->messages + (q->size * idx);
  }
  return NULL;
}

int ddi_message_enqueue(ddi_message_queue_t *q, const uint8_t *data, uint16_t len)
{
  int idx;
  if (!q || (data && (len > q->size)))
    return -2;

  idx = ddi_queue_in(&q->q);
  if (idx >= 0)
  {
    uint8_t *pbuf = q->messages + (q->size * idx);
    if (data && len)
    {
//    printf("enqueue:[%d]\n", len);
      q->len[idx] = len;
      memcpy(pbuf, data, len);
    }
    else // zero fill message buffer
    {
      q->len[idx] = 0;
      memset(pbuf, 0, q->size);
    }
    idx = ddi_enqueue(&q->q);
  }
  return idx;
}

int ddi_message_dequeue(ddi_message_queue_t *q, uint8_t *data, uint32_t *len)
{
  int idx;
  if ( (!q) || (data && (!len)) ) // if requesting data: require len ptr
    return -2;

  idx = ddi_queue_out(&q->q);
  if (idx >= 0)
  {
    if (data && *len)
    {
      uint32_t size = MIN(*len, q->len[idx]);
      uint8_t *pbuf = q->messages + (q->size * idx);
      memcpy(data, pbuf, size);
      *len = size;
    }
    ddi_dequeue(&q->q);
  }
  return idx;
}

void ddi_message_queue_init(ddi_message_queue_t *q, uint8_t *messages, uint32_t *lengths, int32_t size, int32_t depth)
{
  ddi_queue_init(&q->q, depth);
  q->size = size;
  q->messages = messages;
  q->len = lengths;
}

