/* ========================================
 *
 * \file main.c
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
 * NOTE:
 * 1. No systick timer usage in this project.
 * 2. Transmit only Decimal values.
 * 3. Use uart baud rate - 115200.
 *
 * ========================================
*/
#include <stdlib.h>
#include "project.h"
#include "global.h"
#include "comms.h"
#include "tft.h"

StreamingRB_t uartRB;          /**< Global streaming UART receive ring buffer instance. */
DynPayloadRB_t sharedRB;       /**< Global dynamic payload ring buffer shared between multiple tasks. */

int main()
{
    /* Enable global interrupts for the whole system. */
    CyGlobalIntEnable; 
    
    /* Configure SysTick to generate a 1 ms tick and enable its interrupt. */
    //EE_systick_set_period(MILLISECONDS_TO_TICKS(1, BCLK__BUS_CLK__HZ));
    //EE_systick_enable_int();
    
    /* Start the OSEK/EE operating system in the default application mode. */
    for(;;) 
    {
        StartOS(OSDEFAULTAPPMODE);
    }
}

/********************************************************************************
 * Task Definitions
 ********************************************************************************/

/**
 * System initialization task.
 * 
 * Initializes MCAL drivers, display, communication buffers and OS-related
 * infrastructure, then activates all application tasks before terminating.
 */
TASK(tsk_init)
{
    /* Initialize UART logging interface. */
    UART_LOG_Start();
    /* Initialize TFT controller and set backlight to full brightness. */
    TFT_init();
    TFT_setBacklight(100);
    
    /* Configure system ISRs with OS parameters; must follow driver init. */
    EE_system_init();
     
    /* Start SysTick after vector table has been updated by the OS. */
    //EE_systick_start();  
    
    /* Initialize UART streaming RX ring buffer. */
    streamRB_init(&uartRB);
    /* Initialize dynamic payload ring buffer (also acceptable via zero-init). */
    dynRB_init(&sharedRB);
    
    /* Print banner announcing the Inter-Task Communication demonstration. */
    UART_LOG_PutString("\r\n===== Inter Task Communication =====\r\n");
    
    /* Flush all communication buffers to ensure a clean startup state. */
    RB_flush_all();
    
    /* Start cyclic alarms if configured (not shown here). */

    /* Activate all extended tasks and the background task. */
    ActivateTask(tsk_sender);
    ActivateTask(tsk_tft);
    ActivateTask(tsk_uart);
    ActivateTask(tsk_background);

    /* Terminate this one-shot initialization task. */
    TerminateTask();
}

/**
 * Sender task.
 * 
 * Waits for complete messages in the streaming buffer, dispatches them to
 * the dynamic payload buffer for TFT and UART consumers, and performs
 * buffer housekeeping and error recovery.
 */
TASK(tsk_sender)
{
    EventMaskType ev = 0;
    
    uint8_t uart_msg[128];   /**< Local buffer for one decoded UART message. */
    uint16_t msg_len;        /**< Length of the received UART message in bytes. */

    while (1)
    {
        /* Block until the sender event is set by the UART RX ISR. */
        WaitEvent(ev_sender);
        GetEvent(tsk_sender, &ev);
        ClearEvent(ev);       

        if(ev & ev_sender)
        {
            /* Extract one complete message terminating at EOM_MARKER. */
            RC_t result = streamRB_read_message(&uartRB, uart_msg, &msg_len);

            if (result == RC_SUCCESS && msg_len > 0) 
            {
                /* Forward the same message to UART and TFT consumers. */
                RC_t resultUart = dynRB_send(&sharedRB, uart_msg, msg_len, UART_ID, ev_uart, tsk_uart);   /**< Enqueue for UART forwarding. */
                RC_t resultTFT  = dynRB_send(&sharedRB, uart_msg, msg_len, TFT_ID,  ev_tft,  tsk_tft);    /**< Enqueue for TFT display. */
                
                /* If any enqueue fails, reset the dynamic buffer to recover. */
                if ((resultTFT != RC_SUCCESS) || (resultUart != RC_SUCCESS)) 
                {
                    dynRB_flush(&sharedRB);
                } else {
                    __asm("nop"); 
                }
                /* Clear streaming buffer after successful processing of message. */
                streamRB_flush(&uartRB);             
            } else {
                __asm("nop");
            }
        }
    }
    
    /* Defensive: task should not reach this point in normal operation. */
    TerminateTask();
}

/**
 * TFT consumer task.
 * 
 * Waits for messages targeted at the TFT, reads them from the dynamic payload
 * buffer and prints the numeric values on the display.
 */
