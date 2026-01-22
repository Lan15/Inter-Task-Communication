/*******************************************************************************
* File Name: TFT_SPI.c
* Version 2.50
*
* Description:
*  This file provides all API functionality of the SPI Master component.
*
* Note:
*  None.
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "TFT_SPI_PVT.h"

#if(TFT_SPI_TX_SOFTWARE_BUF_ENABLED)
    volatile uint8 TFT_SPI_txBuffer[TFT_SPI_TX_BUFFER_SIZE];
    volatile uint8 TFT_SPI_txBufferFull;
    volatile uint8 TFT_SPI_txBufferRead;
    volatile uint8 TFT_SPI_txBufferWrite;
#endif /* (TFT_SPI_TX_SOFTWARE_BUF_ENABLED) */

#if(TFT_SPI_RX_SOFTWARE_BUF_ENABLED)
    volatile uint8 TFT_SPI_rxBuffer[TFT_SPI_RX_BUFFER_SIZE];
    volatile uint8 TFT_SPI_rxBufferFull;
    volatile uint8 TFT_SPI_rxBufferRead;
    volatile uint8 TFT_SPI_rxBufferWrite;
#endif /* (TFT_SPI_RX_SOFTWARE_BUF_ENABLED) */

uint8 TFT_SPI_initVar = 0u;

volatile uint8 TFT_SPI_swStatusTx;
volatile uint8 TFT_SPI_swStatusRx;


/*******************************************************************************
* Function Name: TFT_SPI_Init
********************************************************************************
*
* Summary:
*  Inits/Restores default SPIM configuration provided with customizer.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Side Effects:
*  When this function is called it initializes all of the necessary parameters
*  for execution. i.e. setting the initial interrupt mask, configuring the
*  interrupt service routine, configuring the bit-counter parameters and
*  clearing the FIFO and Status Register.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void TFT_SPI_Init(void) 
{
    /* Initialize the Bit counter */
    TFT_SPI_COUNTER_PERIOD_REG = TFT_SPI_BITCTR_INIT;

    /* Init TX ISR  */
    #if(0u != TFT_SPI_INTERNAL_TX_INT_ENABLED)
        CyIntDisable         (TFT_SPI_TX_ISR_NUMBER);
        CyIntSetPriority     (TFT_SPI_TX_ISR_NUMBER,  TFT_SPI_TX_ISR_PRIORITY);
        (void) CyIntSetVector(TFT_SPI_TX_ISR_NUMBER, &TFT_SPI_TX_ISR);
    #endif /* (0u != TFT_SPI_INTERNAL_TX_INT_ENABLED) */

    /* Init RX ISR  */
    #if(0u != TFT_SPI_INTERNAL_RX_INT_ENABLED)
        CyIntDisable         (TFT_SPI_RX_ISR_NUMBER);
        CyIntSetPriority     (TFT_SPI_RX_ISR_NUMBER,  TFT_SPI_RX_ISR_PRIORITY);
        (void) CyIntSetVector(TFT_SPI_RX_ISR_NUMBER, &TFT_SPI_RX_ISR);
    #endif /* (0u != TFT_SPI_INTERNAL_RX_INT_ENABLED) */

    /* Clear any stray data from the RX and TX FIFO */
    TFT_SPI_ClearFIFO();

    #if(TFT_SPI_RX_SOFTWARE_BUF_ENABLED)
        TFT_SPI_rxBufferFull  = 0u;
        TFT_SPI_rxBufferRead  = 0u;
        TFT_SPI_rxBufferWrite = 0u;
    #endif /* (TFT_SPI_RX_SOFTWARE_BUF_ENABLED) */

    #if(TFT_SPI_TX_SOFTWARE_BUF_ENABLED)
        TFT_SPI_txBufferFull  = 0u;
        TFT_SPI_txBufferRead  = 0u;
        TFT_SPI_txBufferWrite = 0u;
    #endif /* (TFT_SPI_TX_SOFTWARE_BUF_ENABLED) */

    (void) TFT_SPI_ReadTxStatus(); /* Clear Tx status and swStatusTx */
    (void) TFT_SPI_ReadRxStatus(); /* Clear Rx status and swStatusRx */

    /* Configure TX and RX interrupt mask */
    TFT_SPI_TX_STATUS_MASK_REG = TFT_SPI_TX_INIT_INTERRUPTS_MASK;
    TFT_SPI_RX_STATUS_MASK_REG = TFT_SPI_RX_INIT_INTERRUPTS_MASK;
}


