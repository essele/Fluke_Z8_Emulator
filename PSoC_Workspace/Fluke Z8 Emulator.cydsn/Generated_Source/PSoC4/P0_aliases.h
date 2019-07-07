/*******************************************************************************
* File Name: P0.h  
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

#if !defined(CY_PINS_P0_ALIASES_H) /* Pins P0_ALIASES_H */
#define CY_PINS_P0_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define P0_0			(P0__0__PC)
#define P0_0_PS		(P0__0__PS)
#define P0_0_PC		(P0__0__PC)
#define P0_0_DR		(P0__0__DR)
#define P0_0_SHIFT	(P0__0__SHIFT)
#define P0_0_INTR	((uint16)((uint16)0x0003u << (P0__0__SHIFT*2u)))

#define P0_1			(P0__1__PC)
#define P0_1_PS		(P0__1__PS)
#define P0_1_PC		(P0__1__PC)
#define P0_1_DR		(P0__1__DR)
#define P0_1_SHIFT	(P0__1__SHIFT)
#define P0_1_INTR	((uint16)((uint16)0x0003u << (P0__1__SHIFT*2u)))

#define P0_2			(P0__2__PC)
#define P0_2_PS		(P0__2__PS)
#define P0_2_PC		(P0__2__PC)
#define P0_2_DR		(P0__2__DR)
#define P0_2_SHIFT	(P0__2__SHIFT)
#define P0_2_INTR	((uint16)((uint16)0x0003u << (P0__2__SHIFT*2u)))

#define P0_3			(P0__3__PC)
#define P0_3_PS		(P0__3__PS)
#define P0_3_PC		(P0__3__PC)
#define P0_3_DR		(P0__3__DR)
#define P0_3_SHIFT	(P0__3__SHIFT)
#define P0_3_INTR	((uint16)((uint16)0x0003u << (P0__3__SHIFT*2u)))

#define P0_INTR_ALL	 ((uint16)(P0_0_INTR| P0_1_INTR| P0_2_INTR| P0_3_INTR))


#endif /* End Pins P0_ALIASES_H */


/* [] END OF FILE */
