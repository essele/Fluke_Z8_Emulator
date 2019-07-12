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

uint8_t             code[4096] = {
    0x31, 0x00,                 // SRP 0x00
    0xb0, 0xea,                 // CLR R10
    0xb0, 0xeb,                 // CLR R11
    0xc2, 0xca,                 // LDC R12, @RR10
    
    0x31, 0x20,                 // SRP 0x20
    0x4c, 0x18,                 // LD R4, 0x18
    0x5c, 0x01,                 // LD R5, 0x01
    0x1c, 0x29,                  // LD R1, 0x29
    0x92, 0x14,                 // LDE @RR4, R1
        
    0x0f
};

/*
062d: 4c 18     LD   R4, #18h               // Looks like KB/DISP init
062f: 5c 01     LD   R5, #01h
0631: 1c 29     LD   R1, #29h
0633: 92 14     LDE  @RR4, R1               // set clock prescaler to /9 (111kHz)
0635: 1c 00     LD   R1, #00h
0637: 92 14     LDE  @RR4, R1               // 8 bit left entry, encoded scan kb
0639: 1c c2     LD   R1, #C2h
063b: 92 14     LDE  @RR4, R1               // clr ram rows (just fifo?)
*/
/*
    0x76, 0xe2, 0x01,           // TM R2, 0x01
    0xeb, 0xfb,                 // JR NZ, the last line
    0x76, 0xe2, 0x01,           // TM R2, 0x01
    0x6b, 0xfb,                 // JR Z, the last line
    0xa0, 0xea,                 // INCW RR10
    0x76, 0xe2, 0x01,           // TM R2, 0x01
    0xeb, 0xf9,                 // JR NZ, the INCW line
    0x0f                        // STOP HERE
};
*/
/*
1c9c: 31 00     SRP  #00h
1c9e: b0 ea     CLR  R10
1ca0: b0 eb     CLR  R11
1ca2: 76 e2 01  TM   R2, #01h               // test port20 (line freq input)
1ca5: eb fb     JR   NZ, 1CA2h              // wait for it to go high...
1ca7: 76 e2 01  TM   R2, #01h
1caa: 6b fb     JR   Z, 1CA7h               // then wait for it to go low...
1cac: a0 ea     INCW RR10                   // increment the RR10 counter
1cae: 76 e2 01  TM   R2, #01h
1cb1: eb f9     JR   NZ, 1CACh              // loop incrementing while it stays low (i.e. count low time)
1cb3: b0 3b     CLR  3Bh                    // default to 400Hz?
*/

/* [] END OF FILE */