/*******************************************************************************
* Function Name: TFT_SPI_Enable
********************************************************************************
*
* Summary:
*  Enable SPIM component.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void TFT_SPI_Enable(void) 
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();
    TFT_SPI_COUNTER_CONTROL_REG |= TFT_SPI_CNTR_ENABLE;
    TFT_SPI_TX_STATUS_ACTL_REG  |= TFT_SPI_INT_ENABLE;
    TFT_SPI_RX_STATUS_ACTL_REG  |= TFT_SPI_INT_ENABLE;
    CyExitCriticalSection(enableInterrupts);

    #if(0u != TFT_SPI_INTERNAL_CLOCK)
        TFT_SPI_IntClock_Enable();
    #endif /* (0u != TFT_SPI_INTERNAL_CLOCK) */

    TFT_SPI_EnableTxInt();
    TFT_SPI_EnableRxInt();
}


/*******************************************************************************
* Function Name: TFT_SPI_Start
********************************************************************************
*
* Summary:
*  Initialize and Enable the SPI Master component.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global variables:
*  TFT_SPI_initVar - used to check initial configuration, modified on
*  first function call.
*
* Theory:
*  Enable the clock input to enable operation.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void TFT_SPI_Start(void) 
{
    if(0u == TFT_SPI_initVar)
    {
        TFT_SPI_Init();
        TFT_SPI_initVar = 1u;
    }

    TFT_SPI_Enable();
}


/*******************************************************************************
* Function Name: TFT_SPI_Stop
********************************************************************************
*
* Summary:
*  Disable the SPI Master component.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Theory:
*  Disable the clock input to enable operation.
*
*******************************************************************************/
void TFT_SPI_Stop(void) 
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();
    TFT_SPI_TX_STATUS_ACTL_REG &= ((uint8) ~TFT_SPI_INT_ENABLE);
    TFT_SPI_RX_STATUS_ACTL_REG &= ((uint8) ~TFT_SPI_INT_ENABLE);
    CyExitCriticalSection(enableInterrupts);

    #if(0u != TFT_SPI_INTERNAL_CLOCK)
        TFT_SPI_IntClock_Disable();
    #endif /* (0u != TFT_SPI_INTERNAL_CLOCK) */

    TFT_SPI_DisableTxInt();
    TFT_SPI_DisableRxInt();
}


/*******************************************************************************
* Function Name: TFT_SPI_EnableTxInt
********************************************************************************
*
* Summary:
*  Enable internal Tx interrupt generation.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Theory:
*  Enable the internal Tx interrupt output -or- the interrupt component itself.
*
*******************************************************************************/
void TFT_SPI_EnableTxInt(void) 
{
    #if(0u != TFT_SPI_INTERNAL_TX_INT_ENABLED)
        CyIntEnable(TFT_SPI_TX_ISR_NUMBER);
    #endif /* (0u != TFT_SPI_INTERNAL_TX_INT_ENABLED) */
}


/*******************************************************************************
* Function Name: TFT_SPI_EnableRxInt
********************************************************************************
*
* Summary:
*  Enable internal Rx interrupt generation.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Theory:
*  Enable the internal Rx interrupt output -or- the interrupt component itself.
*
*******************************************************************************/
void TFT_SPI_EnableRxInt(void) 
{
    #if(0u != TFT_SPI_INTERNAL_RX_INT_ENABLED)
        CyIntEnable(TFT_SPI_RX_ISR_NUMBER);
    #endif /* (0u != TFT_SPI_INTERNAL_RX_INT_ENABLED) */
}


/*******************************************************************************
* Function Name: TFT_SPI_DisableTxInt
********************************************************************************
*
* Summary:
*  Disable internal Tx interrupt generation.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Theory:
*  Disable the internal Tx interrupt output -or- the interrupt component itself.
*
*******************************************************************************/
void TFT_SPI_DisableTxInt(void) 
{
    #if(0u != TFT_SPI_INTERNAL_TX_INT_ENABLED)
        CyIntDisable(TFT_SPI_TX_ISR_NUMBER);
    #endif /* (0u != TFT_SPI_INTERNAL_TX_INT_ENABLED) */
}


