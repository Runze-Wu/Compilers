#include "hashtab.h"

#include "symtab.h"

void init_hashtable() {
    for (int i = 0; i < HASHTABLE_SIZE; i++) {
        hashtable[i] = NULL;
    }
    init_symbol_table();
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

FieldList look_up(char* name, bool need_insert) {
    unsigned int pos = hash(name);
    HashNode node = hashtable[pos];
    while (node != NULL) {
        if (strcmp(node->data->name, name) == 0) {
            // insert id non-existence in current stack top
            assert(node->depth <= StackTop->stack_depth);
            if (need_insert && node->depth < StackTop->stack_depth) {
                if (node->data->type->kind == STRUCTTAG) return node->data;
            } else {
                return node->data;
            }
        }
        node = node->link;
    }
    return NULL;
}