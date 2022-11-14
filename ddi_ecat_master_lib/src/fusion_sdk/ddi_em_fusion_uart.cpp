/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <signal.h>
#include <AtEthercat.h>
#include "ddi_debug.h"
#include "ddi_em_api.h"
#include "ddi_em_config.h"
#include "ddi_em_translate.h"
#include "ddi_em_fusion_uart_api.h"
#include "ddi_em_logging.h"
#include "ddi_em.h"
#include "ddi_em_fusion_uart.h"

// Store the EtherCAT slot and UART process data descriptor
// This information is used to map a UART logical instance (aka the 'UART handle')
// to an EtherCAT slot
typedef struct {
  uint16_t slot;
  fusion_pd_desc_t uart_pd_desc;
} uart_input_pd_mapping;

/** @struct uart_instance
 *  @brief This structure represents a single UART channel
 */
typedef struct {
  ddi_em_handle em_handle;                         /**< @brief EtherCAT Master Handle */
  ddi_es_handle es_handle;                         /**< @brief EtherCAT Slave Handle  */
  uint16_t      config_index;                      /**< @brief Configuration index (0x5nn5)  */
  uint16_t      info_index;                        /**< @brief Configuration index (0x5nn6)  */
  uint16_t      tx_index_base;                     /**< @brief Transmit index base address (0x5nn7)  */
  uint16_t      rx_index_base;                     /**< @brief Transmit index base address (0x5nnB)  */
  uart_channel  channel;                           /**< @brief Ranges from 0 to 3, inclusive */
  uint8_t       is_allocated;                      /**< @brief Is this instance in use? */
  uint8_t       control_byte;                      /**< @brief Used to retain control byte (transmit hold status) */
  uint16_t      uart_physical_channel;             /**< @brief The physical UART channel (0-63) this UART index corresponds to */
  // Event Section
  uint16_t      threshold_event_level;             /**< @brief The threshold level set by the application */
  uart_event_threshold_type threshold_event_type;  /**< @brief The threshold type - rising or falling */
  uint8_t       threshold_event_registered;        /**< @brief Is the threshold event registered? */
  uint8_t       error_event_registered;            /**< @brief Is the error event triggered? */
  uint8_t       rising_threshold_event_triggered;  /**< @brief Is the threshold event triggered? */
  uint8_t       falling_threshold_event_triggered; /**< @brief Is the threshold event triggered? */
  uint8_t       falling_rising_edge_triggered;     /**< @brief Is the threshold rising edge event triggered? */
  uint8_t       tx_overflow_triggered;             /**< @brief Tx overflow triggered */
  uint8_t       rx_overflow_triggered;             /**< @brief Rx overflow triggered */
  uint16_t      threshold_event_trigger_level;     /**< @brief The level which the UART event was triggered */
  ddi_uart_event_func *event_callback;             /**< @brief The UART event callback registered */
  uint32_t      threshold_event_flags;             /**< @brief Stores the UART threshold event flags */
  void          *event_user_data;                  /**< @brief Event callback user data */
} uart_instance;

// 64 logical UART handles
static uart_instance g_uart_instance[MAX_UART_INSTANCES];

// Maps UART physical channels to EtherCAT indices
static uart_input_pd_mapping g_uart_pd_mapping[MAX_UART_INSTANCES];

// Set the EtherCAT index of a physical UART channel
void ddi_fusion_uart_map_slot_to_channel(uint16_t slot, uint16_t uart_channel)
{
  g_uart_pd_mapping[uart_channel].slot = slot;
}

// Get the next available UART instance, generates an index from 0 to 63
static ddi_em_result get_next_uart_instance (ddi_fusion_uart_handle *handle)
{
  int count;
  for ( count = 0; count < MAX_UART_INSTANCES; count++)
  {
    if ( g_uart_instance[count].is_allocated == 0 )
    {
      // Clear the instance state and configuration data
      memset(&g_uart_instance[count], 0, sizeof(uart_instance));
      *handle = count;
      g_uart_instance[count].is_allocated = 1;
      return DDI_EM_STATUS_OK;
    }
  }
  return DDI_EM_STATUS_NO_RESOURCES;
}

// Detect Rx overflow event from the UART input process data
static void detect_rx_overflow_event (uart_instance *uart_instance_ptr, uint16_t* uart_input_pd, uint16_t uart_buffer_level)
{
  uart_event uart_event;
  if ( (*uart_input_pd & DDI_FUSION_UART_STATUS_RX_BUFFER_ALMOST_FULL) && !uart_instance_ptr->rx_overflow_triggered )
  {
    uart_instance_ptr->rx_overflow_triggered = 1;
      // Setup the event to be returned to the application
    uart_event.event = UART_EVENT_THRESHOLD;
    uart_event.uart_handle = uart_instance_ptr->channel;
    uart_event.threshold_type = UART_EVENT_THRESHOLD_RX_ALMOST_FULL;
    uart_event.threshold_level = uart_buffer_level;

    // Call the event callback
    uart_instance_ptr->event_callback(&uart_event, uart_instance_ptr->event_user_data);
  }
  else
  {
    uart_instance_ptr->rx_overflow_triggered = 0;
  }
}

