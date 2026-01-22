/* ========================================
 *
 * \file comms.c
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

#include "comms.h"

/* ========================================
 *  Streaming Buffer
 * ========================================
*/

/**
 * Initialize a streaming ring buffer instance.
 * 
 * @param rb Pointer to streaming ring buffer object to be initialized.
 * @return RC_SUCCESS on successful initialization.
 */
RC_t streamRB_init(StreamingRB_t *rb) 
{
    rb->readIdx = rb->writeIdx = rb->fillLevel = 0;
    return RC_SUCCESS;
}

/**
 * Write one byte into the streaming ring buffer.
 * 
 * @param rb   Pointer to streaming ring buffer instance (IN/OUT).
 * @param byte Pointer to byte value to be written into the buffer (IN).
 * @return RC_SUCCESS if written, RC_ERROR_BUFFER_FULL if no space is available.
 */
RC_t streamRB_write(StreamingRB_t *rb, uint8_t *byte) 
{
    if (rb->fillLevel < RB_SIZE) {
        rb->buffer[rb->writeIdx++] = *byte;
        rb->writeIdx %= RB_SIZE;
        rb->fillLevel++;
        return RC_SUCCESS;
    }
    return RC_ERROR_BUFFER_FULL;
}

/**
 * Read one complete message from the streaming ring buffer.
 * 
 * A message is defined as a contiguous sequence of bytes ending with the
 * EOM_MARKER. The function copies bytes into the provided buffer until
 * the marker is found or an error occurs.
 * 
 * @param rb      Pointer to streaming ring buffer instance (IN/OUT).
 * @param msg     Destination buffer for the assembled message (OUT).
 * @param msg_len Pointer to variable that receives the message length (OUT).
 * @return RC_SUCCESS on complete message, RC_ERROR_WRITE_FAILS if too long,
 *         RC_ERROR_READ_FAILS if EOM not found.
 */
RC_t streamRB_read_message(StreamingRB_t *rb, uint8_t *msg, uint16_t *msg_len) 
{
    *msg_len = 0;
    
    /* Protect buffer access on task level with a resource. */
    GetResource(res_stream);  // Protect task access
    
    /* Extract one complete message terminated with EOM_MARKER. */
    while (rb->fillLevel > 0) 
    {
        uint8_t byte;
        streamRB_read_byte(rb, &byte);
        
        msg[(*msg_len)++] = byte;
        
        if (byte == EOM_MARKER) {  // End of message found
            ReleaseResource(res_stream);
            return RC_SUCCESS;
        }
        
        if (*msg_len >= MAX_MSG_LEN) {
            ReleaseResource(res_stream);
            return RC_ERROR_WRITE_FAILS;   // RC_ERROR_TOO_LONG
        }
    }
    ReleaseResource(res_stream);
    
    return RC_ERROR_READ_FAILS;  // No EOM found - RC_ERROR_INCOMPLETE
}

/**
 * Read a single byte from the streaming ring buffer.
 * 
 * @param rb   Pointer to streaming ring buffer instance (IN/OUT).
 * @param byte Pointer to destination byte receiving the read value (OUT).
 * @return RC_SUCCESS if a byte was read, RC_ERROR_BUFFER_EMTPY if buffer is empty.
 */
RC_t streamRB_read_byte(StreamingRB_t *rb, uint8_t *byte) 
{
    if (rb->fillLevel > 0) {
        *byte = rb->buffer[rb->readIdx++];
        rb->readIdx = rb->readIdx % RB_SIZE;
        rb->fillLevel--;
        return RC_SUCCESS;
    }
    return RC_ERROR_BUFFER_EMTPY;
}

/**
 * Flush all pending data in the streaming ring buffer.
 * 
 * Sets the read index equal to the write index and resets the fill level
 * so that the buffer appears empty to all users.
 * 
 * @param rb Pointer to streaming ring buffer instance to be flushed (IN/OUT).
 * @return RC_SUCCESS after the buffer is cleared.
 */
RC_t streamRB_flush(StreamingRB_t *rb) 
{
    GetResource(res_stream);
    rb->readIdx = rb->writeIdx;   // Reader catches writer = empty
    rb->fillLevel = 0;            // Force empty
    ReleaseResource(res_stream);
    return RC_SUCCESS;
}

/**
 * Initialize dynamic payload ring buffer instance.
 * 
 * @param rb Pointer to dynamic payload ring buffer object (IN/OUT).
 * @return RC_SUCCESS on successful initialization.
 */
RC_t dynRB_init(DynPayloadRB_t *rb) 
{
    rb->readIdx_tft = rb->readIdx_uart = rb->writeIdx = rb->fillLevel = 0;
    return RC_SUCCESS;
}

