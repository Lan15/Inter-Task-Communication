/*******************************************************************************
* File Name: TFT_BackLight_PM.c
* Version 3.30
*
* Description:
*  This file provides the power management source code to API for the
*  PWM.
*
* Note:
*
********************************************************************************
* Copyright 2008-2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "TFT_BackLight.h"

static TFT_BackLight_backupStruct TFT_BackLight_backup;


/*******************************************************************************
* Function Name: TFT_BackLight_SaveConfig
********************************************************************************
*
* Summary:
*  Saves the current user configuration of the component.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  TFT_BackLight_backup:  Variables of this global structure are modified to
*  store the values of non retention configuration registers when Sleep() API is
*  called.
*
*******************************************************************************/
void TFT_BackLight_SaveConfig(void) 
{

    #if(!TFT_BackLight_UsingFixedFunction)
        #if(!TFT_BackLight_PWMModeIsCenterAligned)
            TFT_BackLight_backup.PWMPeriod = TFT_BackLight_ReadPeriod();
        #endif /* (!TFT_BackLight_PWMModeIsCenterAligned) */
        TFT_BackLight_backup.PWMUdb = TFT_BackLight_ReadCounter();
        #if (TFT_BackLight_UseStatus)
            TFT_BackLight_backup.InterruptMaskValue = TFT_BackLight_STATUS_MASK;
        #endif /* (TFT_BackLight_UseStatus) */

        #if(TFT_BackLight_DeadBandMode == TFT_BackLight__B_PWM__DBM_256_CLOCKS || \
            TFT_BackLight_DeadBandMode == TFT_BackLight__B_PWM__DBM_2_4_CLOCKS)
            TFT_BackLight_backup.PWMdeadBandValue = TFT_BackLight_ReadDeadTime();
        #endif /*  deadband count is either 2-4 clocks or 256 clocks */

        #if(TFT_BackLight_KillModeMinTime)
             TFT_BackLight_backup.PWMKillCounterPeriod = TFT_BackLight_ReadKillTime();
        #endif /* (TFT_BackLight_KillModeMinTime) */

        #if(TFT_BackLight_UseControl)
            TFT_BackLight_backup.PWMControlRegister = TFT_BackLight_ReadControlRegister();
        #endif /* (TFT_BackLight_UseControl) */
    #endif  /* (!TFT_BackLight_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: TFT_BackLight_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the current user configuration of the component.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  TFT_BackLight_backup:  Variables of this global structure are used to
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void TFT_BackLight_RestoreConfig(void) 
{
        #if(!TFT_BackLight_UsingFixedFunction)
            #if(!TFT_BackLight_PWMModeIsCenterAligned)
                TFT_BackLight_WritePeriod(TFT_BackLight_backup.PWMPeriod);
            #endif /* (!TFT_BackLight_PWMModeIsCenterAligned) */

            TFT_BackLight_WriteCounter(TFT_BackLight_backup.PWMUdb);

            #if (TFT_BackLight_UseStatus)
                TFT_BackLight_STATUS_MASK = TFT_BackLight_backup.InterruptMaskValue;
            #endif /* (TFT_BackLight_UseStatus) */

            #if(TFT_BackLight_DeadBandMode == TFT_BackLight__B_PWM__DBM_256_CLOCKS || \
                TFT_BackLight_DeadBandMode == TFT_BackLight__B_PWM__DBM_2_4_CLOCKS)
                TFT_BackLight_WriteDeadTime(TFT_BackLight_backup.PWMdeadBandValue);
            #endif /* deadband count is either 2-4 clocks or 256 clocks */

            #if(TFT_BackLight_KillModeMinTime)
                TFT_BackLight_WriteKillTime(TFT_BackLight_backup.PWMKillCounterPeriod);
            #endif /* (TFT_BackLight_KillModeMinTime) */

            #if(TFT_BackLight_UseControl)
                TFT_BackLight_WriteControlRegister(TFT_BackLight_backup.PWMControlRegister);
            #endif /* (TFT_BackLight_UseControl) */
        #endif  /* (!TFT_BackLight_UsingFixedFunction) */
    }


/*******************************************************************************
* Function Name: TFT_BackLight_Sleep
********************************************************************************
*
* Summary:
*  Disables block's operation and saves the user configuration. Should be called
*  just prior to entering sleep.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  TFT_BackLight_backup.PWMEnableState:  Is modified depending on the enable
*  state of the block before entering sleep mode.
*
*******************************************************************************/
void TFT_BackLight_Sleep(void) 
{
    #if(TFT_BackLight_UseControl)
        if(TFT_BackLight_CTRL_ENABLE == (TFT_BackLight_CONTROL & TFT_BackLight_CTRL_ENABLE))
        {
            /*Component is enabled */
            TFT_BackLight_backup.PWMEnableState = 1u;
        }
        else
        {
            /* Component is disabled */
            TFT_BackLight_backup.PWMEnableState = 0u;
        }
    #endif /* (TFT_BackLight_UseControl) */

    /* Stop component */
    TFT_BackLight_Stop();

    /* Save registers configuration */
    TFT_BackLight_SaveConfig();
}


/*******************************************************************************
* Function Name: TFT_BackLight_Wakeup
********************************************************************************
*
* Summary:
*  Restores and enables the user configuration. Should be called just after
*  awaking from sleep.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  TFT_BackLight_backup.pwmEnable:  Is used to restore the enable state of
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void TFT_BackLight_Wakeup(void) 
{
     /* Restore registers values */
    TFT_BackLight_RestoreConfig();

    if(TFT_BackLight_backup.PWMEnableState != 0u)
    {
        /* Enable component's operation */
        TFT_BackLight_Enable();
    } /* Do nothing if component's block was disabled before */

}


/* [] END OF FILE */