// Detect Tx overflow event from the UART input process data
static void detect_tx_overflow_event (uart_instance *uart_instance_ptr, uint16_t* uart_input_pd, uint16_t uart_buffer_level)
{
  uart_event uart_event;
  if ( (*uart_input_pd & DDI_FUSION_UART_STATUS_TX_BUFFER_ALMOST_FULL) && !uart_instance_ptr->tx_overflow_triggered)
  {
    uart_instance_ptr->tx_overflow_triggered = 1;
      // Setup the event to be returned to the application
    uart_event.event = UART_EVENT_THRESHOLD;
    uart_event.uart_handle = uart_instance_ptr->channel;
    uart_event.threshold_type = UART_EVENT_THRESHOLD_TX_ALMOST_FULL;
    uart_event.threshold_level = uart_buffer_level;

    // Call the event callback
    uart_instance_ptr->event_callback(&uart_event, uart_instance_ptr->event_user_data);
  }
  else
  {
    uart_instance_ptr->tx_overflow_triggered = 0;
  }
}

// Detect falling edge threshold event from the UART buffer level
static void detect_falling_edge_event(uart_instance *uart_instance_ptr, uint16_t uart_buffer_level)
{
  uart_event uart_event;

  /* Falling edge detection
    * ---------------------------------------------------------------------------------------------------------------------
    *                         4095 <---- max buffer level
    *                          |
    *                          |   <---- 1. Buffer level = 40
    * Event threshold  ---->   |
    * (e.g. 35)                |   <---- 2. Buffer level = 30
    *                          |
    *                          0   <---- min buffer level
    *
    * On the transition from 1) to 2), the buffer level crosses above the event threshold, this triggers a rising edge event
    *
    * No further events are sent until the 'buffer level' drops below the event treshold
    */
  if ( uart_buffer_level >= uart_instance_ptr->threshold_event_level)  // Buffer level is above the threshold below
  {
    uart_instance_ptr->falling_rising_edge_triggered = 1;
  }
  else if ( uart_instance_ptr->falling_rising_edge_triggered )  // Buffer is below the threshold level
  {
      // Setup the event to be returned to the application
    uart_event.event = UART_EVENT_THRESHOLD;
    uart_event.uart_handle = uart_instance_ptr->channel;
    uart_event.threshold_type = UART_EVENT_THRESHOLD_FALLING_EDGE;
    uart_event.threshold_level = uart_buffer_level;

    // Call the event callback
    uart_instance_ptr->event_callback(&uart_event, uart_instance_ptr->event_user_data);

    // Reset the state event tracking
    uart_instance_ptr->falling_rising_edge_triggered = 0;
    uart_instance_ptr->falling_threshold_event_triggered = 1;
  }
}

// Detect rising edge threshold event from the UART buffer level
void detect_rising_edge_event(uart_instance *uart_instance_ptr, uint16_t uart_buffer_level)
{
  uart_event uart_event;
  /* Rising edge detection
    * ---------------------------------------------------------------------------------------------------------------------
    *                         4095 <---- max buffer level
    *                          |
    *                          |   <---- 2. Buffer level = 40
    * Event threshold  ---->   |
    * (e.g. 35)                |   <---- 1. Buffer level = 30
    *                          |
    *                          0   <---- min buffer level
    *
    * On the transition from 1) to 2), the buffer level crosses above the event threshold, this triggers a rising edge event
    *
    * No further events are sent until the 'buffer level' drops below the event treshold
    */
  if (uart_instance_ptr->threshold_event_flags & UART_EVENT_THRESHOLD_RISING_EDGE)
  {
    if (uart_buffer_level >= uart_instance_ptr->threshold_event_level && (!uart_instance_ptr->rising_threshold_event_triggered))
    {
      // The rising edge has been detected, set the threshold_event_triggered flag so that continuous events are not resent
      uart_instance_ptr->rising_threshold_event_triggered = 1;
      uart_instance_ptr->threshold_event_trigger_level = uart_buffer_level;

      // Setup the event to be returned to the application
      uart_event.event = UART_EVENT_THRESHOLD;
      uart_event.uart_handle = uart_instance_ptr->channel;
      uart_event.threshold_type = UART_EVENT_THRESHOLD_RISING_EDGE;
      uart_event.threshold_level = uart_buffer_level;

      // Call the event callback
      uart_instance_ptr->event_callback(&uart_event, uart_instance_ptr->event_user_data);
    }
    else if ( uart_buffer_level < uart_instance_ptr->threshold_event_level )
    {
      // Threshold is below trigger level, disable the event
      uart_instance_ptr->rising_threshold_event_triggered = 0;
    }
  }
}

