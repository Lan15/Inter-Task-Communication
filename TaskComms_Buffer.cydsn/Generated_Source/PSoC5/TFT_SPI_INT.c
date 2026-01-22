/*******************************************************************************
* File Name: TFT_SPI_INT.c
* Version 2.50
*
* Description:
*  This file provides all Interrupt Service Routine (ISR) for the SPI Master
*  component.
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


/* User code required at start of ISR */
/* `#START TFT_SPI_ISR_START_DEF` */

/* `#END` */


/*******************************************************************************
* Function Name: TFT_SPI_TX_ISR
********************************************************************************
*
* Summary:
*  Interrupt Service Routine for TX portion of the SPI Master.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global variables:
*  TFT_SPI_txBufferWrite - used for the account of the bytes which
*  have been written down in the TX software buffer.
*  TFT_SPI_txBufferRead - used for the account of the bytes which
*  have been read from the TX software buffer, modified when exist data to
*  sending and FIFO Not Full.
*  TFT_SPI_txBuffer[TFT_SPI_TX_BUFFER_SIZE] - used to store
*  data to sending.
*  All described above Global variables are used when Software Buffer is used.
*
*******************************************************************************/
CY_ISR(TFT_SPI_TX_ISR)
{
    #if(TFT_SPI_TX_SOFTWARE_BUF_ENABLED)
        uint8 tmpStatus;
    #endif /* (TFT_SPI_TX_SOFTWARE_BUF_ENABLED) */

    #ifdef TFT_SPI_TX_ISR_ENTRY_CALLBACK
        TFT_SPI_TX_ISR_EntryCallback();
    #endif /* TFT_SPI_TX_ISR_ENTRY_CALLBACK */

    /* User code required at start of ISR */
    /* `#START TFT_SPI_TX_ISR_START` */

    /* `#END` */
    
    #if(TFT_SPI_TX_SOFTWARE_BUF_ENABLED)
        /* Check if TX data buffer is not empty and there is space in TX FIFO */
        while(TFT_SPI_txBufferRead != TFT_SPI_txBufferWrite)
        {
            tmpStatus = TFT_SPI_GET_STATUS_TX(TFT_SPI_swStatusTx);
            TFT_SPI_swStatusTx = tmpStatus;

            if(0u != (TFT_SPI_swStatusTx & TFT_SPI_STS_TX_FIFO_NOT_FULL))
            {
                if(0u == TFT_SPI_txBufferFull)
                {
                   TFT_SPI_txBufferRead++;

                    if(TFT_SPI_txBufferRead >= TFT_SPI_TX_BUFFER_SIZE)
                    {
                        TFT_SPI_txBufferRead = 0u;
                    }
                }
                else
                {
                    TFT_SPI_txBufferFull = 0u;
                }

                /* Put data element into the TX FIFO */
                CY_SET_REG8(TFT_SPI_TXDATA_PTR, 
                                             TFT_SPI_txBuffer[TFT_SPI_txBufferRead]);
            }
            else
            {
                break;
            }
        }

        if(TFT_SPI_txBufferRead == TFT_SPI_txBufferWrite)
        {
            /* TX Buffer is EMPTY: disable interrupt on TX NOT FULL */
            TFT_SPI_TX_STATUS_MASK_REG &= ((uint8) ~TFT_SPI_STS_TX_FIFO_NOT_FULL);
        }

    #endif /* (TFT_SPI_TX_SOFTWARE_BUF_ENABLED) */

    /* User code required at end of ISR (Optional) */
    /* `#START TFT_SPI_TX_ISR_END` */

    /* `#END` */
    
    #ifdef TFT_SPI_TX_ISR_EXIT_CALLBACK
        TFT_SPI_TX_ISR_ExitCallback();
    #endif /* TFT_SPI_TX_ISR_EXIT_CALLBACK */
}


/*******************************************************************************
* Function Name: TFT_SPI_RX_ISR
********************************************************************************
*
* Summary:
*  Interrupt Service Routine for RX portion of the SPI Master.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global variables:
*  TFT_SPI_rxBufferWrite - used for the account of the bytes which
*  have been written down in the RX software buffer modified when FIFO contains
*  new data.
*  TFT_SPI_rxBufferRead - used for the account of the bytes which
*  have been read from the RX software buffer, modified when overflow occurred.
*  TFT_SPI_rxBuffer[TFT_SPI_RX_BUFFER_SIZE] - used to store
*  received data, modified when FIFO contains new data.
*  All described above Global variables are used when Software Buffer is used.
*
*******************************************************************************/
CY_ISR(TFT_SPI_RX_ISR)
{
    #if(TFT_SPI_RX_SOFTWARE_BUF_ENABLED)
        uint8 tmpStatus;
        uint8 rxData;
    #endif /* (TFT_SPI_RX_SOFTWARE_BUF_ENABLED) */

    #ifdef TFT_SPI_RX_ISR_ENTRY_CALLBACK
        TFT_SPI_RX_ISR_EntryCallback();
    #endif /* TFT_SPI_RX_ISR_ENTRY_CALLBACK */

    /* User code required at start of ISR */
    /* `#START TFT_SPI_RX_ISR_START` */

    /* `#END` */
    
    #if(TFT_SPI_RX_SOFTWARE_BUF_ENABLED)

        tmpStatus = TFT_SPI_GET_STATUS_RX(TFT_SPI_swStatusRx);
        TFT_SPI_swStatusRx = tmpStatus;

        /* Check if RX data FIFO has some data to be moved into the RX Buffer */
        while(0u != (TFT_SPI_swStatusRx & TFT_SPI_STS_RX_FIFO_NOT_EMPTY))
        {
            rxData = CY_GET_REG8(TFT_SPI_RXDATA_PTR);

            /* Set next pointer. */
            TFT_SPI_rxBufferWrite++;
            if(TFT_SPI_rxBufferWrite >= TFT_SPI_RX_BUFFER_SIZE)
            {
                TFT_SPI_rxBufferWrite = 0u;
            }

            if(TFT_SPI_rxBufferWrite == TFT_SPI_rxBufferRead)
            {
                TFT_SPI_rxBufferRead++;
                if(TFT_SPI_rxBufferRead >= TFT_SPI_RX_BUFFER_SIZE)
                {
                    TFT_SPI_rxBufferRead = 0u;
                }

                TFT_SPI_rxBufferFull = 1u;
            }

            /* Move data from the FIFO to the Buffer */
            TFT_SPI_rxBuffer[TFT_SPI_rxBufferWrite] = rxData;

            tmpStatus = TFT_SPI_GET_STATUS_RX(TFT_SPI_swStatusRx);
            TFT_SPI_swStatusRx = tmpStatus;
        }

    #endif /* (TFT_SPI_RX_SOFTWARE_BUF_ENABLED) */

    /* User code required at end of ISR (Optional) */
    /* `#START TFT_SPI_RX_ISR_END` */

    /* `#END` */
    
    #ifdef TFT_SPI_RX_ISR_EXIT_CALLBACK
        TFT_SPI_RX_ISR_ExitCallback();
    #endif /* TFT_SPI_RX_ISR_EXIT_CALLBACK */
}

/* [] END OF FILE */
