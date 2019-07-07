/*******************************************************************************
* File Name: P2.h  
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

#if !defined(CY_PINS_P2_ALIASES_H) /* Pins P2_ALIASES_H */
#define CY_PINS_P2_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define P2_0			(P2__0__PC)
#define P2_0_PS		(P2__0__PS)
#define P2_0_PC		(P2__0__PC)
#define P2_0_DR		(P2__0__DR)
#define P2_0_SHIFT	(P2__0__SHIFT)
#define P2_0_INTR	((uint16)((uint16)0x0003u << (P2__0__SHIFT*2u)))

#define P2_1			(P2__1__PC)
#define P2_1_PS		(P2__1__PS)
#define P2_1_PC		(P2__1__PC)
#define P2_1_DR		(P2__1__DR)
#define P2_1_SHIFT	(P2__1__SHIFT)
#define P2_1_INTR	((uint16)((uint16)0x0003u << (P2__1__SHIFT*2u)))

#define P2_2			(P2__2__PC)
#define P2_2_PS		(P2__2__PS)
#define P2_2_PC		(P2__2__PC)
#define P2_2_DR		(P2__2__DR)
#define P2_2_SHIFT	(P2__2__SHIFT)
#define P2_2_INTR	((uint16)((uint16)0x0003u << (P2__2__SHIFT*2u)))

#define P2_3			(P2__3__PC)
#define P2_3_PS		(P2__3__PS)
#define P2_3_PC		(P2__3__PC)
#define P2_3_DR		(P2__3__DR)
#define P2_3_SHIFT	(P2__3__SHIFT)
#define P2_3_INTR	((uint16)((uint16)0x0003u << (P2__3__SHIFT*2u)))

#define P2_4			(P2__4__PC)
#define P2_4_PS		(P2__4__PS)
#define P2_4_PC		(P2__4__PC)
#define P2_4_DR		(P2__4__DR)
#define P2_4_SHIFT	(P2__4__SHIFT)
#define P2_4_INTR	((uint16)((uint16)0x0003u << (P2__4__SHIFT*2u)))

#define P2_5			(P2__5__PC)
#define P2_5_PS		(P2__5__PS)
#define P2_5_PC		(P2__5__PC)
#define P2_5_DR		(P2__5__DR)
#define P2_5_SHIFT	(P2__5__SHIFT)
#define P2_5_INTR	((uint16)((uint16)0x0003u << (P2__5__SHIFT*2u)))

#define P2_6			(P2__6__PC)
#define P2_6_PS		(P2__6__PS)
#define P2_6_PC		(P2__6__PC)
#define P2_6_DR		(P2__6__DR)
#define P2_6_SHIFT	(P2__6__SHIFT)
#define P2_6_INTR	((uint16)((uint16)0x0003u << (P2__6__SHIFT*2u)))

#define P2_7			(P2__7__PC)
#define P2_7_PS		(P2__7__PS)
#define P2_7_PC		(P2__7__PC)
#define P2_7_DR		(P2__7__DR)
#define P2_7_SHIFT	(P2__7__SHIFT)
#define P2_7_INTR	((uint16)((uint16)0x0003u << (P2__7__SHIFT*2u)))

#define P2_INTR_ALL	 ((uint16)(P2_0_INTR| P2_1_INTR| P2_2_INTR| P2_3_INTR| P2_4_INTR| P2_5_INTR| P2_6_INTR| P2_7_INTR))


#endif /* End Pins P2_ALIASES_H */


/* [] END OF FILE */
