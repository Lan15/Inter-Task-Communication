/*******************************************************************************
* File Name: TFT_SPI.h
* Version 2.50
*
* Description:
*  Contains the function prototypes, constants and register definition
*  of the SPI Master Component.
*
* Note:
*  None
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_SPIM_TFT_SPI_H)
#define CY_SPIM_TFT_SPI_H

#include "cyfitter.h"
#include "cytypes.h"
#include "CyLib.h" /* For CyEnterCriticalSection() and CyExitCriticalSection() functions */


/***************************************
*   Conditional Compilation Parameters
***************************************/

#define TFT_SPI_INTERNAL_CLOCK             (1u)

#if(0u != TFT_SPI_INTERNAL_CLOCK)
    #include "TFT_SPI_IntClock.h"
#endif /* (0u != TFT_SPI_INTERNAL_CLOCK) */

#define TFT_SPI_MODE                       (1u)
#define TFT_SPI_DATA_WIDTH                 (8u)
#define TFT_SPI_MODE_USE_ZERO              (1u)
#define TFT_SPI_BIDIRECTIONAL_MODE         (0u)

/* Internal interrupt handling */
#define TFT_SPI_TX_BUFFER_SIZE             (4u)
#define TFT_SPI_RX_BUFFER_SIZE             (4u)
#define TFT_SPI_INTERNAL_TX_INT_ENABLED    (0u)
#define TFT_SPI_INTERNAL_RX_INT_ENABLED    (0u)

#define TFT_SPI_SINGLE_REG_SIZE            (8u)
#define TFT_SPI_USE_SECOND_DATAPATH        (TFT_SPI_DATA_WIDTH > TFT_SPI_SINGLE_REG_SIZE)

#define TFT_SPI_FIFO_SIZE                  (4u)
#define TFT_SPI_TX_SOFTWARE_BUF_ENABLED    ((0u != TFT_SPI_INTERNAL_TX_INT_ENABLED) && \
                                                     (TFT_SPI_TX_BUFFER_SIZE > TFT_SPI_FIFO_SIZE))

#define TFT_SPI_RX_SOFTWARE_BUF_ENABLED    ((0u != TFT_SPI_INTERNAL_RX_INT_ENABLED) && \
                                                     (TFT_SPI_RX_BUFFER_SIZE > TFT_SPI_FIFO_SIZE))


/***************************************
*        Data Struct Definition
***************************************/

/* Sleep Mode API Support */
typedef struct
{
    uint8 enableState;
    uint8 cntrPeriod;
} TFT_SPI_BACKUP_STRUCT;


/***************************************
*        Function Prototypes
***************************************/

void  TFT_SPI_Init(void)                           ;
void  TFT_SPI_Enable(void)                         ;
void  TFT_SPI_Start(void)                          ;
void  TFT_SPI_Stop(void)                           ;

void  TFT_SPI_EnableTxInt(void)                    ;
void  TFT_SPI_EnableRxInt(void)                    ;
void  TFT_SPI_DisableTxInt(void)                   ;
void  TFT_SPI_DisableRxInt(void)                   ;

void  TFT_SPI_Sleep(void)                          ;
void  TFT_SPI_Wakeup(void)                         ;
void  TFT_SPI_SaveConfig(void)                     ;
void  TFT_SPI_RestoreConfig(void)                  ;

void  TFT_SPI_SetTxInterruptMode(uint8 intSrc)     ;
void  TFT_SPI_SetRxInterruptMode(uint8 intSrc)     ;
uint8 TFT_SPI_ReadTxStatus(void)                   ;
uint8 TFT_SPI_ReadRxStatus(void)                   ;
void  TFT_SPI_WriteTxData(uint8 txData)  \
                                                            ;
uint8 TFT_SPI_ReadRxData(void) \
                                                            ;
