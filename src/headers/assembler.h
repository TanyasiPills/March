#pragma once

#include <MachineTable.h>
#include <bare.h>

typedef struct { uint8_t *buf; size_t len, cap; } Code;

// ---- Reg to Reg ----

void add_rr (Code* c, Reg src, Reg dst);
void sub_rr (Code* c, Reg src, Reg dst);
void and_rr (Code* c, Reg src, Reg dst);
void or_rr  (Code* c, Reg src, Reg dst);
void xor_rr (Code* c, Reg src, Reg dst);
void cmp_rr (Code* c, Reg src, Reg dst);
void mov_rr (Code* c, Reg src, Reg dst);
void imul_rr(Code* c, Reg src, Reg dst);

// ---- Imd to Reg ----

void add_ri(Code* c, int32_t imd, Reg dst);
void sub_ri(Code* c, int32_t imd, Reg dst);
void and_ri(Code* c, int32_t imd, Reg dst);
void or_ri(Code* c, int32_t imd, Reg dst);
void xor_ri(Code* c, int32_t imd, Reg dst);
void cmp_ri(Code* c, int32_t imd, Reg dst);
void mov_ri(Code* c, uint64_t imd, Reg dst);

// ---- With Memory ----

void mov_rm(Code* c, Reg src, Reg base, int32_t disp);
void mov_mr(Code* c, Reg base, int32_t disp, Reg dst);
void lea   (Code* c, Reg base, int32_t disp, Reg dst);
void add_mr(Code* c, Reg base, int32_t disp, Reg dst);
void sub_mr(Code* c, Reg base, int32_t disp, Reg dst);

void push  (Code* c, Reg src);
void pop   (Code* c, Reg dst);

// ---- No Input ----

void cqo(Code* c);
void ret(Code* c);
void syscall(Code* c);

// ---- One Input ----

void neg_r (Code* c, Reg r);
void not_r (Code* c, Reg r);
void idiv_r(Code* c, Reg divider);
void inc_r (Code* c, Reg r);
void dec_r (Code* c, Reg r);

// ---- conditions ----

void setcc(Code* c, uint8_t cc, Reg r);

// ---- Jumps ----

uint64_t jmp(Code* c);
uint64_t jcc(Code* c, uint8_t cc);
void   patch(Code* c, uint64_t pos);

void  jmp_to(Code* c, uint64_t pos);
void  jcc_to(Code* c, uint64_t pos, uint8_t cc);
