#include "hashtab.h"
int var_number = 0;  // 变量编号，便于优化处理
void init_hashtable() {
    for (int i = 0; i < HASHTABLE_SIZE; i++) {
        hashtable[i] = NULL;
    }
}

unsigned int hash(char* name) {
    unsigned int val = 0, i;
    for (; *name; name++) {
        val = (val << 2) + *name;
        if (i = val & ~HASHTABLE_SIZE) val = (val ^ (i >> 12)) & HASHTABLE_SIZE;
    }
    assert(val <= HASHTABLE_SIZE);
    return val;
}

void insert_field(FieldList field) {
    unsigned int pos = hash(field->name);
    HashNode node = (HashNode)malloc(sizeof(struct HashNode_));
    assert(node != NULL);  // out of memory
    if (field->type->kind == BASIC)
        field->var_id = var_number++;
    else
        field->var_id = -1;
    node->data = field;
    node->link = hashtable[pos];
    hashtable[pos] = node;
}

FieldList look_up(char* name) {
    unsigned int pos = hash(name);
    HashNode node = hashtable[pos];
    while (node != NULL) {
        if (strcmp(node->data->name, name) == 0) {
            return node->data;
        }
        node = node->link;
    }
    return NULL;
}