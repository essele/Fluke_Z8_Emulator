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


/**
 * Globals that represent the main state of the CPU
 */

uint16_t            pc;
uint8_t             reg[256];
uint8_t             code[4096] = {
    0x31, 0x00,                 // SRP 0x00
    0xb0, 0xea,                 // CLR R10
    0xb0, 0xeb,                 // CLR R11
    0x76, 0xe2, 0x01,           // TM R2, 0x01
    0xeb, 0xfb,                 // JR NZ, the last line
    0x76, 0xe2, 0x01,           // TM R2, 0x01
    0x6b, 0xfb,                 // JR Z, the last line
    0xa0, 0xea,                 // INCW RR10
    0x76, 0xe2, 0x01,           // TM R2, 0x01
    0xeb, 0xf9,                 // JR NZ, the INCW line
    0x0f                        // STOP HERE
};

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

// We use ints for the flags and keep them independently for speed, when accessing the
// flags register we'll have to pull them together
int                 C, Z, S, V, D, H;

// We use a special flag to indicate whether we run with interrupts disabled (post DI)
// or enabled (post EI)
int                 interrupts_disabled;

// Do we need to use extended bus timing for read accesses to the external bus
int                 extended_bus_timing;

/**
 * Defines to allow us to get at register types
 */
#define PRE0        reg[0xf5]
#define P2M         reg[0xf6]

#define P01M        reg[0xf8]
#define IPR         reg[0xf9]
#define IRQ         reg[0xfa]
#define IMR         reg[0xfb]
#define FLAGS       reg[0xfc]
#define RP          reg[0xfd]
#define SPH         reg[0xfe]
#define SPL         reg[0xff]

#define ARG_R       (regE0map(ARG_IM))
#define ARG_IR      (reg[ARG_R])
#define ARG_IM      (code[pc++])

// Special macros to work with working registers...
#define ARG_r(n)    (RP|n)
#define ARG_Ir(n)   (reg[ARG_r(n)])

// Some useful helpers...
#define PUSH16(v)   reg[--SPL] = (v & 0x00ff); reg[--SPL] = (v & 0xff00) >> 8;

#define BKPT        asm("BKPT");


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

/**
 * We need to support mapping 0xE? values into working
 * registers
 */
static inline uint8_t regE0map(uint8_t r) {
    if ((r&0xf0) == 0xe0) return RP|(r&0x0f);
    return r;
}

/**
 * Loading and storing values into registers requires checking if they are special
 * in some way
 */
static inline uint8_t LOAD_VAL_FROM_REG(uint8_t r) {
    if (reg_read[r]) {
        return reg_read[r]();
    } else {
        return reg[r];
    }
}
static inline void STORE_VAL_IN_REG(uint8_t r, uint8_t val) {
    if (reg_write[r]) {
        reg_write[r](val);
    } else {
        reg[r] = val;
    }
}
// NOTE: these assume we are clear of control registers etc.
#define LOAD_VAL_FROM_REGPAIR(r)    ((reg[r]<<8) | (reg[r+1]))

#define STORE_VAL_IN_REGPAIR(r, v)  reg[r] = (v & 0xff00) >> 8; \
                                    reg[r+1] = (v & 0x00ff);

/**
 * Defines for the different argument arrangements, these macros vary slightly as some of them
 * take an argument to allow them to be used where the arg is in the opcode.
 */
#define args_r1_r2 \
    uint8_t tmp = ARG_IM; \
    uint8_t dst = ARG_r(tmp >> 4); \
    uint8_t src = LOAD_VAL_FROM_REG(ARG_r((tmp & 0x0f)));

#define args_r1_Ir2 \
    uint8_t tmp = ARG_IM; \
    uint8_t dst = ARG_r(tmp >> 4); \
    uint8_t src = LOAD_VAL_FROM_REG(ARG_Ir((tmp & 0x0f)));

#define args_Irr1 \
    ;

#define args_r1_Irr2 \
    ;

#define args_Ir1_Irr2 \
    ;

#define args_r2_Irr1 \
    ;

#define args_Ir2_Irr1 \
    ;

#define args_R2_R1 \
    uint8_t src = LOAD_VAL_FROM_REG(ARG_R); \
    uint8_t dst = ARG_R;

#define args_IR2_R1 \
    uint8_t src = LOAD_VAL_FROM_REG(ARG_IR); \
    uint8_t dst = ARG_R;

#define args_R1_IM \
    uint8_t dst = ARG_R; \
    uint8_t src = ARG_IM;

#define args_IR1_IM \
    uint8_t dst = ARG_IR; \
    uint8_t src = ARG_IM;

#define args_r1(n) \
    uint8_t dst = ARG_r(n);

#define args_R1 \
    uint8_t dst = ARG_R;

#define args_R2 \
    uint8_t src = LOAD_VAL_FROM_REG(ARG_R);

#define args_RR1 \
    uint8_t dst = ARG_R;

#define args_IR1 \
    uint8_t dst = ARG_IR;

