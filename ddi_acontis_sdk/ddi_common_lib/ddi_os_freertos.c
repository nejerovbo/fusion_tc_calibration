

#include "ddi_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdlib.h>

typedef struct
{
  TaskHandle_t tid;
  uint32_t delete_on_exit   : 1;
  uint32_t exited           : 1;
  int retval;
} ddi_thread_t;

typedef struct
{
  ddi_thread_handle_t thread;
  volatile uint32_t value;
} ddi_event_t;

typedef struct
{
  QueueSetHandle_t handle;
  uint32_t depth;
  uint32_t size;
  uint8_t *buffers;
  int32_t idx_in;
  int32_t idx_out;
  int32_t t_full;
} ddi_queue_t;

static int is_interrupt_context()
{
#if   defined ( __CC_ARM )
  register uint32_t __regIPSR          __asm("ipsr");
  return (__regIPSR != 0);
#elif defined ( __GNUC__ )
  uint32_t result;
  __asm volatile ("MRS %0, ipsr" : "=r" (result) );
  return (result != 0);
#else
#error need to define is_interrupt_context for this compiler
#endif
}

/*
 * Threads
 */
ddi_os_result_t ddi_thread_create(ddi_thread_handle_t *phandle, int priority, uint32_t stack_size, const char *thread_name, ddi_thread_entry_t *entry, void *user_data)
{
  ddi_thread_t *thread = NULL;
  BaseType_t status;
  TaskHandle_t tid;

  // FreeRTOS priorities increase from tskIDLE_PRIORITY = 0 to highest priority.
  // we may want to use 0 as the highest ddi os priority and decrease priority as the number increases.
  // if so do the convertion here:

  status = xTaskCreate((TaskFunction_t)entry, thread_name, stack_size, user_data, (UBaseType_t)priority, &tid);
  if (status == pdPASS)
  {
    thread = (ddi_thread_t *)malloc(sizeof(ddi_thread_t));
    thread->tid = tid;
    status = ddi_os_result_ok;
  }
  else
  {
    status = ddi_os_result_no_resources;
  }

  *phandle = (ddi_thread_handle_t *)thread;

  return status;
}

ddi_os_result_t ddi_thread_join(ddi_thread_handle_t handle, int *p_retval)
{
  ddi_thread_t *thread = (ddi_thread_t *)handle;
  if (thread == NULL)
    return ddi_os_result_param_err;

  // Block until the thread's eTaskState is eDeleted..
  // which for FreeRTOS means the thread function returned without calling vTaskDelete
  while (eTaskGetState(thread->tid) != eDeleted)
  {
    ddi_delay(1); // sorry this is ugly
  }

  // Then delete the thread to free the TCB
  vTaskDelete(thread->tid);
  thread->tid = 0;
  *p_retval = thread->retval;
  free(thread);

  return ddi_os_result_ok;
}

// set the thread handle to delete the thread when "ddi_thread_exit" is called
ddi_os_result_t ddi_thread_detach(ddi_thread_handle_t handle)
{
  ddi_thread_t *thread = (ddi_thread_t *)handle;
  if (thread == NULL)
    return ddi_os_result_param_err;

  thread->delete_on_exit = 1;

  return ddi_os_result_ok;
}

