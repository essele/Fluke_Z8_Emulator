/*******************************************************************************
* File Name: CTRL1_PM.c
* Version 1.80
*
* Description:
*  This file contains the setup, control, and status commands to support 
*  the component operation in the low power mode. 
*
* Note:
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "CTRL1.h"

/* Check for removal by optimization */
#if !defined(CTRL1_Sync_ctrl_reg__REMOVED)

static CTRL1_BACKUP_STRUCT  CTRL1_backup = {0u};

    
/*******************************************************************************
* Function Name: CTRL1_SaveConfig
********************************************************************************
*
* Summary:
*  Saves the control register value.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void CTRL1_SaveConfig(void) 
{
    CTRL1_backup.controlState = CTRL1_Control;
}


/*******************************************************************************
* Function Name: CTRL1_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the control register value.
*
* Parameters:
*  None
*
* Return:
*  None
*
*
*******************************************************************************/
void CTRL1_RestoreConfig(void) 
{
     CTRL1_Control = CTRL1_backup.controlState;
}


/*******************************************************************************
* Function Name: CTRL1_Sleep
********************************************************************************
*
* Summary:
*  Prepares the component for entering the low power mode.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void CTRL1_Sleep(void) 
{
    CTRL1_SaveConfig();
}


/*******************************************************************************
* Function Name: CTRL1_Wakeup
********************************************************************************
*
* Summary:
*  Restores the component after waking up from the low power mode.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void CTRL1_Wakeup(void)  
{
    CTRL1_RestoreConfig();
}

#endif /* End check for removal by optimization */


/* [] END OF FILE */
