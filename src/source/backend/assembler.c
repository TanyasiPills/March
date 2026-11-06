#include <assembler.h>

static void emit8(Code *c, uint8_t b) {
    if (c->len == c->cap) {
        c->cap = c->cap ? c->cap * 2 : 256;
        c->buf = realloc(c->buf, c->cap);
    }
    c->buf[c->len++] = b;
}

static void emit32(Code *c, uint32_t v) { 
	for (int i = 0; i < 4; i++) 
		emit8(c, (v >> (i * 8)) & 0xFF); 
}
static void emit64(Code *c, uint64_t v) { 
	for (int i = 0; i < 8; i++) 
		emit8(c, (v >> (i * 8)) & 0xFF); 
}

static void emit_rr(Code *c, uint8_t op, Reg reg, Reg rm) {
    emit8(c, REX_BASE | (REG_EXT(reg) << 2) | REG_EXT(rm));
    emit8(c, op);
    emit8(c, MM_REG | (REG_LOW(reg) << 3) | REG_LOW(rm));
}

// REX.W + opcode + ModRM + [SIB] + [disp].  Memory operand is [base + disp].
static void emit_rm(Code *c, uint8_t op, Reg reg, Reg base, int32_t disp) {
    emit8(c, REX_BASE | (REG_EXT(reg) << 2) | REG_EXT(base));
    emit8(c, op);

    int mod;
    if (disp == 0 && REG_LOW(base) != 5) mod = MM_MEM;           // rbp/r13 can't use mod=00
    else if (disp >= -128 && disp <= 127) mod = MM_MEM_OFS_S;
    else                                  mod = MM_MEM_OFS_L;

    emit8(c, (mod) | (REG_LOW(reg) << 3) | REG_LOW(base));

    if (REG_LOW(base) == 4) emit8(c, SIB(base, SIB_NO_INDEX, SIB_X1));                 // rsp/r12 need SIB (no index)

    if (mod == 1) emit8(c, (uint8_t)disp);
    if (mod == 2) emit32(c, (uint32_t)disp);
}

// Immediate group: 83 /n imm8  or  81 /n imm32
static void emit_ri(Code *c, uint8_t ext, Reg rm, int32_t imm) {
    emit8(c, REX_BASE | REG_EXT(rm));
    if (imm >= -128 && imm <= 127) {
        emit8(c, IMM8);
        emit8(c, MM_REG | (ext << 3) | REG_LOW(rm));
        emit8(c, (uint8_t)imm);
    } else {
        emit8(c, IMM32);
        emit8(c, MM_REG | (ext << 3) | REG_LOW(rm));
        emit32(c, (uint32_t)imm);
    }
}

// One-operand group (F7 /n, FF /n)
static void emit_ext(Code *c, uint8_t op, uint8_t ext, Reg rm) {
    emit8(c, REX_BASE | REG_EXT(rm));
    emit8(c, op);
    emit8(c, MM_REG | (ext << 3) | REG_LOW(rm));
}

// ---- Reg to Reg ----

void add_rr (Code* c, Reg src, Reg dst) {
	emit_rr(c, ADD_RTR, src, dst);
}
void sub_rr (Code* c, Reg src, Reg dst) {
	emit_rr(c, SUB_RTR, src, dst);
}
void and_rr (Code* c, Reg src, Reg dst) {
	emit_rr(c, AND_RTR, src, dst);
}
void or_rr  (Code* c, Reg src, Reg dst) {
	emit_rr(c, OR_RTR, src, dst);
}
void xor_rr (Code* c, Reg src, Reg dst) {
	emit_rr(c, XOR_RTR, src, dst);
}
void cmp_rr (Code* c, Reg src, Reg dst) {
	emit_rr(c, CMP_RTR, src, dst);
}
void mov_rr (Code* c, Reg src, Reg dst) {
	emit_rr(c, MOV_RTR, src, dst);
}
void imul_rr(Code* c, Reg src, Reg dst) {
	emit8(c, REX_BASE | (REG_EXT(dst) << 2) | (REG_EXT(src)));
	emit8(c, TWO_BYTE);
	emit8(c, IMUL_2);
	emit8(c, ModRM(MM_REG, REG_LOW(dst), REG_LOW(src)));
}

// ---- Imd to Reg ----

