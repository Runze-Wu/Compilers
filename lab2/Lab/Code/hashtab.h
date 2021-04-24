#ifndef __HASHTAB_H__
#define __HASHTAB_H__

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASHTABLE_SIZE 0x3fff
typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
typedef struct HashNode_* HashNode;
typedef struct FuncDecList_* FuncDecList;
HashNode hashtable[HASHTABLE_SIZE];
FuncDecList funcdeclist;
struct Type_ {
    enum { BASIC, ARRAY, STRUCTURE, STRUCTTAG, FUNCTION, FUNCTIONDEC } kind;
    bool need_free;
    bool struct_def_done;
    union {
        enum { NUM_INT, NUM_FLOAT } basic;  // 基本类型
        struct {
            Type elem;
            int size;
        } array;              // 数组类型信息包括元素类型与数组大小构成
        FieldList structure;  // point to a struct definition
        FieldList member;     // struct definition
        struct {
            int argc;  // number of arguments
            FieldList argv;
            Type ret;  // return type
        } function;
    } u;
};

struct FieldList_ {
    char* name;      // 域的名字
    Type type;       // 域的类型
    FieldList tail;  // 下一个域
};

struct HashNode_ {
    FieldList data;
    HashNode link;  // 哈希冲突后的下一个结点
    int depth;      // 结点所在层次
};

struct FuncDecList_ {
    char* name;        // 函数声明的名字
    int lineno;        // 函数声明的行号
    FuncDecList tail;  // 下一个函数名字
};
void init_funcdeclist();                      // initialize the funcdeclist
void init_hashtable();                        // initialize the hashtable
unsigned int hash(char* name);                // hash function
void insert_field(FieldList field);           // insert filed to hashtable
void insert_funcdec(char* name, int lineno);  // insert function declaration name
FieldList look_up(char* name, bool need_insert,
                  bool struct_def);  // look up the item, if insert only look up in current stacktop
bool has_def(char* name);            // find if the declaration has definition
#endif