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
#include <stdlib.h>
#include "project.h"
#include "global.h"
#include "comms.h"
#include "tft.h"

StreamingRB_t uartRB;
DynPayloadRB_t sharedRB;  // Shared between 3 tasks

int main()
{
    // Enable global interrupts.
    CyGlobalIntEnable; 
    
    //Set systick period to 1 ms. Enable the INT and start it.
	EE_systick_set_period(MILLISECONDS_TO_TICKS(1, BCLK__BUS_CLK__HZ));
	EE_systick_enable_int();
    
    // Start Operating System
    for(;;) 
    {
    	StartOS(OSDEFAULTAPPMODE);
    }
}

/********************************************************************************
 * Task Definitions
 ********************************************************************************/

TASK(tsk_init)
{
    //Init MCAL Drivers
    UART_LOG_Start();
    TFT_init();
    TFT_setBacklight(100);
    
    //Init your buffers here
    streamRB_init(&uartRB); // Streaming RX buffer
    //UART_LOG_PutString("Streaming buffer initialized\r\n");
    
    dynRB_init(&sharedRB);  // If needed (zero-init OK)
    //UART_LOG_PutString("Dynamic buffer initialized\r\n");
    
    RB_flush_all();
    
    //Reconfigure ISRs with OS parameters.
    //This line MUST be called after the hardware driver initialisation!
    EE_system_init();
     
    //Start SysTick
	//Must be done here, because otherwise the isr vector is not overwritten yet
    EE_systick_start();  
	
    //Start the cyclic alarms 

    //Activate all extended and the background task
    ActivateTask(tsk_sender);
    ActivateTask(tsk_tft);
    ActivateTask(tsk_uart);
    ActivateTask(tsk_background);

    TerminateTask();
}

TASK(tsk_sender)
{
    EventMaskType ev = 0;
    
    uint8_t uart_msg[128];
    uint16_t msg_len;
    //UART_LOG_PutString("\r\nC2\n\r");
    while (1)
    {
        //Wait, read and clear the event
        WaitEvent(ev_sender);
        GetEvent(tsk_sender, &ev);
        ClearEvent(ev);       

        if(ev & ev_sender)
        {
            RC_t result = streamRB_read_message(&uartRB, uart_msg, &msg_len);
            //UART_LOG_PutString("\r\nC3\n\r");
            if (result == RC_SUCCESS && msg_len > 0) 
            {
                //UART_LOG_PutString("\r\nC4\n\r");
                // Forward to consumers with ID
                RC_t resultUart = dynRB_send(&sharedRB, uart_msg, msg_len, UART_ID, ev_uart, tsk_uart);   // For UART forward
                RC_t resultTFT = dynRB_send(&sharedRB, uart_msg, msg_len, TFT_ID, ev_tft, tsk_tft);    // For TFT 
                
                if ((resultTFT != RC_SUCCESS) || (resultUart != RC_SUCCESS)) 
                {
                    dynRB_flush(&sharedRB);
                    //UART_LOG_PutString("Dynamic buffer flushed!");  
                } else {
                    __asm("nop"); 
                }
                streamRB_flush(&uartRB);
                //UART_LOG_PutString("\r\nC5\n\r");                 
            } else {
                __asm("nop");
            }
        }
    }
    
    //Just in Case
	TerminateTask();
}

TASK(tsk_tft) 
{
    EventMaskType ev = 0;
    
    uint8_t tft_data[128];
    uint16_t len;
    //UART_LOG_PutString("\r\nC7\n\r");
    while (1)
    {
        //Wait, read and clear the event
        WaitEvent(ev_tft);
        GetEvent(tsk_tft, &ev);
        ClearEvent(ev);       

        if(ev & ev_tft)
        {
            //UART_LOG_PutString("\r\nTFT\n\r");
            //TFT_print("Task Comms\n");
            
            if (dynRB_receive(&sharedRB, TFT_ID, tft_data, &len) == RC_SUCCESS) 
            {
                //UART_LOG_PutString("\r\nC13\n\r");
                for (uint16_t i = 0; i < (len - 1); i++) // -1 to avoid \0
                {
                    //UART_LOG_PutString("\r\nC14\n\r");
                    TFT_setCursor(0 + i * 16, 20);  // Position each value
                    TFT_printInt(tft_data[i]);
                    //UART_LOG_PutString("\r\nC15\n\r");
                }
                //UART_LOG_PutString("\r\nC16\n\r");
                dynRB_flush(&sharedRB);    
            } else {
                __asm("nop");
            }
        }
    }
    
    //Just in Case
	TerminateTask();
}

TASK(tsk_uart) 
{
    EventMaskType ev = 0;
    
    uint8_t uart_fwd[128];
    uint16_t len;
    //UART_LOG_PutString("\r\nC8\n\r");
    while (1)
    {
        //Wait, read and clear the event
        WaitEvent(ev_uart);
        GetEvent(tsk_uart, &ev);
        ClearEvent(ev);       

        if(ev & ev_uart)
        {
            //UART_LOG_PutString("\r\nUART\n\r");
            if (dynRB_receive(&sharedRB, UART_ID, uart_fwd, &len) == RC_SUCCESS) 
            {
                //UART_LOG_PutString("\r\nC11\n\r");
                // Forward via UART TX (use buffered TX if available)
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
    
    //Just in Case
	TerminateTask();
}


TASK(tsk_background)
{
    while(1)
    {
        //do something with low prioroty
        __asm("nop");
    }
    
    TerminateTask();
}

/********************************************************************************
 * ISR Definitions
 ********************************************************************************/

// ISR which will increment the systick counter every ms
ISR(systick_handler)
{
    CounterTick(cnt_systick);
}

// Handle UART RX interrupt
ISR2(isr_uartRX) {
    isr_uartRX_ClearPending();
    
    uint8_t rxByte = UART_LOG_GetByte();
    
     RC_t result = streamRB_write(&uartRB, &rxByte);
    
    if (result == RC_SUCCESS) 
    {
        //UART_LOG_PutString("\r\nC1\n\r");
        if (rxByte == '\0') 
        {
            //UART_LOG_PutString("\r\nC6\n\r");
            TFT_clearScreen();
            SetEvent(tsk_sender, ev_sender);
        } else if (result == RC_ERROR_BUFFER_FULL) {
            // New data started mid-message → flush to next EOM
            streamRB_flush(&uartRB);
        } else {
            __asm("nop");
        }
    }
    
}

/* [main.c] END OF FILE */
