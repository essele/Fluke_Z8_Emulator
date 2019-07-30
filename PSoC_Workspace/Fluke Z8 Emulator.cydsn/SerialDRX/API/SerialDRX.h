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
#if !defined(`$INSTANCE_NAME`_H) 
#define `$INSTANCE_NAME`_H

#include "cyfitter.h"

static inline uint8_t `$INSTANCE_NAME`_Read() {
    return *(reg8 *)`$INSTANCE_NAME`_shifter__F1_REG;
}
    
#endif
    
/* [] END OF FILE */
