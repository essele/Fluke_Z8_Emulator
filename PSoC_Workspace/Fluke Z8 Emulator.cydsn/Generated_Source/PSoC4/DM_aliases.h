/*******************************************************************************
* File Name: DM.h  
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

#if !defined(CY_PINS_DM_ALIASES_H) /* Pins DM_ALIASES_H */
#define CY_PINS_DM_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define DM_0			(DM__0__PC)
#define DM_0_PS		(DM__0__PS)
#define DM_0_PC		(DM__0__PC)
#define DM_0_DR		(DM__0__DR)
#define DM_0_SHIFT	(DM__0__SHIFT)
#define DM_0_INTR	((uint16)((uint16)0x0003u << (DM__0__SHIFT*2u)))

#define DM_INTR_ALL	 ((uint16)(DM_0_INTR))


#endif /* End Pins DM_ALIASES_H */


/* [] END OF FILE */