uint8 TFT_SPI_GetRxBufferSize(void)                ;
uint8 TFT_SPI_GetTxBufferSize(void)                ;
void  TFT_SPI_ClearRxBuffer(void)                  ;
void  TFT_SPI_ClearTxBuffer(void)                  ;
void  TFT_SPI_ClearFIFO(void)                              ;
void  TFT_SPI_PutArray(const uint8 buffer[], uint8 byteCount) \
                                                            ;

#if(0u != TFT_SPI_BIDIRECTIONAL_MODE)
    void  TFT_SPI_TxEnable(void)                   ;
    void  TFT_SPI_TxDisable(void)                  ;
#endif /* (0u != TFT_SPI_BIDIRECTIONAL_MODE) */

CY_ISR_PROTO(TFT_SPI_TX_ISR);
CY_ISR_PROTO(TFT_SPI_RX_ISR);


/***************************************
*   Variable with external linkage
***************************************/

extern uint8 TFT_SPI_initVar;


/***************************************
*           API Constants
***************************************/

#define TFT_SPI_TX_ISR_NUMBER     ((uint8) (TFT_SPI_TxInternalInterrupt__INTC_NUMBER))
#define TFT_SPI_RX_ISR_NUMBER     ((uint8) (TFT_SPI_RxInternalInterrupt__INTC_NUMBER))

#define TFT_SPI_TX_ISR_PRIORITY   ((uint8) (TFT_SPI_TxInternalInterrupt__INTC_PRIOR_NUM))
#define TFT_SPI_RX_ISR_PRIORITY   ((uint8) (TFT_SPI_RxInternalInterrupt__INTC_PRIOR_NUM))


/***************************************
*    Initial Parameter Constants
***************************************/

#define TFT_SPI_INT_ON_SPI_DONE    ((uint8) (0u   << TFT_SPI_STS_SPI_DONE_SHIFT))
#define TFT_SPI_INT_ON_TX_EMPTY    ((uint8) (0u   << TFT_SPI_STS_TX_FIFO_EMPTY_SHIFT))
#define TFT_SPI_INT_ON_TX_NOT_FULL ((uint8) (0u << \
                                                                           TFT_SPI_STS_TX_FIFO_NOT_FULL_SHIFT))
#define TFT_SPI_INT_ON_BYTE_COMP   ((uint8) (0u  << TFT_SPI_STS_BYTE_COMPLETE_SHIFT))
#define TFT_SPI_INT_ON_SPI_IDLE    ((uint8) (0u   << TFT_SPI_STS_SPI_IDLE_SHIFT))

/* Disable TX_NOT_FULL if software buffer is used */
#define TFT_SPI_INT_ON_TX_NOT_FULL_DEF ((TFT_SPI_TX_SOFTWARE_BUF_ENABLED) ? \
                                                                        (0u) : (TFT_SPI_INT_ON_TX_NOT_FULL))

/* TX interrupt mask */
#define TFT_SPI_TX_INIT_INTERRUPTS_MASK    (TFT_SPI_INT_ON_SPI_DONE  | \
                                                     TFT_SPI_INT_ON_TX_EMPTY  | \
                                                     TFT_SPI_INT_ON_TX_NOT_FULL_DEF | \
                                                     TFT_SPI_INT_ON_BYTE_COMP | \
                                                     TFT_SPI_INT_ON_SPI_IDLE)

#define TFT_SPI_INT_ON_RX_FULL         ((uint8) (0u << \
                                                                          TFT_SPI_STS_RX_FIFO_FULL_SHIFT))
#define TFT_SPI_INT_ON_RX_NOT_EMPTY    ((uint8) (0u << \
                                                                          TFT_SPI_STS_RX_FIFO_NOT_EMPTY_SHIFT))
#define TFT_SPI_INT_ON_RX_OVER         ((uint8) (0u << \
                                                                          TFT_SPI_STS_RX_FIFO_OVERRUN_SHIFT))

