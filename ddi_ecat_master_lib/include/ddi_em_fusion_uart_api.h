/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

#ifndef DDI_UART_LIB_H
#define DDI_UART_LIB_H

// This file provides UART library functionality

/// @file ddi_em_fusion_uart_api.h

#include <stdint.h>
#include "ddi_em_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @var MAX_NUMBER_UART_ETHERCAT_MODULES
  @brief Defines the maximum number of Fusion UART EtherCAT modules
*/
#define MAX_NUMBER_UART_ETHERCAT_MODULES      16

/*! @var MAX_NUMBER_UART_PER_MODULE
  @brief Defines the maximum numbers of channels supported per UART channel
*/
#define MAX_NUMBER_UART_PER_MODULE             4

/*! @var DDI_FUSION_UART_ETHERCAT_MODULE
  @brief Defines the UART module to be opened by ddi_fusion_uart_open()
  The EtherCAT module number of the UART channel can be passed to this macro.
  The value produced by this macro can be used as the index argument to ddi_fusion_uart_open()
*/
#define DDI_FUSION_UART_ETHERCAT_MODULE(n)     (0x5005 + (DDI_FUSION_SLOT_INCREMENT * (n-1)))

/*! @var DDI_FUSION_UART_CONFIG_SDO_INDEX
  @brief Defines the UART configuration index
*/
#define DDI_FUSION_UART_CONFIG_SDO_INDEX       0x5005
#define DDI_FUSION_UART_CONFIG_INTERFACE_SI    1
#define DDI_FUSION_UART_CONFIG_BAUD_SI         2
#define DDI_FUSION_UART_CONFIG_PARITY_SI       3
#define DDI_FUSION_UART_CONFIG_DATA_BITS_SI    4
#define DDI_FUSION_UART_CONFIG_STOP_BITS_SI    5
#define DDI_FUSION_UART_CONFIG_FLOW_CONTROL_SI 6

/*! @var DDI_FUSION_UART_INFO_SDO_INDEX
  @brief Defines the UART information (control + status) index
*/
#define DDI_FUSION_UART_INFO_SDO_INDEX         0x5006
#define DDI_FUSION_UART_CONTROL_CH0_SI         1
#define DDI_FUSION_UART_CONTROL_CH1_SI         2
#define DDI_FUSION_UART_CONTROL_CH2_SI         3
#define DDI_FUSION_UART_CONTROL_CH3_SI         4
#define DDI_FUSION_UART_STATUS_CH0_SI          5
#define DDI_FUSION_UART_STATUS_CH1_SI          6
#define DDI_FUSION_UART_STATUS_CH2_SI          7
#define DDI_FUSION_UART_STATUS_CH3_SI          8
#define DDI_FUSION_UART_ERR_DETAILS_CH0_SI     9
#define DDI_FUSION_UART_ERR_DETAILS_CH1_SI     0xA
#define DDI_FUSION_UART_ERR_DETAILS_CH2_SI     0xB
#define DDI_FUSION_UART_ERR_DETAILS_CH3_SI     0xC

/*! @var DDI_FUSION_UART_SDO_TX_CH0
  @brief Defines the UART Transmit SDO value for Channel 0
*/
#define DDI_FUSION_UART_SDO_TX_CH0            0x5007

/*! @var DDI_FUSION_UART_SDO_TX_CH1
  @brief Defines the UART Transmit SDO value for Channel 1
*/
#define DDI_FUSION_UART_SDO_TX_CH1            0x5008

/*! @var DDI_FUSION_UART_SDO_TX_CH2
  @brief Defines the UART Transmit SDO value for Channel 2
*/
#define DDI_FUSION_UART_SDO_TX_CH2            0x5009

/*! @var DDI_FUSION_UART_SDO_TX_CH3
  @brief Defines the UART Transmit SDO value for Channel 3
*/
#define DDI_FUSION_UART_SDO_TX_CH3            0x500A

/*! @var DDI_FUSION_UART_SDO_RX_CH0
  @brief Defines the UART Transmit SDO value for Channel 0
*/
#define DDI_FUSION_UART_SDO_RX_CH0            0x500B

