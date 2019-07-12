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

#define BKPT        asm("BKPT");

extern uint8_t      code[];
extern uint16_t     pc;
extern uint8_t      reg[];

extern int          C, Z, S, V, D, H;
extern int          interrupts_disabled;
extern int          extended_bus_timing;

extern uint8_t      (*reg_read[256])();
extern void         (*reg_write[256])(uint8_t);

extern void         write_IMR(uint8_t val);
extern void         write_FLAGS(uint8_t val);

extern void         (*map[256])();



extern void setup_emulator();
extern void execute();

/* [] END OF FILE */