/* RX interrupt mask */
#define TFT_SPI_RX_INIT_INTERRUPTS_MASK    (TFT_SPI_INT_ON_RX_FULL      | \
                                                     TFT_SPI_INT_ON_RX_NOT_EMPTY | \
                                                     TFT_SPI_INT_ON_RX_OVER)
/* Nubmer of bits to receive/transmit */
#define TFT_SPI_BITCTR_INIT            (((uint8) (TFT_SPI_DATA_WIDTH << 1u)) - 1u)


/***************************************
*             Registers
***************************************/
#if(CY_PSOC3 || CY_PSOC5)
    #define TFT_SPI_TXDATA_REG (* (reg8 *) \
                                                TFT_SPI_BSPIM_sR8_Dp_u0__F0_REG)
    #define TFT_SPI_TXDATA_PTR (  (reg8 *) \
                                                TFT_SPI_BSPIM_sR8_Dp_u0__F0_REG)
    #define TFT_SPI_RXDATA_REG (* (reg8 *) \
                                                TFT_SPI_BSPIM_sR8_Dp_u0__F1_REG)
    #define TFT_SPI_RXDATA_PTR (  (reg8 *) \
                                                TFT_SPI_BSPIM_sR8_Dp_u0__F1_REG)
#else   /* PSOC4 */
    #if(TFT_SPI_USE_SECOND_DATAPATH)
        #define TFT_SPI_TXDATA_REG (* (reg16 *) \
                                          TFT_SPI_BSPIM_sR8_Dp_u0__16BIT_F0_REG)
        #define TFT_SPI_TXDATA_PTR (  (reg16 *) \
                                          TFT_SPI_BSPIM_sR8_Dp_u0__16BIT_F0_REG)
        #define TFT_SPI_RXDATA_REG (* (reg16 *) \
                                          TFT_SPI_BSPIM_sR8_Dp_u0__16BIT_F1_REG)
        #define TFT_SPI_RXDATA_PTR (  (reg16 *) \
                                          TFT_SPI_BSPIM_sR8_Dp_u0__16BIT_F1_REG)
    #else
        #define TFT_SPI_TXDATA_REG (* (reg8 *) \
                                                TFT_SPI_BSPIM_sR8_Dp_u0__F0_REG)
        #define TFT_SPI_TXDATA_PTR (  (reg8 *) \
                                                TFT_SPI_BSPIM_sR8_Dp_u0__F0_REG)
        #define TFT_SPI_RXDATA_REG (* (reg8 *) \
                                                TFT_SPI_BSPIM_sR8_Dp_u0__F1_REG)
        #define TFT_SPI_RXDATA_PTR (  (reg8 *) \
                                                TFT_SPI_BSPIM_sR8_Dp_u0__F1_REG)
    #endif /* (TFT_SPI_USE_SECOND_DATAPATH) */
#endif     /* (CY_PSOC3 || CY_PSOC5) */

#define TFT_SPI_AUX_CONTROL_DP0_REG (* (reg8 *) \
                                        TFT_SPI_BSPIM_sR8_Dp_u0__DP_AUX_CTL_REG)
#define TFT_SPI_AUX_CONTROL_DP0_PTR (  (reg8 *) \
                                        TFT_SPI_BSPIM_sR8_Dp_u0__DP_AUX_CTL_REG)

#if(TFT_SPI_USE_SECOND_DATAPATH)
    #define TFT_SPI_AUX_CONTROL_DP1_REG  (* (reg8 *) \
                                        TFT_SPI_BSPIM_sR8_Dp_u1__DP_AUX_CTL_REG)
    #define TFT_SPI_AUX_CONTROL_DP1_PTR  (  (reg8 *) \
                                        TFT_SPI_BSPIM_sR8_Dp_u1__DP_AUX_CTL_REG)
#endif /* (TFT_SPI_USE_SECOND_DATAPATH) */