/*! @var DDI_FUSION_UART_SDO_RX_CH1
  @brief Defines the UART Transmit SDO value for Channel 1
*/
#define DDI_FUSION_UART_SDO_RX_CH1            0x500C

/*! @var DDI_FUSION_UART_SDO_RX_CH2
  @brief Defines the UART Transmit SDO value for Channel 2
*/
#define DDI_FUSION_UART_SDO_RX_CH2            0x500D

/*! @var DDI_FUSION_UART_SDO_RX_CH3
  @brief Defines the UART Transmit SDO value for Channel 3
*/
#define DDI_FUSION_UART_SDO_RX_CH3            0x500E

/*! @var DDI_FUSION_UART_TXPDO_ENTRY
  @brief Defines the UART TxPDO Entry Index type
*/
#define DDI_FUSION_UART_TXPDO_ENTRY_TYPE      0x600A

/*! @var DDI_FUSION_UART_SDO_DATA_SIZE_MAX
  @brief Defines the maximum UART Transmit SDO size for any UART channel
*/
#define DDI_FUSION_UART_SDO_DATA_SIZE_MAX     255

// Control messages
#define DDI_FUSION_UART_CONTROL_HOLD           (1<<0)
#define DDI_FUSION_UART_CONTROL_FLUSH          (1<<1)

// Status fields
#define DDI_FUSION_UART_STATUS_CTS                   (1<<0)
#define DDI_FUSION_UART_STATUS_RX_BUFFER_ALMOST_FULL (1<<15)
#define DDI_FUSION_UART_STATUS_TX_BUFFER_ALMOST_FULL (1<<14)
#define DDI_FUSION_UART_STATUS_ERROR_CONDITION       (1<<13)
// Rx Bytes available
#define DDI_FUSION_UART_STATUS_RX_BYTE_MASK          (0xFFF)

// This is the UART input process status (TxPDO)
typedef struct __attribute__((packed)) {
  uint16_t status[MAX_NUMBER_UART_PER_MODULE];
} ddi_fusion_uart_pd_in;

/*! @enum uart_channel
  @brief UART channel enumeration

  This enumeration enumerates the 4 different UART channels available
  on the DDI UART slot-card EtherCAT module
*/
typedef enum {
  UART_CH0 = 0, /**< @brief UART Channel 0 */
  UART_CH1 = 1, /**< @brief UART Channel 1 */
  UART_CH2 = 2, /**< @brief UART Channel 2 */
  UART_CH3 = 3  /**< @brief UART Channel 3 */
} uart_channel;

/*! @var ddi_fusion_uart_handle
  @brief Defines the UART handle type
*/
typedef int32_t ddi_fusion_uart_handle;

/** ddi_fusion_uart_open
 @brief Opens a UART instance handle
 This function opens a UART instance handle. This UART instance handle will be used in other UART calls
 @param[in] em_handle The DDI ECAT master initialization parameters @see ddi_em_init_params
 @param[in] es_handle The EtherCAT master slave instance
 @param[in] module_index The UART EtherCAT module index
 @param[in] channel The UART channel value @see ddi_uart_channel
 @param[in] uart_flags The UART flags, currently reserved
 @param[out] handle The UART handle returned by open
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_open (ddi_em_handle em_handle, ddi_es_handle es_handle, uint16_t module_index, uart_channel channel,
                                      uint32_t flags, ddi_fusion_uart_handle *handle);

/** ddi_fusion_uart_close
 @brief Closes a UART instance handle
 This function closes a UART instance handle.
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_close (ddi_fusion_uart_handle handle);

/** ddi_fusion_uart_tx_data
 @brief Send data to the uart channel represented by handle
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @param[in] source_buffer The EtherCAT UART source buffer location
 @param[in] tx_length The transfer length
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_tx_data (ddi_fusion_uart_handle handle, uint8_t *source_buffer, uint32_t tx_length);

/** ddi_fusion_uart_rx_data
 @brief Receive up to 255 bytes from the uart channel represented by handle
 @param[in]  handle The UART handle opened by ddi_fusion_uart_open
 @param[out] dest_buffer The EtherCAT UART destination buffer location. This buffer must be at least 255 bytes
 @param[out] rx_length The received data byte length
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_rx_data (ddi_fusion_uart_handle handle, uint8_t *dest_buffer, uint32_t *rx_length);

/*! @enum uart_baud
  @brief Represents the baud rate modes available in the Fusion UART subsystem
*/
typedef enum {
  UART_BAUD_1200   = 0, /**< @brief UART 1200 bps baud rate */
  UART_BAUD_2400   = 1, /**< @brief UART 2400 bps baud rate */
  UART_BAUD_4800   = 2, /**< @brief UART 4800 bps baud rate */
  UART_BAUD_9600   = 3, /**< @brief UART 9600 bps baud rate */
  UART_BAUD_19200  = 4, /**< @brief UART 19200 bps baud rate */
  UART_BAUD_31200  = 5, /**< @brief UART 31200 bps baud rate */
  UART_BAUD_38400  = 6, /**< @brief UART 38400 bps baud rate */
  UART_BAUD_57600  = 7, /**< @brief UART 57600 bps baud rate */
  UART_BAUD_115200 = 8  /**< @brief UART 115200 bps baud rate */
} uart_baud;