#define args_IR2 \
    uint8_t src = LOAD_VAL_FROM_REG(ARG_IR);

#define args_r1_IM(n) \
    uint8_t dst = ARG_r(n); \
    uint8_t src = ARG_IM;

#define args_r1_R2(n) \
    uint8_t dst = ARG_r(n); \
    uint8_t src = LOAD_VAL_FROM_REG(ARG_R);

#define args_r2_R1(n) \
    uint8_t src = LOAD_VAL_FROM_REG(ARG_r(n)); \
    uint8_t dst = ARG_R;

#define args_Ir1_r2 \
    uint8_t tmp = ARG_IM; \
    uint8_t dst = ARG_Ir(tmp >> 4); \
    uint8_t src = LOAD_VAL_FROM_REG(ARG_r((tmp & 0x0f)));

#define args_R2_IR1 \
    uint8_t src = LOAD_VAL_FROM_REG(ARG_R); \
    uint8_t dst = ARG_IR;


// This is only used by djnz, so we don't do much here and do it all
// in the main code (only if we are taking the jump)
#define args_r1_RA(n) \
    uint8_t dst = ARG_r(n);

// This is only used by jr, so we don't do anything here and do it all
// in the main code (only if we are taking the jump)
#define args_cc_RA(n) ;

// This is only used by jp, so we don't do anything here and do it all
// in the main code (only if we are taking the jump)
#define args_cc_DA(n) ;

// Only used by call and jump, so we can load as if a DA (need to use intermediate
// variable to ensure ordering is correct with load from regpair
#define args_IRR1 \
    uint16_t rs = ARG_IR; \
    uint16_t da = LOAD_VAL_FROM_REGPAIR(rs);

#define args_DA \
    uint16_t da = ARG_IM << 8; \
    da |= ARG_IM;

#define args_IM \
    uint8_t src = ARG_IM;

/**
 * Main code for the re-usable core opcode functions
 */

// ----------------------------------------------------------------------
#define func_dec            uint8_t v = LOAD_VAL_FROM_REG(dst) - 1; \
                            Z = (v == 0); \
                            S = ((v & 0x80) == 0x80); \
                            V = (v == 0x7f); \
                            STORE_VAL_IN_REG(dst, v);
// ----------------------------------------------------------------------
#define func_decw           uint16_t v = LOAD_VAL_FROM_REGPAIR(dst) - 1; \
                            Z = (v == 0); \
                            S = ((v & 0x8000) == 0x8000); \
                            V = (v == 0x7fff); \
                            STORE_VAL_IN_REGPAIR(dst, v);
// ----------------------------------------------------------------------
#define func_inc            uint8_t v = LOAD_VAL_FROM_REG(dst) + 1; \
                            Z = (v == 0); \
                            S = ((v & 0x80) == 0x80); \
                            V = (v == 0x80); \
                            STORE_VAL_IN_REG(dst, v);
// ----------------------------------------------------------------------
#define func_incw           uint16_t v = LOAD_VAL_FROM_REGPAIR(dst) + 1; \
                            Z = (v == 0); \
                            S = ((v & 0x8000) == 0x8000); \
                            V = (v == 0x8000); \
                            STORE_VAL_IN_REGPAIR(dst, v);
// ----------------------------------------------------------------------
#define func_ld             STORE_VAL_IN_REG(dst, src);
// ----------------------------------------------------------------------
// This one is optimised to only process RA if we actually take the jump
#define func_djnz           uint8_t v = LOAD_VAL_FROM_REG(dst) + 1; \
                            if (v == 0) { \
                                int8_t ra = (int8_t)ARG_IM; \
                                pc += ra; \
                            } else { \
                                pc++; \
                            }
// ----------------------------------------------------------------------
// This one is optimised to only process RA if we actually take the jump
#define func_jr(cond)       if (cond) { \
                                int8_t   ra = (int8_t)ARG_IM; \
                                pc += ra; \
                            } else { \
                                pc++; \
                            }
// ----------------------------------------------------------------------
#define func_jp_always      pc = da;
// ----------------------------------------------------------------------
// This one is optimised to only process DA if we actually take the jump
#define func_jp(cond)       if (cond) { \
                                uint16_t da = ARG_IM << 8; \
                                da |= ARG_IM; \
                                pc = da; \
                            } else { \
                                pc += 2; \
                            }
// ----------------------------------------------------------------------
#define func_add            uint8_t     d = LOAD_VAL_FROM_REG(dst); \
                            uint16_t    new = d + src; \
                            \
                            C = ((new & 0x100) == 0x100); \
                            new &= 0xff; \
                            Z = (new == 0); \
                            S = ((new & 0x80) == 0x80); \
                            V = (((d & 0x80) == (src & 0x80)) && ((new & 0x80) != (src & 0x80))); \
                            D = 0; \
                            H = (((d & 0x1f) == 0x0f) && ((new & 0x1f) == 0x10)); \
                            STORE_VAL_IN_REG(dst, new);
