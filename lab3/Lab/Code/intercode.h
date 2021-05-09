#ifndef __INTERCODE_H__
#define __INTERCODE_H__

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Operand_* Operand;
typedef struct InterCode_* InterCode;
typedef struct InterCodeList_* InterCodeList;
typedef struct ArgList_* ArgList;
InterCodeList ir_list_head;  // 循环双向链表头
unsigned int temp_number;    // 临时变量编号
unsigned int label_number;   // 跳转编号
struct Operand_ {
    enum { OP_CONSTANT, OP_TEMP, OP_LABEL, OP_VARIABLE, OP_ADDRESS, OP_FUNCTION, OP_ARRAY, OP_STRUCTURE } kind;
    union {
        unsigned int const_val;  // 常量值
        unsigned int number;     // 临时变量||跳转编号
        char* name;              // 变量名||取地址的变量名||函数名||数组名||结构名
    } u;
};

struct InterCode_ {
    enum {
        IR_LABEL,    // LABEL x :
        IR_FUNC,     // FUNCTION f :
        IR_ASSIGN,   // x := y
        IR_ADD,      // x := y+z
        IR_SUB,      // x := y-z
        IR_MUL,      // x := y*z
        IR_DIV,      // x := y/z
        IR_ADDR,     // x := &y
        IR_LOAD,     // x := *y
        IR_STORE,    // *x := y
        IR_GOTO,     // GOTO x
        IR_IF_GOTO,  // IF x [relop] y GOTO z
        IR_RETURN,   // RETURN x
        IR_DEC,      // DEC x [size]
        IR_ARG,      // ARG x
        IR_CALL,     // x := CALL f
        IR_PARAM,    // PARAM x
        IR_READ,     // READ x
        IR_WRITE     // WRITE x
    } kind;
    union {
        struct {
            Operand op;
        } unary_ir;  // IR_LABEL IR_FUNC IR_GOTO IR_RETURN IR_ARG IR_PARAM IR_READ IR_WRITE
        struct {
            Operand op;
            int size;
        } dec;  // IR_DEC x [size]
        struct {
            Operand right, left;
        } binary_ir;  // IR_ASSIGN IR_ADDR IR_LOAD IR_STORE IR_CALL
        struct {
            Operand res, op1, op2;
        } ternary_ir;  // IR_ADD IR_SUB IR_MUL IR_DIV
        struct {
            Operand x, y, z;
            char relop[64];
        } if_goto;  // IR_IF_GOTO
    } u;
};

struct InterCodeList_ {  // 双向链表存储IR
    InterCode code;
    InterCodeList prev, next;
};

struct ArgList_ {  // 参数链表
    Operand arg;
    ArgList next;
};

void init_ir_list();                       // 初始化IR双向链表头
void add_ir(InterCode ir);                 // 将ir1添加到ir尾部,返回表头
void show_ir_list(FILE* ir_out);           // 打印IR链表
void show_ir(InterCode ir, FILE* ir_out);  // 打印IR
void show_op(Operand op, FILE* ir_out);    // 打印OP
void gen_ir(int ir_kind, Operand op1, Operand op2, Operand op3, int dec_size, char* relop);  // 生成IR
Operand gen_operand(int operand_kind, int val, int no, char* name);                          // 产生Operand
Operand new_temp();                                                                          // 产生一个临时变量
Operand new_label();                                                                         // 产生一个跳转标记
ArgList add_arg(ArgList head, Operand arg);                                                  // 将arg设为链表head
#endif