/** ddi_fusion_uart_set_baud
 @brief Set the baud rate of the UART channel represented by handle
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @param[in] baud The UART baud rate to set @see uart_baud
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_set_baud (ddi_fusion_uart_handle handle, uart_baud baud);

/** ddi_fusion_uart_get_baud
 @brief Get the baud rate of the UART channel represented by handle
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @param[out] baud The UART baud rate currently set @see uart_baud
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_get_baud (ddi_fusion_uart_handle handle, uart_baud *baud);

/*! @enum uart_interface
  @brief Represents the physical interface modes available in the Fusion UART subsystem
*/
typedef enum {
  UART_INTERFACE_RS232  = 0, /**< @brief RS-232 mode */
  UART_INTERFACE_RS485  = 1, /**< @brief RS-485 half-duplex Mode */
  UART_INTERFACE_RS485_WITH_TERMINATION_RESISTOR  = 2 /**< @brief RS-485 half-duplex mode with termination resistor */
} uart_interface;

/** ddi_fusion_uart_set_interface
 @brief Set the interface (RS-232, RS-485, RS-485 with Termination Resistor) of the given UART channel
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @param[in] interface The UART interface to set @see uart_interface
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_set_interface (ddi_fusion_uart_handle handle, uart_interface interface);

/** ddi_fusion_uart_get_interface
 @brief Get the interface (RS-232, RS-485, RS-485 with Termination Resistor) of the given UART channel
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @param[out] interface The UART interface to get @see uart_interface
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_get_interface (ddi_fusion_uart_handle handle, uart_interface *interface);

/*! @enum uart_parity
  @brief Represents the physical interface modes available in the Fusion UART subsystem
*/
typedef enum {
  UART_PARITY_NONE = 0, /**< @brief No parity */
  UART_PARITY_EVEN = 1, /**< @brief Even parity */
  UART_PARITY_ODD  = 2  /**< @brief Odd parity */
} uart_parity;

/** ddi_fusion_uart_set_parity_mode
 @brief Sets the parity mode of the given UART channel
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @param[in] parity The parity mode to set @see uart_parity
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_set_parity_mode (ddi_fusion_uart_handle handle, uart_parity parity);

/** ddi_fusion_uart_get_parity_mode
 @brief Gets the parity mode of the given UART channel
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @param[out] parity The parity mode to get @see uart_parity
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_get_parity_mode (ddi_fusion_uart_handle handle, uart_parity *parity);

/*! @enum uart_stop_bits
  @brief Represents the stop bit modes available in the Fusion UART subsystem
*/
typedef enum {
  UART_STOP_BITS_1 = 0, /**< @brief 1 stop bit */
  UART_STOP_BITS_2 = 1  /**< @brief 2 stop bits */
} uart_stop_bits;

