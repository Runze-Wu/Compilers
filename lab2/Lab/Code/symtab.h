#ifndef __SYMTAB_H__
#define __SYMTAB_H__
#include "hashtab.h"
typedef struct Symbol_* Symbol;
typedef struct Stack_* Stack;
Stack StackTop;
struct Symbol_ {
    unsigned int entry_idx;  // 哈希结点所在的下标
    Symbol down;             // 该层次下一个结点
};

struct Stack_ {
    int stack_depth;
    Symbol layer_head;  // 当前层的哈希结点头指针
    Stack right;        // 上一层次的栈顶指针
};
void init_symbol_table();                         // initialize the symbol table
void enter_scope();                               // enter scope, depth++
void exist_scope();                               // exist scope, depth--
void insert_hashnode(unsigned int hashnode_idx);  // insert hashnode
#endif