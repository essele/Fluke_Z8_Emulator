/*******************************************************************************
* File Name: RW.h  
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

#if !defined(CY_PINS_RW_ALIASES_H) /* Pins RW_ALIASES_H */
#define CY_PINS_RW_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define RW_0			(RW__0__PC)
#define RW_0_PS		(RW__0__PS)
#define RW_0_PC		(RW__0__PC)
#define RW_0_DR		(RW__0__DR)
#define RW_0_SHIFT	(RW__0__SHIFT)
#define RW_0_INTR	((uint16)((uint16)0x0003u << (RW__0__SHIFT*2u)))

#define RW_INTR_ALL	 ((uint16)(RW_0_INTR))


#endif /* End Pins RW_ALIASES_H */


/* [] END OF FILE */
