#ifndef __HASHTAB_H__
#define __HASHTAB_H__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define HASHTABLE_SIZE 0x3fff
typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
typedef struct HashNode_* HashNode;
HashNode hashtable[HASHTABLE_SIZE];

struct Type_ {
    enum { BASIC, ARRAY, STRUCTURE, STRUCTTAG, FUNCTION } kind;
    union {
        enum { TYPE_INT, TYPE_FLOAT } basic;  // 基本类型unc
        struct {
            Type elem;
            int size;
        } array;              // 数组类型信息包括元素类型与数组大小构成
        FieldList structure;  // 结构体类型信息是一个链表
        FieldList tag;        // 结构类型变量
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
    HashNode link;
};
void init_hashtable();               // initialize the hashtable
unsigned int hash(char* name);       // hash function
void insert_field(FieldList field);  // insert filed to hashtable
int look_up(char* name);             // look up the item, if find return the index
#endif