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
#include <stdio.h>
#include <stdlib.h>
#include "emulator.h"

/**
 * Globals that represent the main state of the CPU
 */

uint16_t            pc;
uint8_t             reg[256];

// We use ints for the flags and keep them independently for speed, when accessing the
// flags register we'll have to pull them together
int                 C, Z, S, V, D, H;

// Do we need to use extended bus timing for read accesses to the external bus
int                 extended_bus_timing;

// Is the bus enabled or disabled (used for reading port 1)
int                 bus_enabled;


// ----------------------------------------------------------------------
// We need to be able to combine the flags for a register read or write
// ----------------------------------------------------------------------
uint8_t read_FLAGS() {
    uint8_t f = FLAGS & 0x03;       // keep the two user flags

    if (C) f |= 0x80;
    if (Z) f |= 0x40;
    if (S) f |= 0x20;
    if (V) f |= 0x10;
    if (D) f |= 0x08;
    if (H) f |= 0x04;
    return f;
}
void write_FLAGS(uint8_t val) {
    FLAGS = val;

    C = ((val & 0x80) == 0x80);
    Z = ((val & 0x40) == 0x40);
    S = ((val & 0x20) == 0x20);
    V = ((val & 0x10) == 0x10);
    D = ((val & 0x08) == 0x08);
    H = ((val & 0x04) == 0x04);
}

// ----------------------------------------------------------------------
// If we try to write a value to a register that we can't support then
// we end up here, with some helpful local variables and a breakpoint to
// the debugger.
// ----------------------------------------------------------------------
#pragma GCC push_options
#pragma GCC optimize ("O0")
void REG_FAIL(uint8_t __attribute__((unused)) reg, uint8_t __attribute__((unused)) val) {
    uint16_t    __attribute__((unused)) pc_after = pc;
    
    BKPT;
}
#pragma GCC pop_options


