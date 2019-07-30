/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <cydevice_trm.h>
#include <CyLib.h>

void `$INSTANCE_NAME`_Init() {
    // Put fifo's into CLR mode so they act as buffers
    (*(reg8 *)`$INSTANCE_NAME`_shifter__DP_AUX_CTL_REG) |= 0x03;
    
    // Initial value for the counter goes in F0
    (*(reg8 *)`$INSTANCE_NAME`_shifter__F0_REG) = 0x57;  // 01010111;   
}

/* [] END OF FILE */
