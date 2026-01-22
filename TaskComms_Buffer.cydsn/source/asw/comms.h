/* ========================================
 *
 * \file comms.h
 * \author V.S. Agilan
 * \date 21.01.26
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include "project.h"
#include "global.h"

#ifndef COMMS_H
#define COMMS_H

/* ========================================
 *  Streaming Buffer
 * ========================================
 */

/** Size of streaming ring buffer in bytes. */
#define RB_SIZE     256
/** Message terminator byte used as end-of-message marker. */
#define EOM_MARKER  '\0'
/** Maximum length of a single extracted message in bytes. */
#define MAX_MSG_LEN 128

/**
 * Streaming ring buffer type for UART reception.
 * 
 * Holds raw byte data and tracking indices used to implement a circular
 * buffer for streaming UART input.
 */
typedef struct {
    uint8_t buffer[RB_SIZE];               /**< Storage array for buffered bytes. */
    volatile uint16_t readIdx, writeIdx;   /**< Read and write indices into buffer. */
    volatile uint16_t fillLevel;           /**< Number of bytes currently stored. */
} StreamingRB_t;

/** Global streaming ring buffer for UART reception (defined in comms.c). */
extern StreamingRB_t uartRB;

/* ========================================
 *  Dynamic Payload Ring Buffer
 * ========================================
 */

/** Maximum payload size per dynamic buffer slot in bytes. */
#define DYN_MAX_SIZE    128
/** Total number of slots in the dynamic payload ring buffer. */
#define DYN_SLOTS       8

/**
 * Identifier type for payload consumers.
 * 
 * Used to distinguish between TFT and UART destinations in shared buffer.
 */
typedef enum {
    NONE_ID,   /**< No valid consumer. */
    TFT_ID,    /**< TFT consumer identifier. */
    UART_ID    /**< UART consumer identifier. */
} dyn_id_t;

/**
 * Single dynamic payload slot.
 * 
 * Contains payload data plus metadata indicating target consumer, associated
 * event and task to be notified.
 */
typedef struct {
    uint8_t payload[DYN_MAX_SIZE]; /**< Raw payload data. */
    uint16_t payload_len;          /**< Length of valid payload data. */
    dyn_id_t msg_id;               /**< Target consumer ID (TFT_ID / UART_ID). */
    EventMaskType event;           /**< Event to be set when payload is ready. */
    char task;                     /**< Task identifier to be notified. */
} dyn_payload_t;

/**
 * Dynamic payload ring buffer structure.
 * 
 * Shared between multiple consumer tasks, maintaining individual read
 * indices for TFT and UART while using a common write index.
 */
typedef struct {
    dyn_payload_t slots[DYN_SLOTS]; /**< Array of message slots. */
    uint16_t readIdx_tft;          /**< Read index for TFT consumer. */
    uint16_t readIdx_uart;         /**< Read index for UART consumer. */
    uint16_t writeIdx;             /**< Global write index for producer. */
    uint16_t fillLevel;            /**< Number of occupied slots. */
} DynPayloadRB_t;

/** Global shared dynamic payload ring buffer (defined in comms.c). */
extern DynPayloadRB_t sharedRB;

/* ========================================
 * Function declarations - StreamingRB_t
 * ========================================
 */

/**
 * Initialize a streaming ring buffer.
 * 
 * @param rb Pointer to ring buffer instance to be initialized (IN/OUT).
 * @return RC_SUCCESS on success.
 */
RC_t streamRB_init(StreamingRB_t *rb);

/**
 * Write a byte into the streaming ring buffer.
 * 
 * @param rb   Pointer to streaming ring buffer instance (IN/OUT).
 * @param byte Pointer to byte to write into the buffer (IN).
 * @return RC_SUCCESS on success, RC_ERROR_BUFFER_FULL if the buffer is full.
 */
RC_t streamRB_write(StreamingRB_t *rb, uint8_t *byte);

/**
 * Read a complete message from the streaming ring buffer.
 * 
 * @param rb      Pointer to streaming ring buffer instance (IN/OUT).
 * @param msg     Destination buffer for the message bytes (OUT).
 * @param msg_len Pointer to length variable for received message (OUT).
 * @return RC_SUCCESS on complete message, error code otherwise.
 */
RC_t streamRB_read_message(StreamingRB_t *rb, uint8_t *msg, uint16_t *msg_len);

/**
 * Read a single byte from the streaming ring buffer.
 * 
 * @param rb   Pointer to streaming ring buffer instance (IN/OUT).
 * @param byte Pointer to destination for the read byte (OUT).
 * @return RC_SUCCESS if a byte was read, RC_ERROR_BUFFER_EMTPY if empty.
 */
RC_t streamRB_read_byte(StreamingRB_t *rb, uint8_t *byte);

/**
 * Flush the streaming ring buffer contents.
 * 
 * @param rb Pointer to streaming ring buffer instance to flush (IN/OUT).
 * @return RC_SUCCESS when the buffer has been cleared.
 */
RC_t streamRB_flush(StreamingRB_t *rb);

/* ========================================
 * Function declarations - DynPayloadRB_t
 * ========================================
 */

/**
 * Initialize a dynamic payload ring buffer.
 * 
 * @param rb Pointer to dynamic payload ring buffer instance (IN/OUT).
 * @return RC_SUCCESS on success.
 */
RC_t dynRB_init(DynPayloadRB_t *rb);

/**
 * Enqueue a payload into the dynamic ring buffer.
 * 
 * @param rb      Pointer to dynamic payload ring buffer instance (IN/OUT).
 * @param data    Pointer to source data buffer (IN).
 * @param len     Length of source payload in bytes (IN).
 * @param msg_id  Target consumer identifier (IN).
 * @param ev      Event mask to raise for consumer task (IN).
 * @param tsk     Task identifier to notify (IN).
 * @return RC_SUCCESS on success, RC_ERROR on size/space error.
 */
RC_t dynRB_send(DynPayloadRB_t *rb, uint8_t *data, uint16_t len, dyn_id_t msg_id, EventMaskType ev, char tsk);

/**
 * Receive a payload from the dynamic ring buffer for a given consumer.
 * 
 * @param rb          Pointer to dynamic payload ring buffer instance (IN/OUT).
 * @param consumer_id Consumer identifier selecting the read index (IN).
 * @param data        Destination buffer for received payload (OUT).
 * @param len         Pointer to length variable receiving payload size (OUT).
 * @return RC_SUCCESS when data is available, RC_ERROR_BUFFER_EMTPY otherwise.
 */
RC_t dynRB_receive(DynPayloadRB_t *rb, dyn_id_t consumer_id, uint8_t *data, uint16_t *len);

/**
 * Flush all entries from the dynamic payload ring buffer.
 * 
 * @param rb Pointer to dynamic payload ring buffer instance (IN/OUT).
 * @return RC_SUCCESS after all entries are cleared.
 */
RC_t dynRB_flush(DynPayloadRB_t *rb);

/* ========================================
 * Function declarations - Common
 * ========================================
 */

/**
 * Flush both streaming and dynamic ring buffers.
 * 
 * Utility for resetting the communication layer to a known empty state.
 */
void RB_flush_all();

/**
 * Print an unsigned integer via UART_LOG.
 * 
 * @param num Unsigned 16-bit value to be transmitted as ASCII (IN).
 */
void UART_LOG_PutInt(uint16_t num);

#endif

/* [comms.h] END OF FILE */