// ----------------------------------------------------------------------
// P01M -- controlling port 0 and port 1 (write only!)
// 
// Bit 7/6: P04-P07 mode (00=output, 01=input, 1X=A12-A15)    [1X NOT SUP]
// Bit 5:   External memory timing (0=normal, 1=extended)  
// Bit 4/3: P10-P17 mode (00=output, 01=input, 10=AD0-AD7, 11=HiZ all)
// Bit 2:   Stack (0=external, 1=internal)                    [INTERNAL only]
// Bit 1/0: P00-P03 mode (00=output, 01=input, 1X=A8-A11)
// 
// KNOWN VALUES for 8840A
//
// 06 -- A8-A11 addressing, internal stack, P04-P07 outputs, P1 byte output
// 16 -- A0-A11 addressing, internal stack, P04-P07 outputs, P1 Addr/Data
// 36 -- A0-A11 addressing, internal stack, P04-P07 outputs, P1 Addr/Data (EXTBUS)
// ----------------------------------------------------------------------
void write_P01M(uint8_t val) {
    switch(val) {
        case 0x06:      
                        *(uint32_t *)CYREG_HSIOM_PORT_SEL1 = 0x00000000;    // GPIO control
                        extended_bus_timing = 0;
                        bus_enabled  = 0;
            break;
        case 0x16:      
                        *(uint32_t *)CYREG_HSIOM_PORT_SEL1 = 0x22222222;    // DSI pin and OE
                        extended_bus_timing = 0;
                        bus_enabled  = 1;
            break;
        case 0x36:
                        *(uint32_t *)CYREG_HSIOM_PORT_SEL1 = 0x22222222;    // DSI pin and OE
                        extended_bus_timing = 1;
                        bus_enabled  = 1;
            break;
        default:           
                        REG_FAIL(P01M, val);
    }
 
    // Always have P00 to P03 connected to DSI, and P04 to P07 as GPIO
    // Always have P00 to P03 as inputs (although not used for that) and P04 to P07 as outputs
    // This is done by the top level design, so don't actually need to do it here...
    /*
    *(uint32_t *)CYREG_HSIOM_PORT_SEL0 = 0x00003333;
    P0_PC = (
        (0x01 << 0) | (0x01 << 3) | (0x01 << 6) | (0x01 << 9) |         // P00 to P03 as input
        (0x06 << 12) | (0x06 << 15) | (0x06 << 18) | (0x06 << 21));     // P04 to P07 as output

    P1_PC = (
        (0x06 << 0) | (0x06 << 3) | (0x06 << 6) | (0x06 << 9) | 
        (0x06 << 12) | (0x06 << 15) | (0x06 << 18) | (0x06 << 21));     // all outputs
    */
}
// ----------------------------------------------------------------------
// P2M -- controlling port 2 (write only)
// 
// Simple mapping to bits to direction (0=output, 1=input)
//
// KNOWN VALUES for 8840A
//
// FB -- all input apart from P22 (test entry check)
// 0B -- P24-P27 output, P22 output, P20/21/23 input (normal)
// ----------------------------------------------------------------------
void write_P2M(uint8_t val) {
    // TODO: write to correct PSoC register
    
    switch(val) {
        case 0xfb:      // All inputs apart from P22
                        P2_PC = (
                            (0x01 << 0) | (0x01 << 3) | (0x06 << 6) |
                            (0x01 << 9) | (0x01 << 12) | (0x01 << 15) |
                            (0x01 << 18) | (0x01 << 21)); 
            break;
        case 0x0b:      // P20/21/23 input, P22 output, P24/25/26/27 output
                        P2_PC = (
                            (0x01 << 0) | (0x01 << 3) | (0x06 << 6) |
                            (0x01 << 9) | (0x06 << 12) | (0x06 << 15) |
                            (0x06 << 18) | (0x06 << 21)); 
            break;
        default:
                        REG_FAIL(P2M, val);
    }
    
}
// ----------------------------------------------------------------------
// P3M - controlling port 3 (write only)
//
// Bit 7:   Parity (0=off, 1=on)
// Bit 6:   P30 (0=input, 1=serialin)   P37 (0=output, 1=serialout)
// Bit 5:   P31 (0=input, 1=handshake)  P36 (0=output, 1=handshake)
// Bit 4/3: 00 (P33=in, P34=out) 01or10 (P33=in, P34=DM), 11 (P33=HS, P34=HS)
// Bit 2:   P32 (0=input, 1=handshake)  P35 (0=output, 1=handshake)
// Bit 1:   RESERVED
// Bit 0:   0=port2-pullups open drain, 1=port2-pullups active
//
// KNOWN VALUES for 8840A
//
// C9 - parity on, serial in/out, P31/P32/P33 input, P34 DM, P35/P36 output
//              port 2 pullups active
//
// PIN MAPPING (Z8 to PSoC)
//
// P30 -> P30 (serial out)
// P31 -> P40 (input)
// P32 -> P41 (input)
// P33 -> P42 (input)
// P34 -> P34 (DM)
// P35 -> P35 (reset for kbd controller)
// P36 -> P36 (T1 output, so ignore here)
// P37 -> P31 (serial in)
// ----------------------------------------------------------------------
void write_P3M(uint8_t val) {
    //
    // We only support the value of C9, and that's how the design is setup, so
    // nothing really to do here.
    //
    
    if (val != 0xC9) REG_FAIL(P3M, val);
}
void write_port3(uint8_t val) {
    // The only adjustable outputs is P35, but we can probably just write to the reg

    
    // WHY IS THIS A PROBLEM   
    P35_DR = (P35_DR & 0xffffffdf) | (uint32_t)(val & 0x20); 
}
uint8_t read_port3() {
    // We need to read P31/2/3 (which are actually PSoC P40/1/2)
    
    uint8_t vv = (uint8_t)((P4_PS & 0x07) << 1);
    return vv;
//    return (uint8_t)((P4_PS & 0x07) << 1);
}

