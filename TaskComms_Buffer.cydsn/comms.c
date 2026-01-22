/* ========================================
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
 *  Streaming B uffer
 * ========================================
*/

RC_t streamRB_init(StreamingRB_t *rb) 
{
    rb->readIdx = rb->writeIdx = rb->fillLevel = 0;
    return RC_SUCCESS;
}

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

RC_t streamRB_read_message(StreamingRB_t *rb, uint8_t *msg, uint16_t *msg_len) 
{
    *msg_len = 0;
    
    GetResource(res_stream);  // Protect task access
    
    while (rb->fillLevel > 0) 
    {  // Extract one complete message
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

RC_t streamRB_flush(StreamingRB_t *rb) 
{
    GetResource(res_stream);
    rb->readIdx = rb->writeIdx;   // Reader catches writer = empty
    rb->fillLevel = 0;            // Force empty
    ReleaseResource(res_stream);
    return RC_SUCCESS;
}

RC_t dynRB_init(DynPayloadRB_t *rb) 
{
    rb->readIdx_tft = rb->readIdx_uart = rb->writeIdx = rb->fillLevel = 0;
    return RC_SUCCESS;
}

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
    
    SetEvent(slot->task, slot->event);
    
    return RC_SUCCESS;
}

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

RC_t dynRB_flush(DynPayloadRB_t *rb) 
{
    GetResource(res_dyn);
    rb->readIdx_tft = rb->writeIdx;   // TFT catches writer
    rb->readIdx_uart = rb->writeIdx;  // UART catches writer
    rb->fillLevel = 0;
    ReleaseResource(res_dyn);
    return RC_SUCCESS;
}

void RB_flush_all() 
{
    streamRB_flush(&uartRB);      // Clear UART RX
    dynRB_flush(&sharedRB);       // Clear shared payload
    UART_LOG_PutString("\r\nAll Buffers flushed!\r\n");
}

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
    // Reverse
    for (uint8_t i = 0; i < idx / 2; i++) {
        char temp = buf[i];
        buf[i] = buf[idx - 1 - i];
        buf[idx - 1 - i] = temp;
    }
    UART_LOG_PutString(buf);
}

/* [comms.c] END OF FILE */