/*******************************************************************************
* Function Name: TFT_SPI_DisableRxInt
********************************************************************************
*
* Summary:
*  Disable internal Rx interrupt generation.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Theory:
*  Disable the internal Rx interrupt output -or- the interrupt component itself.
*
*******************************************************************************/
void TFT_SPI_DisableRxInt(void) 
{
    #if(0u != TFT_SPI_INTERNAL_RX_INT_ENABLED)
        CyIntDisable(TFT_SPI_RX_ISR_NUMBER);
    #endif /* (0u != TFT_SPI_INTERNAL_RX_INT_ENABLED) */
}


/*******************************************************************************
* Function Name: TFT_SPI_SetTxInterruptMode
********************************************************************************
*
* Summary:
*  Configure which status bits trigger an interrupt event.
*
* Parameters:
*  intSrc: An or'd combination of the desired status bit masks (defined in the
*  header file).
*
* Return:
*  None.
*
* Theory:
*  Enables the output of specific status bits to the interrupt controller.
*
*******************************************************************************/
void TFT_SPI_SetTxInterruptMode(uint8 intSrc) 
{
    TFT_SPI_TX_STATUS_MASK_REG = intSrc;
}


/*******************************************************************************
* Function Name: TFT_SPI_SetRxInterruptMode
********************************************************************************
*
* Summary:
*  Configure which status bits trigger an interrupt event.
*
* Parameters:
*  intSrc: An or'd combination of the desired status bit masks (defined in the
*  header file).
*
* Return:
*  None.
*
* Theory:
*  Enables the output of specific status bits to the interrupt controller.
*
*******************************************************************************/
void TFT_SPI_SetRxInterruptMode(uint8 intSrc) 
{
    TFT_SPI_RX_STATUS_MASK_REG  = intSrc;
}


/*******************************************************************************
* Function Name: TFT_SPI_ReadTxStatus
********************************************************************************
*
* Summary:
*  Read the Tx status register for the component.
*
* Parameters:
*  None.
*
* Return:
*  Contents of the Tx status register.
*
* Global variables:
*  TFT_SPI_swStatusTx - used to store in software status register,
*  modified every function call - resets to zero.
*
* Theory:
*  Allows the user and the API to read the Tx status register for error
*  detection and flow control.
*
* Side Effects:
*  Clear Tx status register of the component.
*
* Reentrant:
*  No.
*
*******************************************************************************/
uint8 TFT_SPI_ReadTxStatus(void) 
{
    uint8 tmpStatus;

    #if(TFT_SPI_TX_SOFTWARE_BUF_ENABLED)
        /* Disable TX interrupt to protect global veriables */
        TFT_SPI_DisableTxInt();

        tmpStatus = TFT_SPI_GET_STATUS_TX(TFT_SPI_swStatusTx);
        TFT_SPI_swStatusTx = 0u;

        TFT_SPI_EnableTxInt();

    #else

        tmpStatus = TFT_SPI_TX_STATUS_REG;

    #endif /* (TFT_SPI_TX_SOFTWARE_BUF_ENABLED) */

    return(tmpStatus);
}


/*******************************************************************************
* Function Name: TFT_SPI_ReadRxStatus
********************************************************************************
*
* Summary:
*  Read the Rx status register for the component.
*
* Parameters:
*  None.
*
* Return:
*  Contents of the Rx status register.
*
* Global variables:
*  TFT_SPI_swStatusRx - used to store in software Rx status register,
*  modified every function call - resets to zero.
*
* Theory:
*  Allows the user and the API to read the Rx status register for error
*  detection and flow control.
*
* Side Effects:
*  Clear Rx status register of the component.
*
* Reentrant:
*  No.
*
*******************************************************************************/
uint8 TFT_SPI_ReadRxStatus(void) 
{
    uint8 tmpStatus;

    #if(TFT_SPI_RX_SOFTWARE_BUF_ENABLED)
        /* Disable RX interrupt to protect global veriables */
        TFT_SPI_DisableRxInt();

        tmpStatus = TFT_SPI_GET_STATUS_RX(TFT_SPI_swStatusRx);
        TFT_SPI_swStatusRx = 0u;

        TFT_SPI_EnableRxInt();

    #else

        tmpStatus = TFT_SPI_RX_STATUS_REG;

    #endif /* (TFT_SPI_RX_SOFTWARE_BUF_ENABLED) */

    return(tmpStatus);
}


