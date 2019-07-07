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

*/
uint8_t bus_read(uint16_t addr, int dm) {
    uint8_t hn;
    uint8_t rc;
    
    hn = (uint8_t)((addr & 0x0f00) >> 8);
    hn |= 0x80;                     // address
    hn |= 0x10;                     // read
    if(dm) hn |= 0x20;              // dm

    // TODO: 16 bit reg write??
    //CyGlobalIntDisable;
    
    uint8_t saveInts = CyEnterCriticalSection();
    
    CTRL1_Write((uint8_t)(addr&0xff));
    CTRL2_Write(hn);
    
    // Seem to need a 21 cycle delay min before reading is ok
    // using 22 to give a bit of a margin
    // TODO: loop of some kind
    asm("nop; nop; nop; nop;");
    asm("nop; nop; nop; nop;");
    asm("nop; nop; nop; nop;");
    asm("nop; nop; nop; nop;");
    asm("nop; nop; nop; nop;");
    asm("nop; nop;");

    rc = DATA1_Read();
    //CyGlobalIntEnable;
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
    hn |= 0x00;                     // write (bit 12 = 0)
    if(dm) hn |= 0x20;              // dm (bit 13)
    
    CTRL1_Write((uint8_t)(addr&0xff));
    CTRL2_Write(hn);
    CyExitCriticalSection(saveInts); 
    
    // Now we probably need to wait for some considerable time before this is
    // processed ... we should probably wait at least 30 cycles for now.
    asm("nop; nop; nop; nop; nop; nop; nop; nop;");
    asm("nop; nop; nop; nop; nop; nop; nop; nop;");
    asm("nop; nop; nop; nop; nop; nop; nop; nop;");
    asm("nop; nop; nop; nop; nop; nop; nop; nop;");
    
    CyDelay(1);
}


int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    PWM_1_Start();
    
    setup_emulator();
    
    execute();
    
    while(1) {
    
        
        CyDelay(500);       // little delay
        
        // Address read of 0x000c (hopefully)
    //    CTRL1_Write(0x1c);
    //    CTRL2_Write(0xb0);      // 15=address, 14=data, 13=dm, 12=rw (w=0)
        
    //    asm("nop; nop; nop; nop;");
    //    asm("nop; nop; nop; nop;");
        
        for(int x=0; x < 32; x++) {
//           resp[x] = DATA1_Read();
            resp[x] = bus_read((uint16_t)x, 1);
        }
        
        

        CyDelay(500);
        
        // Now attempt some display stuff...
        bus_write(0x1801, 0x29, 0);
        bus_write(0x1801, 0x00, 0);
        bus_write(0x1801, 0xc2, 0);
        bus_write(0x1801, 0xA0, 0);             // Blanking ... A0 = unblank, A3 = blank (I think)
        bus_write(0x1801, 0x90, 0);
  
        CyDelay(10);
        
//        bus_write(0x1800, 0x08, 0);
//        bus_write(0x1800, 0x80, 0);
//        bus_write(0x1800, 0x08, 0);
//        bus_write(0x1800, 0x80, 0);
    
      
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
