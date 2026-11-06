#pragma once

// instruction:
// [prefix] [REX] [opcode] [ModRM] [SIB] [displacement] [immediate]
//
// Prefix	0-1 byte	Modifies the instruction (e.g. F2 makes it a double float op)
//
// REX	0-1 byte	Says “64-bit” and gives extra bits for r8-r15
//
// Opcode	1-3 bytes	What operation (89 = mov, 01 = add, ...)
//
// ModRM	0-1 byte	Which operands (registers / memory)
//
// SIB	0-1 byte	Extra info for fancy memory addresses
//
// Displacement	0, 1, 4 bytes	A number added to a memory address (the +8 in [rbp+8])
//
// Immediate	0, 1, 2, 4, 8 bytes	A constant inside the instruction (the 42 in mov rax, 42)


// Memory Modes

#define MM_REG			0xC0
#define MM_MEM			0x00
#define MM_MEM_OFS_S 	0x40 // [mem + disp8]	-> -128 to 127
#define MM_MEM_OFS_L 	0x80 // [mem + disp32]	-> -2^31 to 2^31-1

// Registers

typedef enum {
    RAX, RCX, RDX, RBX, RSP, RBP, RSI, RDI,
    R8,  R9,  R10, R11, R12, R13, R14, R15
} Reg;

#define REG_LOW(r)   ((r) & 7)
#define REG_EXT(r)   ((r) >> 3)

// ModRM
//  bit: 7 6 | 5 4 3 | 2 1 0
//		 mod |  reg  |  rm

#define ModRM(mod, reg, rm) (((mod) << 6) | (REG_LOW(reg) << 3) | REG_LOW(rm))

// SIB
// its like array indexing: [base + index * scale + disp]
//  bit: 7 6 | 5 4 3 | 2 1 0
//     scale | index | base
//  base & index are registers
//  it needs to be not in MM_REG caus it needs disp
//  and it need rm as x04

// so its like a filler when rm is 100b so it can work with disp or just with the reg itself

#define SIB_X1			0x00
#define SIB_X2			0x01
#define SIB_X4			0x02
#define SIB_X8			0x03

#define SIB_NO_INDEX	0x04

#define SIB(base,index,scale)	(((scale) << 6) | ((index) << 3) | (base))

// REX
// bits: 0100WRXB
// W: is 64bits 
// R: ModRM reg is r8-r15
// X: SIB INDEX register is r8-r15
// B: ModRM rm or SIB BASE register is r8-r15

#define REX_W			0x08
#define REX_R			0x04
#define REX_X			0x02
#define REX_B			0x01
#define REX				0x40
#define REX_BASE		0x48 // caus its always 64bit

// ---- Moves ----

#define MOV_RTM        0x89   // mov r/m64, r64
#define MOV_RTR        0x89
#define MOV_MTR        0x8B   // mov r64, r/m64
#define MOV_RI64       0xB8   // mov r64, imm64   (+ reg number added to opcode, no ModRM)
#define MOV_MI32       0xC7   // mov r/m64, imm32 (/0)
#define LEA            0x8D   // lea r64, [mem]
#define MOVSXD         0x63   // movsxd r64, r/m32
// movzx r64, r/m8 = 0F B6, movsx r64, r/m8 = 0F BE  (two-byte opcodes)
#define MOVZX8_2       0xB6   // after 0x0F
#define MOVSX8_2       0xBE   // after 0x0F

// ---- Stack ----
#define PUSH           0x50   // + reg low 3 bits (REX.B for r8-r15)
#define POP            0x58   // + reg low 3 bits

// ---- Arithmetic ----

#define ADD_RTM        0x01
#define ADD_RTR        0x01
#define ADD_MTR        0x03
#define SUB_RTM        0x29
#define SUB_RTR        0x29
#define SUB_MTR        0x2B
#define AND_RTM        0x21
#define AND_RTR        0x21
#define AND_MTR        0x23
#define OR_RTM         0x09
#define OR_RTR         0x09
#define OR_MTR         0x0B
#define XOR_RTM        0x31
#define XOR_RTR        0x31
#define XOR_MTR        0x33
#define CMP_RTM        0x39
#define CMP_RTR        0x39
#define CMP_MTR        0x3B
#define TEST_RTM       0x85
#define TEST_RTR       0x85

// ---- Jumps / calls ----

#define JMP_REL8       0xEB
#define JMP_REL32      0xE9
#define CALL_REL32     0xE8
#define JCC_REL8       0x70   // + cc
#define JCC_REL32_2    0x80   // after 0x0F, + cc
#define SETCC_2        0x90   // after 0x0F, + cc
#define CMOVCC_2       0x40   // after 0x0F, + cc
#define TWO_BYTE       0x0F   // escape byte for two-byte opcodes
#define SYSCALL_2      0x05   // 0F 05

// ---- Condition ----

#define OVERFLOW		0x00
#define NO_OVERFLOW		0x01
#define U_LOWER			0x02
#define U_BIGOREQU		0x03
#define EQUAL			0x04
#define NO_EQUAL		0x05
#define U_LOWOREQU		0x06
#define U_BIGGER		0x07
#define SIGN			0x08
#define NO_SIGN			0x09
#define PARITY			0x0A
#define NO_PARITY		0x0B
#define S_LOWER			0x0C
#define S_BIGOREQU		0x0D
#define S_LOWOREQU		0x0E
#define S_BIGGER		0x0F

// ---- Immediate group: 81 /n imm32  or  83 /n imm8 ----

#define IMM32          0x81
#define IMM8           0x83
#define EXT_ADD        0x00      // the /n values, placed in ModRM.reg
#define EXT_OR         0x01
#define EXT_AND        0x04
#define EXT_SUB        0x05
#define EXT_XOR        0x06
#define EXT_CMP        0x07

#define GRP_F7         0xF7
#define EXT_NOT        0x02
#define EXT_NEG        0x03
#define EXT_MUL        0x04      // rdx:rax = rax * r/m
#define EXT_IMUL1      0x05
#define EXT_DIV        0x06
#define EXT_IDIV       0x07

// ---- inc/dec: FF /0, FF /1 ----

#define GRP_FF         0xFF
#define EXT_INC        0
#define EXT_DEC        1
#define EXT_CALL_IND   2      // call r/m64
#define EXT_JMP_IND    4      // jmp r/m64

// ---- Multiply (two-byte) ----

#define IMUL_2         0xAF   // 0F AF /r : imul r64, r/m64

// ---- Shifts: D3 /n (by cl), C1 /n imm8 ----

#define SHIFT_CL       0xD3
#define SHIFT_IMM      0xC1
#define EXT_SHL        4
#define EXT_SHR        5
#define EXT_SAR        7

// ---- Misc ----

#define CQO            0x99   // with REX.W: 48 99
#define NOP            0x90
#define RET            0xC3
#define LEAVE          0xC9
#define INT3           0xCC

// ---- Prefixes ----

#define PFX_SD 0xF2          // scalar double
#define PFX_SS 0xF3          // scalar float
#define PFX_66 0x66