// Detect error conditions
void detect_error_condition (uart_instance *uart_instance_ptr, uint16_t *uart_input_pd )
{
  uart_event uart_event;

  // Check for an error conditions
  if ( *uart_input_pd & DDI_FUSION_UART_STATUS_ERROR_CONDITION )
  {
    uart_instance_ptr->event_callback(&uart_event, uart_instance_ptr->event_user_data);

    // Setup the event to be returned to the application
    uart_event.event = UART_EVENT_ERROR;
    uart_event.uart_handle = uart_instance_ptr->channel;

    // Call the event callback
    uart_instance_ptr->event_callback(&uart_event, uart_instance_ptr->event_user_data);
  }
}

/*
 * Handle UART event detection
 * ---------------------------
 * The version 1.3 UART detection has both treshold and error events
 * UART_EVENT_THRESHOLD_RISING_EDGE events are triggered when the number of rx bytes in a buffer reaches a certain event level
 * UART_EVENT_THRESHOLD_FALLING_EDGE events are triggered when the number of rx bytes has previously exceeded a certain threshold and has dropped below
 *  the threshold events.
 * UART_EVENT_THRESHOLD_TX_ALMOST_FULL when the Fusion firmware Tx buffer is within 8 bytes of being full
 * UART_EVENT_THRESHOLD_RX_ALMOST_FULL when the Fusion firmware Rx buffer is within 8 bytes of being full
 * Error events are triggered when the the error bit (bit 12) are set in the process data payload
 */
static void handle_uart_events (uart_instance *uart_instance_ptr, fusion_pd_desc_t *uart_status_pd)
{
  uint16_t *uart_input_pd = (uint16_t *)&uart_status_pd->pd_input[uart_status_pd->byte_offset];
  uint16_t uart_buffer_level = *uart_input_pd & DDI_FUSION_UART_STATUS_RX_BYTE_MASK;

  // Handle UART threshold event detection and dispatching
  if (uart_instance_ptr->threshold_event_registered)
  {
    if (uart_instance_ptr->threshold_event_flags & UART_EVENT_THRESHOLD_RISING_EDGE) // Handle UART rising edge threshold events
    {
      detect_rising_edge_event(uart_instance_ptr, uart_buffer_level);
    }

    if (uart_instance_ptr->threshold_event_flags & UART_EVENT_THRESHOLD_FALLING_EDGE) // Handle UART falling edge threshold events
    {
      detect_falling_edge_event(uart_instance_ptr, uart_buffer_level);
    }

    if (uart_instance_ptr->threshold_event_flags & UART_EVENT_THRESHOLD_TX_ALMOST_FULL) // Handle Fusion firmware UART Tx buffer almost full
    {
      detect_tx_overflow_event(uart_instance_ptr, uart_input_pd, uart_buffer_level);
    }

    if (uart_instance_ptr->threshold_event_flags & UART_EVENT_THRESHOLD_RX_ALMOST_FULL) // Handle Fusion firmware UART Rx buffer almost full
    {
      detect_rx_overflow_event(uart_instance_ptr, uart_input_pd, uart_buffer_level);
    }

    if (uart_instance_ptr->error_event_registered) // Handle UART error event detection and dispatching
    {
      detect_error_condition(uart_instance_ptr, uart_input_pd);
    }
  }
}

// Return a UART process data descriptor pointer
fusion_pd_desc_t* ddi_fusion_uart_get_pd_desc(uint16_t uart_physical_channel)
{
  return &g_uart_pd_mapping[uart_physical_channel].uart_pd_desc;
}

// Check for UART events
ddi_em_result ddi_fusion_uart_check_for_events (ddi_em_handle em_handle)
{
  uint8_t is_event_registered = false;
  uint16_t uart_count = 0;
  uart_instance *uart_instance_ptr;
  fusion_pd_desc_t *uart_desc_ptr;

  // Check for UART events - iterate through all UART instances
  for ( uart_count = 0; uart_count < MAX_UART_INSTANCES; uart_count++ )
  {
    uart_instance_ptr = &g_uart_instance[uart_count];
    uart_desc_ptr = &g_uart_pd_mapping[g_uart_instance[uart_count].uart_physical_channel].uart_pd_desc;

    // Determine if the UART event handling is registered for this event
    is_event_registered = uart_instance_ptr->error_event_registered | uart_instance_ptr->threshold_event_registered;
    if (uart_instance_ptr->is_allocated && is_event_registered)
    {
      // Detect any new UART events or errors using the process data allocated for this channel
      handle_uart_events(uart_instance_ptr, uart_desc_ptr);
    }
  }
  return DDI_EM_STATUS_OK;
}