TASK(tsk_tft) 
{
    EventMaskType ev = 0;
    
    uint8_t tft_data[128];   /**< Local buffer storing message data for TFT. */
    uint16_t len;            /**< Payload length fetched for TFT. */

    while (1)
    {
        /* Block until TFT event indicates available payload. */
        WaitEvent(ev_tft);
        GetEvent(tsk_tft, &ev);
        ClearEvent(ev);       

        if(ev & ev_tft)
        {            
            /* Retrieve next TFT-specific payload from shared ring buffer. */
            if (dynRB_receive(&sharedRB, TFT_ID, tft_data, &len) == RC_SUCCESS) 
            {
                /* Print all values except the terminating EOM marker. */
                for (uint16_t i = 0; i < (len - 1); i++) // -1 to avoid \0
                {
                    TFT_setCursor(0 + i * 16, 20);  /**< Place cursor for each integer output. */
                    TFT_printInt(tft_data[i]);
                }
                /* Flush dynamic buffer so that subsequent messages start cleanly. */
                dynRB_flush(&sharedRB);    
            } else {
                __asm("nop");
            }
        }
    }
    
    /* Defensive: task should not reach this point in normal operation. */
    TerminateTask();
}

/**
 * UART forwarder task.
 * 
 * Waits for UART-specific messages in the dynamic payload buffer, then
 * retransmits the numeric values over UART in a formatted way.
 */
TASK(tsk_uart) 
{
    EventMaskType ev = 0;
    
    uint8_t uart_fwd[128];   /**< Local buffer for data to forward via UART. */
    uint16_t len;            /**< Length of data to forward over UART. */

    while (1)
    {
        /* Block until UART event signals data availability. */
        WaitEvent(ev_uart);
        GetEvent(tsk_uart, &ev);
        ClearEvent(ev);       

        if(ev & ev_uart)
        {
            /* Retrieve next UART-specific payload from shared ring buffer. */
            if (dynRB_receive(&sharedRB, UART_ID, uart_fwd, &len) == RC_SUCCESS) 
            {
                /* Forward all data bytes except EOM marker as formatted integers. */
                UART_LOG_PutString("\r\nReceived: ");
                for (uint16_t i = 0; i < (len - 1); i++) // -1 to avoid \0
                {
                    UART_LOG_PutInt(uart_fwd[i]);
                    
                    if(i < (len - 2)) {
                        UART_LOG_PutString(", ");
                    }
                }
                UART_LOG_PutString(".\n\r");
            } else {
                __asm("nop");
            }
        }
    }
    
    /* Defensive: task should not reach this point in normal operation. */
    TerminateTask();
}

/**
 * Background task.
 * 
 * Lowest-priority task intended for optional background processing or
 * idle-time activities.
 */
TASK(tsk_background)
{
    while(1)
    {
        /* Placeholder for low-priority or idle processing. */
        __asm("nop");
    }
    
    TerminateTask();
}

/********************************************************************************
 * ISR Definitions
 ********************************************************************************/

/**
 * SysTick interrupt service routine.
 * 
 * Increments the system counter each millisecond to drive alarms and
 * OS time-related services.
 */
/*ISR(systick_handler)
{
    CounterTick(cnt_systick);
}*/

/**
 * UART RX interrupt service routine (category 2).
 * 
 * Reads incoming bytes from UART, writes them into the streaming ring buffer
 * and signals the sender task once an end-of-message marker is received.
 */
ISR2(isr_uartRX) {
    /* Clear pending interrupt flag for the UART RX source. */
    isr_uartRX_ClearPending();
    
    /* Fetch one received byte from the UART peripheral. */
    uint8_t rxByte = UART_LOG_GetByte();
    
    /* Attempt to store the received byte into the streaming buffer. */
    RC_t result = streamRB_write(&uartRB, &rxByte);
    //UART_LOG_PutString("\r\nC1\n\r");
    if (result == RC_SUCCESS) 
    {
        if (rxByte == '\0') 
        {
            /* Clear display for a fresh rendering of the new message. */
            TFT_clearScreen();
            TFT_print("Task Comms\n");
            /* Notify sender task that a complete message is available. */
            SetEvent(tsk_sender, ev_sender);
        } else if (result == RC_ERROR_BUFFER_FULL) {
            /* If write failed due to overflow, flush the streaming buffer. */
            streamRB_flush(&uartRB);
        } else {
            __asm("nop");
        }
    }
}

/* [main.c] END OF FILE */