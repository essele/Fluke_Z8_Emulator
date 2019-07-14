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

*/
uint8_t bus_read(uint16_t addr, int dm, int ex) {
    uint8_t hn;
    uint8_t rc;
    
    hn = (uint8_t)((addr & 0x0f00) >> 8);
    hn |= 0x80;                     // address
    if(ex) hn |= 0x10;              // extended bus timing
    if(dm) hn |= 0x20;              // dm

    // TODO: 16 bit reg write??
    
    uint8_t saveInts = CyEnterCriticalSection();
    
    CTRL1_Write((uint8_t)(addr&0xff));
    CTRL2_Write(hn);
    
    // We need a delay to ensure the data is ready to read, this seems
    // to be a minimum of 8 (we'll go 10) for the non-extended and an extra
    // 4 for extended. (Be careful with anything that changes the execution
    // time here. The time to do the math is important!)
    CyDelayCycles(10 + (ex * 8));
    
    rc = DATA1_Read();
    CyExitCriticalSection(saveInts); 
    return rc;
}

void bus_write(uint16_t addr, uint8_t data, int dm) {
    uint8_t hn;
    
    // First we write the data to the data register
    uint8_t saveInts = CyEnterCriticalSection();
    CTRL1_Write(data);
    CTRL2_Write(0x40);      // data strobe (loads data)
    
    hn = (uint8_t)((addr & 0x0f00) >> 8);
    hn |= 0x80;                     // address (bit 15 = 1)
    hn |= 0x00;                     // don't set extended timing (bit 12 = 0)
    if(dm) hn |= 0x20;              // dm (bit 13)
    
    CTRL1_Write((uint8_t)(addr&0xff));
    CTRL2_Write(hn);
    CyExitCriticalSection(saveInts); 
    
    // Now we probably need to wait for some considerable time before this is
    // processed ... we should probably wait at least 30 cycles for now.
    // TODO: need to figure out how to optimise this
    CyDelayCycles(30);
}


int main(void)
{
    //CyGlobalIntEnable; /* Enable global interrupts. */
    
    
    // Check a few values...
    volatile uint32_t p0 = *(uint32_t *)CYREG_HSIOM_PORT_SEL0;
    volatile uint32_t p1 = *(uint32_t *)CYREG_HSIOM_PORT_SEL1;
    volatile uint32_t p2 = *(uint32_t *)CYREG_HSIOM_PORT_SEL2;
    volatile uint32_t p3 = *(uint32_t *)CYREG_HSIOM_PORT_SEL3;
    volatile uint32_t p4 = *(uint32_t *)CYREG_HSIOM_PORT_SEL4;
    
    volatile uint32_t pp0 = P0_PC;
    volatile uint32_t pp1 = P1_PC;
    volatile uint32_t pp2 = P2_PC;
    volatile uint32_t pp3 = P35_PC;
    
    volatile uint8_t dmm = (P35_PC & (0x07 << 15)) >> 15;

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    PWM_1_Start();
    
    Timer1_Init();
    
    UART_Start();

    setup_emulator();
    
    execute();
    
    while(1) {
    
        
        CyDelay(500);       // little delay
        
        // Address read of 0x000c (hopefully)
    //    CTRL1_Write(0x1c);
    //    CTRL2_Write(0xb0);      // 15=address, 14=data, 13=dm, 12=rw (w=0)
        
        
        for(int x=0; x < 32; x++) {
            resp[x] = bus_read((uint16_t)x, 1, 0);
        }
        
//        for(int x=0; x < 5; x++) {
//            resp[x] = bus_read((uint16_t)0x1f00, 0, 1);
//        }
        

        CyDelay(500);
        
        // Now attempt some display stuff...
        bus_write(0x1801, 0x29, 0);
        bus_write(0x1801, 0x00, 0);
        bus_write(0x1801, 0xc2, 0);
        bus_write(0x1801, 0xA0, 0);             // Blanking ... A0 = unblank, A3 = blank (I think)
        bus_write(0x1801, 0x90, 0);
  
        CyDelay(10);
        
        bus_write(0x1800, 0x08, 0);
        bus_write(0x1800, 0x80, 0);
        bus_write(0x1800, 0x08, 0);
        bus_write(0x1800, 0x80, 0);
    
      
        bus_write(0x1800, 0x00, 0);
        bus_write(0x1800, 0x0A, 0);
        bus_write(0x1800, 0x00, 0);
        bus_write(0x1800, 0x14, 0);
        bus_write(0x1800, 0x00, 0);
        bus_write(0x1800, 0x7b, 0);
        bus_write(0x1800, 0x01, 0);
        bus_write(0x1800, 0x99, 0);
        
        CyDelay(10);
    }
        
   //PWM_1_Stop(); 
}

/* [] END OF FILE */