/*******************************************************************************
* Function Name: TFT_SPI_WriteTxData
********************************************************************************
*
* Summary:
*  Write a byte of data to be sent across the SPI.
*
* Parameters:
*  txDataByte: The data value to send across the SPI.
*
* Return:
*  None.
*
* Global variables:
*  TFT_SPI_txBufferWrite - used for the account of the bytes which
*  have been written down in the TX software buffer, modified every function
*  call if TX Software Buffer is used.
*  TFT_SPI_txBufferRead - used for the account of the bytes which
*  have been read from the TX software buffer.
*  TFT_SPI_txBuffer[TFT_SPI_TX_BUFFER_SIZE] - used to store
*  data to sending, modified every function call if TX Software Buffer is used.
*
* Theory:
*  Allows the user to transmit any byte of data in a single transfer.
*
* Side Effects:
*  If this function is called again before the previous byte is finished then
*  the next byte will be appended to the transfer with no time between
*  the byte transfers. Clear Tx status register of the component.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void TFT_SPI_WriteTxData(uint8 txData) 
{
    #if(TFT_SPI_TX_SOFTWARE_BUF_ENABLED)

        uint8 tempStatus;
        uint8 tmpTxBufferRead;

        /* Block if TX buffer is FULL: don't overwrite */
        do
        {
            tmpTxBufferRead = TFT_SPI_txBufferRead;
            if(0u == tmpTxBufferRead)
            {
                tmpTxBufferRead = (TFT_SPI_TX_BUFFER_SIZE - 1u);
            }
            else
            {
                tmpTxBufferRead--;
            }

        }while(tmpTxBufferRead == TFT_SPI_txBufferWrite);

        /* Disable TX interrupt to protect global veriables */
        TFT_SPI_DisableTxInt();

        tempStatus = TFT_SPI_GET_STATUS_TX(TFT_SPI_swStatusTx);
        TFT_SPI_swStatusTx = tempStatus;


        if((TFT_SPI_txBufferRead == TFT_SPI_txBufferWrite) &&
           (0u != (TFT_SPI_swStatusTx & TFT_SPI_STS_TX_FIFO_NOT_FULL)))
        {
            /* Put data element into the TX FIFO */
            CY_SET_REG8(TFT_SPI_TXDATA_PTR, txData);
        }
        else
        {
            /* Add to the TX software buffer */
            TFT_SPI_txBufferWrite++;
            if(TFT_SPI_txBufferWrite >= TFT_SPI_TX_BUFFER_SIZE)
            {
                TFT_SPI_txBufferWrite = 0u;
            }

            if(TFT_SPI_txBufferWrite == TFT_SPI_txBufferRead)
            {
                TFT_SPI_txBufferRead++;
                if(TFT_SPI_txBufferRead >= TFT_SPI_TX_BUFFER_SIZE)
                {
                    TFT_SPI_txBufferRead = 0u;
                }
                TFT_SPI_txBufferFull = 1u;
            }

            TFT_SPI_txBuffer[TFT_SPI_txBufferWrite] = txData;

            TFT_SPI_TX_STATUS_MASK_REG |= TFT_SPI_STS_TX_FIFO_NOT_FULL;
        }

        TFT_SPI_EnableTxInt();

    #else
        /* Wait until TX FIFO has a place */
        while(0u == (TFT_SPI_TX_STATUS_REG & TFT_SPI_STS_TX_FIFO_NOT_FULL))
        {
        }

        /* Put data element into the TX FIFO */
        CY_SET_REG8(TFT_SPI_TXDATA_PTR, txData);

    #endif /* (TFT_SPI_TX_SOFTWARE_BUF_ENABLED) */
}


