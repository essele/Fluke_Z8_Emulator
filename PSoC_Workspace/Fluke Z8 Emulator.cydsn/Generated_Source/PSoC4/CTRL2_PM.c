/*******************************************************************************
* File Name: CTRL2_PM.c
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

#include "CTRL2.h"

/* Check for removal by optimization */
#if !defined(CTRL2_Sync_ctrl_reg__REMOVED)

static CTRL2_BACKUP_STRUCT  CTRL2_backup = {0u};

    
/*******************************************************************************
* Function Name: CTRL2_SaveConfig
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
void CTRL2_SaveConfig(void) 
{
    CTRL2_backup.controlState = CTRL2_Control;
}


/*******************************************************************************
* Function Name: CTRL2_RestoreConfig
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
void CTRL2_RestoreConfig(void) 
{
     CTRL2_Control = CTRL2_backup.controlState;
}


/*******************************************************************************
* Function Name: CTRL2_Sleep
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
void CTRL2_Sleep(void) 
{
    CTRL2_SaveConfig();
}


/*******************************************************************************
* Function Name: CTRL2_Wakeup
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
void CTRL2_Wakeup(void)  
{
    CTRL2_RestoreConfig();
}

#endif /* End check for removal by optimization */


/* [] END OF FILE */
