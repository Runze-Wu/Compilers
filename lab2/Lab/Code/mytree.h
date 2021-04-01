#ifndef _NODE_H_
#define _NODE_H_
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
enum DATATYPE { TYPE_INT, TYPE_OCT, TYPE_HEX, TYPE_FLOAT, TYPE_ID, TYPE_TYPE, OTHER };
struct treenode {
    int line;                // 非终结符结点对应的行号
    char name[32];           // 每个结点的名字类型
    int tokenFlag;           // 是否是终结符
    enum DATATYPE datatype;  // 如果是int float ID TYPE要保存其对应的值
    union {                  // 存储对应类型的值
        unsigned val_int;
        float val_float;
        char var_ID[32];
    } data;
    struct treenode* child;  // 第一个子结点
    struct treenode* bro;    // 第一个兄弟结点
};
void print_tree(struct treenode* mynode, int depth);  // 打印AST，depth表示深度
struct treenode* token_node(const char* name, enum DATATYPE datatype,
                            const char* val);  // 添加终结符结点，说明类型和具体值
struct treenode* nonterminal_node(const char* name, int line, int node_num,
                                  ...);  // 添加非终结符结点，说明名称，以及列值
void set_parent_brother(struct treenode* parent, int node_num,
                        va_list valist);  // 设置父节点和兄弟节点，由于存在节点为空的情况，采用可变参数写法
#endif