/*******************************************************************************
* File Name: P31.c  
* Version 2.20
*
* Description:
*  This file contains APIs to set up the Pins component for low power modes.
*
* Note:
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "cytypes.h"
#include "P31.h"

static P31_BACKUP_STRUCT  P31_backup = {0u, 0u, 0u};


/*******************************************************************************
* Function Name: P31_Sleep
****************************************************************************//**
*
* \brief Stores the pin configuration and prepares the pin for entering chip 
*  deep-sleep/hibernate modes. This function applies only to SIO and USBIO pins.
*  It should not be called for GPIO or GPIO_OVT pins.
*
* <b>Note</b> This function is available in PSoC 4 only.
*
* \return 
*  None 
*  
* \sideeffect
*  For SIO pins, this function configures the pin input threshold to CMOS and
*  drive level to Vddio. This is needed for SIO pins when in device 
*  deep-sleep/hibernate modes.
*
* \funcusage
*  \snippet P31_SUT.c usage_P31_Sleep_Wakeup
*******************************************************************************/
void P31_Sleep(void)
{
    #if defined(P31__PC)
        P31_backup.pcState = P31_PC;
    #else
        #if (CY_PSOC4_4200L)
            /* Save the regulator state and put the PHY into suspend mode */
            P31_backup.usbState = P31_CR1_REG;
            P31_USB_POWER_REG |= P31_USBIO_ENTER_SLEEP;
            P31_CR1_REG &= P31_USBIO_CR1_OFF;
        #endif
    #endif
    #if defined(CYIPBLOCK_m0s8ioss_VERSION) && defined(P31__SIO)
        P31_backup.sioState = P31_SIO_REG;
        /* SIO requires unregulated output buffer and single ended input buffer */
        P31_SIO_REG &= (uint32)(~P31_SIO_LPM_MASK);
    #endif  
}


/*******************************************************************************
* Function Name: P31_Wakeup
****************************************************************************//**
*
* \brief Restores the pin configuration that was saved during Pin_Sleep(). This 
* function applies only to SIO and USBIO pins. It should not be called for
* GPIO or GPIO_OVT pins.
*
* For USBIO pins, the wakeup is only triggered for falling edge interrupts.
*
* <b>Note</b> This function is available in PSoC 4 only.
*
* \return 
*  None
*  
* \funcusage
*  Refer to P31_Sleep() for an example usage.
*******************************************************************************/
void P31_Wakeup(void)
{
    #if defined(P31__PC)
        P31_PC = P31_backup.pcState;
    #else
        #if (CY_PSOC4_4200L)
            /* Restore the regulator state and come out of suspend mode */
            P31_USB_POWER_REG &= P31_USBIO_EXIT_SLEEP_PH1;
            P31_CR1_REG = P31_backup.usbState;
            P31_USB_POWER_REG &= P31_USBIO_EXIT_SLEEP_PH2;
        #endif
    #endif
    #if defined(CYIPBLOCK_m0s8ioss_VERSION) && defined(P31__SIO)
        P31_SIO_REG = P31_backup.sioState;
    #endif
}


/* [] END OF FILE */
