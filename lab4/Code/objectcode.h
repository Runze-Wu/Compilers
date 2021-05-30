#ifndef __OBJECTCODE_H__
#define __OBJECTCODE_H__
#include "controlflow.h"
#include "intercode.h"
#define REGISTER_NUM 32
typedef struct Register_ Register;
typedef struct Variable_* Variable;
Register regs[REGISTER_NUM];
char* reg_names[] = {"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
                     "s0",   "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"};

struct Register_ {
    char* name;
    enum { FREE, BUSY } state;
    Variable var;
};

struct Variable_ {
    Operand op;       // 变量对应的操作数
    int addr_offset;  // 变量在内存中的偏移量
    int reg_no;       // 变量所在寄存器编号
};

void init_registers();
int get_regs();

#endif