// Open a UART handle
ddi_em_result ddi_fusion_uart_open (ddi_em_handle em_handle, ddi_es_handle es_handle, uint16_t index, uart_channel channel,
                                      uint32_t flags, ddi_fusion_uart_handle *handle)
{
  uint uart_instance_count = 0;
  ddi_em_result result;
  ddi_fusion_uart_handle uart_handle;
  // Validate the master instance
  VALIDATE_INSTANCE(em_handle);
  // Validate the return parameter has a valid address
  if ( handle == NULL )
  {
    return DDI_EM_STATUS_INVALID_ARG;
  }

  // Validate the index contains 0x5nn5
  if ( (index & DDI_FUSION_UART_CONFIG_SDO_INDEX) != DDI_FUSION_UART_CONFIG_SDO_INDEX )
  {
    ELOG(em_handle, "UART Index should be of the form 0x5nn5, index 0x%04x was entered \n", index);
    return DDI_EM_STATUS_INVALID_ARG;
  }

  // Validate the UART channel value is between 0 and 3
  if ( (channel < UART_CH0) ||  (channel > UART_CH3) )
  {
    ELOG(em_handle, "UART Channel should be in the range 0 to 3 \n", index);
    return DDI_EM_STATUS_INVALID_ARG;
  }

  // Get the next avilable instance
  result = get_next_uart_instance (&uart_handle);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG(em_handle, "No available UART channels were detected \n");
    return result;
  }
  *handle = uart_handle;
  g_uart_instance[uart_handle].config_index  = index;     // e.g., 5005
  g_uart_instance[uart_handle].info_index    = index + 1; // e.g., 5006
  g_uart_instance[uart_handle].tx_index_base = index + 2; // e.g., 5007
  g_uart_instance[uart_handle].rx_index_base = index + 6; // e.g., 500B
  g_uart_instance[uart_handle].es_handle     = es_handle;
  g_uart_instance[uart_handle].em_handle     = em_handle;
  g_uart_instance[uart_handle].channel       = channel;

  // Map the physical channel that matches this UART instance
  for ( uart_instance_count = 0; uart_instance_count < MAX_UART_INSTANCES; uart_instance_count++)
  {
    uint16_t slot = (index / DDI_FUSION_SLOT_INCREMENT) & 0xFF; // Calculate the slot number
    if (g_uart_pd_mapping[uart_instance_count].slot == slot)
    {
      // Found an matching physical uart channel
      g_uart_instance[uart_handle].uart_physical_channel = uart_instance_count + channel;
      break;
    }
  }
  return DDI_EM_STATUS_OK;
}

// Close a UART handle
ddi_em_result ddi_fusion_uart_close (ddi_fusion_uart_handle handle)
{
  g_uart_instance[handle].is_allocated = 0;
  return DDI_EM_STATUS_OK;
}

// Close a UART handle
ddi_em_result ddi_fusion_uart_close_all_handles (void)
{
  int count = 0;
  for ( count = 0; count < MAX_UART_INSTANCES; count++)
  {
    g_uart_instance[count].is_allocated = 0;
  }
  return DDI_EM_STATUS_OK;
}

// Transmit data to a UART channel, use complete access (starting the write from subindex 0)
EM_API ddi_em_result ddi_fusion_uart_tx_data (ddi_fusion_uart_handle handle, uint8_t *source_buffer, uint32_t tx_length)
{
  ddi_em_result result;
  uint16_t *SI0;
  // Copy the transmitted data into a contiguous buffer so the write can be used staring from SI0
  uint8_t tx_data[DDI_FUSION_UART_SDO_DATA_SIZE_MAX + SIZEOF_SI0];
  uart_instance *instance;
  uint16_t tx_index;
  instance = &g_uart_instance[handle];
  if ( (tx_length > DDI_FUSION_UART_SDO_DATA_SIZE_MAX) || (source_buffer == NULL) )
  {
    return DDI_EM_STATUS_INVALID_ARG;
  }
  // Calculate the Tx index from the base tx index plus the channel number
  tx_index = instance->tx_index_base + instance->channel;
  // Set Subindex 0 to the size of the transmit
  SI0 = (uint16_t *)tx_data;
  *SI0 = tx_length;
  memcpy(&tx_data[SIZEOF_SI0], source_buffer, tx_length);
  // Write the UART Tx data
  result = ddi_em_coe_write(instance->em_handle, instance->es_handle, tx_index, 0, tx_data, tx_length + SIZEOF_SI0, UART_DEFAULT_TIMEOUT_MS, UART_COMPLETE_ACCESS);
  return result;
}