void add_ri(Code* c, int32_t imd, Reg dst) {
	emit_ri(c, EXT_ADD, dst, imd);
}
void sub_ri(Code* c, int32_t imd, Reg dst) {
	emit_ri(c, EXT_SUB, dst, imd);
}
void and_ri(Code* c, int32_t imd, Reg dst) {
	emit_ri(c, EXT_AND, dst, imd);
}
void or_ri(Code* c, int32_t imd, Reg dst) {
	emit_ri(c, EXT_OR, dst, imd);
}
void xor_ri(Code* c, int32_t imd, Reg dst) {
	emit_ri(c, EXT_XOR, dst, imd);
}
void cmp_ri(Code* c, int32_t imd, Reg dst) {
	emit_ri(c, EXT_CMP, dst, imd);
}
void mov_ri(Code* c, uint64_t imd, Reg dst) {
	emit8(c, REX_BASE | REG_EXT(dst));
	emit8(c, MOV_RI64 + REG_LOW(dst));
	emit64(c, imd);
}

// ---- With Memory ----

void mov_rm(Code* c, Reg src, Reg base, int32_t disp){
	emit_rm(c, MOV_RTM, src, base, disp);
}
void mov_mr(Code* c, Reg base, int32_t disp, Reg dst) {
	emit_rm(c, MOV_MTR, dst, base, disp);
}
void lea   (Code* c, Reg base, int32_t disp, Reg dst) {
	emit_rm(c, LEA, dst, base, disp);
}
void add_mr(Code* c, Reg base, int32_t disp, Reg dst) {
	emit_rm(c, ADD_MTR, dst, base, disp);
}
void sub_mr(Code* c, Reg base, int32_t disp, Reg dst) {
	emit_rm(c, SUB_MTR, dst, base, disp);
}

void push  (Code* c, Reg src) {
	if(src >= 8) emit8(c, REX | REX_B);
	emit8(c, PUSH + src);
}
void pop   (Code* c, Reg dst) {
	if(dst >= 8) emit8(c, REX | REX_B);
	emit8(c, POP + dst);
}

// ---- No Input ----

void cqo(Code* c) {
	emit8(c, REX_BASE);
	emit8(c, CQO);
}
void ret(Code* c) {
	emit8(c, RET);
}
void syscall(Code* c) {
	emit8(c, TWO_BYTE);
	emit8(c, SYSCALL_2);
}

// ---- One Input ----

void neg_r (Code* c, Reg r) {
	emit_ext(c, GRP_F7, EXT_NEG, r);
}
void not_r (Code* c, Reg r) {
	emit_ext(c, GRP_F7, EXT_NOT, r);
}
void idiv_r(Code* c, Reg divider) {
	emit_ext(c, GRP_F7, EXT_IDIV, divider);

}
void inc_r (Code* c, Reg r) {
	emit_ext(c, GRP_FF, EXT_INC, r);
}
void dec_r (Code* c, Reg r) {
	emit_ext(c, GRP_FF, EXT_DEC, r);
}

// ---- conditions ----

void setcc(Code* c, uint8_t cc, Reg r) {
	emit8(c, REX | REG_EXT(r));
	emit8(c, TWO_BYTE);
	emit8(c, SETCC_2 + cc);
	emit8(c, ModRM(MM_REG, 0x00, REG_LOW(r)));
}

// ---- Jumps ----

uint64_t jmp(Code* c) {
	emit8(c, JMP_REL32);
	emit32(c, 0);
	uint64_t p = c->len;
	return p;
}
uint64_t jcc(Code* c, uint8_t cc) {
	emit8(c, TWO_BYTE);
	emit8(c, JCC_REL32_2 + cc);
	emit32(c, 0);
	uint64_t p = c->len;
	return p;
}
void   patch(Code* c, uint64_t pos) {
	int32_t rel = (int32_t)(c->len - pos);
	for (int i = 0; i < 4; i++) {
		c->buf[pos - 4 + i] = (rel >> (i*8)) & 0xFF;
	}
}

void  jmp_to(Code* c, uint64_t pos) {
	emit8(c, JMP_REL32);
	int32_t rel = (int32_t)((int64_t)pos - (int64_t)(c->len+4));
	emit32(c, (uint32_t)rel);
}
void  jcc_to(Code* c, uint64_t pos, uint8_t cc) {
	emit8(c, TWO_BYTE);
	emit8(c, JCC_REL32_2 + cc);
	int32_t rel = (int32_t)((int64_t)pos - (int64_t)(c->len+4));
	emit32(c, (uint32_t)rel);
}
