

#include "ddi_os.h"
#include "cmsis_compiler.h"
#include "cmsis_os.h"
#include <stdlib.h>
#include <string.h>

static inline ddi_status_t os_to_ddi_status(osStatus status)
{
  switch (status)
  {
    case osOK:                    ///< function completed; no error or event occurred.
    case osEventSignal:         ///< function completed; signal event occurred.
    case osEventMessage:        ///< function completed; message event occurred.
    case osEventMail:           ///< function completed; mail event occurred.
      return ddi_status_ok;
    case osEventTimeout:           ///< function completed; timeout occurred.
      return ddi_status_timeout;
    case osErrorParameter:         ///< parameter error: a mandatory parameter was missing or specified an incorrect object.
    case osErrorValue:      ///< value of a parameter is out of range.
      return ddi_status_param_err;
    case osErrorResource:          ///< resource not available: a specified resource was not available.
    case osErrorTimeoutResource:     ///< resource not available within given time: a specified resource was not available within the timeout period.
    case osErrorNoMemory:      ///< system is out of memory: it was impossible to allocate or reserve memory for the operation.
      return ddi_status_no_resources;
    case osErrorISR:                ///< not allowed in ISR context: the function cannot be called from interrupt service routines.
    case osErrorISRRecursive:       ///< function called multiple times from ISR with same object.
      return ddi_status_not_allowed;
    case osErrorPriority:      ///< system cannot determine priority or thread has illegal priority.
    case osErrorOS:     ///< unspecified RTOS error: run-time error but no other error message fits.
    default:
      return ddi_status_err;
  }
}

#define DDI_TO_OS_TIMEOUT(ms) (((ms) == DDI_TIMEOUT_FOREVER) ? osWaitForever : (ms))

typedef struct
{
  osThreadId tid;
  osThreadDef_t def;
  uint32_t delete_on_exit   : 1;
  uint32_t exited           : 1;
  int retval;
} ddi_thread_t;

typedef struct
{
  osMutexDef_t def;
  osMutexId id;
  uint32_t cb[4];
} ddi_mutex_t;

typedef struct
{
  osSemaphoreDef_t def;
  osSemaphoreId id;
  uint32_t cb[2];
} ddi_semaphore_t;

typedef struct
{
  uint32_t *q;
  uint32_t *m;
  void *   p[2];
  osMailQDef_t def;
  osMailQId id;

} ddi_mailqueue_t;

typedef struct
{
  uint32_t cb[6];
  osTimerDef_t def;
  osTimerId id;
} ddi_timer_t;

void *zalloc(size_t size)
{
  void *obj = malloc(size);
  if (obj)
    memset(obj, 0, size);
  return obj;
}

/*
 * Threads
 */

ddi_status_t ddi_thread_create(ddi_thread_handle_t *phandle, int priority, uint32_t stack_size, const char *thread_name, ddi_thread_entry_t *entry, void *user_data)
{
  ddi_thread_t *thread;
  ddi_status_t status = ddi_status_ok;

  thread = (ddi_thread_t *)zalloc(sizeof(ddi_thread_t));
  if (thread == NULL)
    return ddi_status_no_resources;

  thread->def.tpriority = priority;
  thread->def.stacksize = stack_size;
  thread->def.instances = 0;
  thread->def.pthread = (os_pthread)entry;
  thread->tid = osThreadCreate(&thread->def, user_data);
  if (thread->tid == 0)
  {
    free(thread);
    thread = NULL;
    status = ddi_status_no_resources;
  }

  *phandle = (ddi_thread_handle_t *)thread;

  return status;
}

ddi_status_t ddi_thread_join(ddi_thread_handle_t handle, int *p_retval)
{
  ddi_thread_t *thread = (ddi_thread_t *)handle;
  if ((thread == NULL) || (thread->tid == 0))
    return ddi_status_param_err;

  // Block until the thread's TCB is deleted..
  // This is the only way I could find to poll if the TCB exists
  while (osThreadGetPriority(thread->tid) != osPriorityError)
  {
    ddi_delay(1); // sorry this is ugly
  }

  // Then delete the thread to free the TCB
  osThreadTerminate(thread->tid);
  thread->tid = 0;
  *p_retval = thread->retval;
  free(thread);

  return ddi_status_ok;
}