// ----------------------------------------------------------------------
#define func_adc            uint8_t     d = LOAD_VAL_FROM_REG(dst); \
                            uint16_t    new = d + src + C; \
                            \
                            C = ((new & 0x100) == 0x100); \
                            new &= 0xff; \
                            Z = (new == 0); \
                            S = ((new & 0x80) == 0x80); \
                            V = (((d & 0x80) == (src & 0x80)) && ((new & 0x80) != (src & 0x80))); \
                            D = 0; \
                            H = (((d & 0x1f) == 0x0f) && ((new & 0x1f) == 0x10)); \
                            STORE_VAL_IN_REG(dst, new);
// ----------------------------------------------------------------------
#define func_sub            uint8_t     d = LOAD_VAL_FROM_REG(dst); \
                            uint16_t    new = d - src; \
                            \
                            C = ((new & 0x100) == 0x100); \
                            new &= 0xff; \
                            Z = (new == 0); \
                            S = ((new & 0x80) == 0x80); \
                            V = (((d & 0x80) != (src & 0x80)) && ((new & 0x80) == (src & 0x80))); \
                            D = 0; \
                            H = !(((d & 0x1f) == 0x0f) && ((new & 0x1f) == 0x10)); \
// ----------------------------------------------------------------------
#define func_sbc            uint8_t     d = LOAD_VAL_FROM_REG(dst); \
                            uint16_t    new = d - src - C; \
                            \
                            C = ((new & 0x100) == 0x100); \
                            new &= 0xff; \
                            Z = (new == 0); \
                            S = ((new & 0x80) == 0x80); \
                            V = (((d & 0x80) != (src & 0x80)) && ((new & 0x80) == (src & 0x80))); \
                            D = 0; \
                            H = !(((d & 0x1f) == 0x0f) && ((new & 0x1f) == 0x10)); \
                            STORE_VAL_IN_REG(dst, new);
// ----------------------------------------------------------------------
#define func_srp            RP = src&0xf0;
// ----------------------------------------------------------------------
#define func_da             uint8_t v = LOAD_VAL_FROM_REG(dst); \
                            uint16_t new = v; \
                            if (D) { \
                                if (H | ((v & 0x0f) > 9)) new -= 6; \
                                if (C | (v > 0x99)) new -= 0x60; \
                            } else { \
                                if (H | ((v & 0x0f) > 9)) new += 6; \
                                if (C | (v > 0x99)) new += 0x60; \
                            } \
                            C = ((new & 0x100) == 0x100); \
                            S = ((new & 0x80) == 0x80); \
                            new &= 0xff; \
                            Z = (new == 0); \
                            STORE_VAL_IN_REG(dst, new);
// ----------------------------------------------------------------------
#define func_push           reg[--SPL] = src;
// ----------------------------------------------------------------------
#define func_pop            STORE_VAL_IN_REG(dst, reg[SPL++]);
// ----------------------------------------------------------------------
#define func_tcm            uint8_t v = (LOAD_VAL_FROM_REG(dst) ^ 0xff) & src; \
                            Z = (v == 0); \
                            S = ((v & 0x80) == 0x80); \
                            V = 0;
// ----------------------------------------------------------------------
#define func_tm             uint8_t v = LOAD_VAL_FROM_REG(dst) & src; \
                            Z = (v == 0); \
                            S = ((v & 0x80) == 0x80); \
                            V = 0;
// ----------------------------------------------------------------------
#define func_rl             uint8_t v = LOAD_VAL_FROM_REG(dst); \
                            uint8_t new = (v << 1) | ((v & 0x80) == 1); \
                            C = ((v & 0x80) == 0x80); \
                            Z = (new == 0); \
                            S = ((new & 0x80) == 0x80); \
                            V = ((v & 0x80) != (new & 0x80)); \
                            STORE_VAL_IN_REG(dst, new);
// ----------------------------------------------------------------------
#define func_rlc            uint8_t v = LOAD_VAL_FROM_REG(dst); \
                            uint8_t new = (v << 1) | C; \
                            C = ((v & 0x80) == 0x80); \
                            Z = (new == 0); \
                            S = ((new & 0x80) == 0x80); \
                            V = ((v & 0x80) != (new & 0x80)); \
                            STORE_VAL_IN_REG(dst, new);
// ----------------------------------------------------------------------
#define func_rr             uint8_t v = LOAD_VAL_FROM_REG(dst); \
                            uint8_t new = ((v & 0x01) << 7) | (v >> 1); \
                            C = (v & 0x01); \
                            Z = (new == 0); \
                            S = ((new & 0x80) == 0x80); \
                            V = ((v & 0x80) != (new & 0x80)); \
                            STORE_VAL_IN_REG(dst, new);
// ----------------------------------------------------------------------
#define func_rrc            uint8_t v = LOAD_VAL_FROM_REG(dst); \
                            uint8_t new = (C << 7) | (v >> 1); \
                            C = (v & 0x01); \
                            Z = (new == 0); \
                            S = ((new & 0x80) == 0x80); \
                            V = ((v & 0x80) != (new & 0x80)); \
                            STORE_VAL_IN_REG(dst, new);
