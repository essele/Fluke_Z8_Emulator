/*******************************************************************************
* File Name: CLK32M.h
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

#if !defined(CY_CLOCK_CLK32M_H)
#define CY_CLOCK_CLK32M_H

#include <cytypes.h>
#include <cyfitter.h>


/***************************************
*        Function Prototypes
***************************************/
#if defined CYREG_PERI_DIV_CMD

void CLK32M_StartEx(uint32 alignClkDiv);
#define CLK32M_Start() \
    CLK32M_StartEx(CLK32M__PA_DIV_ID)

#else

void CLK32M_Start(void);

#endif/* CYREG_PERI_DIV_CMD */

void CLK32M_Stop(void);

void CLK32M_SetFractionalDividerRegister(uint16 clkDivider, uint8 clkFractional);

uint16 CLK32M_GetDividerRegister(void);
uint8  CLK32M_GetFractionalDividerRegister(void);

#define CLK32M_Enable()                         CLK32M_Start()
#define CLK32M_Disable()                        CLK32M_Stop()
#define CLK32M_SetDividerRegister(clkDivider, reset)  \
    CLK32M_SetFractionalDividerRegister((clkDivider), 0u)
#define CLK32M_SetDivider(clkDivider)           CLK32M_SetDividerRegister((clkDivider), 1u)
#define CLK32M_SetDividerValue(clkDivider)      CLK32M_SetDividerRegister((clkDivider) - 1u, 1u)


/***************************************
*             Registers
***************************************/
#if defined CYREG_PERI_DIV_CMD

#define CLK32M_DIV_ID     CLK32M__DIV_ID

#define CLK32M_CMD_REG    (*(reg32 *)CYREG_PERI_DIV_CMD)
#define CLK32M_CTRL_REG   (*(reg32 *)CLK32M__CTRL_REGISTER)
#define CLK32M_DIV_REG    (*(reg32 *)CLK32M__DIV_REGISTER)

#define CLK32M_CMD_DIV_SHIFT          (0u)
#define CLK32M_CMD_PA_DIV_SHIFT       (8u)
#define CLK32M_CMD_DISABLE_SHIFT      (30u)
#define CLK32M_CMD_ENABLE_SHIFT       (31u)

#define CLK32M_CMD_DISABLE_MASK       ((uint32)((uint32)1u << CLK32M_CMD_DISABLE_SHIFT))
#define CLK32M_CMD_ENABLE_MASK        ((uint32)((uint32)1u << CLK32M_CMD_ENABLE_SHIFT))

#define CLK32M_DIV_FRAC_MASK  (0x000000F8u)
#define CLK32M_DIV_FRAC_SHIFT (3u)
#define CLK32M_DIV_INT_MASK   (0xFFFFFF00u)
#define CLK32M_DIV_INT_SHIFT  (8u)

#else 

#define CLK32M_DIV_REG        (*(reg32 *)CLK32M__REGISTER)
#define CLK32M_ENABLE_REG     CLK32M_DIV_REG
#define CLK32M_DIV_FRAC_MASK  CLK32M__FRAC_MASK
#define CLK32M_DIV_FRAC_SHIFT (16u)
#define CLK32M_DIV_INT_MASK   CLK32M__DIVIDER_MASK
#define CLK32M_DIV_INT_SHIFT  (0u)

#endif/* CYREG_PERI_DIV_CMD */

#endif /* !defined(CY_CLOCK_CLK32M_H) */

/* [] END OF FILE */