// ----------------------------------------------------------------------
// TMR 
//
// bit 7/6:  clock output (00=none, 01=t0, 10=t1, 11=internal), pin is P36
// bit 5/4:  TIN modes (00=extin, 01=gatein, 10/11 = trigger in)
// bit 3:    enable/disable T1
// ----------------------------------------------------------------------
void write_TMR(uint8_t val) {
    
    // Bits 7/6 control external clocking
    switch((val & 0xc0) >> 6) {
        case 0x00:      // no output, disconnect pin (GPIO mode)
                        *(uint32_t *)CYREG_HSIOM_PORT_SEL3 &= 0xf0ffffff;
                        break;
        case 0x02:      // T1 out, reconect pin .. 0x08 is ACT_0 (TCPWM)
                        *(uint32_t *)CYREG_HSIOM_PORT_SEL3 |= 0x08000000;
                        break;
        case 0x01:      // T0 out, we don't support this
        case 0x03:      // Internal clock out, we don't support this
                        REG_FAIL(TMR, val);
    }
    // Bits 5/4 control external inputs (we don't do anything)
    switch((val &0x30) >> 4) {
        case 0x00:      break;      // do nothing here
        default:        REG_FAIL(TMR, val);
    }

    // Bit 2 load T1
    if (val & 0x04) {
        // can set values here, but how do we make it happen on reload?
        
        Timer1_WriteCounter(0);    
    }    
    
    // Bit 3 enable (1) or disable (0) T1
    if (val & 0x08) {
        Timer1_Enable();
    } else {
        Timer1_Stop();
    }
    // Bit 1 and 0 we will ignore because T0 is not used separately
    TMR = val & 0xfa;       // clear the load bits
}

// ----------------------------------------------------------------------
// T1 / PRE1 -- these control timer1, with prescaler and count values, we
//              map them into PWM equivalent values, however they shouldn't
//              really be loaded until we trigger the load, but this is
//              simpler and should be sufficient.
// ----------------------------------------------------------------------
void write_T1(uint8_t val) {
    T1 = val;       // store for later use
    
    uint32_t period = ((PRE1 & 0xfc) >> 2) * T1;
    Timer1_WritePeriod(period);
    Timer1_WriteCompare(period >> 1);
    Timer1_WriteCounter(0);
}
void write_PRE1(uint8_t val) {
    PRE1 = val;     // store for later use
    
    // We don't do single shot...
    if(!(val&0x01)) REG_FAIL(PRE1, val);

    uint32_t period = ((PRE1 & 0xfc) >> 2) * T1;
    Timer1_WritePeriod(period);
    Timer1_WriteCompare(period >> 1);
    Timer1_WriteCounter(0);
}


// ----------------------------------------------------------------------
// IMR - interrupt mask register
//
// Following bits are only ever set: 0010 1110 (irq4 and irq0 not used)
// bit 0 = not used
// bit 1 = keyboard = P4 (pin2) [P33]
// bit 2 = ADC = P4 (pin0) [P31]
// bit 3 = Serial IRQ
// bit 4 = not used
// bit 5 = timer
// ----------------------------------------------------------------------
void write_IMR(uint8_t val) {
    // Write the value so that we can read it back
    IMR = val;
    
    
    // TODO: delta rather than everything, plus maybe turn of or clear pending on P4??
    
    if(val & 0x02) {
        asm("nop");
    }
    
    // Bit 1 is the keyboard
    P4_SetInterruptMode(P4_P33_INTR, ((val & 0x02) ? P4_INTR_FALLING : P4_INTR_NONE));

    // Bit 2 is the ADC
    P4_SetInterruptMode(P4_P31_INTR, ((val & 0x04) ? P4_INTR_FALLING : P4_INTR_NONE));
    
    // Bit 3 is the serial IRQ
    if ((val & 0x08)) {
        IRQ_Serial_Enable();
    } else {
        IRQ_Serial_Disable();
    }
    
    // Bit 5 is the timer
    if ((val & 0x20)) {
        IRQ_Timer1_Enable();
    } else {
        IRQ_Timer1_Disable();
    }
}
// ----------------------------------------------------------------------
// IPR - interrupt priority register
//
// KNOWN VALUES
// xx110001
//
// 31 - A:3>5 B:2>0 C:1>4  B>C>A == priority is 2,0,1,4,3,5
//
// 2=P31        (ADC irq)
// 0=P32        (not used, input)
// 1=P33        (keyboard irq)
// 4=SerialOut  (not used, polled?)
// 3=SerialIn
// 5=T1         (timer?)
// ----------------------------------------------------------------------
void write_IPR(uint8_t val) {
    // TODO: set priorities as above (if possible)
}
// ----------------------------------------------------------------------
// PRE0 - Prescaler 0 (write only) [likely serial clock]
//
// Known value: 05 (continuous, div by 1, so /8 overall) (could be 02)
// initial value = 01
// 
// Looks like it equates to a serial speed of 62500 (could be 31250)
// ----------------------------------------------------------------------
void write_PRE0(uint8_t val) {
    // TODO: we can probably ignore this since it's just used to set
    // the baud rate for the serial port
}

