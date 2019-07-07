/*******************************************************************************
* File Name: P1.h  
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

#if !defined(CY_PINS_P1_ALIASES_H) /* Pins P1_ALIASES_H */
#define CY_PINS_P1_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define P1_0			(P1__0__PC)
#define P1_0_PS		(P1__0__PS)
#define P1_0_PC		(P1__0__PC)
#define P1_0_DR		(P1__0__DR)
#define P1_0_SHIFT	(P1__0__SHIFT)
#define P1_0_INTR	((uint16)((uint16)0x0003u << (P1__0__SHIFT*2u)))

#define P1_1			(P1__1__PC)
#define P1_1_PS		(P1__1__PS)
#define P1_1_PC		(P1__1__PC)
#define P1_1_DR		(P1__1__DR)
#define P1_1_SHIFT	(P1__1__SHIFT)
#define P1_1_INTR	((uint16)((uint16)0x0003u << (P1__1__SHIFT*2u)))

#define P1_2			(P1__2__PC)
#define P1_2_PS		(P1__2__PS)
#define P1_2_PC		(P1__2__PC)
#define P1_2_DR		(P1__2__DR)
#define P1_2_SHIFT	(P1__2__SHIFT)
#define P1_2_INTR	((uint16)((uint16)0x0003u << (P1__2__SHIFT*2u)))

#define P1_3			(P1__3__PC)
#define P1_3_PS		(P1__3__PS)
#define P1_3_PC		(P1__3__PC)
#define P1_3_DR		(P1__3__DR)
#define P1_3_SHIFT	(P1__3__SHIFT)
#define P1_3_INTR	((uint16)((uint16)0x0003u << (P1__3__SHIFT*2u)))

#define P1_4			(P1__4__PC)
#define P1_4_PS		(P1__4__PS)
#define P1_4_PC		(P1__4__PC)
#define P1_4_DR		(P1__4__DR)
#define P1_4_SHIFT	(P1__4__SHIFT)
#define P1_4_INTR	((uint16)((uint16)0x0003u << (P1__4__SHIFT*2u)))

#define P1_5			(P1__5__PC)
#define P1_5_PS		(P1__5__PS)
#define P1_5_PC		(P1__5__PC)
#define P1_5_DR		(P1__5__DR)
#define P1_5_SHIFT	(P1__5__SHIFT)
#define P1_5_INTR	((uint16)((uint16)0x0003u << (P1__5__SHIFT*2u)))

#define P1_6			(P1__6__PC)
#define P1_6_PS		(P1__6__PS)
#define P1_6_PC		(P1__6__PC)
#define P1_6_DR		(P1__6__DR)
#define P1_6_SHIFT	(P1__6__SHIFT)
#define P1_6_INTR	((uint16)((uint16)0x0003u << (P1__6__SHIFT*2u)))

#define P1_7			(P1__7__PC)
#define P1_7_PS		(P1__7__PS)
#define P1_7_PC		(P1__7__PC)
#define P1_7_DR		(P1__7__DR)
#define P1_7_SHIFT	(P1__7__SHIFT)
#define P1_7_INTR	((uint16)((uint16)0x0003u << (P1__7__SHIFT*2u)))

#define P1_INTR_ALL	 ((uint16)(P1_0_INTR| P1_1_INTR| P1_2_INTR| P1_3_INTR| P1_4_INTR| P1_5_INTR| P1_6_INTR| P1_7_INTR))


#endif /* End Pins P1_ALIASES_H */


/* [] END OF FILE */