#define TFT_SPI_COUNTER_PERIOD_REG     (* (reg8 *) TFT_SPI_BSPIM_BitCounter__PERIOD_REG)
#define TFT_SPI_COUNTER_PERIOD_PTR     (  (reg8 *) TFT_SPI_BSPIM_BitCounter__PERIOD_REG)
#define TFT_SPI_COUNTER_CONTROL_REG    (* (reg8 *) TFT_SPI_BSPIM_BitCounter__CONTROL_AUX_CTL_REG)
#define TFT_SPI_COUNTER_CONTROL_PTR    (  (reg8 *) TFT_SPI_BSPIM_BitCounter__CONTROL_AUX_CTL_REG)

#define TFT_SPI_TX_STATUS_REG          (* (reg8 *) TFT_SPI_BSPIM_TxStsReg__STATUS_REG)
#define TFT_SPI_TX_STATUS_PTR          (  (reg8 *) TFT_SPI_BSPIM_TxStsReg__STATUS_REG)
#define TFT_SPI_RX_STATUS_REG          (* (reg8 *) TFT_SPI_BSPIM_RxStsReg__STATUS_REG)
#define TFT_SPI_RX_STATUS_PTR          (  (reg8 *) TFT_SPI_BSPIM_RxStsReg__STATUS_REG)

#define TFT_SPI_CONTROL_REG            (* (reg8 *) \
                                      TFT_SPI_BSPIM_BidirMode_CtrlReg__CONTROL_REG)
#define TFT_SPI_CONTROL_PTR            (  (reg8 *) \
                                      TFT_SPI_BSPIM_BidirMode_CtrlReg__CONTROL_REG)

#define TFT_SPI_TX_STATUS_MASK_REG     (* (reg8 *) TFT_SPI_BSPIM_TxStsReg__MASK_REG)
#define TFT_SPI_TX_STATUS_MASK_PTR     (  (reg8 *) TFT_SPI_BSPIM_TxStsReg__MASK_REG)
#define TFT_SPI_RX_STATUS_MASK_REG     (* (reg8 *) TFT_SPI_BSPIM_RxStsReg__MASK_REG)
#define TFT_SPI_RX_STATUS_MASK_PTR     (  (reg8 *) TFT_SPI_BSPIM_RxStsReg__MASK_REG)

#define TFT_SPI_TX_STATUS_ACTL_REG     (* (reg8 *) TFT_SPI_BSPIM_TxStsReg__STATUS_AUX_CTL_REG)
#define TFT_SPI_TX_STATUS_ACTL_PTR     (  (reg8 *) TFT_SPI_BSPIM_TxStsReg__STATUS_AUX_CTL_REG)
#define TFT_SPI_RX_STATUS_ACTL_REG     (* (reg8 *) TFT_SPI_BSPIM_RxStsReg__STATUS_AUX_CTL_REG)
#define TFT_SPI_RX_STATUS_ACTL_PTR     (  (reg8 *) TFT_SPI_BSPIM_RxStsReg__STATUS_AUX_CTL_REG)

#if(TFT_SPI_USE_SECOND_DATAPATH)
    #define TFT_SPI_AUX_CONTROLDP1     (TFT_SPI_AUX_CONTROL_DP1_REG)
#endif /* (TFT_SPI_USE_SECOND_DATAPATH) */


/***************************************
*       Register Constants
***************************************/

/* Status Register Definitions */
#define TFT_SPI_STS_SPI_DONE_SHIFT             (0x00u)
#define TFT_SPI_STS_TX_FIFO_EMPTY_SHIFT        (0x01u)
#define TFT_SPI_STS_TX_FIFO_NOT_FULL_SHIFT     (0x02u)
#define TFT_SPI_STS_BYTE_COMPLETE_SHIFT        (0x03u)
#define TFT_SPI_STS_SPI_IDLE_SHIFT             (0x04u)
#define TFT_SPI_STS_RX_FIFO_FULL_SHIFT         (0x04u)
#define TFT_SPI_STS_RX_FIFO_NOT_EMPTY_SHIFT    (0x05u)
#define TFT_SPI_STS_RX_FIFO_OVERRUN_SHIFT      (0x06u)