// ----------------------------------------------------------------------
// IRQ -- the 8840A doesn't write 1's to IRQ it only clears bits, so we
//        just need to track clearing for writes. For read we need to pull
//        in the correct pending bits.
//        Serial output is polled, so we need to put the FIFO status into
//        the read of IRQ.
// ----------------------------------------------------------------------
void write_IRQ(uint8_t val) { 
    // Bit 1 maps to keyboard (P33 which is P42
    if ((val & 0x02) == 0) { 
        // Clear pending bit...
        P4_INTSTAT = 0x04;
    }
    // Bit 2 maps to the ADC (P31 which is P40)
    if ((val & 0x04) == 0) {
        // Clear pending bit...
        P4_INTSTAT = 0x01;
    }
    // Bit 3 is serial in...
    if ((val & 0x08) == 0) {
        *IRQ_Serial_INTC_CLR_PD = IRQ_Serial__INTC_MASK;
    }
    // Bit 4 is serial out ... which we do clear
    if ((val & 0x10) == 0) {
        UART_INTR_TX_REG |= 0x00000002;
    }
    
    // Bit 5 is timer1
    if ((val & 0x20) == 0) {
        *IRQ_Timer1_INTC_CLR_PD = IRQ_Timer1__INTC_MASK;
    }
}
uint8_t read_IRQ() {
    uint8_t val = 0;
    
    if (P4_INTSTAT & 0x04) val |= 0x02;
    if (P4_INTSTAT & 0x01) val |= 0x04;
    if ((*IRQ_Serial_INTC_SET_EN & (uint32)IRQ_Serial__INTC_MASK)) val |= 0x08;
    if (UART_INTR_TX_REG & 0x00000002) val |= 0x10;
    if ((*IRQ_Timer1_INTC_SET_EN & (uint32)IRQ_Timer1__INTC_MASK)) val |= 0x20;

    return val;
}

void write_SIO(uint8_t val) {
    UART_TX_FIFO_WR_REG = val;        
}
uint8_t read_SIO() {
    return (UART_RX_FIFO_RD_REG & 0xff);
}

/**
 * We need to be able to map special registers for read and write
 */

// ----------------------------------------------------------------------
// Port 0: should just be a simple read/write of the register. The lower
//         four bits are fixed to the DSI, so they should return 0x0F and
//         the higher four bits will be valid.
// ----------------------------------------------------------------------
uint8_t read_port0() { return (uint8_t)(P0_PS | 0x0F); }
//void write_port0(uint8_t val) { P0_DR = (uint32_t)val & 0xFF; }

// If we & 0xF0 then the extra bits don't go off!!
void write_port0(uint8_t val) { 
    P0_DR = (P0_DR & 0xffffff0f) | (val & 0xf0);
}