// set the thread handle to delete the thread when "ddi_thread_exit" is called
ddi_status_t ddi_thread_detach(ddi_thread_handle_t handle)
{
  ddi_thread_t *thread = (ddi_thread_t *)handle;
  if (thread == NULL)
    return ddi_status_param_err;

  thread->delete_on_exit = 1;

  return ddi_status_ok;
}

void ddi_thread_exit(ddi_thread_handle_t handle, int retval)
{
  ddi_thread_t *thread = (ddi_thread_t *)handle;
  if (thread == NULL)
    return;

  if (thread->delete_on_exit)
  {
    osThreadTerminate(thread->tid);
    thread->tid = 0;
    free(thread);
  }
  else // wait for join
  {
    thread->retval = retval;
    thread->exited = 1;
  }
}


/*
 * Mutex
 */

ddi_status_t ddi_mutex_create(ddi_mutex_handle_t *phandle)
{
  ddi_status_t status = ddi_status_ok;
  ddi_mutex_t *mutex = (ddi_mutex_t *)zalloc(sizeof(ddi_mutex_t));
  if (mutex == NULL)
    return ddi_status_no_resources;

  mutex->def.mutex = mutex->cb;
  mutex->id = osMutexCreate(&mutex->def);
  if (mutex->id == 0)
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
  osStatus status;
  ddi_mutex_t *mutex = (ddi_mutex_t *)handle;
  status = osMutexWait(mutex->id, DDI_TO_OS_TIMEOUT(timeout_ms));
  if (status == osOK)
    return ddi_status_ok;
  return os_to_ddi_status(status);
}

ddi_status_t ddi_mutex_unlock(ddi_mutex_handle_t handle)
{
  ddi_mutex_t *mutex = (ddi_mutex_t *)handle;
  if (osMutexRelease(mutex->id) != osOK)
    return ddi_status_err;
  return ddi_status_ok;
}

/*
 * Semaphores
 */

ddi_status_t ddi_semaphore_create(ddi_semaphore_handle_t *phandle, uint32_t max_count, uint32_t initial_value)
{
  ddi_status_t status = ddi_status_ok;
  ddi_semaphore_t *semaphore = (ddi_semaphore_t *)zalloc(sizeof(ddi_semaphore_t));
  if (semaphore == NULL)
    return ddi_status_no_resources;
  semaphore->def.semaphore = semaphore->cb;
  semaphore->id = osSemaphoreCreate(&semaphore->def, max_count);
  if (semaphore->id == 0)
  {
    free(semaphore);
    semaphore = NULL;
    status = ddi_status_no_resources;
  }
  *phandle = (ddi_semaphore_handle_t)semaphore;
  return status;
}
  
ddi_status_t ddi_semaphore_decrement(ddi_semaphore_handle_t handle, uint32_t timeout_ms)
{
  ddi_semaphore_t *semaphore = (ddi_semaphore_t *)handle;
  int32_t count = osSemaphoreWait(semaphore->id, DDI_TO_OS_TIMEOUT(timeout_ms));
  if (count == 0)
    return ddi_status_timeout;
  if (count < 0)
    return ddi_status_err;
  return ddi_status_ok;
}

ddi_status_t ddi_semaphore_increment(ddi_semaphore_handle_t handle)
{
  ddi_semaphore_t *semaphore = (ddi_semaphore_t *)handle;
  osStatus status = osSemaphoreRelease(semaphore->id);
  if (status == osErrorParameter)
    return ddi_status_param_err;

  if (status == osErrorResource)
    return ddi_status_no_resources;

  return ddi_status_ok;
}

/*
 * Events
 */

ddi_status_t ddi_event_create(ddi_event_handle_t *phandle, ddi_thread_handle_t thread)
{
  *phandle = (ddi_event_handle_t)thread;
  return ddi_status_ok;
}

ddi_status_t ddi_event_signal(ddi_event_handle_t handle, uint32_t value)
{
  ddi_thread_t *thread = (ddi_thread_t *)handle;

  osSignalSet(thread->tid, value);
  return ddi_status_ok;
}

