#include "mytree.h"
void print_tree(struct treenode* mynode, int depth) {
    if (mynode == NULL) return;
    for (int i = 0; i < depth; i++) printf("  ");
    printf("%s", mynode->name);
    if (mynode->tokenFlag) {
        switch (mynode->datatype) {
            case TYPE_ID:
                printf(": %s", mynode->data.var_ID);
                break;
            case TYPE_TYPE:
                printf(": %s", mynode->data.var_ID);
                break;
            case TYPE_FLOAT:
                printf(": %.6f", mynode->data.val_float);
                break;
            case TYPE_INT:
                printf(": %u", mynode->data.val_int);
                break;
            default:
                break;
        }
    } else {
        printf(" (%d)", mynode->line);
    }
    printf("\n");
    print_tree(mynode->child, depth + 1);
    print_tree(mynode->bro, depth);
}
struct treenode* token_node(const char* name, enum DATATYPE datatype, const char* val) {
    struct treenode* mynode = (struct treenode*)malloc(sizeof(struct treenode));
    mynode->tokenFlag = 1;
    mynode->bro = NULL;
    mynode->child = NULL;
    mynode->datatype = datatype;
    sscanf(name, "%s", mynode->name);
    switch (mynode->datatype) {
        case TYPE_ID:
            sscanf(val, "%s", mynode->data.var_ID);
            break;
        case TYPE_TYPE:
            sscanf(val, "%s", mynode->data.var_ID);
            break;
        case TYPE_FLOAT:
            sscanf(val, "%f", &mynode->data.val_float);
            break;
        case TYPE_INT:
            sscanf(val, "%u", &mynode->data.val_int);
            break;
        case TYPE_OCT:
            sscanf(val, "%o", &mynode->data.val_int);
            mynode->datatype = TYPE_INT;
            break;
        case TYPE_HEX:
            sscanf(val, "%x", &mynode->data.val_int);
            mynode->datatype = TYPE_INT;
            break;
        default:
            break;
    }
    return mynode;
}
struct treenode* nonterminal_node(const char* name, int line, int node_num, ...) {
    struct treenode* mynode = (struct treenode*)malloc(sizeof(struct treenode));
    mynode->bro = NULL;
    mynode->child = NULL;
    mynode->line = line;
    mynode->tokenFlag = 0;
    sscanf(name, "%s", mynode->name);
    va_list valist;
    va_start(valist, node_num);
    set_parent_brother(mynode, node_num, valist);
    va_end(valist);
    return mynode;
}
void set_parent_brother(struct treenode* parent, int node_num, va_list valist) {
    int i = 0;
    struct treenode* node;
    for (; i < node_num; i++) {
        node = va_arg(valist, struct treenode*);
        if (node != NULL) break;
    }
    ++i;
    parent->child = node;
    for (; i < node_num; i++) {
        if ((node->bro = va_arg(valist, struct treenode*)) != NULL) node = node->bro;
    }
}