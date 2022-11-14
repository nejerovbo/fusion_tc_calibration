/*****************************************************************************
 * (c) Copyright 2018 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
/*  ddi_queue.h
 *  Created by Johana Lehrer on 2018-08-15
 */

#ifndef _DDI_QUEUE_H
#define _DDI_QUEUE_H

#include <stdint.h>
#include "ddi_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

/** ddi_queue_t
 This is the core queue structure. It is a 32-bit bitfield which can be manipulated in its entirety
 with atomic memory load/store operations.
 
 The queue depth is limited to 255 entries but the message buffer of each entry may be any size.
 Typically this structure is not used directly by clients.
*/
PACKED_BEGIN
typedef volatile struct PACKED
{
  uint32_t depth       : 8;  /**< The depth of the message queue i.e. the max elements in the queue */
  uint32_t idx_in      : 8;  /**< The current in index */
  uint32_t idx_out     : 8;  /**< The current out index */
  uint32_t count       : 8;  /**< The number of elements in the queue */
} ddi_queue_t;
PACKED_END

/** ddi_message_queue_t
 This is a message queue structure. It contains a ddi_queue_t for the core atomic queueing mechanism
 and a pointer to the message buffer memory.

 The message buffer is organized as a two-dimensional memory array:
 uint8_t messages[depth][payload_size];

 A ddi_message_queue_t may be created dynamically or statically.
 If allocated dynamically the queue should be initialized by calling ddi_message_queue_init().
 If allocated statically the queue should be instanciated with the DDI_QUEUE_DEF macro.

 Usage:
 The ddi_message_queue_t is thread safe but only between one producer and one consumer thread.
 The producer will typically get the current input message buffer and write data into it
 then enqueue the message into the queue.
 The consumer will typically get the current output message buffer and read from it
 then dequeue the message from the queue.

 Example1:

 Producer:
 #define QUEUE_DEPTH          4
 #define QUEUE_PAYLOAD_SIZE   256
 DDI_QUEUE_DEF(my_queue, QUEUE_DEPTH, QUEUE_PAYLOAD_SIZE);

 uint8_t *message = ddi_message_queue_in(&my_queue);
 if (message)
 {
  write data into message
  ddi_message_enqueue(&my_queue, NULL, 0);
 }

 Consumer:
 uint8_t *message = ddi_message_queue_out(&my_queue);
 if (message)
 {
   read data from message
  ddi_message_dequeue(&my_queue, NULL, 0);
 }

 Example2:

 Producer:
 #define QUEUE_DEPTH          4
 #define QUEUE_PAYLOAD_SIZE   256
 DDI_QUEUE_DEF(my_queue, QUEUE_DEPTH, QUEUE_PAYLOAD_SIZE);

 uint8_t message[QUEUE_PAYLOAD_SIZE];
 format message buffer to be enqueued
 if (ddi_message_enqueue(&my_queue, message, QUEUE_PAYLOAD_SIZE) >= 0)
  success! the message data was copied into the queue and enqueued

 Consumer:
 uint8_t message[QUEUE_PAYLOAD_SIZE];
 if (ddi_message_dequeue(&my_queue, message, QUEUE_PAYLOAD_SIZE) >= 0)
 {
  success! the message was copied from the queue and dequeued
  now use the data received data in the message buffer
 }

 Example3: Dynamically allocated message queue

 #define QUEUE_DEPTH          4
 #define QUEUE_PAYLOAD_SIZE   256

 uint8_t *messages = (uint8_t *)malloc(sizeof(uint8_t) * (QUEUE_DEPTH * QUEUE_PAYLOAD_SIZE));
 ddi_message_queue_t *my_queue = (ddi_message_queue_t *)malloc(sizeof(ddi_message_queue_t));

 ddi_message_queue_init(my_queue, messages, QUEUE_DEPTH, QUEUE_PAYLOAD_SIZE);
 
 ready to use the queue now
*/
typedef struct
{
  ddi_queue_t q;        /**< The core queue structure */
  uint32_t size;        /**< The payload size of each message buffer */
  uint8_t *messages;    /**< The message buffer array */
  uint32_t *len;        /**< The message buffer len array */
} ddi_message_queue_t;

/** DDI_QUEUE_DEF
 This macro is used for instanciating a statically allocated message queue.
 The message queue and the message buffer array are both instanciated by this macro.
 @param NAME The name of the ddi_message_queue_t instance.
 @param DEPTH The message queue depth i.e. number of elements in the queue. (1 to 16)
 @param PAYLOAD_SIZE The size of the message buffer of each element in the queue. (size unlimited)
*/
#define DDI_QUEUE_DEF(NAME,DEPTH,PAYLOAD_SIZE) \
static uint8_t NAME##_messages[DEPTH][PAYLOAD_SIZE]; \
static uint32_t NAME##_messages_len[DEPTH]; \
ddi_message_queue_t NAME = { \
  .q = { \
    .depth = DEPTH, \
    .idx_in = (DEPTH-1), \
    .idx_out = (DEPTH-1), \
    .count = 0, \
  }, \
  .size = PAYLOAD_SIZE, \
  .messages = (uint8_t *)NAME##_messages, \
  .len = (uint32_t *)NAME##_messages_len \
}