// Transmit data from a UART channel, use complete access (starting the read from subindex 0)
EM_API ddi_em_result ddi_fusion_uart_rx_data (ddi_fusion_uart_handle handle, uint8_t *dest_buffer, uint32_t *rx_length)
{
  ddi_em_result result;
  uint32_t length;
  // Copy the received data into a contiguous buffer so the amount of bytes read can be determined from SI0
  uint8_t rx_data[DDI_FUSION_UART_SDO_DATA_SIZE_MAX + SIZEOF_SI0];
  uart_instance *instance;
  uint16_t rx_index, *SI0;
  instance = &g_uart_instance[handle];
  if ( (dest_buffer == NULL) || ( rx_length == NULL ) )
  {
    return DDI_EM_STATUS_INVALID_ARG;
  }

  rx_index = instance->rx_index_base + instance->channel;
  // Read the UART Rx data
  result = ddi_em_coe_read(instance->em_handle, instance->es_handle, rx_index, 0, rx_data,
    DDI_FUSION_UART_SDO_DATA_SIZE_MAX + SIZEOF_SI0, &length, UART_DEFAULT_TIMEOUT_MS, UART_COMPLETE_ACCESS);
  if ( result == DDI_EM_STATUS_OK )
  {
    // Copy the amount of bytes back read in SI0
    SI0 = (uint16_t *)rx_data;
    // Set the received length to the value in SI0
    *rx_length = *(uint8_t *)SI0;
    // Copy the read bytes
    memcpy(dest_buffer, &rx_data[SIZEOF_SI0], *rx_length);
  }
  return result;
}

// Return a 16-bit status reading of the object at index.subindex
static ddi_em_result get_uart_status (ddi_fusion_uart_handle handle, uint16_t *status)
{
  ddi_em_result result;
  uint32_t len;
  uart_instance *instance;
  uint16_t si;
  instance = &g_uart_instance[handle];
  si = DDI_FUSION_UART_STATUS_CH0_SI + instance->channel;
  // Write the parameter selection
  result = ddi_em_coe_read(instance->em_handle, instance->es_handle, instance->info_index, si,
   (uint8_t *)status, sizeof(status), &len, UART_DEFAULT_TIMEOUT_MS, 0);
  if ( result != DDI_EM_STATUS_OK )
  {
    return result;
  }
  return result;
}

// Return a 8-bit status/control reading of the object at index.subindex
static ddi_em_result get_uart_control (ddi_fusion_uart_handle handle, uint8_t *param)
{
  ddi_em_result result;
  uint32_t len;
  uart_instance *instance;
  uint16_t si;
  instance = &g_uart_instance[handle];
  si = DDI_FUSION_UART_CONTROL_CH0_SI + instance->channel;
  // Write the parameter selection
  result = ddi_em_coe_read(instance->em_handle, instance->es_handle, instance->info_index, si, param, sizeof(param), &len, UART_DEFAULT_TIMEOUT_MS, 0);
  if ( result != DDI_EM_STATUS_OK )
  {
    return result;
  }
  return result;
}

// Set a 8-bit control paramter of the object at index.subindex
static ddi_em_result set_uart_control (ddi_fusion_uart_handle handle, uint8_t param)
{
  ddi_em_result result;
  uart_instance *instance;
  uint16_t si;
  instance = &g_uart_instance[handle];
  si = DDI_FUSION_UART_CONTROL_CH0_SI + instance->channel;
  // Write the parameter selection
  result = ddi_em_coe_write(instance->em_handle, instance->es_handle, instance->info_index, si, &param, sizeof(uint8_t), UART_DEFAULT_TIMEOUT_MS, 0);
  if ( result != DDI_EM_STATUS_OK )
  {
    return result;
  }
  return result;
}

// Set a 8-bit configuration paramter of the object at index.subindex
// This should be used with the 0x5nn5 SDO as the channel selection mechanism is used
static ddi_em_result set_uart_config (ddi_fusion_uart_handle handle, uint16_t subindex , uint8_t param)
{
  ddi_em_result result;
  uart_instance *instance;
  uint16_t config_subindex;
  instance = &g_uart_instance[handle];
  // Calculate the configuration subindex using the subindex base + channel index being indexed
  config_subindex = subindex + instance->channel * SIZEOF_5005_CHANNEL;
  // Write the parameter selection
  result = ddi_em_coe_write(instance->em_handle, instance->es_handle, instance->config_index, config_subindex, &param, sizeof(uint8_t), UART_DEFAULT_TIMEOUT_MS, 0);
  if ( result != DDI_EM_STATUS_OK )
  {
    return result;
  }
  return result;
}

