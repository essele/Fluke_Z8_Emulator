/*******************************************************************************
* File Name: AS.h  
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

#if !defined(CY_PINS_AS_H) /* Pins AS_H */
#define CY_PINS_AS_H

#include "cytypes.h"
#include "cyfitter.h"
#include "AS_aliases.h"


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
} AS_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   AS_Read(void);
void    AS_Write(uint8 value);
uint8   AS_ReadDataReg(void);
#if defined(AS__PC) || (CY_PSOC4_4200L) 
    void    AS_SetDriveMode(uint8 mode);
#endif
void    AS_SetInterruptMode(uint16 position, uint16 mode);
uint8   AS_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void AS_Sleep(void); 
void AS_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(AS__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define AS_DRIVE_MODE_BITS        (3)
    #define AS_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - AS_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the AS_SetDriveMode() function.
         *  @{
         */
        #define AS_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define AS_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define AS_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define AS_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define AS_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define AS_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define AS_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define AS_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define AS_MASK               AS__MASK
#define AS_SHIFT              AS__SHIFT
#define AS_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in AS_SetInterruptMode() function.
     *  @{
     */
        #define AS_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define AS_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define AS_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define AS_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(AS__SIO)
    #define AS_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(AS__PC) && (CY_PSOC4_4200L)
    #define AS_USBIO_ENABLE               ((uint32)0x80000000u)
    #define AS_USBIO_DISABLE              ((uint32)(~AS_USBIO_ENABLE))
    #define AS_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define AS_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define AS_USBIO_ENTER_SLEEP          ((uint32)((1u << AS_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << AS_USBIO_SUSPEND_DEL_SHIFT)))
    #define AS_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << AS_USBIO_SUSPEND_SHIFT)))
    #define AS_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << AS_USBIO_SUSPEND_DEL_SHIFT)))
    #define AS_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(AS__PC)
    /* Port Configuration */
    #define AS_PC                 (* (reg32 *) AS__PC)
#endif
/* Pin State */
#define AS_PS                     (* (reg32 *) AS__PS)
/* Data Register */
#define AS_DR                     (* (reg32 *) AS__DR)
/* Input Buffer Disable Override */
#define AS_INP_DIS                (* (reg32 *) AS__PC2)

/* Interrupt configuration Registers */
#define AS_INTCFG                 (* (reg32 *) AS__INTCFG)
#define AS_INTSTAT                (* (reg32 *) AS__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define AS_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(AS__SIO)
    #define AS_SIO_REG            (* (reg32 *) AS__SIO)
#endif /* (AS__SIO_CFG) */

/* USBIO registers */
#if !defined(AS__PC) && (CY_PSOC4_4200L)
    #define AS_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define AS_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define AS_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define AS_DRIVE_MODE_SHIFT       (0x00u)
#define AS_DRIVE_MODE_MASK        (0x07u << AS_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins AS_H */


/* [] END OF FILE */