/*******************************************************************************
* Function Name: TFT_SPI_ReadRxData
********************************************************************************
*
* Summary:
*  Read the next byte of data received across the SPI.
*
* Parameters:
*  None.
*
* Return:
*  The next byte of data read from the FIFO.
*
* Global variables:
*  TFT_SPI_rxBufferWrite - used for the account of the bytes which
*  have been written down in the RX software buffer.
*  TFT_SPI_rxBufferRead - used for the account of the bytes which
*  have been read from the RX software buffer, modified every function
*  call if RX Software Buffer is used.
*  TFT_SPI_rxBuffer[TFT_SPI_RX_BUFFER_SIZE] - used to store
*  received data.
*
* Theory:
*  Allows the user to read a byte of data received.
*
* Side Effects:
*  Will return invalid data if the FIFO is empty. The user should Call
*  GetRxBufferSize() and if it returns a non-zero value then it is safe to call
*  ReadByte() function.
*
* Reentrant:
*  No.
*
*******************************************************************************/
uint8 TFT_SPI_ReadRxData(void) 
{
    uint8 rxData;

    #if(TFT_SPI_RX_SOFTWARE_BUF_ENABLED)

        /* Disable RX interrupt to protect global veriables */
        TFT_SPI_DisableRxInt();

        if(TFT_SPI_rxBufferRead != TFT_SPI_rxBufferWrite)
        {
            if(0u == TFT_SPI_rxBufferFull)
            {
                TFT_SPI_rxBufferRead++;
                if(TFT_SPI_rxBufferRead >= TFT_SPI_RX_BUFFER_SIZE)
                {
                    TFT_SPI_rxBufferRead = 0u;
                }
            }
            else
            {
                TFT_SPI_rxBufferFull = 0u;
            }
        }

        rxData = TFT_SPI_rxBuffer[TFT_SPI_rxBufferRead];

        TFT_SPI_EnableRxInt();

    #else

        rxData = CY_GET_REG8(TFT_SPI_RXDATA_PTR);

    #endif /* (TFT_SPI_RX_SOFTWARE_BUF_ENABLED) */

    return(rxData);
}


/*******************************************************************************
* Function Name: TFT_SPI_GetRxBufferSize
********************************************************************************
*
* Summary:
*  Returns the number of bytes/words of data currently held in the RX buffer.
*  If RX Software Buffer not used then function return 0 if FIFO empty or 1 if
*  FIFO not empty. In another case function return size of RX Software Buffer.
*
* Parameters:
*  None.
*
* Return:
*  Integer count of the number of bytes/words in the RX buffer.
*
* Global variables:
*  TFT_SPI_rxBufferWrite - used for the account of the bytes which
*  have been written down in the RX software buffer.
*  TFT_SPI_rxBufferRead - used for the account of the bytes which
*  have been read from the RX software buffer.
*
* Side Effects:
*  Clear status register of the component.
*
*******************************************************************************/
uint8 TFT_SPI_GetRxBufferSize(void) 
{
    uint8 size;

    #if(TFT_SPI_RX_SOFTWARE_BUF_ENABLED)

        /* Disable RX interrupt to protect global veriables */
        TFT_SPI_DisableRxInt();

        if(TFT_SPI_rxBufferRead == TFT_SPI_rxBufferWrite)
        {
            size = 0u;
        }
        else if(TFT_SPI_rxBufferRead < TFT_SPI_rxBufferWrite)
        {
            size = (TFT_SPI_rxBufferWrite - TFT_SPI_rxBufferRead);
        }
        else
        {
            size = (TFT_SPI_RX_BUFFER_SIZE - TFT_SPI_rxBufferRead) + TFT_SPI_rxBufferWrite;
        }

        TFT_SPI_EnableRxInt();

    #else

        /* We can only know if there is data in the RX FIFO */
        size = (0u != (TFT_SPI_RX_STATUS_REG & TFT_SPI_STS_RX_FIFO_NOT_EMPTY)) ? 1u : 0u;

    #endif /* (TFT_SPI_TX_SOFTWARE_BUF_ENABLED) */

    return(size);
}