uint32_t ddi_event_wait(ddi_event_handle_t handle, uint32_t timeout_ms)
{
  ddi_thread_t *thread = (ddi_thread_t *)handle;
  osEvent e = osSignalWait(0, DDI_TO_OS_TIMEOUT(timeout_ms));
  osSignalClear(thread->tid, e.value.v);
  return e.value.v;
}

/*
 * Queues
 */

ddi_status_t ddi_queue_create(ddi_queue_handle_t *phandle, uint32_t item_size, uint32_t depth)
{
  ddi_mailqueue_t *queue = (ddi_mailqueue_t *)zalloc(sizeof(ddi_mailqueue_t));
  if (queue == NULL)
    return ddi_status_no_resources;
  queue->q = (uint32_t *)zalloc(sizeof(uint32_t) * (depth + 4));
  if (queue->q == NULL)
    return ddi_status_no_resources;
  queue->m = (uint32_t *)zalloc(sizeof(uint32_t) * (3 + ((item_size + 3)/4) * depth));
  if (queue->m == NULL)
    return ddi_status_no_resources;

  queue->p[0] = queue->q;
  queue->p[1] = queue->m;
  queue->def.pool = queue->p;
  queue->def.item_sz = item_size;
  queue->def.queue_sz = depth;
  queue->id = osMailCreate(&queue->def, NULL);
  *phandle = (ddi_queue_handle_t)queue;
  return ddi_status_ok;
}

ddi_status_t ddi_queue_send(ddi_queue_handle_t handle, void *message, uint32_t size, uint32_t timeout_ms)
{
  ddi_mailqueue_t *queue = (ddi_mailqueue_t *)handle;
  void *pdata = osMailAlloc(queue->id, DDI_TO_OS_TIMEOUT(timeout_ms));
  if (size > queue->def.item_sz)
    return ddi_status_param_err;
  memcpy(pdata, message, size);
  osMailPut(queue->id,pdata);
  osThreadYield();
  return ddi_status_ok;
}

ddi_status_t ddi_queue_receive(ddi_queue_handle_t handle, void *message, uint32_t size, uint32_t timeout_ms)
{
  ddi_mailqueue_t *queue = (ddi_mailqueue_t *)handle;
  osEvent event;
  event = osMailGet(queue->id, DDI_TO_OS_TIMEOUT(timeout_ms));
  if (event.status == osEventTimeout)
    return ddi_status_timeout;
  if (event.status == osEventMail)
  {
    memcpy(message, event.value.p, size);
    osMailFree(queue->id,event.value.p);
  }
  return ddi_status_ok;
}

/* Critical Section
 *
 */

uint32_t ddi_enter_critical(void)
{
  uint32_t mask = __get_PRIMASK();
  __disable_irq ();
  return mask;
}

void ddi_exit_critical(uint32_t mask)
{
  __set_PRIMASK(mask);
}

/*
 * Delay
 */

void ddi_delay(int milliseconds)
{
  osDelay(milliseconds);
}

uint64_t ddi_time_us(void)
{
  return osKernelSysTickMicroSec(osKernelSysTick());
}

ddi_status_t ddi_timer_create(ddi_timer_handle_t *phandle, int oneshot, ddi_timer_callback *callback, void *param)
{
  ddi_timer_t *timer = (ddi_timer_t *)zalloc(sizeof(ddi_timer_t));
  timer->def.ptimer = (os_ptimer)callback;
  timer->def.timer = timer->cb;
  timer->id = osTimerCreate(&timer->def, oneshot ? osTimerOnce : osTimerPeriodic, param);
  return timer->id ? ddi_status_ok : ddi_status_no_resources;
}

ddi_status_t ddi_timer_start(ddi_timer_handle_t handle, uint32_t millisec)
{
  ddi_timer_t *timer = (ddi_timer_t *)handle;
  osStatus status = osTimerStart(timer->id, millisec);
  return os_to_ddi_status(status);
}

ddi_status_t ddi_timer_stop(ddi_timer_handle_t handle)
{
  ddi_timer_t *timer = (ddi_timer_t *)handle;
  osStatus status = osTimerStop(timer->id);
  return os_to_ddi_status(status);
}

ddi_status_t ddi_os_start(void)
{
  osStatus status = osKernelStart();
  return os_to_ddi_status(status);
}


#ifdef __cplusplus
}
#endif

