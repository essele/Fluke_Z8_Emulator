/*******************************************************************************
* File Name: DM.h  
* Version 2.20
*
* Description:
*  This file contains Pin function prototypes and register defines
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_DM_H) /* Pins DM_H */
#define CY_PINS_DM_H

#include "cytypes.h"
#include "cyfitter.h"
#include "DM_aliases.h"


/***************************************
*     Data Struct Definitions
***************************************/

/**
* \addtogroup group_structures
* @{
*/
    
/* Structure for sleep mode support */
typedef struct
{
    uint32 pcState; /**< State of the port control register */
    uint32 sioState; /**< State of the SIO configuration */
    uint32 usbState; /**< State of the USBIO regulator */
} DM_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   DM_Read(void);
void    DM_Write(uint8 value);
uint8   DM_ReadDataReg(void);
#if defined(DM__PC) || (CY_PSOC4_4200L) 
    void    DM_SetDriveMode(uint8 mode);
#endif
void    DM_SetInterruptMode(uint16 position, uint16 mode);
uint8   DM_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void DM_Sleep(void); 
void DM_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(DM__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define DM_DRIVE_MODE_BITS        (3)
    #define DM_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - DM_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the DM_SetDriveMode() function.
         *  @{
         */
        #define DM_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define DM_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define DM_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define DM_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define DM_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define DM_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define DM_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define DM_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define DM_MASK               DM__MASK
#define DM_SHIFT              DM__SHIFT
#define DM_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in DM_SetInterruptMode() function.
     *  @{
     */
        #define DM_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define DM_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define DM_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define DM_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(DM__SIO)
    #define DM_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(DM__PC) && (CY_PSOC4_4200L)
    #define DM_USBIO_ENABLE               ((uint32)0x80000000u)
    #define DM_USBIO_DISABLE              ((uint32)(~DM_USBIO_ENABLE))
    #define DM_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define DM_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define DM_USBIO_ENTER_SLEEP          ((uint32)((1u << DM_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << DM_USBIO_SUSPEND_DEL_SHIFT)))
    #define DM_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << DM_USBIO_SUSPEND_SHIFT)))
    #define DM_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << DM_USBIO_SUSPEND_DEL_SHIFT)))
    #define DM_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(DM__PC)
    /* Port Configuration */
    #define DM_PC                 (* (reg32 *) DM__PC)
#endif
/* Pin State */
#define DM_PS                     (* (reg32 *) DM__PS)
/* Data Register */
#define DM_DR                     (* (reg32 *) DM__DR)
/* Input Buffer Disable Override */
#define DM_INP_DIS                (* (reg32 *) DM__PC2)

/* Interrupt configuration Registers */
#define DM_INTCFG                 (* (reg32 *) DM__INTCFG)
#define DM_INTSTAT                (* (reg32 *) DM__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define DM_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(DM__SIO)
    #define DM_SIO_REG            (* (reg32 *) DM__SIO)
#endif /* (DM__SIO_CFG) */

/* USBIO registers */
#if !defined(DM__PC) && (CY_PSOC4_4200L)
    #define DM_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define DM_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define DM_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define DM_DRIVE_MODE_SHIFT       (0x00u)
#define DM_DRIVE_MODE_MASK        (0x07u << DM_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins DM_H */


/* [] END OF FILE */
