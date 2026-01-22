Inter-Task Communication with Dual Ring Buffers

Summary:
	Implemented robust inter-task communication framework using two specialized ring buffers for UART message handling and consumer dispatching.

Key Features:
	• StreamingRB_t (uartRB): UART RX buffering with EOM detection (\0 terminator)
	• DynPayloadRB_t (sharedRB): Multi-consumer payload dispatch (TFT + UART forwarding)
	• 4-task architecture: tsk_sender → tsk_tft + tsk_uart + tsk_background
	• ISR-driven UART reception with overflow recovery
	• Thread-safe buffer access via OSEK/EE resources
	• Automatic buffer flushing for error recovery

Workflow:
	1. UART ISR → uartRB (bytes until \0)
	2. tsk_sender: Extract message → dynRB_send(TFT_ID) + dynRB_send(UART_ID)
	3. tsk_tft: dynRB_receive(TFT_ID) → TFT_printInt() values
	4. tsk_uart: dynRB_receive(UART_ID) → UART_LOG_PutInt() formatted output

Demo Input:
	Send "10,20,100,120\0" → TFT displays numbers, UART echoes "10, 20, 100, 120."

Files:
	• main.c: Task definitions + ISR handlers
	• comms.h: Buffer types + function prototypes  
	• comms.c: Ring buffer implementation + UART_LOG_PutInt()

Status: Fully functional, tested on PSoC5LP with UART @ 115200 8N1.
