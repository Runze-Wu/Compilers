#ifndef __OBJECTCODE_H__
#define __OBJECTCODE_H__
#include "controlflow.h"
#define REGISTER_NUM 32
typedef struct Register_ Register;
typedef struct Variable_* Variable;

char* reg_names[] = {"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
                     "s0",   "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"};
struct Register_ {
    char* name;                 // 寄存器的别名
    enum { FREE, BUSY } state;  // 寄存器是否被使用
    Variable var;               // 寄存器当前存放的变量
} regs[REGISTER_NUM];

struct Variable_ {
    Operand op;  // 变量对应的操作数
    int offset;  // 变量在栈中的偏移量，相对于fp
    int reg_no;  // 变量所在寄存器编号
};
int local_offset;  // 函数内部变量偏移量

void gencode(FILE* code_out);                    // 输出目标代码入口
void gencode_bb(BasicBlock bb, FILE* code_out);  // 输出BB中的指令
void gencode_ir(InterCode ir, FILE* code_out);   // 输出一条中间指令的MIPS32指令
void gen_ir_LABEL(InterCode ir, FILE* code_out);
void gen_ir_FUNC(InterCode ir, FILE* code_out);
void gen_ir_GOTO(InterCode ir, FILE* code_out);
void gen_ir_RETURN(InterCode ir, FILE* code_out);
void gen_ir_ARG(InterCode ir, FILE* code_out);
void gen_ir_PARAM(InterCode ir, FILE* code_out);
void gen_ir_READ(InterCode ir, FILE* code_out);
void gen_ir_WRITE(InterCode ir, FILE* code_out);
void gen_ir_DEC(InterCode ir, FILE* code_out);
void gen_ir_ASSIGN(InterCode ir, FILE* code_out);
void gen_ir_ADDR(InterCode ir, FILE* code_out);
void gen_ir_LOAD(InterCode ir, FILE* code_out);
void gen_ir_STORE(InterCode ir, FILE* code_out);
void gen_ir_CALL(InterCode ir, FILE* code_out);
void gen_ir_ADD(InterCode ir, FILE* code_out);
void gen_ir_SUB(InterCode ir, FILE* code_out);
void gen_ir_MUL(InterCode ir, FILE* code_out);
void gen_ir_DIV(InterCode ir, FILE* code_out);

void init_registers();
int get_regs(Operand op);

#endif