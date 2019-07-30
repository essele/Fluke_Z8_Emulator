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
#include "Z8_instructions.h"


// TODO: these should be part of the component, but it needs a rework to
//       include the control registers etc in the actual component.
extern uint8_t bus_read(uint16_t addr, int dm, int ex);
extern void bus_write(uint16_t addr, uint8_t data, int dm);


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
//    if ((r & 0xf0) == 0xe0) BKPT;
    if (reg_read[r]) {
        return reg_read[r]();
    } else {
        return reg[r];
    }
}
static inline void STORE_VAL_IN_REG(uint8_t r, uint8_t val) {
//    if ((r & 0xf0) == 0xe0) BKPT;
    if (reg_write[r]) {
        reg_write[r](val);
    } else {
        reg[r] = val;
    }
}

/**
 * If we execute an illegal opcode, just bring some values into local scope 
 * so they are easy to see in the debugger, and then cause a breakpoint.
 */
#pragma GCC push_options
#pragma GCC optimize ("O0")
void ILLEGAL() {
    uint16_t    from_pc = pc-1;
    uint8_t     failed_opcode __attribute__((unused)) = code[from_pc];
    
    BKPT;
}
void MEM_WRITE_FAIL(uint16_t __attribute__((unused)) addr, uint8_t __attribute__((unused)) dm) {
    uint16_t    __attribute__((unused)) pc_after = pc;

    BKPT;
}
#pragma GCC pop_options



/**
 * Functions to read and write memory...
 */
// ----------------------------------------------------------------------
void LDC_read() {   // OPCODE C2
    uint8_t tmp = ARG_IM;
    uint8_t dst = ARG_r(tmp >> 4);
    uint8_t src = ARG_r((tmp & 0x0f));
    uint16_t addr = LOAD_VAL_FROM_REGPAIR(src);

    if (addr < 0x2000) {
        STORE_VAL_IN_REG(dst, code[addr]);
    } else {
        // dm=1 (means not dm .. i.e. code)
        STORE_VAL_IN_REG(dst, bus_read(addr, 1, extended_bus_timing));
    }
}
// ----------------------------------------------------------------------
void LDCI_read() {  // OPCODE C3
    uint8_t tmp = ARG_IM;
    uint8_t dst = ARG_r(tmp >> 4);
    uint8_t src = ARG_r((tmp & 0x0f));
    uint16_t addr = LOAD_VAL_FROM_REGPAIR(src);
    uint8_t idst = reg[dst];
   
    if (addr < 0x2000) {
       STORE_VAL_IN_REG(idst, code[addr]);
    } else {
        // dm=1 (means not dm .. i.e. code)
        STORE_VAL_IN_REG(idst, bus_read(addr, 1, extended_bus_timing));
    } 

    // Incremement...
    STORE_VAL_IN_REG(dst, idst + 1); 
    STORE_VAL_IN_REGPAIR(src, (addr + 1));
}
// ----------------------------------------------------------------------
void LDC_write() {      // OPCODE D2
    uint8_t tmp = ARG_IM;
    uint8_t src = ARG_r(tmp >> 4);
    uint8_t dst = ARG_r((tmp & 0x0f));
    uint16_t addr = LOAD_VAL_FROM_REGPAIR(dst);
    
    if (addr < 0x2000) {
        MEM_WRITE_FAIL(addr, 1);
    } else {
        bus_write(addr, LOAD_VAL_FROM_REG(src), 1);
    }
}
// ----------------------------------------------------------------------
void LDCI_write() {     // OPCODE D3
    uint8_t tmp = ARG_IM;
    uint8_t src = ARG_r(tmp >> 4);
    uint8_t dst = ARG_r((tmp & 0x0f));
    uint16_t addr = LOAD_VAL_FROM_REGPAIR(src);
    uint8_t isrc = reg[src];

    if (addr < 0x2000) {
        MEM_WRITE_FAIL(addr, 1);
    } else {
        bus_write(addr, LOAD_VAL_FROM_REG(isrc), 1);
    }
    
    // Increment
    STORE_VAL_IN_REG(src, isrc + 1);
    STORE_VAL_IN_REGPAIR(dst, (addr + 1));
}
// ----------------------------------------------------------------------
void LDE_read() {   // OPCODE 82
    uint8_t tmp = ARG_IM;
    uint8_t dst = ARG_r(tmp >> 4);
    uint8_t src = ARG_r((tmp & 0x0f));
    uint16_t addr = LOAD_VAL_FROM_REGPAIR(src);

    // dm=0 (means dm .. i.e. data)
    STORE_VAL_IN_REG(dst, bus_read(addr, 0, extended_bus_timing));
}
// ----------------------------------------------------------------------
void LDEI_read() {  // OPCODE 83
    uint8_t tmp = ARG_IM;
    uint8_t dst = ARG_r(tmp >> 4);
    uint8_t src = ARG_r((tmp & 0x0f));
    uint16_t addr = LOAD_VAL_FROM_REGPAIR(src);
    uint8_t idst = reg[dst];
   
    // dm=0 (means dm .. i.e. data)
    STORE_VAL_IN_REG(idst, bus_read(addr, 0, extended_bus_timing));

    // Incremement...
    STORE_VAL_IN_REG(dst, idst + 1); 
    STORE_VAL_IN_REGPAIR(src, (addr + 1));
}
// ----------------------------------------------------------------------
void LDE_write() {      // OPCODE 92
    uint8_t tmp = ARG_IM;
    uint8_t src = ARG_r(tmp >> 4);
    uint8_t dst = ARG_r((tmp & 0x0f));
    uint16_t addr = LOAD_VAL_FROM_REGPAIR(dst);
    
    bus_write(addr, LOAD_VAL_FROM_REG(src), 0);
}
// ----------------------------------------------------------------------
void LDEI_write() {     // OPCODE 93
    uint8_t tmp = ARG_IM;
    uint8_t src = ARG_r(tmp >> 4);
    uint8_t dst = ARG_r((tmp & 0x0f));
    uint16_t addr = LOAD_VAL_FROM_REGPAIR(src);
    uint8_t isrc = reg[src];

    bus_write(addr, LOAD_VAL_FROM_REG(isrc), 0);
    
    // Increment
    STORE_VAL_IN_REG(src, isrc + 1);
    STORE_VAL_IN_REGPAIR(dst, (addr + 1));
}