#define TFT_SPI_STS_SPI_DONE           ((uint8) (0x01u << TFT_SPI_STS_SPI_DONE_SHIFT))
#define TFT_SPI_STS_TX_FIFO_EMPTY      ((uint8) (0x01u << TFT_SPI_STS_TX_FIFO_EMPTY_SHIFT))
#define TFT_SPI_STS_TX_FIFO_NOT_FULL   ((uint8) (0x01u << TFT_SPI_STS_TX_FIFO_NOT_FULL_SHIFT))
#define TFT_SPI_STS_BYTE_COMPLETE      ((uint8) (0x01u << TFT_SPI_STS_BYTE_COMPLETE_SHIFT))
#define TFT_SPI_STS_SPI_IDLE           ((uint8) (0x01u << TFT_SPI_STS_SPI_IDLE_SHIFT))
#define TFT_SPI_STS_RX_FIFO_FULL       ((uint8) (0x01u << TFT_SPI_STS_RX_FIFO_FULL_SHIFT))
#define TFT_SPI_STS_RX_FIFO_NOT_EMPTY  ((uint8) (0x01u << TFT_SPI_STS_RX_FIFO_NOT_EMPTY_SHIFT))
#define TFT_SPI_STS_RX_FIFO_OVERRUN    ((uint8) (0x01u << TFT_SPI_STS_RX_FIFO_OVERRUN_SHIFT))

/* TX and RX masks for clear on read bits */
#define TFT_SPI_TX_STS_CLR_ON_RD_BYTES_MASK    (0x09u)
#define TFT_SPI_RX_STS_CLR_ON_RD_BYTES_MASK    (0x40u)

/* StatusI Register Interrupt Enable Control Bits */
/* As defined by the Register map for the AUX Control Register */
#define TFT_SPI_INT_ENABLE     (0x10u) /* Enable interrupt from statusi */
#define TFT_SPI_TX_FIFO_CLR    (0x01u) /* F0 - TX FIFO */
#define TFT_SPI_RX_FIFO_CLR    (0x02u) /* F1 - RX FIFO */
#define TFT_SPI_FIFO_CLR       (TFT_SPI_TX_FIFO_CLR | TFT_SPI_RX_FIFO_CLR)

/* Bit Counter (7-bit) Control Register Bit Definitions */
/* As defined by the Register map for the AUX Control Register */
#define TFT_SPI_CNTR_ENABLE    (0x20u) /* Enable CNT7 */

/* Bi-Directional mode control bit */
#define TFT_SPI_CTRL_TX_SIGNAL_EN  (0x01u)

/* Datapath Auxillary Control Register definitions */
#define TFT_SPI_AUX_CTRL_FIFO0_CLR         (0x01u)
#define TFT_SPI_AUX_CTRL_FIFO1_CLR         (0x02u)
#define TFT_SPI_AUX_CTRL_FIFO0_LVL         (0x04u)
#define TFT_SPI_AUX_CTRL_FIFO1_LVL         (0x08u)
#define TFT_SPI_STATUS_ACTL_INT_EN_MASK    (0x10u)

/* Component disabled */
#define TFT_SPI_DISABLED   (0u)


/***************************************
*       Macros
***************************************/

/* Returns true if componentn enabled */
#define TFT_SPI_IS_ENABLED (0u != (TFT_SPI_TX_STATUS_ACTL_REG & TFT_SPI_INT_ENABLE))

/* Retuns TX status register */
#define TFT_SPI_GET_STATUS_TX(swTxSts) ( (uint8)(TFT_SPI_TX_STATUS_REG | \
                                                          ((swTxSts) & TFT_SPI_TX_STS_CLR_ON_RD_BYTES_MASK)) )
/* Retuns RX status register */
#define TFT_SPI_GET_STATUS_RX(swRxSts) ( (uint8)(TFT_SPI_RX_STATUS_REG | \
                                                          ((swRxSts) & TFT_SPI_RX_STS_CLR_ON_RD_BYTES_MASK)) )


