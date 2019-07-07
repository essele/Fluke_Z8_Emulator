/*******************************************************************************
* File Name: P30.h  
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

#if !defined(CY_PINS_P30_H) /* Pins P30_H */
#define CY_PINS_P30_H

#include "cytypes.h"
#include "cyfitter.h"
#include "P30_aliases.h"


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
} P30_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   P30_Read(void);
void    P30_Write(uint8 value);
uint8   P30_ReadDataReg(void);
#if defined(P30__PC) || (CY_PSOC4_4200L) 
    void    P30_SetDriveMode(uint8 mode);
#endif
void    P30_SetInterruptMode(uint16 position, uint16 mode);
uint8   P30_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void P30_Sleep(void); 
void P30_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(P30__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define P30_DRIVE_MODE_BITS        (3)
    #define P30_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - P30_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the P30_SetDriveMode() function.
         *  @{
         */
        #define P30_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define P30_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define P30_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define P30_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define P30_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define P30_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define P30_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define P30_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define P30_MASK               P30__MASK
#define P30_SHIFT              P30__SHIFT
#define P30_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in P30_SetInterruptMode() function.
     *  @{
     */
        #define P30_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define P30_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define P30_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define P30_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(P30__SIO)
    #define P30_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(P30__PC) && (CY_PSOC4_4200L)
    #define P30_USBIO_ENABLE               ((uint32)0x80000000u)
    #define P30_USBIO_DISABLE              ((uint32)(~P30_USBIO_ENABLE))
    #define P30_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define P30_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define P30_USBIO_ENTER_SLEEP          ((uint32)((1u << P30_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << P30_USBIO_SUSPEND_DEL_SHIFT)))
    #define P30_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << P30_USBIO_SUSPEND_SHIFT)))
    #define P30_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << P30_USBIO_SUSPEND_DEL_SHIFT)))
    #define P30_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(P30__PC)
    /* Port Configuration */
    #define P30_PC                 (* (reg32 *) P30__PC)
#endif
/* Pin State */
#define P30_PS                     (* (reg32 *) P30__PS)
/* Data Register */
#define P30_DR                     (* (reg32 *) P30__DR)
/* Input Buffer Disable Override */
#define P30_INP_DIS                (* (reg32 *) P30__PC2)

/* Interrupt configuration Registers */
#define P30_INTCFG                 (* (reg32 *) P30__INTCFG)
#define P30_INTSTAT                (* (reg32 *) P30__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define P30_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(P30__SIO)
    #define P30_SIO_REG            (* (reg32 *) P30__SIO)
#endif /* (P30__SIO_CFG) */

/* USBIO registers */
#if !defined(P30__PC) && (CY_PSOC4_4200L)
    #define P30_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define P30_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define P30_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define P30_DRIVE_MODE_SHIFT       (0x00u)
#define P30_DRIVE_MODE_MASK        (0x07u << P30_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins P30_H */


/* [] END OF FILE */
