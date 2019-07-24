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
#include "project.h"

#include "emulator.h"

uint8_t resp[64];

/**
    Read the external bus ...
        addr - 16 bit address
        dm - is is data memory (0=dm)
        ex - should we use extended timing (1=extend)

    TODO: move this code into the component API, but that will mean moving the
          control and status registers into there as well.
*/
uint8_t bus_read(uint16_t addr, int dm, int ex) {
    uint8_t hn;
    uint8_t rc;
    
    if (addr == 0x1f00) {
        if (IMR & 0x80) BKPT;
        //if (ex != 1) BKPT;
    }
        
    
    hn = (uint8_t)((addr & 0x0f00) >> 8);
    hn |= 0x80;                     // address
    if(ex) hn |= 0x10;              // extended bus timing
    if(dm) hn |= 0x20;              // dm

    // TODO: 16 bit reg write??
    
//    uint8_t saveInts = CyEnterCriticalSection();
    
//    (*(reg8 *)CYREG_UDB_W8_CTL0) = (uint8_t)(addr & 0xff);
//    CTRL1_Write((uint8_t)(addr&0xff));
//    (*(reg8 *)CYREG_UDB_W8_CTL1) = (uint8_t)hn;
//    CTRL2_Write(hn);
    
    uint16_t vvv = (hn << 8);
  
    vvv |= (addr & 0xff);

    IRQ_DVALID_ClearPending();
    (*(reg16 *)CYREG_UDB_W16_CTL0) = vvv;
//    CyDelayCycles(100);
//    (*(reg16 *)CYREG_UDB_W16_CTL1) = (uint16_t)(hn) | ((addr & 0xff) << 8);
    
    // We need a delay to ensure the data is ready to read, this seems
    // to be a minimum of 8 (we'll go 10) for the non-extended and an extra
    // 4 for extended. (Be careful with anything that changes the execution
    // time here. The time to do the math is important!)
//    CyDelayCycles(10 + (ex * 8));
//    CyDelayCycles(100);
    
    while(!(*IRQ_DVALID_INTC_SET_PD & (uint32)IRQ_DVALID__INTC_MASK));
    
    rc = DATA1_Read();
//    CyExitCriticalSection(saveInts); 
    return rc;
}

void bus_write(uint16_t addr, uint8_t data, int dm) {
    uint8_t hn;
    
    // First we write the data to the data register
    uint8_t saveInts = CyEnterCriticalSection();

    CTRL1_Write(data);
    CTRL2_Write(0x40);      // data strobe (loads data)
//   (*(reg16 *)CYREG_UDB_W16_CTL0) = (uint16_t)data | (0x40 << 8);
    
    hn = (uint8_t)((addr & 0x0f00) >> 8);
    hn |= 0x80;                     // address (bit 15 = 1)
    hn |= 0x00;                     // don't set extended timing (bit 12 = 0)
    if(dm) hn |= 0x20;              // dm (bit 13)
    
    CTRL1_Write((uint8_t)(addr&0xff));
    CTRL2_Write(hn);
//    (*(reg16 *)CYREG_UDB_W16_CTL0) = (uint16_t)(addr & 0xff) | (hn << 8);

    // Now we probably need to wait for some considerable time before this is
    // processed ... we should probably wait at least 30 cycles for now.
    // TODO: need to figure out how to optimise this
    // (Seems to work ok without any!)
//    CyDelayCycles(100);
    CyExitCriticalSection(saveInts); 
}

/**
 * MAIN entry point
 */
int main(void)
{
    // Start the external 8Mhz clock generator
    PWM8M_Start();
    
    // Setup timer1 (as per Z8 timer1)
    Timer1_Init();
    
    // Setup the UART with the default 8840A settings...
    UART_Start();
    
    // Prepare the CPU emulator and setup IRQ's...
    setup_emulator();
    
    //IRQ_SRX_Start();
    
  
    #define AUX_REG (* (reg8 *)SerialRX_1_RxBitCounter__CONTROL_AUX_CTL_REG)
    
//    CY_SET_REG8(SerialRX_1_RxBitCounter__CONTROL_AUX_CTL_REG, 
//        CY_GET_REG8(SerialRX_1_RxBitCounter__CONTROL_AUX_CTL_REG | 0x20));

    AUX_REG |= (1<<5);
    
    int c;
    uint8_t vals[16];

    /*
    
    for (c=0; c < 15; c++) {
        
        
        while ((*IRQ_SRX_INTC_SET_PD & (uint32)IRQ_SRX__INTC_MASK) == 0u);

        uint8_t vv = SR1_Read();
        vals[c] = vv;
        IRQ_SRX_ClearPending();
    }        
    
     */  
    
    
    // Run the emulator forever...
    execute();
}

/* [] END OF FILE */
