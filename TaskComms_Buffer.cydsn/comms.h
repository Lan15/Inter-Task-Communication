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

#include "project.h"
#include "global.h"

#ifndef COMMS_H
#define COMMS_H

/* ========================================
 *  Streaming Buffer
 * ========================================
 */
#define RB_SIZE     256      // Ring buffer size (bytes)
#define EOM_MARKER  '\0'       // End-of-message byte
#define MAX_MSG_LEN 128

typedef struct {
    uint8_t buffer[RB_SIZE];
    volatile uint16_t readIdx, writeIdx, fillLevel;
} StreamingRB_t;

extern StreamingRB_t uartRB;

/* ========================================
 *  Dynamic Payload Ring Buffer
 * ========================================
 */
#define DYN_MAX_SIZE    128
#define DYN_SLOTS       8

typedef enum {
    NONE_ID,
    TFT_ID,
    UART_ID
} dyn_id_t;

typedef struct {
    uint8_t payload[DYN_MAX_SIZE];
    uint16_t payload_len;
    dyn_id_t msg_id;  // For dispatching (TFT vs UART)
    EventMaskType event;
    char task;
} dyn_payload_t;

typedef struct {
    dyn_payload_t slots[DYN_SLOTS];
    uint16_t readIdx_tft, readIdx_uart, writeIdx, fillLevel;
} DynPayloadRB_t;

extern DynPayloadRB_t sharedRB;

/* ========================================
 * Function declarations - StreamingRB_t
 * ========================================
 */

RC_t streamRB_init(StreamingRB_t *rb);

RC_t streamRB_write(StreamingRB_t *rb, uint8_t *byte); // send

RC_t streamRB_read_message(StreamingRB_t *rb, uint8_t *msg, uint16_t *msg_len); // receive

RC_t streamRB_read_byte(StreamingRB_t *rb, uint8_t *byte);

RC_t streamRB_flush(StreamingRB_t *rb);

/* ========================================
 * Function declarations - DynPayloadRB_t
 * ========================================
 */

RC_t dynRB_init(DynPayloadRB_t *rb);

RC_t dynRB_send(DynPayloadRB_t *rb, uint8_t *data, uint16_t len, dyn_id_t msg_id, EventMaskType ev, char tsk);

RC_t dynRB_receive(DynPayloadRB_t *rb, dyn_id_t consumer_id, uint8_t *data, uint16_t *len);

RC_t dynRB_flush(DynPayloadRB_t *rb);

/* ========================================
 * Function declarations - Common
 * ========================================
 */

void RB_flush_all();

void UART_LOG_PutInt(uint16_t num);

#endif

/* [comms.h] END OF FILE */