// ----------------------------------------------------------------------
// Port 1: This is either all on the DSI (returning 0xFF) or all set as
//         byte output, in which case we should return the normal register.
//         Writing is totally standard.
// ----------------------------------------------------------------------
uint8_t read_port1() { return (bus_enabled ? 0xff : (uint8_t)P1_PS); }
//void write_port1(uint8_t val) { P1_DR = (uint32_t)val; }
void write_port1(uint8_t val) {
    P1_DR = (P1_DR & 0xffffff00) | val;
}


// ----------------------------------------------------------------------
// Port 2: this is a simple 8 bit controllable port, so the easy approach
// ----------------------------------------------------------------------
//void write_port2(uint8_t val) { P2_DR = (uint32_t)val; }
uint8_t read_port2() { return (uint8_t)P2_PS; }
void write_port2(uint8_t val) {
    P2_DR = (P2_DR & 0xffffff00) | val;
}


uint8_t (*reg_read[256])() = {
    read_port0, 
    read_port1, 
    read_port2, 
    read_port3,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x04 - 0x1f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x20 - 0x3f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x40 - 0x5f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x60 - 0x7f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x80 - 0x9f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xa0 - 0xbf
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xc0 - 0xdf
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                                                 // 0xe0 - 0xef
    read_SIO,           // 240 - SIO
    0,                  // 241 - TMR (read is ok)
    0,                  // 242 - T1 (read is not used by 8840)
    0,                  // 243 - PRE1 (read is ok)
    0, // 244 - T0
    0, // 245 - PRE0
    0,                  // 246 - P2M (write only)
    0,                  // 247 - P3M (write only)
    0,                  // 248 - P01M (write only)
    0,                  // 249 - IPR (write only)
    read_IRQ,           // 250 - IRQ
    0,                  // 251 - IMR (read is ok)
    read_FLAGS,         // 252
    0,                  // 253 - RP (ok)
    0,                  // 254 - SPH (ok)
    0,                  // 255 - SPL (ok)
};

void (*reg_write[256])(uint8_t) = {
    write_port0, 
    write_port1, 
    write_port2, 
    write_port3,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x04 - 0x1f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x20 - 0x3f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x40 - 0x5f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x60 - 0x7f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x80 - 0x9f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xa0 - 0xbf
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xc0 - 0xdf
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                                                 // 0xe0 - 0xef
    write_SIO,          // 240 - SIO
    write_TMR,          // 241 - TMR
    write_T1,           // 242 - T1
    write_PRE1,         // 243 - PRE1
    0, // 244 - T0
    0, // 245 - PRE0
    write_P2M,          // 246 - P2M
    write_P3M,          // 247 - P3M
    write_P01M,         // 248 - P01M
    write_IPR,          // 249 - IPR
    write_IRQ,          // 250 - IRQ
    write_IMR,          // 251 - IMR
    write_FLAGS,        // 252
    0,                  // 253 - RP (ok)
    0,                  // 254 - SPH (ok)
    0,                  // 255 - SPL (ok)
};





CY_ISR(Handle_IRQ_P4) {
    // TODO:  in this handler
    //        I should push pc, and flags, and then set the PC
    //        to the correct IRQ handler, and then return
    //        (TODO: read up about what is disabled etc and re-enabled on a ret)
    
    // I can read the pending status stuff from GPIO_PRT4_INTR, writing a 1 clears the
    // bit, so I'll check the ADC pin first, then the keyboard... 
    
    
    // We don't really want to be interrupted since the whole priority thing isn't going
    // to work well the way I'm doing this...
    CyGlobalIntDisable;
    
    uint32_t p4is = P4_INTSTAT;

    if(P4_INTSTAT & 0x01) {
        // We have an ADC interrupt pending... IRQ2
        PUSH16(pc);
        PUSH(FLAGS);
        pc = (code[0x0004] << 8) | code[0x0005];
        DI();
        
        P4_INTSTAT = 0x01;     // TODO: this should be done by IRQ write!!!
        // Clear the bit... P4_INTSTAT |= 0x01 ... will be done by Z8 write to IRQ
        
    } else if (P4_INTSTAT & 0x04) {
        // We have a keyboard interrupt pending... IRQ1

        PUSH16(pc);
        PUSH(FLAGS);
        pc = (code[0x0002] << 8) | code[0x0003];
        DI();
      
        P4_INTSTAT = 0x04;      // TODO: wrong here, but why does it work!
        // Clear the bit... P4_INTSTAT |= 0x04 ... will be done by Z8 write to IRQ
    } else if(P4_INTSTAT & 0xff) {
        // Error condition
        uint32_t __attribute__((unused)) vvv = P4_INTSTAT;
        BKPT;
    }
}