/** ddi_fusion_uart_set_stop_bits
 @brief Sets the stop bits mode of the given UART channel
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @param[in] stop_bits The stop bit mode to set @see uart_stop_bits
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_set_stop_bits (ddi_fusion_uart_handle handle, uart_stop_bits stop_bits);

/** ddi_fusion_uart_get_stop_bits
 @brief Gets the stop bits mode of the given UART channel
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @param[out] stop_bits The stop bit mode to get @see uart_stop_bits
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_get_stop_bits (ddi_fusion_uart_handle handle, uart_stop_bits *stop_bits);

/*! @enum uart_data_bits
  @brief Represents the data bit modes available in the Fusion UART subsystem
*/
typedef enum {
  UART_DATA_BITS_5 = 0, /**< @brief 5 data bits */
  UART_DATA_BITS_6 = 1, /**< @brief 6 data bits */
  UART_DATA_BITS_7 = 2, /**< @brief 7 data bits */
  UART_DATA_BITS_8 = 3  /**< @brief 8 data bits */
} uart_data_bits;

/** ddi_fusion_uart_set_data_bits
 @brief Sets the data bit mode of the given UART channel
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @param[in] data_bits The data bit mode to set @see uart_data_bits
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_set_data_bits (ddi_fusion_uart_handle handle, uart_data_bits data_bits);

/** ddi_fusion_uart_get_data_bits
 @brief Gets the data bit mode of the given UART channel
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @param[out] data_bits The data bit mode to get @see uart_data_bits
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_get_data_bits (ddi_fusion_uart_handle handle, uart_data_bits *data_bits);

/*! @enum uart_flow_control
  @brief Represents the flow control modes available in the Fusion UART subsystem
*/
typedef enum {
  UART_FLOW_CONTROL_OFF = 0,      /**< @brief No flow control */
  UART_FLOW_CONTROL_XON_XOFF = 1, /**< @brief Software flow control */
  UART_FLOW_CONTROL_RTS_CTS = 2   /**< @brief HW (RTS/CTS) flow control */
} uart_flow_control;

/** ddi_fusion_uart_set_flow_control
 @brief Sets the flow control mode of the given UART channel
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @param[in] flow_control The flow control mode to set @see uart_flow_control
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_set_flow_control (ddi_fusion_uart_handle handle, uart_flow_control flow_control);

/** ddi_fusion_uart_get_flow_control
 @brief Gets the flow control mode of the given UART channel
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @param[out] flow_control The flow control mode to set @see uart_flow_control
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_get_flow_control (ddi_fusion_uart_handle handle, uart_flow_control *flow_control);

/** ddi_fusion_uart_channel_flush
 @brief Flush the tx and rx buffers of the given UART channel
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_channel_flush (ddi_fusion_uart_handle handle);

/** ddi_fusion_uart_set_transmit_hold
 @brief Set the transmit hold for the given UART channel
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_set_transmit_hold (ddi_fusion_uart_handle handle);

/** ddi_fusion_uart_enable_transmit
 @brief Set the transmit enable for the given UART channel
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_enable_transmit (ddi_fusion_uart_handle handle);

/** ddi_fusion_uart_get_transmit_hold_status
 @brief Get the transmit enable status for the given UART channel
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @param[out] is_hold_enabled Is the transmit hold currently enabled?
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_get_transmit_hold_status (ddi_fusion_uart_handle handle, uint8_t *is_hold_enabled);

#define UART_RX_OVERFLOW_ERROR  (1 << 0)
#define UART_RX_FRAMING_ERROR   (1 << 1)
#define UART_RX_PARITY_ERROR    (1 << 2)
#define UART_TX_OVERFLOW_ERROR  (1 << 3)

/** ddi_fusion_uart_get_error_status
 @brief Get the error detail status for the given UART channel
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @param[out] error_details The error detail status
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_get_error_status (ddi_fusion_uart_handle handle, uint8_t *error_details);

/** ddi_fusion_uart_get_rx_bytes_avail
 @brief Get the amount of receive bytes for the given UART channel
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @param[out] bytes_avail The rx bytes available for this particular channel
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_get_rx_bytes_avail (ddi_fusion_uart_handle handle, uint16_t *bytes_avail);

/*! @enum uart_event_type
  @brief Represents the event handler types of the Fusion UART subsystem
*/
typedef enum {
  UART_EVENT_THRESHOLD,       /**< @brief Treshold event detected */
  UART_EVENT_ERROR            /**< @brief Error condition detected */
} uart_event_type;