/**
 * Enqueue a payload into the dynamic payload ring buffer.
 * 
 * Stores the given data together with metadata identifying the consumer,
 * event and target task, then triggers the corresponding event.
 * 
 * @param rb      Pointer to dynamic payload ring buffer instance (IN/OUT).
 * @param data    Pointer to data buffer to be enqueued (IN).
 * @param len     Length of the payload in bytes (IN).
 * @param msg_id  Identifier of target consumer (e.g. TFT_ID, UART_ID) (IN).
 * @param ev      Event mask to be set for the target task (IN).
 * @param tsk     Task identifier that should receive the event (IN).
 * @return RC_SUCCESS if enqueued, RC_ERROR if buffer is full or size exceeds limit.
 */
RC_t dynRB_send(DynPayloadRB_t *rb, uint8_t *data, uint16_t len, dyn_id_t msg_id, EventMaskType ev, char tsk) 
{
    if (len > DYN_MAX_SIZE || rb->fillLevel >= DYN_SLOTS) 
    {
        return RC_ERROR;
    }
    
    GetResource(res_dyn);
    dyn_payload_t *slot = &rb->slots[rb->writeIdx];
    memcpy(slot->payload, data, len);
    slot->payload_len = len;
    slot->msg_id = msg_id;
    slot->event = ev;
    slot->task = tsk;
    
    rb->writeIdx = rb->writeIdx % DYN_SLOTS;
    rb->fillLevel++;
    ReleaseResource(res_dyn);
    
    /* Notify the destination task that a new payload is available. */
    SetEvent(slot->task, slot->event);
    
    return RC_SUCCESS;
}

/**
 * Receive a payload from the dynamic payload ring buffer.
 * 
 * Selects the appropriate read index based on consumer ID, copies the
 * payload into the destination buffer and updates read indices and fill level.
 * 
 * @param rb          Pointer to dynamic payload ring buffer instance (IN/OUT).
 * @param consumer_id Identifier of the consumer (TFT_ID or UART_ID) (IN).
 * @param data        Destination buffer for received payload (OUT).
 * @param len         Pointer to variable receiving the payload length (OUT).
 * @return RC_SUCCESS if a message was received, RC_ERROR_BUFFER_EMTPY if none.
 */
RC_t dynRB_receive(DynPayloadRB_t *rb, dyn_id_t consumer_id, uint8_t *data, uint16_t *len) 
{
    GetResource(res_dyn);
    uint16_t *readIdx = (consumer_id == TFT_ID) ? &rb->readIdx_tft : &rb->readIdx_uart;
    
    if (rb->fillLevel == 0) {
        ReleaseResource(res_dyn);
        return RC_ERROR_BUFFER_EMTPY;   // RC_ERROR_EMPTY
    }
    
    dyn_payload_t *slot = &rb->slots[*readIdx];
    *len = slot->payload_len;
    memcpy(data, slot->payload, *len);
    
    *readIdx = *readIdx % DYN_SLOTS;
    rb->fillLevel--;
    ReleaseResource(res_dyn);
    return RC_SUCCESS;
}

/**
 * Flush the dynamic payload ring buffer.
 * 
 * Sets both TFT and UART read indices equal to the write index, drops
 * all pending messages and resets the fill level.
 * 
 * @param rb Pointer to dynamic payload ring buffer instance (IN/OUT).
 * @return RC_SUCCESS after the buffer is cleared.
 */
RC_t dynRB_flush(DynPayloadRB_t *rb) 
{
    GetResource(res_dyn);
    rb->readIdx_tft = rb->writeIdx;   // TFT catches writer
    rb->readIdx_uart = rb->writeIdx;  // UART catches writer
    rb->fillLevel = 0;
    ReleaseResource(res_dyn);
    return RC_SUCCESS;
}

/**
 * Flush both streaming and dynamic buffers.
 * 
 * Convenience function to clear all communication-related buffers and
 * print a diagnostic message over UART.
 */
void RB_flush_all() 
{
    streamRB_flush(&uartRB);      // Clear UART RX
    dynRB_flush(&sharedRB);       // Clear shared payload
    UART_LOG_PutString("\r\nAll Buffers flushed!\r\n");
}

/**
 * Print a 16-bit unsigned integer as ASCII via UART_LOG.
 * 
 * Performs decimal conversion into a local buffer, reverses the character
 * order and transmits the resulting string using UART_LOG_PutString().
 * 
 * @param num Unsigned integer value to be printed (IN).
 */
void UART_LOG_PutInt(uint16_t num) {
    if (num == 0) {
        UART_LOG_PutString("0");
        return;
    }
    char buf[6];
    uint8_t idx = 0;
    while (num > 0) {
        buf[idx++] = '0' + (num % 10);
        num /= 10;
    }
    buf[idx] = '\0';
    /* Reverse string in-place to restore correct digit order. */
    for (uint8_t i = 0; i < idx / 2; i++) {
        char temp = buf[i];
        buf[i] = buf[idx - 1 - i];
        buf[idx - 1 - i] = temp;
    }
    UART_LOG_PutString(buf);
}

/* [comms.c] END OF FILE */