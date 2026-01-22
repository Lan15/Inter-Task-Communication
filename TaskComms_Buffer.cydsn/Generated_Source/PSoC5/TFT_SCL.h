/*******************************************************************************
* File Name: TFT_SCL.h  
* Version 2.20
*
* Description:
*  This file contains Pin function prototypes and register defines
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_TFT_SCL_H) /* Pins TFT_SCL_H */
#define CY_PINS_TFT_SCL_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "TFT_SCL_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 TFT_SCL__PORT == 15 && ((TFT_SCL__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    TFT_SCL_Write(uint8 value);
void    TFT_SCL_SetDriveMode(uint8 mode);
uint8   TFT_SCL_ReadDataReg(void);
uint8   TFT_SCL_Read(void);
void    TFT_SCL_SetInterruptMode(uint16 position, uint16 mode);
uint8   TFT_SCL_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the TFT_SCL_SetDriveMode() function.
     *  @{
     */
        #define TFT_SCL_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define TFT_SCL_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define TFT_SCL_DM_RES_UP          PIN_DM_RES_UP
        #define TFT_SCL_DM_RES_DWN         PIN_DM_RES_DWN
        #define TFT_SCL_DM_OD_LO           PIN_DM_OD_LO
        #define TFT_SCL_DM_OD_HI           PIN_DM_OD_HI
        #define TFT_SCL_DM_STRONG          PIN_DM_STRONG
        #define TFT_SCL_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define TFT_SCL_MASK               TFT_SCL__MASK
#define TFT_SCL_SHIFT              TFT_SCL__SHIFT
#define TFT_SCL_WIDTH              1u

/* Interrupt constants */
#if defined(TFT_SCL__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in TFT_SCL_SetInterruptMode() function.
     *  @{
     */
        #define TFT_SCL_INTR_NONE      (uint16)(0x0000u)
        #define TFT_SCL_INTR_RISING    (uint16)(0x0001u)
        #define TFT_SCL_INTR_FALLING   (uint16)(0x0002u)
        #define TFT_SCL_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define TFT_SCL_INTR_MASK      (0x01u) 
#endif /* (TFT_SCL__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define TFT_SCL_PS                     (* (reg8 *) TFT_SCL__PS)
/* Data Register */
#define TFT_SCL_DR                     (* (reg8 *) TFT_SCL__DR)
/* Port Number */
#define TFT_SCL_PRT_NUM                (* (reg8 *) TFT_SCL__PRT) 
/* Connect to Analog Globals */                                                  
#define TFT_SCL_AG                     (* (reg8 *) TFT_SCL__AG)                       
/* Analog MUX bux enable */
#define TFT_SCL_AMUX                   (* (reg8 *) TFT_SCL__AMUX) 
/* Bidirectional Enable */                                                        
#define TFT_SCL_BIE                    (* (reg8 *) TFT_SCL__BIE)
/* Bit-mask for Aliased Register Access */
#define TFT_SCL_BIT_MASK               (* (reg8 *) TFT_SCL__BIT_MASK)
/* Bypass Enable */
#define TFT_SCL_BYP                    (* (reg8 *) TFT_SCL__BYP)
/* Port wide control signals */                                                   
#define TFT_SCL_CTL                    (* (reg8 *) TFT_SCL__CTL)
/* Drive Modes */
#define TFT_SCL_DM0                    (* (reg8 *) TFT_SCL__DM0) 
#define TFT_SCL_DM1                    (* (reg8 *) TFT_SCL__DM1)
#define TFT_SCL_DM2                    (* (reg8 *) TFT_SCL__DM2) 
/* Input Buffer Disable Override */
#define TFT_SCL_INP_DIS                (* (reg8 *) TFT_SCL__INP_DIS)
/* LCD Common or Segment Drive */
#define TFT_SCL_LCD_COM_SEG            (* (reg8 *) TFT_SCL__LCD_COM_SEG)
/* Enable Segment LCD */
#define TFT_SCL_LCD_EN                 (* (reg8 *) TFT_SCL__LCD_EN)
/* Slew Rate Control */
#define TFT_SCL_SLW                    (* (reg8 *) TFT_SCL__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define TFT_SCL_PRTDSI__CAPS_SEL       (* (reg8 *) TFT_SCL__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define TFT_SCL_PRTDSI__DBL_SYNC_IN    (* (reg8 *) TFT_SCL__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define TFT_SCL_PRTDSI__OE_SEL0        (* (reg8 *) TFT_SCL__PRTDSI__OE_SEL0) 
#define TFT_SCL_PRTDSI__OE_SEL1        (* (reg8 *) TFT_SCL__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define TFT_SCL_PRTDSI__OUT_SEL0       (* (reg8 *) TFT_SCL__PRTDSI__OUT_SEL0) 
#define TFT_SCL_PRTDSI__OUT_SEL1       (* (reg8 *) TFT_SCL__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define TFT_SCL_PRTDSI__SYNC_OUT       (* (reg8 *) TFT_SCL__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(TFT_SCL__SIO_CFG)
    #define TFT_SCL_SIO_HYST_EN        (* (reg8 *) TFT_SCL__SIO_HYST_EN)
    #define TFT_SCL_SIO_REG_HIFREQ     (* (reg8 *) TFT_SCL__SIO_REG_HIFREQ)
    #define TFT_SCL_SIO_CFG            (* (reg8 *) TFT_SCL__SIO_CFG)
    #define TFT_SCL_SIO_DIFF           (* (reg8 *) TFT_SCL__SIO_DIFF)
#endif /* (TFT_SCL__SIO_CFG) */

/* Interrupt Registers */
#if defined(TFT_SCL__INTSTAT)
    #define TFT_SCL_INTSTAT            (* (reg8 *) TFT_SCL__INTSTAT)
    #define TFT_SCL_SNAP               (* (reg8 *) TFT_SCL__SNAP)
    
	#define TFT_SCL_0_INTTYPE_REG 		(* (reg8 *) TFT_SCL__0__INTTYPE)
#endif /* (TFT_SCL__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_TFT_SCL_H */


/* [] END OF FILE */