/** ddi_queue_init
 Initializes a ddi_queue_t structure.

 @param q The address of the ddi_queue_t structure to initialize.
 @param depth The number of elements in the queue.
 */
void ddi_queue_init(ddi_queue_t *q, uint32_t depth);

/** ddi_enqueue
 This is a low-level function for incrementing a ddi_queue_t idx_in index in a thread-safe
 non-blocking atomic manner.

 This function should not be called directly by clients. It is used internally by the message queue.
 Clients should call the ddi_message_enqueue API instead.

 @param q The address of the ddi_queue_t structure.
 @returns the current idx_in or -1 if the queue is full.
*/
int ddi_enqueue(ddi_queue_t *q);

/** ddi_dequeue
 This is a low-level function for incrementing a ddi_queue_t idx_out index in a thread-safe
 non-blocking atomic manner.

 This function should not be called directly by clients. It is used internally by the message queue.
 Clients should call the ddi_message_dequeue API instead.

 @param q The address of the ddi_queue_t structure.
 @returns the current idx_out or -1 if the queue is empty.
 */
int ddi_dequeue(ddi_queue_t *q);

/** ddi_queue_full
 Tests if the queue is full.

 @param q The address of the ddi_queue_t structure.
 @returns 1 if the queue is full; 0 otherwise.
*/
int ddi_queue_full(ddi_queue_t *q);

/** ddi_queue_empty
 Tests if the queue is empty.

 @param q The address of the ddi_queue_t structure.
 @returns 1 if the queue is empty; 0 otherwise.
 */
int ddi_queue_empty(ddi_queue_t *q);

/** ddi_queue_available
 Returns the number of queue entries available.

 @param q The address of the ddi_queue_t structure.
 @returns the number of entries available
 */
int ddi_queue_available(ddi_queue_t *q);

/** ddi_queue_out
 Gets the current idx_out

 @param q The address of the ddi_queue_t structure.
 @returns the current out_idx or -1 if the queue is empty.
 */
int ddi_queue_out(ddi_queue_t *q);

/** ddi_queue_in
 Gets the current idx_in

 @param q The address of the ddi_queue_t structure.
 @returns the current idx_in or -1 if the queue is full.
 */
int ddi_queue_in(ddi_queue_t *q);

/** ddi_message_queue_init
 Initializes a ddi_message_queue_t structure with a reference to the message buffer memory to use.

 @param q The address of the ddi_message_queue_t structure to initialize.
 @param messages The address of the message buffer memory to use.
 @param size The size of each message buffer.
 @param depth The number of elements in the message queue.
 */
void ddi_message_queue_init(ddi_message_queue_t *q, uint8_t *messages, uint32_t *lengths, int32_t size, int32_t depth);

/** ddi_message_enqueue
 Enqueues a message in the queue.

 @param q The address of the ddi_message_queue_t structure.
 @param data The address of the data to enqueue. If not NULL the data is copied into the message buffer.
 @param len The size of the message to copy into the message buffer.
 @returns the in index of the queue or -1 if the queue is full.
 */
int ddi_message_enqueue(ddi_message_queue_t *q, const uint8_t *data, uint16_t len);

/** ddi_message_dequeue
 Dequeues a message from the queue.

 @param q The address of the ddi_message_queue_t structure.
 @param data The address of a buffer which receives the message buffer contents. If not NULL the message buffer is copied to the data address.
 @param len The size of the message to copy from the message buffer.
 @returns the out index of the queue or -1 if the queue is empty.
 */
int ddi_message_dequeue(ddi_message_queue_t *q, uint8_t *data, uint32_t *len);

/** ddi_message_queue_in
 Gets the current in index

 @param q The address of the ddi_message_queue_t structure.
 @returns the next input message buffer at the head of the queue or NULL if the queue is full.
*/
uint8_t *ddi_message_queue_in(ddi_message_queue_t *q);

/** ddi_message_queue_out
 Gets the current out index

 @param q The address of the ddi_message_queue_t structure.
 @returns the next output message buffer at the tail of the queue or NULL if the queue is empty.
 */
uint8_t *ddi_message_queue_out(ddi_message_queue_t *q);

#ifdef __cplusplus
}
#endif

#endif // _DDI_QUEUE_H
