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


// CyGlobalIntDisable; \

// Generic IRQ calling macro
#define     CALL_IRQ(n)     PUSH16(pc) \
                            PUSH(FLAGS) \
                            pc = code[(n*2)] << 8 | code[(n*2)+1]; \
                            CyGlobalIntDisable; \
                            IRQ = IRQ & (0xff ^ (1 << n)); \
                            CyGlobalIntEnable; \
                            IMR &= 0x7f;    /* DI */
                       
/**
 * Globals that represent the main state of the CPU
 */
uint16_t            pc;
volatile uint8_t             reg[256];

// We use ints for the flags and keep them independently for speed, when accessing the
// flags register we'll have to pull them together
int                 C, Z, S, V, D, H;

// Do we need to use extended bus timing for read accesses to the external bus
int                 extended_bus_timing;

// Is the bus enabled or disabled (used for reading port 1)
int                 bus_enabled;


/**
 * FLAGS: We need to be able to combine the flags for a register read or write
 */
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

/**
 * If we try to write a value to a register that we can't support then
 * we end up here, with some helpful local variables and a breakpoint to
 * the debugger.
 */
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
    // We only support the value of C9, and that's how the design is setup, so
    // nothing really to do here.
    if (val != 0xC9) REG_FAIL(P3M, val);
}
void write_port3(uint8_t val) {
    // The only adjustable outputs is P35, so let's make sure we only write that bit
    P35_DR = (P35_DR & 0xffffffdf) | (uint32_t)(val & 0x20); 
}
uint8_t read_port3() {
    // We need to read P31/2/3 (which are actually PSoC P40/1/2)
    return (uint8_t)((P4_PS & 0x07) << 1);
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

    /*
    if (IMR & 0x01) { // IRQ 0 -- not used
    }
    if (IMR & 0x02) { // IRQ 1 -- keyboard (P42/P33)
        P4_SetInterruptMode(P4_P33_INTR, P4_INTR_FALLING);
    } else {
        P4_SetInterruptMode(P4_P33_INTR, P4_INTR_NONE);
        P4_INTSTAT = 0x04;
    }
    if (IMR & 0x04) { // IRQ 2 -- ADC (P40/P31)
        P4_SetInterruptMode(P4_P31_INTR, P4_INTR_FALLING);
    } else {
        P4_SetInterruptMode(P4_P31_INTR, P4_INTR_NONE);
        P4_INTSTAT = 0x01;
    }
    if (IMR & 0x08) { // IRQ 3 -- Serial Input
        IRQ_Serial_Enable();
    } else {
        IRQ_Serial_Disable();
        IRQ_Serial_ClearPending();
    }
    if (IMR & 0x10) { // IRQ 4 -- Serial Output (not used)
    }
    if (IMR & 0x20) { // IRQ 5 -- Timer
    }
    */
    
    return;
    
    // If interrupts are enabled then check if we need to run anything
    // (in priority order.)
    if (IMR & 0x80) {
        if (IMR & IRQ & 0x04) {
            CALL_IRQ(2);
        } else if (IMR & IRQ & 0x01) {
            CALL_IRQ(0);
        } else if (IMR & IRQ & 0x02) {
            CALL_IRQ(1);
        } else if (IMR & IRQ & 0x10) {
            CALL_IRQ(4);
        } else if (IMR & IRQ & 0x08) {
            CALL_IRQ(3);
        } else if (IMR & IRQ & 0x20) {
            CALL_IRQ(5);
        }
    }
}


// ----------------------------------------------------------------------
/**
 * IPR - interrupt priority register
 *
 * Only ever set to 0x31 by 8840A
 *
 * 31 - A:3>5 B:2>0 C:1>4  B>C>A == priority is 2 (ADC),0 (n/a),1 (kbd),4 (SerOut),3 (SetIn),5 (T1)
 */
void write_IPR(uint8_t val) {
    // Don't actually need to do anything here because priorities are set by the design
    // and IPR is unreadable
    IPR = val;
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
    PRE0 = val;
}

uint8_t read_IRQ() {
    uint8_t val = IRQ & 0xef;
    
    if ((UART_INTR_TX_REG & UART_INTR_TX_EMPTY)) {
        val |= 0x10;
        UART_INTR_TX_REG = UART_INTR_TX_EMPTY;
    }
    
    return val; // | 0x10;
}

