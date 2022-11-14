/*****************************************************************************
 * (c) Copyright 2018 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
/*  ddi_os.h
 *  Created by Johana Lehrer on 2018-08-15
 */

#ifndef __DDI_OS_H
#define __DDI_OS_H

#include <stdint.h>
#include "ddi_status.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DDI_TIMEOUT_FOREVER     0xFFFFFFFF
#define DDI_TIMEOUT_IMMEDIATE   0

/** Opaque thread client handle
 * @see ddi_thread_create
 */
typedef void *ddi_thread_handle_t;

/** Opaque mutex client handle
 * @see ddi_mutex_create
 */
typedef void *ddi_mutex_handle_t;

/** Opaque semaphore client handle
 * @see ddi_semaphore_create
 */
typedef void *ddi_semaphore_handle_t;

/** Opaque queue client handle
 * @see ddi_queue_create
 */
typedef void *ddi_queue_handle_t;

/** Opaque buffer queue client handle
 * @see ddi_buffer_queue_create
 */
typedef void *ddi_buffer_queue_handle_t;

/** Opaque event client handle
 * @see ddi_event_create
 */
typedef void *ddi_event_handle_t;

/** Opaque timer client handle
 * @see ddi_timer_create
 */
typedef void *ddi_timer_handle_t;

/** Prototype of a thread entry function
 * Client thread entry functions must match this prototype.
 * The client's thread entry function is passed the user args specified when the thread was created.
 * @see ddi_thread_create
 */
typedef void *(ddi_thread_entry_t)(const void *user_data);

/** @brief Disables interrupts and thread scheduling
 * Notes:
 * Holding the critical section should be limited to very short procedures!
 * this function may be called from an interrupt context.
 * @return mask which is passed to ddi_exit_critical
 */
uint32_t ddi_enter_critical(void);

/** @brief Re-enables interrupts and thread scheduling
 * @param mask which was returned from ddi_enter_critical
 */
void ddi_exit_critical(uint32_t mask);

/** @brief Creates a new thread
 * @param phandle Pointer to a ddi_thread_handle_t which receives the handle of the newly created thread.
 * @param priority The priority of the thread: 0 is the highest priority
 * @param stack_size The stack size in bytes for the thread
 * @param thread_name The name of the thread
 * @param entry The thread entry function pointer
 * @param user_args Pointer to client specific parameter which is passed to the thread entry function.
 * @return ddi_status_ok if successful; ddi_status_no_resources if the thread could not be created.
 */
ddi_status_t ddi_thread_create(ddi_thread_handle_t *phandle, int priority, uint32_t stack_size, const char *thread_name, ddi_thread_entry_t *entry, void *user_data);

/** @brief Creates a new thread with a argument for scheduler selection (SCHED_OTHER, SCHED_RR or SCHED_FIFO).  This is a POSIX-specific API.
 * @param phandle Pointer to a ddi_thread_handle_t which receives the handle of the newly created thread.
 * @param sched_type Argument for scheduler selection (SCHED_OTHER, SCHED_RR or SCHED_FIFO)
 * @param priority The priority of the thread: 0 is the highest priority
 * @param stack_size The stack size in bytes for the thread
 * @param thread_name The name of the thread
 * @param entry The thread entry function pointer
 * @param user_args Pointer to client specific parameter which is passed to the thread entry function.
 * @return ddi_status_ok if successful; ddi_status_no_resources if the thread could not be created.
 */
ddi_status_t ddi_thread_create_with_scheduler(ddi_thread_handle_t *phandle, int sched_type, int req_priority, uint32_t stack_size, const char *thread_name, ddi_thread_entry_t *entry, void *user_data);

/** @brief Waits until the thread entry function returns then frees the thread resources.
 * @param thread_id The ddi_thread_handle_t which was returned when the thread was created.
 * @return ddi_status_ok if successful; ddi_status_param_err if the thread handle is invalid.
 */
ddi_status_t ddi_thread_join(ddi_thread_handle_t handle, void **p_retval);

