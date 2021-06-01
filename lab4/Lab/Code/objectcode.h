#ifndef __OBJECTCODE_H__
#define __OBJECTCODE_H__
#include "controlflow.h"
#include "intercode.h"
#include "translator.h"
#define REGISTER_NUM 32
typedef struct Register_ Register;
typedef struct Variable_* Variable;
typedef struct VariableList_* VariableList;

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
struct VariableList_ {
    Variable var;
    VariableList next;
};
int local_offset;            // 函数内部变量偏移量
int arg_num;                 // 函数调用传递的参数
int param_num;               // 函数形式参数
VariableList local_varlist;  // 函数内部变量链表

void gencode(FILE* code_out);                          // 输出目标代码入口
void init_environment(FILE* code_out);                 // 初始化数据段，read、write两个内置函数
void initsymbol_fb(FunctionBlock fb, FILE* code_out);  // 初始化FB中的符号表
void initsymbol_bb(BasicBlock bb, FILE* code_out);     // 初始化BB中的符号表
void initsymbol_ir(InterCode ir, FILE* code_out);      // 初始化一条IR中的符号
void gencode_fb(FunctionBlock fb, FILE* code_out);     // 输出FB中的指令
void gencode_bb(BasicBlock bb, FILE* code_out);        // 输出BB中的指令
void gencode_ir(InterCode ir, FILE* code_out);         // 输出一条中间指令的MIPS32指令
void gen_ir_LABEL(InterCode ir, FILE* code_out);
void gen_ir_FUNC(InterCode ir, FILE* code_out);
void gen_ir_GOTO(InterCode ir, FILE* code_out);
void gen_ir_RETURN(InterCode ir, FILE* code_out);
void gen_ir_ARG(InterCode ir, FILE* code_out);
void gen_ir_PARAM(InterCode ir, FILE* code_out);
void gen_ir_READ(InterCode ir, FILE* code_out);
void gen_ir_WRITE(InterCode ir, FILE* code_out);
void gen_ir_DEC(InterCode ir, FILE* code_out);
void gen_ir_ASSIGN_ADDR(InterCode ir, FILE* code_out);
void gen_ir_LOAD_STORE(InterCode ir, FILE* code_out);
void gen_ir_CALL(InterCode ir, FILE* code_out);
void gen_ir_ARITH(InterCode ir, FILE* code_out);  // 算术运算
void gen_ir_IF_GOTO(InterCode ir, FILE* code_out);

void init_registers();                               // 初始化寄存器
void init_varlist();                                 // 初始化字符表
void release_varlist();                              // 释放旧字符表
void show_varlist();                                 // 展示字符表
void insert_var(Variable var);                       // 插入变量
Variable find_var(Operand op);                       // 查找变量是否在当前变量链表中
void insert_op(Operand op);                          // 将op对应的变量加入符号表中
int get_reg(Operand op, bool left, FILE* code_out);  // 为操作数选择寄存器
void store_reg(int reg_no, FILE* code_out);          // 将寄存器保存变量存回内存
void clear_reg(int reg_no);                          // 将寄存器置为空闲
#endif