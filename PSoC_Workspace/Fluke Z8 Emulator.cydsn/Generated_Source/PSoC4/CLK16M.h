/*******************************************************************************
* File Name: CLK16M.h
* Version 2.20
*
*  Description:
*   Provides the function and constant definitions for the clock component.
*
*  Note:
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_CLOCK_CLK16M_H)
#define CY_CLOCK_CLK16M_H

#include <cytypes.h>
#include <cyfitter.h>


/***************************************
*        Function Prototypes
***************************************/
#if defined CYREG_PERI_DIV_CMD

void CLK16M_StartEx(uint32 alignClkDiv);
#define CLK16M_Start() \
    CLK16M_StartEx(CLK16M__PA_DIV_ID)

#else

void CLK16M_Start(void);

#endif/* CYREG_PERI_DIV_CMD */

void CLK16M_Stop(void);

void CLK16M_SetFractionalDividerRegister(uint16 clkDivider, uint8 clkFractional);

uint16 CLK16M_GetDividerRegister(void);
uint8  CLK16M_GetFractionalDividerRegister(void);

#define CLK16M_Enable()                         CLK16M_Start()
#define CLK16M_Disable()                        CLK16M_Stop()
#define CLK16M_SetDividerRegister(clkDivider, reset)  \
    CLK16M_SetFractionalDividerRegister((clkDivider), 0u)
#define CLK16M_SetDivider(clkDivider)           CLK16M_SetDividerRegister((clkDivider), 1u)
#define CLK16M_SetDividerValue(clkDivider)      CLK16M_SetDividerRegister((clkDivider) - 1u, 1u)


/***************************************
*             Registers
***************************************/
#if defined CYREG_PERI_DIV_CMD

#define CLK16M_DIV_ID     CLK16M__DIV_ID

#define CLK16M_CMD_REG    (*(reg32 *)CYREG_PERI_DIV_CMD)
#define CLK16M_CTRL_REG   (*(reg32 *)CLK16M__CTRL_REGISTER)
#define CLK16M_DIV_REG    (*(reg32 *)CLK16M__DIV_REGISTER)

#define CLK16M_CMD_DIV_SHIFT          (0u)
#define CLK16M_CMD_PA_DIV_SHIFT       (8u)
#define CLK16M_CMD_DISABLE_SHIFT      (30u)
#define CLK16M_CMD_ENABLE_SHIFT       (31u)

#define CLK16M_CMD_DISABLE_MASK       ((uint32)((uint32)1u << CLK16M_CMD_DISABLE_SHIFT))
#define CLK16M_CMD_ENABLE_MASK        ((uint32)((uint32)1u << CLK16M_CMD_ENABLE_SHIFT))

#define CLK16M_DIV_FRAC_MASK  (0x000000F8u)
#define CLK16M_DIV_FRAC_SHIFT (3u)
#define CLK16M_DIV_INT_MASK   (0xFFFFFF00u)
#define CLK16M_DIV_INT_SHIFT  (8u)

#else 

#define CLK16M_DIV_REG        (*(reg32 *)CLK16M__REGISTER)
#define CLK16M_ENABLE_REG     CLK16M_DIV_REG
#define CLK16M_DIV_FRAC_MASK  CLK16M__FRAC_MASK
#define CLK16M_DIV_FRAC_SHIFT (16u)
#define CLK16M_DIV_INT_MASK   CLK16M__DIVIDER_MASK
#define CLK16M_DIV_INT_SHIFT  (0u)

#endif/* CYREG_PERI_DIV_CMD */

#endif /* !defined(CY_CLOCK_CLK16M_H) */

/* [] END OF FILE */