void ddi_thread_exit(ddi_thread_handle_t handle, int retval)
{
  ddi_thread_t *thread = (ddi_thread_t *)handle;
  if (thread == NULL)
    return;

  if (thread->delete_on_exit)
  {
    vTaskDelete(thread->tid);
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

ddi_os_result_t ddi_mutex_create(ddi_mutex_handle_t *phandle)
{
  QueueHandle_t queue;

  queue = xSemaphoreCreateBinary();
  *phandle = (ddi_mutex_handle_t *)queue;

  return ddi_os_result_ok;
}

ddi_os_result_t ddi_mutex_lock(ddi_mutex_handle_t handle)
{
  xSemaphoreTake(handle, portMAX_DELAY); // block indefinitely
  return ddi_os_result_ok;
}

ddi_os_result_t ddi_mutex_unlock(ddi_mutex_handle_t handle)
{
  ddi_os_result_t status = ddi_os_result_ok;
  if (is_interrupt_context())
  {
    BaseType_t taskWoken = pdFALSE;
    if (xSemaphoreGiveFromISR(handle, &taskWoken) != pdTRUE)
      status = ddi_os_result_not_ready;
    portEND_SWITCHING_ISR(taskWoken);
  }
  else
  {
    if (xSemaphoreGive(handle) != pdTRUE)
      status = ddi_os_result_not_ready;
  }
  return status;
}

/*
 * Semaphores
 */

ddi_os_result_t ddi_semaphore_create(ddi_semaphore_handle_t *phandle, uint32_t max_count, uint32_t initial_value)
{
  xSemaphoreCreateCounting(initial_value, initial_value);
  return ddi_os_result_ok;
}
  
ddi_os_result_t ddi_semaphore_decrement(ddi_semaphore_handle_t handle)
{
  if (xSemaphoreTake((QueueHandle_t)handle, portMAX_DELAY) == pdTRUE)
    return ddi_os_result_ok;
  return ddi_os_result_timeout;
}

ddi_os_result_t ddi_semaphore_increment(ddi_semaphore_handle_t handle)
{
  if (xSemaphoreGive((QueueHandle_t)handle) == pdTRUE)
    return ddi_os_result_ok;
  return ddi_os_result_timeout;
}

/*
 * Events
 */

ddi_os_result_t ddi_event_create(ddi_event_handle_t *phandle, ddi_thread_handle_t thread)
{
  ddi_event_t *event = (ddi_event_t *)malloc(sizeof(ddi_event_t));
  event->thread = thread;
  event->value = 0;
  *phandle = (ddi_event_handle_t *)event;
  return ddi_os_result_ok;
}

ddi_os_result_t ddi_event_signal(ddi_event_handle_t handle, uint32_t value)
{
  ddi_event_t *event = (ddi_event_t *)handle;
  ddi_thread_t *thread = (ddi_thread_t *)event->thread;

  __atomic_or_fetch(&event->value, value, __ATOMIC_SEQ_CST);
  if (is_interrupt_context())
  {
    BaseType_t taskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(thread->tid, &taskWoken);
    portEND_SWITCHING_ISR(taskWoken);
  }
  else
  {
    xTaskNotifyGive(thread->tid);
  }
  return ddi_os_result_ok;
}

uint32_t ddi_event_wait(ddi_event_handle_t handle)
{
  ddi_event_t *event = (ddi_event_t *)handle;
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

  // atomically read the event value and reset it to 0
  return __atomic_fetch_and(&event->value, 0, __ATOMIC_SEQ_CST);
}

/*
 * Queues
 */

ddi_os_result_t ddi_queue_create(ddi_queue_handle_t *phandle, uint32_t item_size, uint32_t depth)
{
  *phandle = (ddi_queue_handle_t)xQueueCreate(depth, item_size);
  return ddi_os_result_ok;
}

ddi_os_result_t ddi_queue_send(ddi_queue_handle_t handle, void *message)
{
  ddi_os_result_t status = ddi_os_result_queue_full;
  if (is_interrupt_context())
  {
    BaseType_t taskWoken = pdFALSE;
    if (xQueueSendFromISR((QueueHandle_t *)handle, message, &taskWoken) == pdTRUE)
    {
      status = ddi_os_result_ok;
      portEND_SWITCHING_ISR(taskWoken);
    }
  }
  else
  {
    if (xQueueSend((QueueHandle_t *)handle, message, portMAX_DELAY) == pdTRUE)
      status = ddi_os_result_ok;
  }
  return status;
}

ddi_os_result_t ddi_queue_receive(ddi_queue_handle_t handle, void *message)
{
  if (xQueueReceive((QueueHandle_t *)handle, message, portMAX_DELAY) != pdTRUE)
    return ddi_os_result_timeout;
  return ddi_os_result_ok;
}

/* Critical Section
 *
 */

uint32_t ddi_enter_critical(void)
{
  uint32_t mask = 0;
  if (is_interrupt_context())
  {
    mask = taskENTER_CRITICAL_FROM_ISR(); // ulPortRaiseBASEPRI
  }
  else
  {
    taskENTER_CRITICAL(); // vPortRaiseBASEPRI
  }
  return mask;
}

void ddi_exit_critical(uint32_t mask)
{
  if (is_interrupt_context())
  {
    taskEXIT_CRITICAL_FROM_ISR(mask); // vPortSetBASEPRI(x)
  }
  else
  {
    taskEXIT_CRITICAL(); // vPortSetBASEPRI(0)
  }
}

/*
 * Delay
 */

ddi_os_result_t ddi_delay(int milliseconds)
{
  TickType_t ticks = milliseconds / portTICK_PERIOD_MS;
    
  vTaskDelay(ticks ? ticks : 1);          /* Minimum delay = 1 tick */

  return ddi_os_result_ok;
}

static int _priv_enqueue(ddi_queue_t *queue)
{
  int idx;
  uint32_t mask = ddi_enter_critical();
  int t = queue->idx_out - queue->idx_in;
  if ((t == 1) || (t == queue->t_full)) // full
  {
    ddi_exit_critical(mask);
    return -1;
  }

  queue->idx_in = (queue->idx_in + 1) % queue->depth;
  idx = queue->idx_in;
  ddi_exit_critical(mask);
  return idx;
}

static int _priv_dequeue(ddi_queue_t *queue)
{
  int idx;
  uint32_t mask = ddi_enter_critical();
  if (queue->idx_out == queue->idx_in) // empty
  {
    ddi_exit_critical(mask);
    return -1;
  }
  queue->idx_out = (queue->idx_out + 1) % queue->depth;
  idx = queue->idx_out;
  ddi_exit_critical(mask);
  return idx;
}

ddi_os_result_t ddi_buffer_queue_create(ddi_buffer_queue_handle_t *phandle, uint32_t item_size, uint32_t depth)
{
  ddi_queue_t *queue = (ddi_queue_t *)malloc(sizeof(ddi_queue_t));

  queue->handle = xQueueCreate(depth, sizeof(uint32_t));
  queue->depth = depth;
  queue->size = item_size;
  queue->buffers = malloc(sizeof(uint8_t) * depth * item_size);
  queue->idx_in = 0;
  queue->idx_out = 0;
  queue->t_full =  -((int)depth - 1);

  *phandle = (ddi_queue_handle_t)queue;
  return ddi_os_result_ok;
}

ddi_os_result_t ddi_buffer_queue_reserve(ddi_buffer_queue_handle_t handle, void **pbuffer, uint32_t *psize)
{
  ddi_queue_t *queue = (ddi_queue_t *)handle;
  if (pbuffer == NULL)
    return ddi_os_result_param_err;

  *pbuffer = queue->buffers + (queue->idx_in * queue->size);
  if (psize)
      *psize = queue->size;

  return ddi_os_result_ok;
}

ddi_os_result_t ddi_buffer_queue_send(ddi_buffer_queue_handle_t handle)
{
  ddi_os_result_t status = ddi_os_result_queue_full;
  ddi_queue_t *queue = (ddi_queue_t *)handle;
  int32_t idx = _priv_enqueue(queue);
  if (idx < 0)
    return ddi_os_result_queue_full;

  if (is_interrupt_context())
  {
    BaseType_t taskWoken = pdFALSE;
    if (xQueueSendFromISR(queue->handle, &idx, &taskWoken) == pdTRUE)
    {
      status = ddi_os_result_ok;
      portEND_SWITCHING_ISR(taskWoken);
    }
  }
  else
  {
    if (xQueueSend(queue->handle, &idx, portMAX_DELAY) == pdTRUE)
      status = ddi_os_result_ok;
  }
  return status;
}

ddi_os_result_t ddi_buffer_queue_receive(ddi_buffer_queue_handle_t handle, void **pbuffer)
{
  ddi_queue_t *queue = (ddi_queue_t *)handle;
  uint32_t idx;

  // Get the last element from the queue - but do not remove it yet
  if (xQueuePeek(queue->handle, &idx, portMAX_DELAY) != pdTRUE)
    return ddi_os_result_timeout;

  *pbuffer = queue->buffers + (idx * queue->size);
  return ddi_os_result_ok;
}

ddi_os_result_t ddi_buffer_queue_release(ddi_buffer_queue_handle_t handle)
{
  ddi_queue_t *queue = (ddi_queue_t *)handle;
  uint32_t ignored;

  // Actually remove the last element from the queue
  if (xQueueReceive(queue->handle, &ignored, portMAX_DELAY) != pdTRUE)
    return ddi_os_result_timeout;
  _priv_dequeue(queue);

  return ddi_os_result_ok;
}


#ifdef __cplusplus
}
#endif