// Get a 8-bit configuration paramter of the object at index.subindex
// This should be used with the 0x5nn5 SDO as the channel selection mechanism is used
static ddi_em_result get_uart_config (ddi_fusion_uart_handle handle, uint16_t subindex , uint8_t *param)
{
  uint8_t param_rx;
  uint32_t len;
  ddi_em_result result;
  uart_instance *instance;
  uint16_t config_subindex;
  if ( param == NULL )
  {
    ELOG(g_uart_instance[handle].em_handle, "Param argument NULL\n");
    return DDI_EM_STATUS_INVALID_ARG;
  }
  instance = &g_uart_instance[handle];
  // Calculate the configuration subindex using the subindex base + channel index being indexed
  config_subindex = subindex + instance->channel * SIZEOF_5005_CHANNEL;
  // Read the UART Parameter
  result = ddi_em_coe_read(instance->em_handle, instance->es_handle, instance->config_index, config_subindex, &param_rx, sizeof(param_rx), &len, UART_DEFAULT_TIMEOUT_MS, 0);
  if ( result != DDI_EM_STATUS_OK )
  {
    return result;
  }
  *param = param_rx;
  return result;
}

// Set a UART baud rate
EM_API ddi_em_result ddi_fusion_uart_set_baud (ddi_fusion_uart_handle handle, uart_baud baud)
{
  ddi_em_result result;
  result = set_uart_config(handle, DDI_FUSION_UART_CONFIG_BAUD_SI, (uint8_t)baud);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG(g_uart_instance[handle].em_handle, "Error setting baud rate: %s \n", ddi_em_get_error_string(result));
  }
  return result;
}

// Return the UART baud rate
EM_API ddi_em_result ddi_fusion_uart_get_baud (ddi_fusion_uart_handle handle, uart_baud *baud)
{
  ddi_em_result result;
  if ( baud == NULL )
  {
    ELOG(g_uart_instance[handle].em_handle, "ddi_fusion_uart_get_baud: baud rate NULL\n");
    return DDI_EM_STATUS_INVALID_ARG;
  }
  result = get_uart_config(handle, DDI_FUSION_UART_CONFIG_BAUD_SI, (uint8_t*)baud);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG(g_uart_instance[handle].em_handle, "Error getting baud rate: %s \n", ddi_em_get_error_string(result));
  }
  return result;
}

// Set the UART interface
EM_API ddi_em_result ddi_fusion_uart_set_interface (ddi_fusion_uart_handle handle, uart_interface interface)
{
  ddi_em_result result;
  result = set_uart_config(handle, DDI_FUSION_UART_CONFIG_INTERFACE_SI, (uint8_t)interface);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG(g_uart_instance[handle].em_handle, "Error setting UART interface: %s \n", ddi_em_get_error_string(result));
  }
  return result;
}

// Get the UART interface setting
EM_API ddi_em_result ddi_fusion_uart_get_interface (ddi_fusion_uart_handle handle, uart_interface *interface)
{
  ddi_em_result result;
  if ( interface == NULL )
  {
    ELOG(g_uart_instance[handle].em_handle, "ddi_fusion_uart_get_interface: Argument NULL\n");
    return DDI_EM_STATUS_INVALID_ARG;
  }
  result = get_uart_config(handle, DDI_FUSION_UART_CONFIG_INTERFACE_SI, (uint8_t*)interface);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG(g_uart_instance[handle].em_handle, "Error getting UART interface: %s \n", ddi_em_get_error_string(result));
  }
  return result;
}

// Set parity mode
EM_API ddi_em_result ddi_fusion_uart_set_parity_mode (ddi_fusion_uart_handle handle, uart_parity parity)
{
  ddi_em_result result;
  result = set_uart_config(handle, DDI_FUSION_UART_CONFIG_PARITY_SI, (uint8_t)parity);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG(g_uart_instance[handle].em_handle, "Error setting UART parity: %s \n", ddi_em_get_error_string(result));
  }
  return result;
}

// Return the selected parity mode for a UART channel
EM_API ddi_em_result ddi_fusion_uart_get_parity_mode (ddi_fusion_uart_handle handle, uart_parity *parity)
{
  ddi_em_result result;
  result = get_uart_config(handle, DDI_FUSION_UART_CONFIG_PARITY_SI, (uint8_t*)parity);
  if ( parity == NULL )
  {
    ELOG(g_uart_instance[handle].em_handle, "ddi_fusion_uart_get_parity_mode: Argument NULL\n");
    return DDI_EM_STATUS_INVALID_ARG;
  }
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG(g_uart_instance[handle].em_handle, "Error getting UART parity: %s \n", ddi_em_get_error_string(result));
  }
  return result;
}

// Set stop bits for a UART channel
EM_API ddi_em_result ddi_fusion_uart_set_stop_bits (ddi_fusion_uart_handle handle, uart_stop_bits stop_bits)
{
  ddi_em_result result;
  result = set_uart_config(handle, DDI_FUSION_UART_CONFIG_STOP_BITS_SI, (uint8_t)stop_bits);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG(g_uart_instance[handle].em_handle, "Error setting UART interface: %s \n", ddi_em_get_error_string(result));
  }
  return result;
}

