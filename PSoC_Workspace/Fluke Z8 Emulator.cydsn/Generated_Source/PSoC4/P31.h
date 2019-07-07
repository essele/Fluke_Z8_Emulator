/*******************************************************************************
* File Name: P31.h  
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

#if !defined(CY_PINS_P31_H) /* Pins P31_H */
#define CY_PINS_P31_H

#include "cytypes.h"
#include "cyfitter.h"
#include "P31_aliases.h"


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
} P31_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   P31_Read(void);
void    P31_Write(uint8 value);
uint8   P31_ReadDataReg(void);
#if defined(P31__PC) || (CY_PSOC4_4200L) 
    void    P31_SetDriveMode(uint8 mode);
#endif
void    P31_SetInterruptMode(uint16 position, uint16 mode);
uint8   P31_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void P31_Sleep(void); 
void P31_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(P31__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define P31_DRIVE_MODE_BITS        (3)
    #define P31_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - P31_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the P31_SetDriveMode() function.
         *  @{
         */
        #define P31_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define P31_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define P31_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define P31_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define P31_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define P31_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define P31_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define P31_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define P31_MASK               P31__MASK
#define P31_SHIFT              P31__SHIFT
#define P31_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in P31_SetInterruptMode() function.
     *  @{
     */
        #define P31_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define P31_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define P31_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define P31_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(P31__SIO)
    #define P31_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(P31__PC) && (CY_PSOC4_4200L)
    #define P31_USBIO_ENABLE               ((uint32)0x80000000u)
    #define P31_USBIO_DISABLE              ((uint32)(~P31_USBIO_ENABLE))
    #define P31_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define P31_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define P31_USBIO_ENTER_SLEEP          ((uint32)((1u << P31_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << P31_USBIO_SUSPEND_DEL_SHIFT)))
    #define P31_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << P31_USBIO_SUSPEND_SHIFT)))
    #define P31_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << P31_USBIO_SUSPEND_DEL_SHIFT)))
    #define P31_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(P31__PC)
    /* Port Configuration */
    #define P31_PC                 (* (reg32 *) P31__PC)
#endif
/* Pin State */
#define P31_PS                     (* (reg32 *) P31__PS)
/* Data Register */
#define P31_DR                     (* (reg32 *) P31__DR)
/* Input Buffer Disable Override */
#define P31_INP_DIS                (* (reg32 *) P31__PC2)

/* Interrupt configuration Registers */
#define P31_INTCFG                 (* (reg32 *) P31__INTCFG)
#define P31_INTSTAT                (* (reg32 *) P31__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define P31_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(P31__SIO)
    #define P31_SIO_REG            (* (reg32 *) P31__SIO)
#endif /* (P31__SIO_CFG) */

/* USBIO registers */
#if !defined(P31__PC) && (CY_PSOC4_4200L)
    #define P31_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define P31_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define P31_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define P31_DRIVE_MODE_SHIFT       (0x00u)
#define P31_DRIVE_MODE_MASK        (0x07u << P31_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins P31_H */


/* [] END OF FILE */
