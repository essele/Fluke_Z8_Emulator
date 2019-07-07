/*******************************************************************************
* File Name: AS.h  
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

#if !defined(CY_PINS_AS_ALIASES_H) /* Pins AS_ALIASES_H */
#define CY_PINS_AS_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define AS_0			(AS__0__PC)
#define AS_0_PS		(AS__0__PS)
#define AS_0_PC		(AS__0__PC)
#define AS_0_DR		(AS__0__DR)
#define AS_0_SHIFT	(AS__0__SHIFT)
#define AS_0_INTR	((uint16)((uint16)0x0003u << (AS__0__SHIFT*2u)))

#define AS_INTR_ALL	 ((uint16)(AS_0_INTR))


#endif /* End Pins AS_ALIASES_H */


/* [] END OF FILE */
