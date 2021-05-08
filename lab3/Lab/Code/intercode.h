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
InterCodeList ir_list_head;  // 循环双向链表头
int temp_number;             // 临时变量编号
int label_number;            // 跳转编号
struct Operand_ {
    enum { CONSTANT, TEMP, LABEL, VARIABLE, ADDRESS, OP_FUNCTION, OP_ARRAY, OP_STRUCTURE } kind;
    union {
        int const_val;  // 常量值
        int number;     // 临时变量||跳转编号
        char* name;     // 变量名||取地址的变量名||函数名||数组名||结构名
    } u;
};

struct InterCode_ {
    enum {
        DEF_LABEL,  // LABEL x :
        DEF_FUNC,   // FUNCTION f :
        ASSIGN,     // x := y
        IR_ADD,     // x := y+z
        IR_SUB,     // x := y-z
        IR_MUL,     // x := y*z
        IR_DIV,     // x := y/z
        GET_ADDR,   // x := &y
        LOAD,       // x := *y
        STORE,      // *x := y
        GOTO,       // GOTO x
        IF_GOTO,    // IF x [relop] y GOTO z
        IR_RETURN,  // RETURN x
        DEC,        // DEC x [size]
        ARG,        // ARG x
        CALL,       // x := CALL f
        PARAM,      // PARAM x
        READ,       // READ x
        WRITE       // WRITE x
    } kind;
    union {
        struct {
            Operand op;
        } unary_ir;  // DEF_LABEL DEF_FUNC GOTO IR_RETURN ARG PARAM READ WRITE
        struct {
            Operand op;
            int size;
        } dec;  // DEC x [size]
        struct {
            Operand right, left;
        } binary_ir;  // ASSIGN GET_ADDR LOAD STORE CALL
        struct {
            Operand res, op1, op2;
        } ternary_ir;  // IR_ADD IR_SUB IR_MUL IR_DIV
        struct {
            Operand x, y, z;
            char* relop;
        } if_goto;  // IF x [relop] y GOTO z
    } u;
};

struct InterCodeList_ {  // 双向链表存储IR
    InterCode code;
    InterCodeList prev, next;
};

void init_ir_list();                                        // 初始化IR双向链表头
InterCodeList add_ir(InterCodeList ir, InterCodeList ir1);  // 将ir1添加到ir尾部,返回表头
void show_ir_list();                                        // 打印IR链表
void show_ir(InterCode ir);                                 // 打印IR
void show_op(Operand op);                                   // 打印OP
InterCodeList gen_ir(int ir_kind, Operand op1, Operand op2, Operand op3, int dec_size, char* relop);  // 生成IR
Operand gen_operand(int operand_kind, int val, int no, char* name);                                   // 产生Operand
Operand new_temp();   // 产生一个临时变量
Operand new_label();  // 产生一个跳转标记

#endif