/*******************************************************************************
* Function Name: TFT_SPI_GetTxBufferSize
********************************************************************************
*
* Summary:
*  Returns the number of bytes/words of data currently held in the TX buffer.
*  If TX Software Buffer not used then function return 0 - if FIFO empty, 1 - if
*  FIFO not full, 4 - if FIFO full. In another case function return size of TX
*  Software Buffer.
*
* Parameters:
*  None.
*
* Return:
*  Integer count of the number of bytes/words in the TX buffer.
*
* Global variables:
*  TFT_SPI_txBufferWrite - used for the account of the bytes which
*  have been written down in the TX software buffer.
*  TFT_SPI_txBufferRead - used for the account of the bytes which
*  have been read from the TX software buffer.
*
* Side Effects:
*  Clear status register of the component.
*
*******************************************************************************/
uint8  TFT_SPI_GetTxBufferSize(void) 
{
    uint8 size;

    #if(TFT_SPI_TX_SOFTWARE_BUF_ENABLED)
        /* Disable TX interrupt to protect global veriables */
        TFT_SPI_DisableTxInt();

        if(TFT_SPI_txBufferRead == TFT_SPI_txBufferWrite)
        {
            size = 0u;
        }
        else if(TFT_SPI_txBufferRead < TFT_SPI_txBufferWrite)
        {
            size = (TFT_SPI_txBufferWrite - TFT_SPI_txBufferRead);
        }
        else
        {
            size = (TFT_SPI_TX_BUFFER_SIZE - TFT_SPI_txBufferRead) + TFT_SPI_txBufferWrite;
        }

        TFT_SPI_EnableTxInt();

    #else

        size = TFT_SPI_TX_STATUS_REG;

        if(0u != (size & TFT_SPI_STS_TX_FIFO_EMPTY))
        {
            size = 0u;
        }
        else if(0u != (size & TFT_SPI_STS_TX_FIFO_NOT_FULL))
        {
            size = 1u;
        }
        else
        {
            size = TFT_SPI_FIFO_SIZE;
        }

    #endif /* (TFT_SPI_TX_SOFTWARE_BUF_ENABLED) */

    return(size);
}


/*******************************************************************************
* Function Name: TFT_SPI_ClearRxBuffer
********************************************************************************
*
* Summary:
*  Clear the RX RAM buffer by setting the read and write pointers both to zero.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global variables:
*  TFT_SPI_rxBufferWrite - used for the account of the bytes which
*  have been written down in the RX software buffer, modified every function
*  call - resets to zero.
*  TFT_SPI_rxBufferRead - used for the account of the bytes which
*  have been read from the RX software buffer, modified every function call -
*  resets to zero.
*
* Theory:
*  Setting the pointers to zero makes the system believe there is no data to
*  read and writing will resume at address 0 overwriting any data that may have
*  remained in the RAM.
*
* Side Effects:
*  Any received data not read from the RAM buffer will be lost when overwritten.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void TFT_SPI_ClearRxBuffer(void) 
{
    /* Clear Hardware RX FIFO */
    while(0u !=(TFT_SPI_RX_STATUS_REG & TFT_SPI_STS_RX_FIFO_NOT_EMPTY))
    {
        (void) CY_GET_REG8(TFT_SPI_RXDATA_PTR);
    }

    #if(TFT_SPI_RX_SOFTWARE_BUF_ENABLED)
        /* Disable RX interrupt to protect global veriables */
        TFT_SPI_DisableRxInt();

        TFT_SPI_rxBufferFull  = 0u;
        TFT_SPI_rxBufferRead  = 0u;
        TFT_SPI_rxBufferWrite = 0u;

        TFT_SPI_EnableRxInt();
    #endif /* (TFT_SPI_RX_SOFTWARE_BUF_ENABLED) */
}