// Return stop bits setting for a UART channel
EM_API ddi_em_result ddi_fusion_uart_get_stop_bits (ddi_fusion_uart_handle handle, uart_stop_bits *stop_bits)
{
  ddi_em_result result;
  if ( stop_bits == NULL )
  {
    ELOG(g_uart_instance[handle].em_handle, "ddi_fusion_uart_get_stop_bits: Argument NULL\n");
    return DDI_EM_STATUS_INVALID_ARG;
  }
  result = get_uart_config(handle, DDI_FUSION_UART_CONFIG_STOP_BITS_SI, (uint8_t*)stop_bits);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG(g_uart_instance[handle].em_handle, "Error getting UART parity: %s \n", ddi_em_get_error_string(result));
  }
  return result;
}

// Set the data bits for a UART channel
EM_API ddi_em_result ddi_fusion_uart_set_data_bits (ddi_fusion_uart_handle handle, uart_data_bits data_bits)
{
  ddi_em_result result;
  result = set_uart_config(handle, DDI_FUSION_UART_CONFIG_DATA_BITS_SI, (uint8_t)data_bits);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG(g_uart_instance[handle].em_handle, "Error setting UART data bits: %s \n", ddi_em_get_error_string(result));
  }
  return result;
}

// Get the data bits for a UART channel
EM_API ddi_em_result ddi_fusion_uart_get_data_bits (ddi_fusion_uart_handle handle, uart_data_bits *data_bits )
{
  ddi_em_result result;
  if ( data_bits == NULL )
  {
    ELOG(g_uart_instance[handle].em_handle, "ddi_fusion_uart_get_data_bits: Argument NULL\n");
    return DDI_EM_STATUS_INVALID_ARG;
  }
  result = get_uart_config(handle, DDI_FUSION_UART_CONFIG_DATA_BITS_SI, (uint8_t*)data_bits);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG(g_uart_instance[handle].em_handle, "Error getting UART data bits: %s \n", ddi_em_get_error_string(result));
  }
  return result;
}

// Set the flow control setting for a UART channel
EM_API ddi_em_result ddi_fusion_uart_set_flow_control (ddi_fusion_uart_handle handle, uart_flow_control flow_control)
{
  ddi_em_result result;
  result = set_uart_config(handle, DDI_FUSION_UART_CONFIG_FLOW_CONTROL_SI, (uint8_t)flow_control);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG(g_uart_instance[handle].em_handle, "Error setting UART flow control: %s \n", ddi_em_get_error_string(result));
  }
  return result;
}

// Get the flow control setting for a UART channel
EM_API ddi_em_result ddi_fusion_uart_get_flow_control (ddi_fusion_uart_handle handle, uart_flow_control *flow_control)
{
  ddi_em_result result;
  if ( flow_control == NULL )
  {
    ELOG(g_uart_instance[handle].em_handle, "ddi_fusion_uart_get_flow_control: Argument NULL\n");
    return DDI_EM_STATUS_INVALID_ARG;
  }
  result = get_uart_config(handle, DDI_FUSION_UART_CONFIG_FLOW_CONTROL_SI, (uint8_t*)flow_control);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG(g_uart_instance[handle].em_handle, "Error getting flow control: %s \n", ddi_em_get_error_string(result));
  }
  return result;
}

// Flush a UART channel (both Tx and Rx buffers)
EM_API ddi_em_result ddi_fusion_uart_channel_flush (ddi_fusion_uart_handle handle)
{
  ddi_em_result result;
  // Don't update the control byte for the flush message, the flush is a one-time only message
  // that doesn't need to be retained in the UART channel instance
  result = set_uart_control(handle, g_uart_instance[handle].control_byte | DDI_FUSION_UART_CONTROL_FLUSH);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG(g_uart_instance[handle].em_handle, "Error flushing UART channel: %s \n", ddi_em_get_error_string(result));
  }
  
  return result;
}

// Set transmit hold for a UART channel
EM_API ddi_em_result ddi_fusion_uart_set_transmit_hold (ddi_fusion_uart_handle handle)
{
  ddi_em_result result;
  // Update the control byte to indicate the hold message, this state needs to be retained in the UART channel instance
  g_uart_instance[handle].control_byte |= DDI_FUSION_UART_CONTROL_HOLD;
  // Send the control message to the UART channel
  result = set_uart_control(handle, g_uart_instance[handle].control_byte);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG(g_uart_instance[handle].em_handle, "Error setting UART transmit hold: %s \n", ddi_em_get_error_string(result));
  }

  return result;
}

// Enables transmit for a UART channel
EM_API ddi_em_result ddi_fusion_uart_enable_transmit (ddi_fusion_uart_handle handle)
{
  ddi_em_result result;
  // Update the control byte to indicate the hold message, this state needs to be retained in the EtherCAT master SDK
  g_uart_instance[handle].control_byte &= ~DDI_FUSION_UART_CONTROL_HOLD;
  // Send the control message to the UART channel
  result = set_uart_control(handle, g_uart_instance[handle].control_byte);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG(g_uart_instance[handle].em_handle, "Error setting UART transmit hold: %s \n", ddi_em_get_error_string(result));
  }
  return result;
}

