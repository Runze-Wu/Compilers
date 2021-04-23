#include "symtab.h"

#include "semant.h"
void init_symbol_table() {
    StackTop = (Stack)malloc(sizeof(struct Stack_));
    StackTop->right = NULL;
    StackTop->layer_head = NULL;
    StackTop->stack_depth = 0;
}
void enter_scope() {
    assert(StackTop != NULL);
    Stack new_scope = (Stack)malloc(sizeof(struct Stack_));
    new_scope->right = StackTop->right;
    new_scope->layer_head = NULL;
    new_scope->stack_depth = ++StackTop->stack_depth;
    StackTop->right = new_scope;
}
void exist_scope() {
    assert(StackTop != NULL && StackTop->right != NULL);
    Stack old_scope = StackTop->right;
    StackTop->right = old_scope->right;
    StackTop->stack_depth--;
    while (old_scope->layer_head != NULL) {
        Symbol old_head = old_scope->layer_head;
        HashNode old_entry = hashtable[old_head->entry_idx];
        old_scope->layer_head = old_scope->layer_head->down;
        if (old_entry != NULL) {
            // skip delete hashnode which has struct field
            if (old_entry->data != NULL && old_entry->data->type->kind == STRUCTTAG) continue;
            hashtable[old_head->entry_idx] = hashtable[old_head->entry_idx]->link;
            free(old_entry);
        }
        free(old_head);
    }
    free(old_scope);
}
void insert_hashnode(unsigned int hashnode_idx) {
    assert(StackTop != NULL && StackTop->right != NULL);
    Symbol new_symbol = (Symbol)malloc(sizeof(struct Symbol_));
    new_symbol->entry_idx = hashnode_idx;
    hashtable[hashnode_idx]->depth = StackTop->right->stack_depth;
    new_symbol->down = StackTop->right->layer_head;
    StackTop->right->layer_head = new_symbol;
}