/*******************************************************************************
* Function Name: TFT_SPI_ClearTxBuffer
********************************************************************************
*
* Summary:
*  Clear the TX RAM buffer by setting the read and write pointers both to zero.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global variables:
*  TFT_SPI_txBufferWrite - used for the account of the bytes which
*  have been written down in the TX software buffer, modified every function
*  call - resets to zero.
*  TFT_SPI_txBufferRead - used for the account of the bytes which
*  have been read from the TX software buffer, modified every function call -
*  resets to zero.
*
* Theory:
*  Setting the pointers to zero makes the system believe there is no data to
*  read and writing will resume at address 0 overwriting any data that may have
*  remained in the RAM.
*
* Side Effects:
*  Any data not yet transmitted from the RAM buffer will be lost when
*  overwritten.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void TFT_SPI_ClearTxBuffer(void) 
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();
    /* Clear TX FIFO */
    TFT_SPI_AUX_CONTROL_DP0_REG |= ((uint8)  TFT_SPI_TX_FIFO_CLR);
    TFT_SPI_AUX_CONTROL_DP0_REG &= ((uint8) ~TFT_SPI_TX_FIFO_CLR);

    #if(TFT_SPI_USE_SECOND_DATAPATH)
        /* Clear TX FIFO for 2nd Datapath */
        TFT_SPI_AUX_CONTROL_DP1_REG |= ((uint8)  TFT_SPI_TX_FIFO_CLR);
        TFT_SPI_AUX_CONTROL_DP1_REG &= ((uint8) ~TFT_SPI_TX_FIFO_CLR);
    #endif /* (TFT_SPI_USE_SECOND_DATAPATH) */
    CyExitCriticalSection(enableInterrupts);

    #if(TFT_SPI_TX_SOFTWARE_BUF_ENABLED)
        /* Disable TX interrupt to protect global veriables */
        TFT_SPI_DisableTxInt();

        TFT_SPI_txBufferFull  = 0u;
        TFT_SPI_txBufferRead  = 0u;
        TFT_SPI_txBufferWrite = 0u;

        /* Buffer is EMPTY: disable TX FIFO NOT FULL interrupt */
        TFT_SPI_TX_STATUS_MASK_REG &= ((uint8) ~TFT_SPI_STS_TX_FIFO_NOT_FULL);

        TFT_SPI_EnableTxInt();
    #endif /* (TFT_SPI_TX_SOFTWARE_BUF_ENABLED) */
}


#if(0u != TFT_SPI_BIDIRECTIONAL_MODE)
    /*******************************************************************************
    * Function Name: TFT_SPI_TxEnable
    ********************************************************************************
    *
    * Summary:
    *  If the SPI master is configured to use a single bi-directional pin then this
    *  will set the bi-directional pin to transmit.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    *******************************************************************************/
    void TFT_SPI_TxEnable(void) 
    {
        TFT_SPI_CONTROL_REG |= TFT_SPI_CTRL_TX_SIGNAL_EN;
    }


    /*******************************************************************************
    * Function Name: TFT_SPI_TxDisable
    ********************************************************************************
    *
    * Summary:
    *  If the SPI master is configured to use a single bi-directional pin then this
    *  will set the bi-directional pin to receive.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    *******************************************************************************/
    void TFT_SPI_TxDisable(void) 
    {
        TFT_SPI_CONTROL_REG &= ((uint8) ~TFT_SPI_CTRL_TX_SIGNAL_EN);
    }

#endif /* (0u != TFT_SPI_BIDIRECTIONAL_MODE) */


/*******************************************************************************
* Function Name: TFT_SPI_PutArray
********************************************************************************
*
* Summary:
*  Write available data from ROM/RAM to the TX buffer while space is available
*  in the TX buffer. Keep trying until all data is passed to the TX buffer.
*
* Parameters:
*  *buffer: Pointer to the location in RAM containing the data to send
*  byteCount: The number of bytes to move to the transmit buffer.
*
* Return:
*  None.
*
* Side Effects:
*  Will stay in this routine until all data has been sent.  May get locked in
*  this loop if data is not being initiated by the master if there is not
*  enough room in the TX FIFO.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void TFT_SPI_PutArray(const uint8 buffer[], uint8 byteCount)
                                                                          
{
    uint8 bufIndex;

    bufIndex = 0u;

    while(byteCount > 0u)
    {
        TFT_SPI_WriteTxData(buffer[bufIndex]);
        bufIndex++;
        byteCount--;
    }
}


/*******************************************************************************
* Function Name: TFT_SPI_ClearFIFO
********************************************************************************
*
* Summary:
*  Clear the RX and TX FIFO's of all data for a fresh start.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Side Effects:
*  Clear status register of the component.
*
*******************************************************************************/
void TFT_SPI_ClearFIFO(void) 
{
    uint8 enableInterrupts;

    /* Clear Hardware RX FIFO */
    while(0u !=(TFT_SPI_RX_STATUS_REG & TFT_SPI_STS_RX_FIFO_NOT_EMPTY))
    {
        (void) CY_GET_REG8(TFT_SPI_RXDATA_PTR);
    }

    enableInterrupts = CyEnterCriticalSection();
    /* Clear TX FIFO */
    TFT_SPI_AUX_CONTROL_DP0_REG |= ((uint8)  TFT_SPI_TX_FIFO_CLR);
    TFT_SPI_AUX_CONTROL_DP0_REG &= ((uint8) ~TFT_SPI_TX_FIFO_CLR);

    #if(TFT_SPI_USE_SECOND_DATAPATH)
        /* Clear TX FIFO for 2nd Datapath */
        TFT_SPI_AUX_CONTROL_DP1_REG |= ((uint8)  TFT_SPI_TX_FIFO_CLR);
        TFT_SPI_AUX_CONTROL_DP1_REG &= ((uint8) ~TFT_SPI_TX_FIFO_CLR);
    #endif /* (TFT_SPI_USE_SECOND_DATAPATH) */
    CyExitCriticalSection(enableInterrupts);
}


