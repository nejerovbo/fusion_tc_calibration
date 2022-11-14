

#include "ddi_os.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include "ddi_queue.h"

//static inline ddi_status_t os_to_ddi_status(osStatus status)
//{
//  switch (status)
//  {
//    case osOK:                    ///< function completed; no error or event occurred.
//    case osEventSignal:         ///< function completed; signal event occurred.
//    case osEventMessage:        ///< function completed; message event occurred.
//    case osEventMail:           ///< function completed; mail event occurred.
//      return ddi_status_ok;
//    case osEventTimeout:           ///< function completed; timeout occurred.
//      return ddi_status_timeout;
//    case osErrorParameter:         ///< parameter error: a mandatory parameter was missing or specified an incorrect object.
//    case osErrorValue:      ///< value of a parameter is out of range.
//      return ddi_status_param_err;
//    case osErrorResource:          ///< resource not available: a specified resource was not available.
//    case osErrorTimeoutResource:     ///< resource not available within given time: a specified resource was not available within the timeout period.
//    case osErrorNoMemory:      ///< system is out of memory: it was impossible to allocate or reserve memory for the operation.
//      return ddi_status_no_resources;
//    case osErrorISR:                ///< not allowed in ISR context: the function cannot be called from interrupt service routines.
//    case osErrorISRRecursive:       ///< function called multiple times from ISR with same object.
//      return ddi_status_not_allowed;
//    case osErrorPriority:      ///< system cannot determine priority or thread has illegal priority.
//    case osErrorOS:     ///< unspecified RTOS error: run-time error but no other error message fits.
//    default:
//      return ddi_status_err;
//  }
//}

#define DDI_TO_OS_TIMEOUT(ms) (((ms) == DDI_TIMEOUT_FOREVER) ? osWaitForever : (ms))

typedef struct
{
  pthread_t tid;
  ddi_thread_entry_t *entry;
  void *user_data;
  int detached;
} ddi_thread_t;

typedef struct
{
  pthread_mutex_t id;
} ddi_mutex_t;

//typedef struct
//{
//  osSemaphoreDef_t def;
//  osSemaphoreId id;
//  uint32_t cb[2];
//} ddi_semaphore_t;

typedef struct _event_t {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  volatile uint32_t val;
} event_t;

typedef struct
{
  uint8_t *messages;
  uint32_t *message_lengths;
  ddi_message_queue_t q;
  event_t event;
} ddi_mailqueue_t;

//typedef struct
//{
//  uint32_t cb[6];
//  osTimerDef_t def;
//  osTimerId id;
//} ddi_timer_t;

/*
 * Threads
 */
static void *thread_entry(void *params)
{
  void *retval;
  ddi_thread_t *thread = (ddi_thread_t *)params;
  retval = thread->entry(thread->user_data);
  if (thread->entry)
  {
    if (thread->detached)
    {
      thread->entry = 0;
      free(thread);
    }
  }
  return retval;
}

