/*******************************************************************************
* File Name: P0H.h  
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

#if !defined(CY_PINS_P0H_ALIASES_H) /* Pins P0H_ALIASES_H */
#define CY_PINS_P0H_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define P0H_0			(P0H__0__PC)
#define P0H_0_PS		(P0H__0__PS)
#define P0H_0_PC		(P0H__0__PC)
#define P0H_0_DR		(P0H__0__DR)
#define P0H_0_SHIFT	(P0H__0__SHIFT)
#define P0H_0_INTR	((uint16)((uint16)0x0003u << (P0H__0__SHIFT*2u)))

#define P0H_1			(P0H__1__PC)
#define P0H_1_PS		(P0H__1__PS)
#define P0H_1_PC		(P0H__1__PC)
#define P0H_1_DR		(P0H__1__DR)
#define P0H_1_SHIFT	(P0H__1__SHIFT)
#define P0H_1_INTR	((uint16)((uint16)0x0003u << (P0H__1__SHIFT*2u)))

#define P0H_2			(P0H__2__PC)
#define P0H_2_PS		(P0H__2__PS)
#define P0H_2_PC		(P0H__2__PC)
#define P0H_2_DR		(P0H__2__DR)
#define P0H_2_SHIFT	(P0H__2__SHIFT)
#define P0H_2_INTR	((uint16)((uint16)0x0003u << (P0H__2__SHIFT*2u)))

#define P0H_3			(P0H__3__PC)
#define P0H_3_PS		(P0H__3__PS)
#define P0H_3_PC		(P0H__3__PC)
#define P0H_3_DR		(P0H__3__DR)
#define P0H_3_SHIFT	(P0H__3__SHIFT)
#define P0H_3_INTR	((uint16)((uint16)0x0003u << (P0H__3__SHIFT*2u)))

#define P0H_INTR_ALL	 ((uint16)(P0H_0_INTR| P0H_1_INTR| P0H_2_INTR| P0H_3_INTR))


#endif /* End Pins P0H_ALIASES_H */


/* [] END OF FILE */