/* Following functions are for version Compatibility, they are obsolete.
*  Please do not use it in new projects.
*/


/*******************************************************************************
* Function Name: TFT_SPI_EnableInt
********************************************************************************
*
* Summary:
*  Enable internal interrupt generation.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Theory:
*  Enable the internal interrupt output -or- the interrupt component itself.
*
*******************************************************************************/
void TFT_SPI_EnableInt(void) 
{
    TFT_SPI_EnableRxInt();
    TFT_SPI_EnableTxInt();
}


/*******************************************************************************
* Function Name: TFT_SPI_DisableInt
********************************************************************************
*
* Summary:
*  Disable internal interrupt generation.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Theory:
*  Disable the internal interrupt output -or- the interrupt component itself.
*
*******************************************************************************/
void TFT_SPI_DisableInt(void) 
{
    TFT_SPI_DisableTxInt();
    TFT_SPI_DisableRxInt();
}


/*******************************************************************************
* Function Name: TFT_SPI_SetInterruptMode
********************************************************************************
*
* Summary:
*  Configure which status bits trigger an interrupt event.
*
* Parameters:
*  intSrc: An or'd combination of the desired status bit masks (defined in the
*  header file).
*
* Return:
*  None.
*
* Theory:
*  Enables the output of specific status bits to the interrupt controller.
*
*******************************************************************************/
void TFT_SPI_SetInterruptMode(uint8 intSrc) 
{
    TFT_SPI_TX_STATUS_MASK_REG  = (intSrc & ((uint8) ~TFT_SPI_STS_SPI_IDLE));
    TFT_SPI_RX_STATUS_MASK_REG  =  intSrc;
}


/*******************************************************************************
* Function Name: TFT_SPI_ReadStatus
********************************************************************************
*
* Summary:
*  Read the status register for the component.
*
* Parameters:
*  None.
*
* Return:
*  Contents of the status register.
*
* Global variables:
*  TFT_SPI_swStatus - used to store in software status register,
*  modified every function call - resets to zero.
*
* Theory:
*  Allows the user and the API to read the status register for error detection
*  and flow control.
*
* Side Effects:
*  Clear status register of the component.
*
* Reentrant:
*  No.
*
*******************************************************************************/
uint8 TFT_SPI_ReadStatus(void) 
{
    uint8 tmpStatus;

    #if(TFT_SPI_TX_SOFTWARE_BUF_ENABLED || TFT_SPI_RX_SOFTWARE_BUF_ENABLED)

        TFT_SPI_DisableInt();

        tmpStatus  = TFT_SPI_GET_STATUS_RX(TFT_SPI_swStatusRx);
        tmpStatus |= TFT_SPI_GET_STATUS_TX(TFT_SPI_swStatusTx);
        tmpStatus &= ((uint8) ~TFT_SPI_STS_SPI_IDLE);

        TFT_SPI_swStatusTx = 0u;
        TFT_SPI_swStatusRx = 0u;

        TFT_SPI_EnableInt();

    #else

        tmpStatus  = TFT_SPI_RX_STATUS_REG;
        tmpStatus |= TFT_SPI_TX_STATUS_REG;
        tmpStatus &= ((uint8) ~TFT_SPI_STS_SPI_IDLE);

    #endif /* (TFT_SPI_TX_SOFTWARE_BUF_ENABLED || TFT_SPI_RX_SOFTWARE_BUF_ENABLED) */

    return(tmpStatus);
}


/* [] END OF FILE */