// ----------------------------------------------------------------------
#define func_sra            uint8_t v = LOAD_VAL_FROM_REG(dst); \
                            uint8_t new = (v & 0x80) | ((v >> 1) & 0x7f); \
                            C = (v & 0x01); \
                            Z = (new == 0); \
                            S = ((new & 0x80) == 0x80); \
                            V = 0; \
                            STORE_VAL_IN_REG(dst, new);
// ----------------------------------------------------------------------
#define func_com            uint8_t v = LOAD_VAL_FROM_REG(dst) ^ 0xff; \
                            Z = (v == 0); \
                            S = ((v & 0x80) == 0x80); \
                            V = 0; \
                            STORE_VAL_IN_REG(dst, v);
// ----------------------------------------------------------------------
#define func_or             uint8_t v = LOAD_VAL_FROM_REG(dst) | src; \
                            Z = (v == 0); \
                            S = ((v & 0x80) == 0x80); \
                            V = 0; \
                            STORE_VAL_IN_REG(dst, v);
// ----------------------------------------------------------------------
#define func_and            uint8_t v = LOAD_VAL_FROM_REG(dst) & src; \
                            Z = (v == 0); \
                            S = ((v & 0x80) == 0x80); \
                            V = 0; \
                            STORE_VAL_IN_REG(dst, v);
// ----------------------------------------------------------------------
#define func_xor            uint8_t v = LOAD_VAL_FROM_REG(dst) ^ src; \
                            Z = (v == 0); \
                            S = ((v & 0x80) == 0x80); \
                            V = 0; \
                            STORE_VAL_IN_REG(dst, v);
// ----------------------------------------------------------------------
#define func_clr            STORE_VAL_IN_REG(dst, 0);
// ----------------------------------------------------------------------
#define func_cp             uint8_t v = LOAD_VAL_FROM_REG(dst); \
                            uint16_t new = v - src; \
                            C = ((new & 0x100) == 0x100); \
                            new &= 0xff; \
                            Z = (new == 0); \
                            S = ((new & 0x80) == 0x80); \
                            V = (((v & 0x80) != (src & 0x80)) && ((new & 0x80) == (src & 0x80)));
// ----------------------------------------------------------------------
#define func_lde ;
#define func_ldei ;
// ----------------------------------------------------------------------
// Load to and from code, we need to trap stuff that would be in firmware
// and return the code from our memory, anything else will need to be an
// external read!
#define func_ldc ;
#define func_ldci ;
// ----------------------------------------------------------------------
#define func_swap           uint8_t v = LOAD_VAL_FROM_REG(dst); \
                            v = (v << 4) | (v >> 4); \
                            Z = (v == 0); \
                            S = ((v & 0x80) == 0x80); \
                            STORE_VAL_IN_REG(dst, v);
// ----------------------------------------------------------------------
#define func_call           PUSH16(pc); \
                            pc = da;
// ----------------------------------------------------------------------


/**
 * Functions to read and write memory...
 */
// ----------------------------------------------------------------------
void LDC_read() {
    uint8_t tmp = ARG_IM;
    uint8_t dst = ARG_r(tmp >> 4);
    uint8_t src = ARG_r((tmp & 0x0f));
    uint16_t addr = LOAD_VAL_FROM_REGPAIR(src);

    if (addr < 0x2000) {
        STORE_VAL_IN_REG(dst, code[addr]);
    } else {
        // TODO: bus read
    }
}
void LDCI_read() {
    uint8_t tmp = ARG_IM;
    uint8_t dst = ARG_r(tmp >> 4);
    uint8_t src = ARG_r((tmp & 0x0f));
    uint16_t addr = LOAD_VAL_FROM_REGPAIR(src);
    uint8_t idst = reg[dst];
   
    if (addr < 0x2000) {
       STORE_VAL_IN_REG(idst, code[addr]);
    } else {
       // TODO: bus read
    } 

    // Incremement...
    STORE_VAL_IN_REG(dst, idst + 1); 
    STORE_VAL_IN_REGPAIR(src, (addr + 1));
}
// ----------------------------------------------------------------------
void LDC_write() {
    uint8_t tmp = ARG_IM;
    uint8_t src = ARG_r(tmp >> 4);
    uint8_t dst = ARG_r((tmp & 0x0f));
    uint16_t addr = LOAD_VAL_FROM_REGPAIR(src);

    if (addr < 0x2000) {
        // Unable to write to code memory
    } else {
        // TODO: bus write
        dst++;
    }
}
// ----------------------------------------------------------------------

/**
 * Function definitions for non-arg opcodes
 */
