/*******************************************************************************
* File Name: XTAL1.h  
* Version 2.20
*
* Description:
*  This file contains the Alias definitions for Per-Pin APIs in cypins.h. 
*  Information on using these APIs can be found in the System Reference Guide.
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_XTAL1_ALIASES_H) /* Pins XTAL1_ALIASES_H */
#define CY_PINS_XTAL1_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define XTAL1_0			(XTAL1__0__PC)
#define XTAL1_0_PS		(XTAL1__0__PS)
#define XTAL1_0_PC		(XTAL1__0__PC)
#define XTAL1_0_DR		(XTAL1__0__DR)
#define XTAL1_0_SHIFT	(XTAL1__0__SHIFT)
#define XTAL1_0_INTR	((uint16)((uint16)0x0003u << (XTAL1__0__SHIFT*2u)))

#define XTAL1_INTR_ALL	 ((uint16)(XTAL1_0_INTR))


#endif /* End Pins XTAL1_ALIASES_H */


/* [] END OF FILE */