// Enables transmit hold for a UART channel
EM_API ddi_em_result ddi_fusion_uart_get_transmit_hold_status (ddi_fusion_uart_handle handle, uint8_t *is_hold_enabled)
{
  ddi_em_result result;
  result = get_uart_control(handle, is_hold_enabled);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG(g_uart_instance[handle].em_handle, "Error getting UART flow control: %s \n", ddi_em_get_error_string(result));
  }
  return result;
}

// Retreives the error details for a UART channel
EM_API ddi_em_result ddi_fusion_uart_get_error_status (ddi_fusion_uart_handle handle, uint8_t *error_details)
{
  ddi_em_result result;
  result = get_uart_control(handle, error_details);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG(g_uart_instance[handle].em_handle, "Error getting UART error details: %s \n", ddi_em_get_error_string(result));
  }
  return result;
}

// Retreives the Rx bytes available for a UART channel
EM_API ddi_em_result ddi_fusion_uart_get_rx_bytes_avail (ddi_fusion_uart_handle handle, uint16_t *bytes_avail)
{
  ddi_em_result result;
  uint16_t status;
  result = get_uart_status(handle, &status);
  if ( result != DDI_EM_STATUS_OK )
  {
    ELOG(g_uart_instance[handle].em_handle, "Error setting UART flow control: %s \n", ddi_em_get_error_string(result));
  }
  *bytes_avail = status & DDI_FUSION_UART_STATUS_RX_BYTE_MASK;
  return result;
}

// Registers a UART event handler
EM_API ddi_em_result ddi_fusion_uart_register_event (ddi_fusion_uart_handle handle, ddi_uart_event_func *callback, void *user_data)
{
  uart_instance *uart_instance_ptr;
  ddi_fusion_sdk_handle sdk_handle;

  VALIDATE_UART_INSTANCE(handle);

  // Enable the UART channel in the is_uart_event_registered master field
  sdk_handle = get_fusion_sdk_handle(g_uart_instance[handle].em_handle, g_uart_instance[handle].es_handle);
  ddi_em_fusion_set_registered_uart_events(g_uart_instance[handle].em_handle, sdk_handle, 1 << handle);

  // Register callback function and corresponding user data
  uart_instance_ptr = &g_uart_instance[handle];
  uart_instance_ptr->event_callback = callback;
  uart_instance_ptr->event_user_data = user_data;

  return DDI_EM_STATUS_OK;
}

// Enable a threshold event on the rx buffer, the UART event handler will be notified when the UART
// channel Rx bytes exceeds the value in the threshold field
EM_API ddi_em_result ddi_fusion_uart_enable_threshold_event (ddi_fusion_uart_handle handle, uint32_t event_flags, uint16_t threshold)
{
  uart_instance *uart_instance_ptr;
  VALIDATE_UART_INSTANCE(handle);
  uart_instance_ptr = &g_uart_instance[handle];
  uart_instance_ptr->threshold_event_registered = DDI_EM_TRUE;
  uart_instance_ptr->threshold_event_level = threshold;
  uart_instance_ptr->threshold_event_flags |= (uint32_t)event_flags;

  return DDI_EM_STATUS_OK;
}

// Disable a threshold event
EM_API ddi_em_result ddi_fusion_uart_disable_threshold_event (ddi_fusion_uart_handle handle, uint32_t event_flags)
{
  uart_instance *uart_instance_ptr;

  VALIDATE_UART_INSTANCE(handle);

  uart_instance_ptr = &g_uart_instance[handle];
  uart_instance_ptr->threshold_event_registered = DDI_EM_FALSE;
  uart_instance_ptr->threshold_event_flags &= ~event_flags;
  return DDI_EM_STATUS_OK;
}

// Enable an error event on the rx buffer, the UART event handler will be notified when the UART
// channel Rx bytes exceeds the value in the threshold field
EM_API ddi_em_result ddi_fusion_uart_enable_error_event (ddi_fusion_uart_handle handle)
{
  uart_instance *uart_instance_ptr;

  VALIDATE_UART_INSTANCE(handle);

  uart_instance_ptr = &g_uart_instance[handle];
  uart_instance_ptr->error_event_registered = DDI_EM_TRUE;
  return DDI_EM_STATUS_OK;
}

// Disable an error event
EM_API ddi_em_result ddi_fusion_uart_disable_error_event (ddi_fusion_uart_handle handle)
{
  uart_instance *uart_instance_ptr;

  VALIDATE_UART_INSTANCE(handle);

  uart_instance_ptr = &g_uart_instance[handle];
  uart_instance_ptr->error_event_registered = DDI_EM_FALSE;
  return DDI_EM_STATUS_OK;
}