/** @brief Set the thread handle to free the thread resources when "ddi_thread_exit" is called.
 * @param thread_id The ddi_thread_handle_t which was returned when the thread was created.
 * @return ddi_status_ok if successful; ddi_status_param_err if the thread handle is invalid.
 */
ddi_status_t ddi_thread_detach(ddi_thread_handle_t handle);

/** @brief If ddi_thread_detatch was called this function will terminate the thread and free the thread resources.
 * otherwise the thread resources will be freed by ddi_thread_join.
 * @param thread_id The ddi_thread_handle_t which was returned when the thread was created.
 * @return ddi_status_ok if successful; ddi_status_param_err if the thread handle is invalid.
 */
void ddi_thread_exit(ddi_thread_handle_t handle, int retval);

/** @brief Creates a new binary mutex
 * @param phandle Pointer to a ddi_mutex_handle_t which receives the handle of the newly created mutex.
 * @return ddi_status_ok if successful; ddi_status_no_resources if the mutex could not be created.
 */
ddi_status_t ddi_mutex_create(ddi_mutex_handle_t *phandle);

/** @brief Locks the mutex
 * Notes:
 *  This function block without a timeout until the mutex is unlocked.
 *  This function can not be called from an interrupt context.
 *  This function can not be called recursively.
 * @param handle The ddi_mutex_handle_t which was returned when the mutex was created.
 * @return ddi_status_ok if successful; ddi_status_param_err if the mutex handle is invalid.
 */
ddi_status_t ddi_mutex_lock(ddi_mutex_handle_t handle, uint32_t timeout_ms);

/** @brief Unlocks the mutex
 * Notes:
 *  This function is safe to be called from an interrupt context.
 * @param handle The ddi_mutex_handle_t which was returned when the mutex was created.
 * @return ddi_status_ok if successful; ddi_status_not_ready if the mutex could not be unlocked; ddi_status_param_err if the mutex handle is invalid.
 */
ddi_status_t ddi_mutex_unlock(ddi_mutex_handle_t handle);

/** @brief Creates a new counting semaphore
 * @param phandle Pointer to a ddi_semaphore_handle_t which receives the handle of the newly created semaphore.
 * @param max_count The highest count which the semaphore may be incremented to.
 * @param initial_value The initial count value of the semaphore.
 * @return ddi_status_ok if successful; ddi_status_no_resources if the semaphore could not be created.
 */
ddi_status_t ddi_semaphore_create(ddi_semaphore_handle_t *phandle, uint32_t max_count, uint32_t initial_value);

/** @brief Decrements a counting semaphore
 * Notes:
 *  This function block without a timeout until the semaphore is decremented.
 *  This function can not be called from an interrupt context.
 * @param phandle Pointer to a ddi_semaphore_handle_t which was returned when the semaphore was created.
 * @return ddi_status_ok if successful; ddi_status_timeout if the semaphore can not be decremented within the timeout; ddi_status_param_err if the semaphore handle is invalid.
 */
ddi_status_t ddi_semaphore_decrement(ddi_semaphore_handle_t handle, uint32_t timeout_ms);

/** @brief Increments a counting semaphore
 * Notes:
 *  This function block without a timeout until the semaphore is incremented.
 *  This function can not be called from an interrupt context.
 * @param phandle Pointer to a ddi_semaphore_handle_t which was returned when the semaphore was created.
 * @return ddi_status_ok if successful; ddi_status_timeout if the semaphore can not be incremented within the timeout; ddi_status_param_err if the semaphore handle is invalid.
 */
ddi_status_t ddi_semaphore_increment(ddi_semaphore_handle_t handle);

/** @brief Creates a new messaging queue
 * @param phandle Pointer to a ddi_queue_handle_t which receives the handle of the newly created queue.
 * @param depth The number of elements in the message queue.
 * @param size The size in bytes (uint8_t) of each queue element to contain the messages.
 * @return ddi_status_ok if successful; ddi_status_no_resources if the queue could not be created.
 */
