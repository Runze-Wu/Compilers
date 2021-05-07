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
InterCodeList ir_list_head;

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
        ADD,        // x := y+z
        SUB,        // x := y-z
        MUL,        // x := y*z
        DIV,        // x := y/z
        GET_ADDR,   // x := &y
        LOAD,       // x := *y
        STORE,      // *x := y
        GOTO,       // GOTO x
        IF_GOTO,    // IF x [relop] y GOTO z
        RETURN,     // RETURN x
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
        } unary_ir;  // DEF_LABEL DEF_FUNC GOTO RETURN ARG PARAM READ WRITE
        struct {
            Operand op;
            int size;
        } dec;  // DEC x [size]
        struct {
            Operand right, left;
        } binary_ir;  // ASSIGN GET_ADDR LOAD STORE CALL
        struct {
            Operand res, op1, op2;
        } ternary_ir;  // ADD SUB MUL DIV
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

void init_ir_list();         // 初始化IR双向链表头
void add_ir(InterCode ir);   // 添加IR到链表尾部
void show_ir_list();         // 打印IR链表
void show_ir(InterCode ir);  // 打印IR
void show_op(Operand op);    // 打印OP
InterCode gen_ir(int ir_kind, Operand op1, Operand op2, Operand op3, int dec_size, char* relop);  // 生成IR
Operand gen_operand(int operand_kind, int val, int no, char* name);                               // 产生Operand
#endif