CY_ISR(Handle_IRQ_Timer1) {     // IRQ 5
    CyGlobalIntDisable;
    
    PUSH16(pc);
    PUSH(FLAGS);
    pc = (code[0x000a] << 8) | code[0x000b];
    DI();    
}

CY_ISR(Handle_IRQ_Serial) {     // IRQ 3
    CyGlobalIntDisable;
    
    PUSH16(pc);
    PUSH(FLAGS);
    pc = (code[0x0006] << 8) | code[0x0007];
    DI();    
}


void setup_emulator() {
    pc = 0x0c;
    SPL = 0x80;
    
    // PORT 0 SETUP
    
    // PORT 1 SETUP
    
    // PORT 2 SETUP
    
    
    // 
    IRQ_Serial_Disable();
    IRQ_Serial_SetVector(&Handle_IRQ_Serial);
    IRQ_Serial_SetPriority((uint8)IRQ_Serial_INTC_PRIOR_NUMBER);
    IRQ_Serial_ClearPending();
    
    IRQ_Timer1_Disable();
    IRQ_Timer1_SetVector(&Handle_IRQ_Timer1);
    IRQ_Timer1_SetPriority((uint8)IRQ_Timer1_INTC_PRIOR_NUMBER);
    IRQ_Timer1_ClearPending();
    
    // PORT 3 SETUP (a mismash on here, P31/32/33 are from Port 4 (0/1/2)...
    IRQ_P4_Disable();
    IRQ_P4_SetVector(&Handle_IRQ_P4);
    IRQ_P4_SetPriority((uint8)IRQ_P4_INTC_PRIOR_NUMBER);

    // Make sure they all start with no interrupts enabled
    P4_SetInterruptMode(P4_P31_INTR, P4_INTR_NONE);
    P4_SetInterruptMode(P4_P32_INTR, P4_INTR_NONE);
    P4_SetInterruptMode(P4_P33_INTR, P4_INTR_NONE);

    // Ensure there are no pending interrupts on the port itself
    P4_ClearInterrupt();

    // Then we can clear the overall pending flag for P4
    IRQ_P4_ClearPending();
    
    // Now we can enable the IRQ for this group of three pins
    IRQ_P4_Enable();
    
    write_IMR(0x00);                // Disable everything at the start

    extended_bus_timing = 1;        // Default is to start with extended bus timing
    bus_enabled = 1;                // We start with bus enabled
}

void execute() {
//    uint8_t opcode = code[pc++];
//    while (code[pc] != 0x0f) {

    
    for (;;) {
        while(!(IMR & 0x80)) {
            CyGlobalIntDisable;
            CyGlobalIntDisable;
            map[code[pc++]]();
        }            
        while((IMR & 0x80)) {
            CyGlobalIntEnable;
            CyGlobalIntDisable;
            map[code[pc++]]();
        }            
    }
    
    
    /*
    for(;;) {
    
        CyGlobalIntDisable;
        map[code[pc++]]();
        
       
        // This check slows it down quite a bit ... have a think .. we're still ok though.
        
        // Now we can give ourselves a short window to acknowledge IRQ's if they are
        // enabled.
        if (interrupts_disabled) {
            CyGlobalIntDisable;
        } else {
            CyGlobalIntEnable;
        }
       
    }
    */
    // STOP HERE
}



/* [] END OF FILE */