void write_IRQ(uint8_t val) {
    uint8_t saveInts = CyEnterCriticalSection();
    
    IRQ = val;
    
    CyExitCriticalSection(saveInts);
}

/**
 * Serial write and read. Simply transmit and receive and also enable the relevant
 * IRQ.
 *
 * For receiving we need to pull the parity status into bit 7, the 8840A uses this
 * to check if things are ok, and disabled serial if there are problems.
 */
void write_SIO(uint8_t val) {
    UART_TX_FIFO_WR_REG = val;     
    UART_INTR_TX_REG = UART_INTR_TX_EMPTY;
    
    // Re-enable the interrupt for empty tx fifo...
    //UART_INTR_TX_MASK_REG |= UART_INTR_TX_EMPTY;
}

uint8_t last100[100];
int32_t     scount = 0;

uint8_t read_SIO() {
    //IRQ &= 0xF7;
    uint8_t val = SIO;
    last100[scount++] = val;
    if(scount==100) scount = 0;
    
    return val;
}


/**
 * Port 0: should just be a simple read/write of the register. The lower
 *         four bits are fixed to the DSI, so they should return 0x0F and
 *         the higher four bits will be valid.
 */
uint8_t read_port0() { return (uint8_t)(P0_PS | 0x0F); }
void write_port0(uint8_t val) { P0_DR = (P0_DR & 0xffffff0f) | (val & 0xf0);}

/**
 * Port 1: This is either all on the DSI (returning 0xFF) or all set as
 *         byte output, in which case we should return the normal register.
 *         Writing is totally standard.
 */
uint8_t read_port1() { return (bus_enabled ? 0xff : (uint8_t)P1_PS); }
void write_port1(uint8_t val) { P1_DR = (P1_DR & 0xffffff00) | val; }

/**
 * Port 2: this is a simple 8 bit controllable port, so the easy approach
 */
uint8_t read_port2() { return (uint8_t)P2_PS; }
void write_port2(uint8_t val) { P2_DR = (P2_DR & 0xffffff00) | val; }

/**
 * Map the registers so that we can see where a special access is required, this is
 * a bit painful, but it's fairly quick to do it this way and will be in flash rather
 * than RAM.
 */
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
    0,                  // 244 - T0
    0,                  // 245 - PRE0
    0,                  // 246 - P2M (write only)
    0,                  // 247 - P3M (write only)
    0,                  // 248 - P01M (write only)
    0,                  // 249 - IPR (write only)
    read_IRQ,                  // 250 - IRQ (read is ok)
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
    0,                  // 244 - T0
    0,                  // 245 - PRE0
    write_P2M,          // 246 - P2M
    write_P3M,          // 247 - P3M
    write_P01M,         // 248 - P01M
    write_IPR,          // 249 - IPR
    write_IRQ,                  // 250 - IRQ (normal reg!)
    write_IMR,          // 251 - IMR
    write_FLAGS,        // 252
    0,                  // 253 - RP (ok)
    0,                  // 254 - SPH (ok)
    0,                  // 255 - SPL (ok)
};


/**
 * Port 4 contains the two IRQ input lines (ADC and Keyboard), so if we receive an IRQ
 * for either line then we clear the pending bit and set the right bit in the IRQ register.
 * If we are then not masked we will cause the ISR to be called, in the right priority
 * order (ADC first!)
 */

CY_ISR(Handle_IRQ_P4) {
    uint8_t saveInts = CyEnterCriticalSection();
    
    uint32_t is = P4_INTSTAT;
    uint32_t is_cfg = P4_INTCFG;

    if (P4_INTSTAT & 0x04) {
        P4_INTSTAT = 0x04;
        IRQ |= 0x02;
    }
    if (P4_INTSTAT & 0x01) {
        P4_INTSTAT = 0x01;
        IRQ |= 0x04;
    }

    CyExitCriticalSection(saveInts);    
}

/**
 * The timer IRQ simply sets the relevant IRQ bit and then calls the ISR if it's
 * properly enabled.
 ( (This never seems to get triggered by the 8840A)
 */
CY_ISR(Handle_IRQ_Timer1) {     // IRQ 5
    uint8_t saveInts = CyEnterCriticalSection();
    IRQ |= 0x20;
    CyExitCriticalSection(saveInts);
}