/**
 * Function definitions for non-arg opcodes
 */
//void DI() { write_IMR(IMR & 0x7f); }
void DI() { IMR &= 0x7f; }
// ----------------------------------------------------------------------
//void EI() { write_IMR(IMR | 0x80); }
void EI() { IMR |= 0x80; }
// ----------------------------------------------------------------------
void RET() { 
    pc = (reg[SPL] << 8) | reg[SPL+1]; 
//    if (pc > 0x2000) BKPT;
    SPL += 2; 
}
// ----------------------------------------------------------------------
void IRET() { 
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
// TODO: probably need to benchmark this, just guessing at the moment
void NOP() { CyDelayCycles((6 * 4 * 6) - 8); }
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
OP(DECW, RR1, decw); OP(DECW, IR1, decw);
// 90 - 97
OP(RL, R1, rl); OP(RL, IR1, rl); 
// A0 - A7
OP(INCW, RR1, incw); OP(INCW, IR1, incw); OP(CP, r1_r2, cp); OP(CP, r1_Ir2, cp);
OP(CP, R2_R1, cp); OP(CP, IR2_R1, cp); OP(CP, R1_IM, cp); OP(CP, IR1_IM, cp);
// B0 - B7
OP(CLR, R1, clr); OP(CLR, IR1, clr); OP(XOR, r1_r2, xor); OP(XOR, r1_Ir2, xor);
OP(XOR, R2_R1, xor); OP(XOR, IR2_R1, xor); OP(XOR, R1_IM, xor); OP(XOR, IR1_IM, xor);
// C0 - C7
OP(RRC, R1, rrc); OP(RRC, IR1, rrc); 
// D0 - D7
OP(SRA, R1, sra); OP(SRA, IR1, sra);
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
    DECW_RR1, DECW_IR1, LDE_read, LDEI_read, ILLEGAL, ILLEGAL, ILLEGAL, ILLEGAL,
        LD_r1_R2_8, LD_r2_R1_8, DJNZ_r1_RA_8, JR_cc_RA_8, LD_r1_IM_8, JP_cc_DA_8, INC_r1_8, DI,
// 90
    RL_R1, RL_IR1, LDE_write, LDEI_write, ILLEGAL, ILLEGAL, ILLEGAL, ILLEGAL,
        LD_r1_R2_9, LD_r2_R1_9, DJNZ_r1_RA_9, JR_cc_RA_9, LD_r1_IM_9, JP_cc_DA_9, INC_r1_9, EI,
// A0
    INCW_RR1, INCW_IR1, CP_r1_r2, CP_r1_Ir2, CP_R2_R1, CP_IR2_R1, CP_R1_IM, CP_IR1_IM, 
        LD_r1_R2_10, LD_r2_R1_10, DJNZ_r1_RA_10, JR_cc_RA_10, LD_r1_IM_10, JP_cc_DA_10, INC_r1_10, RET,
// B0
    CLR_R1, CLR_IR1, XOR_r1_r2, XOR_r1_Ir2, XOR_R2_R1, XOR_IR2_R1, XOR_R1_IM, XOR_IR1_IM, 
        LD_r1_R2_11, LD_r2_R1_11, DJNZ_r1_RA_11, JR_cc_RA_11, LD_r1_IM_11, JP_cc_DA_11, INC_r1_11, IRET,
// C0
    RRC_R1, RRC_IR1, LDC_read, LDCI_read, ILLEGAL, ILLEGAL, ILLEGAL, /* LD indexed */ ILLEGAL,
        LD_r1_R2_12, LD_r2_R1_12, DJNZ_r1_RA_12, JR_cc_RA_12, LD_r1_IM_12, JP_cc_DA_12, INC_r1_12, RCF,
// D0
    SRA_R1, SRA_IR1, LDC_write, LDCI_write, CALL_IRR1, ILLEGAL, CALL_DA, /* LD indexed */ ILLEGAL,
        LD_r1_R2_13, LD_r2_R1_13, DJNZ_r1_RA_13, JR_cc_RA_13, LD_r1_IM_13, JP_cc_DA_13, INC_r1_13, SCF,
// E0
    RR_R1, RR_IR1, ILLEGAL, LD_r1_Ir2, LD_R2_R1, LD_IR2_R1, LD_R1_IM, LD_IR1_IM,
        LD_r1_R2_14, LD_r2_R1_14, DJNZ_r1_RA_14, JR_cc_RA_14, LD_r1_IM_14, JP_cc_DA_14, INC_r1_14, CCF,
// F0
    SWAP_R1, SWAP_IR1, ILLEGAL, LD_Ir1_r2, ILLEGAL, LD_R2_IR1, ILLEGAL, ILLEGAL,
        LD_r1_R2_15, LD_r2_R1_15, DJNZ_r1_RA_15, JR_cc_RA_15, LD_r1_IM_15, JP_cc_DA_15, INC_r1_15, NOP,
};


/* [] END OF FILE */
