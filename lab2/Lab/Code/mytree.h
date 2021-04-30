#ifndef _NODE_H_
#define _NODE_H_
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
enum DATATYPE { TYPE_INT, TYPE_OCT, TYPE_HEX, TYPE_FLOAT, TYPE_ID, TYPE_TYPE, OTHER };
typedef struct treenode_* Node;
#define MAX_CHILD_NODES 10
struct treenode_ {
    int line;                // 非终结符结点对应的行号
    char name[32];           // 每个结点的名字类型
    char val[64];            // BUG --> not enough for float
    int tokenFlag;           // 是否是终结符
    enum DATATYPE datatype;  // 如果是int float ID TYPE要保存其对应的值
    union {                  // 存储对应类型的值
        unsigned val_int;
        float val_float;
        char var_ID[64];
    } data;
    int child_num;
    Node childs[MAX_CHILD_NODES];
};
void print_tree(Node mynode, int depth);                                     // 打印AST，depth表示深度
Node token_node(const char* name, enum DATATYPE datatype, const char* val);  // 添加终结符结点，说明类型和具体值
Node nonterminal_node(const char* name, int line, int node_num, ...);  // 添加非终结符结点，说明名称，以及列值
Node get_child(Node mynode, int child_idx);                            // 按照下标访问子节点
#endif