/***************************************
* The following code is DEPRECATED and 
* should not be used in new projects.
***************************************/

#define TFT_SPI_WriteByte   TFT_SPI_WriteTxData
#define TFT_SPI_ReadByte    TFT_SPI_ReadRxData
void  TFT_SPI_SetInterruptMode(uint8 intSrc)       ;
uint8 TFT_SPI_ReadStatus(void)                     ;
void  TFT_SPI_EnableInt(void)                      ;
void  TFT_SPI_DisableInt(void)                     ;

#define TFT_SPI_TXDATA                 (TFT_SPI_TXDATA_REG)
#define TFT_SPI_RXDATA                 (TFT_SPI_RXDATA_REG)
#define TFT_SPI_AUX_CONTROLDP0         (TFT_SPI_AUX_CONTROL_DP0_REG)
#define TFT_SPI_TXBUFFERREAD           (TFT_SPI_txBufferRead)
#define TFT_SPI_TXBUFFERWRITE          (TFT_SPI_txBufferWrite)
#define TFT_SPI_RXBUFFERREAD           (TFT_SPI_rxBufferRead)
#define TFT_SPI_RXBUFFERWRITE          (TFT_SPI_rxBufferWrite)

#define TFT_SPI_COUNTER_PERIOD         (TFT_SPI_COUNTER_PERIOD_REG)
#define TFT_SPI_COUNTER_CONTROL        (TFT_SPI_COUNTER_CONTROL_REG)
#define TFT_SPI_STATUS                 (TFT_SPI_TX_STATUS_REG)
#define TFT_SPI_CONTROL                (TFT_SPI_CONTROL_REG)
#define TFT_SPI_STATUS_MASK            (TFT_SPI_TX_STATUS_MASK_REG)
#define TFT_SPI_STATUS_ACTL            (TFT_SPI_TX_STATUS_ACTL_REG)

#define TFT_SPI_INIT_INTERRUPTS_MASK  (TFT_SPI_INT_ON_SPI_DONE     | \
                                                TFT_SPI_INT_ON_TX_EMPTY     | \
                                                TFT_SPI_INT_ON_TX_NOT_FULL_DEF  | \
                                                TFT_SPI_INT_ON_RX_FULL      | \
                                                TFT_SPI_INT_ON_RX_NOT_EMPTY | \
                                                TFT_SPI_INT_ON_RX_OVER      | \
                                                TFT_SPI_INT_ON_BYTE_COMP)
                                                
#define TFT_SPI_DataWidth                  (TFT_SPI_DATA_WIDTH)
#define TFT_SPI_InternalClockUsed          (TFT_SPI_INTERNAL_CLOCK)
#define TFT_SPI_InternalTxInterruptEnabled (TFT_SPI_INTERNAL_TX_INT_ENABLED)
#define TFT_SPI_InternalRxInterruptEnabled (TFT_SPI_INTERNAL_RX_INT_ENABLED)
#define TFT_SPI_ModeUseZero                (TFT_SPI_MODE_USE_ZERO)
#define TFT_SPI_BidirectionalMode          (TFT_SPI_BIDIRECTIONAL_MODE)
#define TFT_SPI_Mode                       (TFT_SPI_MODE)
#define TFT_SPI_DATAWIDHT                  (TFT_SPI_DATA_WIDTH)
#define TFT_SPI_InternalInterruptEnabled   (0u)

#define TFT_SPI_TXBUFFERSIZE   (TFT_SPI_TX_BUFFER_SIZE)
#define TFT_SPI_RXBUFFERSIZE   (TFT_SPI_RX_BUFFER_SIZE)

#define TFT_SPI_TXBUFFER       TFT_SPI_txBuffer
#define TFT_SPI_RXBUFFER       TFT_SPI_rxBuffer

#endif /* (CY_SPIM_TFT_SPI_H) */


/* [] END OF FILE */