void DI() {
    write_IMR(IMR & 0x7f);
}
// ----------------------------------------------------------------------
void EI() {
    write_IMR(IMR | 0x80);
}
// ----------------------------------------------------------------------
void RET() { 
    pc = (reg[SPL] << 8) | reg[SPL+1]; 
    SPL += 2; 
}
// ----------------------------------------------------------------------
void IRET() { 
    // TODO: do we need to re-enable interrupts? Yes I think we do.
    write_FLAGS(reg[SPL++]); 
    pc = (reg[SPL] << 8) | reg[SPL+1]; 
    SPL += 2; 
    EI();
}
// ----------------------------------------------------------------------
void RCF() { C = 0; }
// ----------------------------------------------------------------------
void SCF() { C = 1; }
// ----------------------------------------------------------------------
void CCF() { C = !C; }
// ----------------------------------------------------------------------
void NOP() { /* TODO: should be 6 Z8 Cycles delay */ }
// ----------------------------------------------------------------------


/**
 * Main macros for defining the opcodes
 */
// Standard one...
#define OP(mnemonic, args, func) \
    void mnemonic##_##args() { \
        args_##args \
        func_##func \
    }

// Where we need opcode related arguments...
#define OPn(mnemonic, args, func, n) \
    void mnemonic##_##args##_##n() { \
        args_##args(n) \
        func_##func \
    }

// With code related changes (typically conditionals)...
#define OPc(mnemonic, args, func, n, code) \
    void mnemonic##_##args##_##n() { \
        args_##args(n) \
        func_##func(code) \
    }

/**
 * Opcode Definitions...
 */
