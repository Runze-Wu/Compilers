#include "hashtab.h"

#include "symtab.h"

void init_funcdeclist() { funcdeclist = NULL; }

void init_hashtable() {
    for (int i = 0; i < HASHTABLE_SIZE; i++) {
        hashtable[i] = NULL;
    }
    init_symbol_table();
    init_funcdeclist();
}

unsigned int hash(char* name) {
    unsigned int val = 0, i;
    for (; *name; name++) {
        val = (val << 2) + *name;
        if (i = val & ~HASHTABLE_SIZE) val = (val ^ (i >> 12)) & HASHTABLE_SIZE;
    }
    assert(val < HASHTABLE_SIZE);
    return val;
}

void insert_field(FieldList field) {
    unsigned int pos = hash(field->name);
    HashNode node = (HashNode)malloc(sizeof(struct HashNode_));
    assert(node != NULL);  // out of memory
    node->data = field;
    node->link = hashtable[pos];
    hashtable[pos] = node;
    insert_hashnode(pos);
}

void insert_funcdec(char* name, int lineno) {
    FuncDecList funcdec = (FuncDecList)malloc(sizeof(struct FuncDecList_));
    assert(funcdec != NULL);
    funcdec->name = name;
    funcdec->lineno = lineno;
    funcdec->tail = funcdeclist;
    funcdeclist = funcdec;
}

FieldList look_up(char* name, bool need_insert, bool struct_def) {
    unsigned int pos = hash(name);
    HashNode node = hashtable[pos];
    while (node != NULL) {
        if (strcmp(node->data->name, name) == 0) {
            if (node->data->type->kind == STRUCTTAG) {  // struct has global scope
                return node->data;
            }
            if (need_insert && node->depth == StackTop->stack_depth) {  // insert variance in local scope
                return node->data;
            }
            if (!need_insert || struct_def) return node->data;  // return the field
        }
        node = node->link;
    }
    return NULL;
}

bool has_def(char* name) {
    unsigned int pos = hash(name);
    HashNode node = hashtable[pos];
    while (node != NULL) {
        if (strcmp(node->data->name, name) == 0) {
            if (node->data->type->kind == FUNCTION) return true;
        }
        node = node->link;
    }
    return false;
}