ddi_status_t ddi_queue_create(ddi_queue_handle_t *phandle, uint32_t item_size, uint32_t depth);

/** @brief Frees a message queue and its internally allocated resources
 * @param phandle Pointer to the ddi_queue_handle_t which was returned when the queue was created.
 * @return ddi_status_ok if successful; ddi_status_param_err if the queue handle is invalid.
 */
ddi_status_t ddi_queue_free(ddi_queue_handle_t handle);

/** @brief Sends a message to the queue
 * @param handle Pointer to a ddi_queue_handle_t which was returned when the queue was created.
 * @param message Address of a buffer to be copied into the next queue element.
 * @return ddi_status_ok if successful; ddi_status_timeout if the timeout expires before a queue element is available; ddi_status_param_err if the queue handle is invalid.
 */
ddi_status_t ddi_queue_send(ddi_queue_handle_t handle, void *message, uint32_t size, uint32_t timeout_ms);

/** @brief Receives a message from the queue
 * @param handle Pointer to a ddi_queue_handle_t which was returned when the queue was created.
 * @param message Address of a buffer to receive a copy of the message data from the next available queue element.
 * @return ddi_status_ok if successful; ddi_status_timeout if the timeout expires before a queue element is available; ddi_status_param_err if the queue handle is invalid.
 */
ddi_status_t ddi_queue_receive(ddi_queue_handle_t handle, void *message, uint32_t *size, uint32_t timeout_ms);

/** @brief Creates a new thread event
 * @param phandle Pointer to a ddi_event_handle_t which receives the handle of the newly created event.
 * @param thread_id The ddi_thread_handle_t associated with the event: (i.e. the event receiver)
 * @return ddi_status_ok if successful; ddi_status_no_resources if the event could not be created.
 */
ddi_status_t ddi_event_create(ddi_event_handle_t *phandle, ddi_thread_handle_t thread_id);

/** @brief Frees a thread event and its internally allocated resources
 * @param phandle Pointer to the ddi_event_handle_t which was returned when the event was created.
 * @return ddi_status_ok if successful; ddi_status_param_err if the event handle is invalid.
 */
ddi_status_t ddi_event_free(ddi_event_handle_t handle);

/** @brief Signals the waiting thread with an event value
 * Note:
 * The 32-bit 'value' passed in is bitwise or'd with the previous event value.
 * This allows using the value as a bitfield for flagging various events.
 * The waiting event receiver will get the accumulated value and atomically clear the events.
 * @param phandle Pointer to the ddi_event_handle_t which was returned when the event was created.
 * @param value A 32-bit value sent to the event receiver.
 * @return ddi_status_ok if successful; ddi_status_param_err if the event handle is invalid.
 */
ddi_status_t ddi_event_signal(ddi_event_handle_t handle, uint32_t value);

/** @brief Waits for an event to be signaled
 * Note:
 * This function blocks until an event is signaled.
 * The 32-bit 'value' returned is the accumulated bitwise 'or' of the event value.
 * The waiting event receiver will get the accumulated value and atomically clear the events.
 * @param phandle Pointer to the ddi_event_handle_t which was returned when the event was created.
 * @return The accumulated 32-bit event value.
 */
uint32_t ddi_event_wait(ddi_event_handle_t handle, uint32_t timeout_ms);

typedef void (ddi_timer_callback)(const void *);

ddi_status_t ddi_timer_create(ddi_timer_handle_t *phandle, int oneshot, ddi_timer_callback *callback, void *param);
ddi_status_t ddi_timer_start(ddi_timer_handle_t handle, uint32_t millisec);
ddi_status_t ddi_timer_stop(ddi_timer_handle_t handle);

ddi_status_t ddi_os_start(void);

/** @brief Delays the calling thread for the sepcified number of milliseconds
 * @param milliseconds The number of milliseconds to delay
 */
void ddi_delay(int milliseconds);

uint64_t ddi_time_us(void);

#ifdef __cplusplus
}
#endif

#endif // __DDI_OS_H