// 00 - 07
OP(DEC, R1, dec); OP(DEC, IR1, dec); OP(ADD, r1_r2, add); OP(ADD, r1_Ir2, add);
OP(ADD, R2_R1, add); OP(ADD, IR2_R1, add); OP(ADD, R1_IM, add); OP(ADD, IR1_IM, add);
// 10 - 17
OP(RLC, R1, rlc); OP(RLC, IR1, rlc); OP(ADC, r1_r2, adc); OP(ADC, r1_Ir2, adc);
OP(ADC, R2_R1, adc); OP(ADC, IR2_R1, adc); OP(ADC, R1_IM, adc); OP(ADC, IR1_IM, adc);
// 20 - 27
OP(INC, R1, inc); OP(INC, IR1, inc); OP(SUB, r1_r2, sub); OP(SUB, r1_Ir2, sub);
OP(SUB, R2_R1, sub); OP(SUB, IR2_R1, sub); OP(SUB, R1_IM, sub); OP(SUB, IR1_IM, sub);
// 30 - 37
OP(JP, IRR1, jp_always); OP(SRP, IM, srp); OP(SBC, r1_r2, sbc); OP(SBC, r1_Ir2, sbc);
OP(SBC, R2_R1, sbc); OP(SBC, IR2_R1, sbc); OP(SBC, R1_IM, sbc); OP(SBC, IR1_IM, sbc);
// 40 - 47
OP(DA, R1, da); OP(DA, IR1, da); OP(OR, r1_r2, or); OP(OR, r1_Ir2, or);
OP(OR, R2_R1, or); OP(OR, IR2_R1, or); OP(OR, R1_IM, or); OP(OR, IR1_IM, or);
// 50 - 57
OP(POP, R1, pop); OP(POP, IR1, pop); OP(AND, r1_r2, and); OP(AND, r1_Ir2, and);
OP(AND, R2_R1, and); OP(AND, IR2_R1, and); OP(AND, R1_IM, and); OP(AND, IR1_IM, and);
// 60 - 67
OP(COM, R1, com); OP(COM, IR1, com); OP(TCM, r1_r2, tcm); OP(TCM, r1_Ir2, tcm);
OP(TCM, R2_R1, tcm); OP(TCM, IR2_R1, tcm); OP(TCM, R1_IM, tcm); OP(TCM, IR1_IM, tcm);
// 70 - 77
OP(PUSH, R2, push); OP(PUSH, IR2, push); OP(TM, r1_r2, tm); OP(TM, r1_Ir2, tm);
OP(TM, R2_R1, tm); OP(TM, IR2_R1, tm); OP(TM, R1_IM, tm); OP(TM, IR1_IM, tm);
// 80 - 87
OP(DECW, RR1, decw); OP(DECW, IR1, decw); OP(LDE, r1_Irr2, lde); OP(LDEI, Ir1_Irr2, ldei);
// 90 - 97
OP(RL, R1, rl); OP(RL, IR1, rl); OP(LDE, Irr1, lde); OP(LDEI, Ir2_Irr1, ldei);
// A0 - A7
OP(INCW, RR1, incw); OP(INCW, IR1, incw); OP(CP, r1_r2, cp); OP(CP, r1_Ir2, cp);
OP(CP, R2_R1, cp); OP(CP, IR2_R1, cp); OP(CP, R1_IM, cp); OP(CP, IR1_IM, cp);
// B0 - B7
OP(CLR, R1, clr); OP(CLR, IR1, clr); OP(XOR, r1_r2, xor); OP(XOR, r1_Ir2, xor);
OP(XOR, R2_R1, xor); OP(XOR, IR2_R1, xor); OP(XOR, R1_IM, xor); OP(XOR, IR1_IM, xor);
// C0 - C7
OP(RRC, R1, rrc); OP(RRC, IR1, rrc); OP(LDC, r1_Irr2, ldc); OP(LDCI, Ir1_Irr2, ldci);
// D0 - D7
OP(SRA, R1, sra); OP(SRA, IR1, sra); OP(LDC, r2_Irr1, ldc); OP(LDCI, Ir2_Irr1, ldci);
OP(CALL, IRR1, call); OP(CALL, DA, call);
// E0 - E7
OP(RR, R1, rr); OP(RR, IR1, rr); OP(LD, r1_Ir2, ld); OP(LD, R2_R1, ld);
OP(LD, IR2_R1, ld); OP(LD, R1_IM, ld); OP(LD, IR1_IM, ld);
// F0 - F7
OP(SWAP, R1, swap); OP(SWAP, IR1, swap); OP(LD, Ir1_r2, ld); OP(LD, R2_IR1, ld);
// *8
OPn(LD, r1_R2, ld, 0); OPn(LD, r1_R2, ld, 1); OPn(LD, r1_R2, ld, 2); OPn(LD, r1_R2, ld, 3); 
OPn(LD, r1_R2, ld, 4); OPn(LD, r1_R2, ld, 5); OPn(LD, r1_R2, ld, 6); OPn(LD, r1_R2, ld, 7); 
OPn(LD, r1_R2, ld, 8); OPn(LD, r1_R2, ld, 9); OPn(LD, r1_R2, ld, 10); OPn(LD, r1_R2, ld, 11); 
OPn(LD, r1_R2, ld, 12); OPn(LD, r1_R2, ld, 13); OPn(LD, r1_R2, ld, 14); OPn(LD, r1_R2, ld, 15);
// *9
OPn(LD, r2_R1, ld, 0); OPn(LD, r2_R1, ld, 1); OPn(LD, r2_R1, ld, 2); OPn(LD, r2_R1, ld, 3);
OPn(LD, r2_R1, ld, 4); OPn(LD, r2_R1, ld, 5); OPn(LD, r2_R1, ld, 6); OPn(LD, r2_R1, ld, 7);
OPn(LD, r2_R1, ld, 8); OPn(LD, r2_R1, ld, 9); OPn(LD, r2_R1, ld, 10); OPn(LD, r2_R1, ld, 11);
OPn(LD, r2_R1, ld, 12); OPn(LD, r2_R1, ld, 13); OPn(LD, r2_R1, ld, 14); OPn(LD, r2_R1, ld, 15);
// *A
OPn(DJNZ, r1_RA, djnz, 0); OPn(DJNZ, r1_RA, djnz, 1); OPn(DJNZ, r1_RA, djnz, 2); OPn(DJNZ, r1_RA, djnz, 3);
OPn(DJNZ, r1_RA, djnz, 4); OPn(DJNZ, r1_RA, djnz, 5); OPn(DJNZ, r1_RA, djnz, 6); OPn(DJNZ, r1_RA, djnz, 7);
OPn(DJNZ, r1_RA, djnz, 8); OPn(DJNZ, r1_RA, djnz, 9); OPn(DJNZ, r1_RA, djnz, 10); OPn(DJNZ, r1_RA, djnz, 11);
OPn(DJNZ, r1_RA, djnz, 12); OPn(DJNZ, r1_RA, djnz, 13); OPn(DJNZ, r1_RA, djnz, 14); OPn(DJNZ, r1_RA, djnz, 15);
// *B
OPc(JR, cc_RA, jr, 0, 0);                   // F
OPc(JR, cc_RA, jr, 1, ((S ^ V)));           // LT
OPc(JR, cc_RA, jr, 2, (Z | (S ^ V)));       // LE
OPc(JR, cc_RA, jr, 3, (C | Z));             // ULE
OPc(JR, cc_RA, jr, 4, (V));                 // OV
OPc(JR, cc_RA, jr, 5, (S));                 // MI
OPc(JR, cc_RA, jr, 6, (Z));                 // Z 
OPc(JR, cc_RA, jr, 7, (C));                 // C 
OPc(JR, cc_RA, jr, 8, (1));                 // T
OPc(JR, cc_RA, jr, 9, (!(S ^ V)));          // GE
OPc(JR, cc_RA, jr, 10, (!(Z | (S ^ V))));   // GT
OPc(JR, cc_RA, jr, 11, ((!C) & (!Z)));      // UGT
OPc(JR, cc_RA, jr, 12, (!V));               // NOV
OPc(JR, cc_RA, jr, 13, (!S));               // PL
OPc(JR, cc_RA, jr, 14, (!Z));               // NZ
OPc(JR, cc_RA, jr, 15, (!C));               // NC
// *C
OPn(LD, r1_IM, ld, 0); OPn(LD, r1_IM, ld, 1); OPn(LD, r1_IM, ld, 2); OPn(LD, r1_IM, ld, 3);
OPn(LD, r1_IM, ld, 4); OPn(LD, r1_IM, ld, 5); OPn(LD, r1_IM, ld, 6); OPn(LD, r1_IM, ld, 7);
OPn(LD, r1_IM, ld, 8); OPn(LD, r1_IM, ld, 9); OPn(LD, r1_IM, ld, 10); OPn(LD, r1_IM, ld, 11);
OPn(LD, r1_IM, ld, 12); OPn(LD, r1_IM, ld, 13); OPn(LD, r1_IM, ld, 14); OPn(LD, r1_IM, ld, 15);
// *D
OPc(JP, cc_DA, jp, 0, 0);                   // F
OPc(JP, cc_DA, jp, 1, ((S ^ V)));           // LT
OPc(JP, cc_DA, jp, 2, (Z | (S ^ V)));       // LE
OPc(JP, cc_DA, jp, 3, (C | Z));             // ULE
OPc(JP, cc_DA, jp, 4, (V));                 // OV
OPc(JP, cc_DA, jp, 5, (S));                 // MI
OPc(JP, cc_DA, jp, 6, (Z));                 // Z 
OPc(JP, cc_DA, jp, 7, (C));                 // C 
OPc(JP, cc_DA, jp, 8, (1));                 // T
OPc(JP, cc_DA, jp, 9, (!(S ^ V)));          // GE
OPc(JP, cc_DA, jp, 10, (!(Z | (S ^ V))));   // GT
OPc(JP, cc_DA, jp, 11, ((!C) & (!Z)));      // UGT
OPc(JP, cc_DA, jp, 12, (!V));               // NOV
OPc(JP, cc_DA, jp, 13, (!S));               // PL
OPc(JP, cc_DA, jp, 14, (!Z));               // NZ
OPc(JP, cc_DA, jp, 15, (!C));               // NC
// *E
OPn(INC, r1, inc, 0); OPn(INC, r1, inc, 1); OPn(INC, r1, inc, 2); OPn(INC, r1, inc, 3);
OPn(INC, r1, inc, 4); OPn(INC, r1, inc, 5); OPn(INC, r1, inc, 6); OPn(INC, r1, inc, 7);
OPn(INC, r1, inc, 8); OPn(INC, r1, inc, 9); OPn(INC, r1, inc, 10); OPn(INC, r1, inc, 11);
OPn(INC, r1, inc, 12); OPn(INC, r1, inc, 13); OPn(INC, r1, inc, 14); OPn(INC, r1, inc, 15);
// *F