ddi_status_t ddi_thread_create(ddi_thread_handle_t *phandle, int req_priority, uint32_t stack_size, const char *thread_name, ddi_thread_entry_t *entry, void *user_data)
{
  ddi_thread_t *thread = NULL;
  pthread_attr_t tattr;
  struct sched_param param;
  int policy = 0;

  if (!phandle || !entry)
    return ddi_status_param_err;

  thread = (ddi_thread_t *)malloc(sizeof(ddi_thread_t));
  thread->user_data = user_data;
  thread->entry = entry;
  thread->detached = 0;
  *phandle = (ddi_thread_handle_t *)thread;

  pthread_attr_init(&tattr);
  pthread_attr_getschedparam(&tattr, &param);
  pthread_attr_setinheritsched(&tattr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_getschedpolicy(&tattr, &policy);

  int m = sched_get_priority_min(policy);
  int x = sched_get_priority_max(policy);
  int priority = req_priority;
  if (priority < m) priority = m;
  if (priority > x) priority = x;
  //printf("thread policy: %d priority: [%d..%d] %d -> %d\n", policy, m, x, req_priority, priority);
  param.sched_priority = priority;
  pthread_attr_setschedparam(&tattr, &param);
  if (pthread_create(&thread->tid, &tattr, thread_entry, thread) == 0)
  {
    return ddi_status_ok;
  }

  *phandle = NULL;
  free(thread);

  return ddi_status_no_resources;
}

ddi_status_t ddi_thread_create_with_scheduler(ddi_thread_handle_t *phandle, int sched_type, int req_priority, uint32_t stack_size, const char *thread_name, ddi_thread_entry_t *entry, void *user_data)
{
  ddi_thread_t *thread = NULL;
  pthread_attr_t tattr;
  struct sched_param param;
  int policy = 0;

  if (!phandle || !entry)
    return ddi_status_param_err;

  thread = (ddi_thread_t *)malloc(sizeof(ddi_thread_t));
  thread->user_data = user_data;
  thread->entry = entry;
  thread->detached = 0;
  *phandle = (ddi_thread_handle_t *)thread;

  pthread_attr_init(&tattr);
  pthread_attr_getschedparam(&tattr, &param);
  pthread_attr_setschedpolicy(&tattr, sched_type);
  pthread_attr_setinheritsched(&tattr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_getschedpolicy(&tattr, &policy);

  int m = sched_get_priority_min(policy);
  int x = sched_get_priority_max(policy);
  int priority = req_priority;
  if (priority < m) priority = m;
  if (priority > x) priority = x;
  //printf("thread policy: %d priority: [%d..%d] %d -> %d\n", policy, m, x, req_priority, priority);
  param.sched_priority = priority;
  pthread_attr_setschedparam(&tattr, &param);
  if (pthread_create(&thread->tid, &tattr, thread_entry, thread) == 0)
  {
    return ddi_status_ok;
  }

  *phandle = NULL;
  free(thread);

  return ddi_status_no_resources;
}

ddi_status_t ddi_thread_join(ddi_thread_handle_t handle, void **p_retval)
{
  ddi_status_t status = ddi_status_err;

  ddi_thread_t *thread = (ddi_thread_t *)handle;
  if ((thread == NULL) || thread->detached || (thread->tid == 0))
    return ddi_status_param_err;
  thread->detached = 0;

  if (pthread_join(thread->tid, p_retval) == 0)
  {
    status = ddi_status_ok;
  }
  free(thread);

  return status;
}

// set the thread handle to delete the thread when "ddi_thread_exit" is called
ddi_status_t ddi_thread_detach(ddi_thread_handle_t handle)
{
  ddi_thread_t *thread = (ddi_thread_t *)handle;
  if (thread == NULL)
    return ddi_status_param_err;

  thread->detached = 1;
  pthread_detach(thread->tid);

  return ddi_status_ok;
}

void ddi_thread_exit(ddi_thread_handle_t handle, int retval)
{
  ddi_thread_t *thread = (ddi_thread_t *)handle;
  if (thread == NULL)
    return;
  pthread_kill(thread->tid, 0);
  free(thread);
}


/*
 * Mutex
 */

ddi_status_t ddi_mutex_create(ddi_mutex_handle_t *phandle)
{
  ddi_status_t status = ddi_status_ok;
  ddi_mutex_t *mutex = (ddi_mutex_t *)malloc(sizeof(ddi_mutex_t));

  if (pthread_mutex_init(&mutex->id, NULL) != 0)
  {
    free(mutex);
    mutex = NULL;
    status = ddi_status_no_resources;
  }

  *phandle = (ddi_mutex_handle_t *)mutex;

  return status;
}

ddi_status_t ddi_mutex_lock(ddi_mutex_handle_t handle, uint32_t timeout_ms)
{
  int status;
  struct timeval tv;
  struct timespec ts;
  gettimeofday(&tv, NULL);
  ts.tv_sec = tv.tv_sec + timeout_ms / 1000;
  ts.tv_nsec = (timeout_ms % 1000) * 1000;

  ddi_mutex_t *mutex = (ddi_mutex_t *)handle;

  if (timeout_ms == DDI_TIMEOUT_FOREVER) //no timeout - wait forever
  {
    status = pthread_mutex_lock(&mutex->id);
  }
  else //used timed lock to wait for the mutex
  {
    status = pthread_mutex_timedlock(&mutex->id, &ts);
  }

  if (status == ETIMEDOUT)
    return ddi_status_timeout;
  return ddi_status_ok;
}

ddi_status_t ddi_mutex_unlock(ddi_mutex_handle_t handle)
{
  ddi_mutex_t *mutex = (ddi_mutex_t *)handle;
  if (pthread_mutex_unlock(&mutex->id) != 0)
    return ddi_status_err;
  return ddi_status_ok;
}

/*
 * Semaphores
 */

//ddi_status_t ddi_semaphore_create(ddi_semaphore_handle_t *phandle, uint32_t max_count, uint32_t initial_value)
//{
//  sem_t *sem;
//  char name[32];
//  sprintf(name, "ddi_sem_%x", phandle);
//  sem = sem_open(name, OC_CREAT, initial_value);
//  if (sem == SEM_FAILED)
//    return ddi_status_err;
//
//  *phandle = (ddi_semaphore_handle_t)sem;
//  return ddi_status_ok;
//}
//  
//ddi_status_t ddi_semaphore_decrement(ddi_semaphore_handle_t handle, uint32_t timeout_ms)
//{
//  sem_t *sem = (sem_t *)handle;
//
//  struct timeval tv;
//  struct timespec ts;
//  gettimeofday(&tv, NULL);
//  ts.tv_sec = tv.tv_sec + timeout_ms / 1000;
//  ts.tv_nsec = (timeout_ms % 1000) * 1000;
//
//  int sem_timedwait(sem
//  ETIMEDOUT
//  if (count == ETIMEDOUT)
//    return ddi_status_timeout;
//  if (count < 0)
//    return ddi_status_err;
//  return ddi_status_ok;
//}
//
//ddi_status_t ddi_semaphore_increment(ddi_semaphore_handle_t handle)
//{
//  sem_t *sem = (sem_t *)handle;
//  if (sem_post(&sem) == -1)
//  if (count == 0)
//    return ddi_status_timeout;
//  if (count < 0)
//    return ddi_status_err;
//  return ddi_status_ok;
//}

/*
 * Events
 */

void event_init(event_t *event) {
  pthread_mutex_init(&event->mutex, NULL);
  pthread_cond_init(&event->cond, NULL);
  event->val = 0;
}

void event_destroy(event_t *event) {
  pthread_mutex_destroy(&event->mutex);
  pthread_cond_destroy(&event->cond);
}

void event_signal(event_t *event, uint16_t val) {
  pthread_mutex_lock(&event->mutex);
  event->val |= val;
  pthread_cond_signal(&event->cond);
  pthread_mutex_unlock(&event->mutex);
}

ddi_status_t event_wait(event_t *event, uint32_t *rval, uint32_t timeout_ms) {
  struct timeval tv;
  struct timespec ts;
  gettimeofday(&tv, NULL);
  ts.tv_sec = tv.tv_sec + timeout_ms / 1000;
  ts.tv_nsec = (timeout_ms % 1000) * 1000;

  pthread_mutex_lock(&event->mutex);
  while (event->val == 0) {
    int status = pthread_cond_timedwait(&event->cond, &event->mutex, &ts);
    if (status == ETIMEDOUT)
    {
      if (rval)
        *rval = 0;
      return ddi_status_timeout;
    }
  }
  if (rval)
    *rval = event->val;
  event->val = 0;
  pthread_mutex_unlock(&event->mutex);
  return ddi_status_ok;
}

ddi_status_t ddi_event_create(ddi_event_handle_t *phandle, ddi_thread_handle_t thread)
{
  event_t *event = (event_t *)malloc(sizeof(event_t));
  event_init(event);

  *phandle = (ddi_event_handle_t)event;
  return ddi_status_ok;
}

ddi_status_t ddi_event_free(ddi_event_handle_t handle)
{
  event_t *event = (event_t *)handle;
  if (event)
  {
    event_destroy(event);
    free(event);
  }
  return ddi_status_ok;
}

ddi_status_t ddi_event_signal(ddi_event_handle_t handle, uint32_t value)
{
  event_t *event = (event_t *)handle;
  event_signal(event, value);
  return ddi_status_ok;
}

uint32_t ddi_event_wait(ddi_event_handle_t handle, uint32_t timeout_ms)
{
  event_t *event = (event_t *)handle;
  uint32_t val;
  event_wait(event, &val, timeout_ms);
  return val;
}

/*
 * Queues
 */

ddi_status_t ddi_queue_create(ddi_queue_handle_t *phandle, uint32_t item_size, uint32_t depth)
{
  ddi_mailqueue_t *queue = (ddi_mailqueue_t *)malloc(sizeof(ddi_mailqueue_t));
  queue->messages = (uint8_t *)malloc(sizeof(uint8_t) * (item_size * depth));
  queue->message_lengths = (uint32_t *)malloc(sizeof(uint32_t) * depth);
  ddi_message_queue_init(&queue->q, queue->messages, queue->message_lengths, item_size, depth);
  event_init(&queue->event);

  *phandle = (ddi_queue_handle_t)queue;
  return ddi_status_ok;
}

ddi_status_t ddi_queue_free(ddi_queue_handle_t handle)
{
  ddi_mailqueue_t *queue = (ddi_mailqueue_t *)handle;
  if (queue)
  {
    if (queue->messages)
    {
      free(queue->messages);
      queue->messages = 0;
    }
    if (queue->message_lengths)
    {
       free(queue->message_lengths);
       queue->message_lengths = 0;
    }
    event_destroy(&queue->event);
  }
  return ddi_status_ok;
}

ddi_status_t ddi_queue_send(ddi_queue_handle_t handle, void *message, uint32_t size, uint32_t timeout_ms)
{
  ddi_mailqueue_t *queue = (ddi_mailqueue_t *)handle;
  int idx = ddi_message_enqueue(&queue->q, message, size);
  if (idx < 0)
    return ddi_status_queue_full;
  event_signal(&queue->event, 1);
  return ddi_status_ok;
}

ddi_status_t ddi_queue_receive(ddi_queue_handle_t handle, void *message, uint32_t *size, uint32_t timeout_ms)
{
  ddi_mailqueue_t *queue = (ddi_mailqueue_t *)handle;

  ddi_status_t status = event_wait(&queue->event, NULL, timeout_ms);
  if (status == ddi_status_ok)
  {
    ddi_message_dequeue(&queue->q, message, size);
  }

  return status;
}

/* Critical Section
 *
 */

uint32_t ddi_enter_critical(void)
{
  uint32_t mask = 0;// __get_PRIMASK();
//  __disable_irq ();
  return mask;
}

void ddi_exit_critical(uint32_t mask)
{
//  __set_PRIMASK(mask);
}

/*
 * Delay
 */

void ddi_delay(int milliseconds)
{
  usleep(milliseconds);
}

uint64_t ddi_time_us(void)
{
  return 0;//osKernelSysTickMicroSec(osKernelSysTick());
}

ddi_status_t ddi_timer_create(ddi_timer_handle_t *phandle, int oneshot, ddi_timer_callback *callback, void *param)
{
//  ddi_timer_t *timer = (ddi_timer_t *)malloc(sizeof(ddi_timer_t));
//  timer->def.ptimer = (os_ptimer)callback;
//  timer->def.timer = timer->cb;
//  timer->id = osTimerCreate(&timer->def, oneshot ? osTimerOnce : osTimerPeriodic, param);
  return /*timer->id ? ddi_status_ok : */ddi_status_no_resources;
}

ddi_status_t ddi_timer_start(ddi_timer_handle_t handle, uint32_t millisec)
{
//  ddi_timer_t *timer = (ddi_timer_t *)handle;
//  osStatus status = osTimerStart(timer->id, millisec);
  return ddi_status_no_resources;//os_to_ddi_status(status);
}

ddi_status_t ddi_timer_stop(ddi_timer_handle_t handle)
{
//  ddi_timer_t *timer = (ddi_timer_t *)handle;
//  osStatus status = osTimerStop(timer->id);
  return ddi_status_no_resources;// os_to_ddi_status(status);
}

ddi_status_t ddi_os_start(void)
{
//  osStatus status = osKernelStart();
  return ddi_status_ok;// os_to_ddi_status(status);
}


#ifdef __cplusplus
}
#endif