/*! @enum uart_event_threshold_type
  @brief Represents the event threshold types of the Fusion UART subsystem
*/
typedef enum {
  UART_EVENT_THRESHOLD_RISING_EDGE    = (1 << 0), /**< @brief Rising edge threshold event detected */
  UART_EVENT_THRESHOLD_FALLING_EDGE   = (1 << 1), /**< @brief Falling edge threshold event detected */
  UART_EVENT_THRESHOLD_TX_ALMOST_FULL = (1 << 2), /**< @brief Tx Buffer almost full (within 8 bytes of an overflow) */
  UART_EVENT_THRESHOLD_RX_ALMOST_FULL = (1 << 3)  /**< @brief Rx Buffer almost full (within 8 bytes of an overflow) */
} uart_event_threshold_type;

/*! @enum uart_event
  @brief Represents the event notification types of the Fusion UART subsystem
*/
typedef struct {
  uart_event_type event; /**< @brief The UART event type, threshold or error */
  uart_event_threshold_type threshold_type; /**< @brief The UART threshold type, rising or falling edge */
  ddi_fusion_uart_handle uart_handle; /**< @brief The DDI EtherCAT UART handle the event occurred on */
  uint16_t threshold_level; /**< @brief The threshold buffer level the event triggered on */
} uart_event;

typedef void (ddi_uart_event_func)(uart_event *event, void *user_data);

/** ddi_fusion_uart_register_event
 @brief Registers a UART event handler
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @param[in] callback The event callback which will be called when the event type specified by ddi_fusion_uart_enable_threshold_event
 or ddi_fusion_uart_enable_error_event is reached.
 @param[in] user_data User-specified pointer that will be passed to the notification callback
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_register_event (ddi_fusion_uart_handle handle, ddi_uart_event_func *callback, void *user_data);

/** ddi_fusion_uart_enable_threshold_event
 @brief Enables the threshold event for the given UART channel
 Enable a threshold event on the rx buffer, the UART event handler will be notified when the UART
 channel Rx bytes exceeds the value in the threshold field
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @param[in] event_flags The UART threshold event(s) to enable @see uart_event_threshold_type
 @param[in] threshold The buffer threshold amount to trigger the event handler registered by ddi_fusion_uart_register_event()
 @return ddi_em_result The result code of the operation @see ddi_em_result
 Example Usage:
 <PRE>
 void setup_threshold_events (ddi_fusion_uart_handle handle, uint32_t uart_event_threshold_level)
 {
    // Setup threshold events: Rising edge, Tx almost full and Rx almost full
    uint32_t threshold_flags = UART_EVENT_THRESHOLD_RISING_EDGE | UART_EVENT_THRESHOLD_TX_ALMOST_FULL | UART_EVENT_THRESHOLD_RX_ALMOST_FULL;
    // Enable threshold event
    ddi_fusion_uart_enable_threshold_event(em_handle, threshold_flags, uart_event_threshold_level);
 }
 </PRE>
 */
ddi_em_result ddi_fusion_uart_enable_threshold_event (ddi_fusion_uart_handle handle, uint32_t event_flags, uint16_t threshold);

/** ddi_fusion_uart_disable_threshold_event
 @brief Disables the threshold event for the given UART channel
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @param[in] event_flags The UART threshold event(s) to disable
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_disable_threshold_event (ddi_fusion_uart_handle handle, uint32_t event_flags);

/** ddi_fusion_uart_enable_error_event
 @brief Enables the error event for the given UART channel
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_enable_error_event (ddi_fusion_uart_handle handle);

/** ddi_fusion_uart_disable_error_event
 @brief Disables the error event for the given UART channel
 @param[in] handle The UART handle opened by ddi_fusion_uart_open
 @return ddi_em_result The result code of the operation @see ddi_em_result
 */
ddi_em_result ddi_fusion_uart_disable_error_event (ddi_fusion_uart_handle handle);

#ifdef __cplusplus
}
#endif

#endif
