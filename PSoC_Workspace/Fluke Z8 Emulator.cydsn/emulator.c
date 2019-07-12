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

// We use a special flag to indicate whether we run with interrupts disabled (post DI)
// or enabled (post EI)
int                 interrupts_disabled;

// Do we need to use extended bus timing for read accesses to the external bus
int                 extended_bus_timing;



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
// Bit 5:   External memory timing (0=normal, 1=extended)     [IGNORED]
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
            break;
        case 0x16:      
                        *(uint32_t *)CYREG_HSIOM_PORT_SEL1 = 0x22222222;    // DSI pin and OE
                        extended_bus_timing = 0;
            break;
        case 0x36:
                        *(uint32_t *)CYREG_HSIOM_PORT_SEL1 = 0x22222222;    // DSI pin and OE
                        extended_bus_timing = 1;
            break;
        default:           
                        REG_FAIL(P01M, val);
    }
    
    // Always have P00 to P03 connected to DSI, and P04 to P07 as GPIO
    *(uint32_t *)CYREG_HSIOM_PORT_SEL0 = 0x00003333;

    // Always have P00 to P03 as inputs (although not used for that) and P04 to P07 as outputs
    // This is done by the top level design, so don't actually need to do it here...
    /*
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
// ----------------------------------------------------------------------
void write_P3M(uint8_t val) {
    // TODO: single setting of C9
    //
    
    if (val == 0xC9) {
        // P31/P32/P33 as inputs (these are all actually from Port 4 P40/41/42

        // P34 DM (no action needed, it's the only option) -- TODO: control from software?
        
        // Serial Parity On
        // TODO: need a UART init for this ... looks horrible
        
        // P35/P36 output
        
        // TODO: do we need to do anything with a pull-up?
    }
    
    
    
    //
    // P36 appears to be a timer output (timer1)A
    // R241 (TMR) ...83 sets timer1 out, but loads and starts timer0
    // R241 (TMR) .. 8E sets tiemr1 out, enables and loads t1, keeps t0 enabled
    // R243 (PRE1/T1) ... 01/01 ... load        -- /1
    //                    67/C8 ... no load?    -- /25 * 200 = 5000 (100Hz)
    //                    67/FA ... no load?    -- /25 * 250 = 6250 (80Hz)
    //                    7B/FA ... no load?    -- /30 * 250 = 7500 (66.67Hz)
    //                    8B/C1 ... no load?    -- /34 * 193 = 6562? (76.19Hz)
    //
    // These are the sample rates for the ADC, so timer1 is used to drive
    // the TR line at these rates.A
    //
    // We will need to recognise specific sets of values and recreate the
    // appropriate ones for the PSoC timer, or see if we can work in a similar
    // way
    // 
}
// ----------------------------------------------------------------------
// IMR - interrupt mask register
//
// Following bits are only ever set: 0010 1110 (irq4 and irq0 not used)
// ----------------------------------------------------------------------
void write_IMR(uint8_t val) {
    // TODO: translate into PSoC mask

    // Write the value so that we can read it back
    IMR = val;
    
    // Update the main signal so we know whether enable ints or not
    interrupts_disabled = ((val & 0x80) == 0x80);
}
// ----------------------------------------------------------------------
// IPR - interrupt priority register
//
// KNOWN VALUES
// xx110001
//
// 31 - A:5>3 B:2>0 C:1>4  C>B>A == priority is 1,4,2,0,5,3
// 1=P33        (keyboard irq)
// 4=SerialOut  (not used, polled?)
// 2=P31        (ADC irq)
// 0=P32        (not used, input)
// 5=T1         (timer?)
// 3=SerialIn
// ----------------------------------------------------------------------
void write_IPR(uint8_t val) {
    // TODO: set priorities as above (if possible)
    
    // TODO: bit 7 is the master (interrupts_disabled)???
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

void write_IRQ(uint8_t val) { }

/**
 * We need to be able to map special registers for read and write
 */

void write_port2(uint8_t val) {
    P2_DR = (uint32_t)val;
}
uint8_t read_port2() {
    return (uint8_t)P2_PS;
}

uint8_t (*reg_read[256])() = {
    0, 
    0, 
    read_port2, 
    0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x04 - 0x1f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x20 - 0x3f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x40 - 0x5f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x60 - 0x7f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x80 - 0x9f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xa0 - 0xbf
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xc0 - 0xdf
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                                                 // 0xe0 - 0xef
    0, // 240 - SIO
    0, // 241 - TMR
    0, // 242 - T1
    0, // 243 - PRE1
    0, // 244 - T0
    0, // 245 - PRE0
    0,                  // 246 - P2M (write only)
    0,                  // 247 - P3M (write only)
    0,                  // 248 - P01M (write only)
    0,                  // 249 - IPR (write only)
    0, // 250 - IRQ
    0,                  // 251 - IMR (read is ok)
    read_FLAGS,         // 252
    0,                  // 253 - RP (ok)
    0,                  // 254 - SPH (ok)
    0,                  // 255 - SPL (ok)
};

void (*reg_write[256])(uint8_t) = {
    0, 
    0, 
    write_port2, 
    0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x04 - 0x1f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x20 - 0x3f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x40 - 0x5f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x60 - 0x7f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x80 - 0x9f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xa0 - 0xbf
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xc0 - 0xdf
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                                                 // 0xe0 - 0xef
    0, // 240 - SIO
    0, // 241 - TMR
    0, // 242 - T1
    0, // 243 - PRE1
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
    int x;
    x++;
    
}



void setup_emulator() {
    pc = 0x00;
    SPL = 0x80;
    
    // PORT 0 SETUP
    
    // PORT 1 SETUP
    
    // PORT 2 SETUP
    
    // PORT 3 SETUP (a mismash on here, P31/32/33 are from Port 4 (0/1/2)...
    P4_IRQ_Disable();
    P4_IRQ_SetVector(&Handle_IRQ_P4);
    P4_IRQ_SetPriority((uint8)P4_IRQ_INTC_PRIOR_NUMBER);

    // Make sure they all start with no interrupts enabled
    P4_SetInterruptMode(P4_P31_INTR, P4_INTR_NONE);
    P4_SetInterruptMode(P4_P32_INTR, P4_INTR_NONE);
    P4_SetInterruptMode(P4_P33_INTR, P4_INTR_NONE);
    
    // Now we can enable to IRQ for this group of three pins
    P4_IRQ_Enable();
    
    interrupts_disabled = 1;        // TODO: needs to be done by flags defaults
    extended_bus_timing = 1;        // Default is to start with extended bus timing
}

void execute() {
//    uint8_t opcode = code[pc++];
//    while (code[pc] != 0x0f) {

    
    for (;;) {
        while(!(IMR & 0x80)) {
            CyGlobalIntDisable;
            map[code[pc++]]();
            CyGlobalIntDisable;
        }            
        while((IMR & 0x80)) {
            CyGlobalIntDisable;
            map[code[pc++]]();
            CyGlobalIntEnable;
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
