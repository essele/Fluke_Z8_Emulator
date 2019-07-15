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

#define ARG_R       (regE0map(ARG_IM))
#define ARG_IR      (reg[ARG_R])
#define ARG_IM      (code[pc++])

// Special macros to work with working registers...
#define ARG_r(n)    (RP|n)
#define ARG_Ir(n)   (reg[ARG_r(n)])

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
    uint16_t rs = ARG_R; \
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
#define func_djnz           uint8_t v = LOAD_VAL_FROM_REG(dst) - 1; \
                            if (v != 0) { \
                                int8_t ra = (int8_t)ARG_IM; \
                                pc += ra; \
                            } else { \
                                pc++; \
                            } \
                            STORE_VAL_IN_REG(dst, v);
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
                            D = 1; \
                            H = !(((d & 0x1f) == 0x0f) && ((new & 0x1f) == 0x10)); \
                            STORE_VAL_IN_REG(dst, new);
// ----------------------------------------------------------------------
#define func_sbc            uint8_t     d = LOAD_VAL_FROM_REG(dst); \
                            uint16_t    new = d - src - C; \
                            \
                            C = ((new & 0x100) == 0x100); \
                            new &= 0xff; \
                            Z = (new == 0); \
                            S = ((new & 0x80) == 0x80); \
                            V = (((d & 0x80) != (src & 0x80)) && ((new & 0x80) == (src & 0x80))); \
                            D = 1; \
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
                            uint8_t new = (v << 1) | ((v & 0x80) == 0x80); \
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
                            uint8_t new = (v & 0x80) | ((v >> 1) & 0x7f);         /* NOT SURE THE & 0x7F is needed?? */ \
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
#define func_swap           uint8_t v = LOAD_VAL_FROM_REG(dst); \
                            v = (v << 4) | (v >> 4); \
                            Z = (v == 0); \
                            S = ((v & 0x80) == 0x80); \
                            STORE_VAL_IN_REG(dst, v);
// ----------------------------------------------------------------------
#define func_call           PUSH16(pc); \
                            pc = da;
// ----------------------------------------------------------------------

/* [] END OF FILE */
