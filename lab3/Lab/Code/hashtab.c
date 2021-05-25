#include "hashtab.h"
int var_number = 0;    // 变量编号，便于优化处理
int array_number = 0;  // 数组编号
int func_number = 0;   // 函数编号
extern int translator_struct;
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
    switch (field->type->kind) {
        case BASIC:
            field->id = var_number++;
            break;
        case ARRAY:
            field->id = array_number++;
            break;
        case FUNCTION:
            field->id = func_number++;
            break;
        case STRUCTURE:
            dump_structure_err();
            break;
        case STRUCTTAG:
            break;
        default:
            assert(0);
            break;
    }
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

void dump_structure_err() {
    if (translator_struct) return;
    printf("Cannot translate: Code contains variables or parameters of structure type.");
    exit(-1);
}