/**
 * A serial IRQ will either be TX_EMPTY or RX_NOT_EMPTY, in either case we set
 * the correct IRQ, clear the pending bit, and then mask of the interrupt so that
 * we don't keep getting it. They will only be re-enabled once we send or receive
 * a byte over serial.
 *
 * If interrupts are enabled and we are not masking the particular ones then this
 * will also cause the Z8 ISR to be called.
 */

uint32_t    sicount = 0;
int         serial_rx_pending = 0;

CY_ISR(Handle_IRQ_Serial) {
    uint8_t saveInts = CyEnterCriticalSection();

    /* This should only be for a received byte */
    if (UART_INTR_RX_REG & UART_INTR_RX_NOT_EMPTY) {
/*        
        uint8_t sz = UART_RX_FIFO_STATUS_REG;
        uint8_t val;

        // Read all the bytes in the fifo!
        for (int i=0; i < sz; i++) {
            val = UART_RX_FIFO_RD_REG;
        }
        
        if(UART_INTR_RX_REG & UART_INTR_RX_PARITY_ERROR) {
            val |= 0x80;
        }
        
        SIO = val;
*/
        SIO = UART_RX_FIFO_RD_REG;
        // Clear the pending bit...
        UART_INTR_RX_REG = UART_INTR_RX_NOT_EMPTY | UART_INTR_RX_PARITY_ERROR;
        
        // Set the IRQ bit...
        IRQ |= 0x08;                // IRQ 3
//        CALL_IRQ(3);
    } else {
        uint32_t rx = UART_INTR_RX_REG;
        uint32_t rxmast = UART_INTR_RX_MASK_REG;
        
        BKPT;
    }
    CyExitCriticalSection(saveInts);
}

void setup_emulator() {
    pc = 0x0c;
    SPL = 0x80;
    
    // PORT 0 SETUP
    
    // PORT 1 SETUP
    
    // PORT 2 SETUP
    
    
    // Setup the serial port IRQ... 
    IRQ_Serial_Disable();
    IRQ_Serial_SetVector(&Handle_IRQ_Serial);
    IRQ_Serial_SetPriority((uint8)IRQ_Serial_INTC_PRIOR_NUMBER);
    IRQ_Serial_ClearPending();
    IRQ_Serial_Enable();

    // Setup the timer IRQ...
    IRQ_Timer1_Disable();
    IRQ_Timer1_SetVector(&Handle_IRQ_Timer1);
    IRQ_Timer1_SetPriority((uint8)IRQ_Timer1_INTC_PRIOR_NUMBER);
    IRQ_Timer1_ClearPending();
    
    // Setup IRQ's for Port 3 (mapped to Port4 PSoC)...
    IRQ_P4_Disable();
    IRQ_P4_SetVector(&Handle_IRQ_P4);
    IRQ_P4_SetPriority((uint8)IRQ_P4_INTC_PRIOR_NUMBER);

    // Make sure they all start with no interrupts enabled
    P4_SetInterruptMode(P4_P31_INTR, P4_INTR_FALLING);
    P4_SetInterruptMode(P4_P32_INTR, P4_INTR_NONE);
    P4_SetInterruptMode(P4_P33_INTR, P4_INTR_FALLING);
    
    // Ensure there are no pending interrupts on the port itself
    P4_ClearInterrupt();

    // Then we can clear the overall pending flag for P4
    IRQ_P4_ClearPending();
    
    // Now we can enable the IRQ for this group of three pins
    IRQ_P4_Enable();
    
    write_IMR(0x00);                // Disable everything at the start

    extended_bus_timing = 1;        // Default is to start with extended bus timing
    bus_enabled = 1;                // We start with bus enabled
    
//    IRQ |= 0x10;                    // TX UART DONE (so we can send)
}


/**
 * Simple main execution loop ... we enable interrupts for a short period so that
 * we can process them at the PSoC level, but then run everything else with IRQ's
 * disabled.
 */

uint16_t    pcs[32];
int pcx = 0;


void execute() {
    uint8_t xxx;
    
    CyGlobalIntEnable;
    
    for (;;) {
        if (IMR > 0x80) {
            if (IMR & IRQ & 0x04) {
                CALL_IRQ(2);
            } else if (IMR & IRQ & 0x02) {
                CALL_IRQ(1);
            } else if (IMR & IRQ & 0x08) {
                CALL_IRQ(3);
            } else if (IMR & IRQ & 0x20) {
                CALL_IRQ(5);
            }
        }
        map[code[pc++]]();
    }
}



/* [] END OF FILE */