// ----------------------------------------------------------------------
// If we execute an illegal opcode, just bring some values into local scope 
// so they are easy to see in the debugger, and then cause a breakpoint.
// ----------------------------------------------------------------------
#pragma GCC push_options
#pragma GCC optimize ("O0")
void ILLEGAL() {
    uint16_t    from_pc = pc-1;
    uint8_t     failed_opcode __attribute__((unused)) = code[from_pc];
    
    BKPT;
}
#pragma GCC pop_options


// ----------------------------------------------------------------------
// The main array of op-codes to function calls
// ----------------------------------------------------------------------
void (*map[256])() = {
// 00
    DEC_R1, DEC_IR1, ADD_r1_r2, ADD_r1_Ir2, ADD_R2_R1, ADD_IR2_R1, ADD_R1_IM, ADD_IR1_IM, 
        LD_r1_R2_0, LD_r2_R1_0, DJNZ_r1_RA_0, JR_cc_RA_0, LD_r1_IM_0, JP_cc_DA_0, INC_r1_0, ILLEGAL,
// 10
    RLC_R1, RLC_IR1, ADC_r1_r2, ADC_r1_Ir2, ADC_R2_R1, ADC_IR2_R1, ADC_R1_IM, ADC_IR1_IM, 
        LD_r1_R2_1, LD_r2_R1_1, DJNZ_r1_RA_1, JR_cc_RA_1, LD_r1_IM_1, JP_cc_DA_1, INC_r1_1, ILLEGAL,
// 20
    INC_R1, INC_IR1, SUB_r1_r2, SUB_r1_Ir2, SUB_R2_R1, SUB_IR2_R1, SUB_R1_IM, SUB_IR1_IM, 
        LD_r1_R2_2, LD_r2_R1_2, DJNZ_r1_RA_2, JR_cc_RA_2, LD_r1_IM_2, JP_cc_DA_2, INC_r1_2, ILLEGAL,
// 30
    JP_IRR1, SRP_IM, SBC_r1_r2, SBC_r1_Ir2, SBC_R2_R1, SBC_IR2_R1, SBC_R1_IM, SBC_IR1_IM, 
        LD_r1_R2_3, LD_r2_R1_3, DJNZ_r1_RA_3, JR_cc_RA_3, LD_r1_IM_3, JP_cc_DA_3, INC_r1_3, ILLEGAL,
// 40
    DA_R1, DA_IR1, OR_r1_r2, OR_r1_Ir2, OR_R2_R1, OR_IR2_R1, OR_R1_IM, OR_IR1_IM, 
        LD_r1_R2_4, LD_r2_R1_4, DJNZ_r1_RA_4, JR_cc_RA_4, LD_r1_IM_4, JP_cc_DA_4, INC_r1_4, ILLEGAL,
// 50
    POP_R1, POP_IR1, AND_r1_r2, AND_r1_Ir2, AND_R2_R1, AND_IR2_R1, AND_R1_IM, AND_IR1_IM, 
        LD_r1_R2_5, LD_r2_R1_5, DJNZ_r1_RA_5, JR_cc_RA_5, LD_r1_IM_5, JP_cc_DA_5, INC_r1_5, ILLEGAL,
// 60
    COM_R1, COM_IR1, TCM_r1_r2, TCM_r1_Ir2, TCM_R2_R1, TCM_IR2_R1, TCM_R1_IM, TCM_IR1_IM, 
        LD_r1_R2_6, LD_r2_R1_6, DJNZ_r1_RA_6, JR_cc_RA_6, LD_r1_IM_6, JP_cc_DA_6, INC_r1_6, ILLEGAL,
// 70
    PUSH_R2, PUSH_IR2, TM_r1_r2, TM_r1_Ir2, TM_R2_R1, TM_IR2_R1, TM_R1_IM, TM_IR1_IM, 
        LD_r1_R2_7, LD_r2_R1_7, DJNZ_r1_RA_7, JR_cc_RA_7, LD_r1_IM_7, JP_cc_DA_7, INC_r1_7, ILLEGAL,
// 80
    DECW_RR1, DECW_IR1, LDE_r1_Irr2, LDEI_Ir1_Irr2, ILLEGAL, ILLEGAL, ILLEGAL, ILLEGAL,
        LD_r1_R2_8, LD_r2_R1_8, DJNZ_r1_RA_8, JR_cc_RA_8, LD_r1_IM_8, JP_cc_DA_8, INC_r1_8, DI,
// 90
    RL_R1, RL_IR1, LDE_Irr1, LDEI_Ir2_Irr1, ILLEGAL, ILLEGAL, ILLEGAL, ILLEGAL,
        LD_r1_R2_9, LD_r2_R1_9, DJNZ_r1_RA_9, JR_cc_RA_9, LD_r1_IM_9, JP_cc_DA_9, INC_r1_9, EI,
// A0
    INCW_RR1, INCW_IR1, CP_r1_r2, CP_r1_Ir2, CP_R2_R1, CP_IR2_R1, CP_R1_IM, CP_IR1_IM, 
        LD_r1_R2_10, LD_r2_R1_10, DJNZ_r1_RA_10, JR_cc_RA_10, LD_r1_IM_10, JP_cc_DA_10, INC_r1_10, RET,
// B0
    CLR_R1, CLR_IR1, XOR_r1_r2, XOR_r1_Ir2, XOR_R2_R1, XOR_IR2_R1, XOR_R1_IM, XOR_IR1_IM, 
        LD_r1_R2_11, LD_r2_R1_11, DJNZ_r1_RA_11, JR_cc_RA_11, LD_r1_IM_11, JP_cc_DA_11, INC_r1_11, IRET,
// C0
    RRC_R1, RRC_IR1, LDC_r1_Irr2, LDCI_Ir1_Irr2, ILLEGAL, ILLEGAL, ILLEGAL, /* LD indexed */ ILLEGAL,
        LD_r1_R2_12, LD_r2_R1_12, DJNZ_r1_RA_12, JR_cc_RA_12, LD_r1_IM_12, JP_cc_DA_12, INC_r1_12, RCF,
// D0
    SRA_R1, SRA_IR1, LDC_r2_Irr1, LDCI_Ir2_Irr1, CALL_IRR1, ILLEGAL, CALL_DA, /* LD indexed */ ILLEGAL,
        LD_r1_R2_13, LD_r2_R1_13, DJNZ_r1_RA_13, JR_cc_RA_13, LD_r1_IM_13, JP_cc_DA_13, INC_r1_13, SCF,
// E0
    RR_R1, RR_IR1, ILLEGAL, LD_r1_Ir2, LD_R2_R1, LD_IR2_R1, LD_R1_IM, LD_IR1_IM,
        LD_r1_R2_14, LD_r2_R1_14, DJNZ_r1_RA_14, JR_cc_RA_14, LD_r1_IM_14, JP_cc_DA_14, INC_r1_14, CCF,
// F0
    SWAP_R1, SWAP_IR1, ILLEGAL, LD_Ir1_r2, ILLEGAL, LD_R2_IR1, ILLEGAL, ILLEGAL,
        LD_r1_R2_15, LD_r2_R1_15, DJNZ_r1_RA_15, JR_cc_RA_15, LD_r1_IM_15, JP_cc_DA_15, INC_r1_15, NOP,
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

    for(;;) {
    
        CyGlobalIntDisable;
        map[code[pc++]]();
        
        // Now we can give ourselves a short window to acknowledge IRQ's if they are
        // enabled.
        if (interrupts_disabled) {
            CyGlobalIntDisable;
        } else {
            CyGlobalIntEnable;
        }
    }
    // STOP HERE
}



/* [] END OF FILE */
