#include "mytree.h"
void print_tree(Node mynode, int depth) {
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
    if (mynode->tokenFlag == 0) {
        for (int i = 0; i < mynode->child_num; i++) {
            print_tree(mynode->childs[i], depth + 1);
        }
    }
}
Node token_node(const char* name, enum DATATYPE datatype, const char* val) {
    Node mynode = (Node)malloc(sizeof(struct treenode_));
    mynode->tokenFlag = 1;
    mynode->datatype = datatype;
    sscanf(name, "%s", mynode->name);
    if (val) sscanf(val, "%s", mynode->val);
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
Node nonterminal_node(const char* name, int line, int node_num, ...) {
    Node mynode = (Node)malloc(sizeof(struct treenode_));
    for (int i = 0; i < MAX_CHILD_NODES; i++) {
        mynode->childs[i] = NULL;
    }
    mynode->line = line;
    mynode->tokenFlag = 0;
    mynode->child_num = node_num;
    sscanf(name, "%s", mynode->name);
    va_list valist;
    va_start(valist, node_num);
    for (int i = 0; i < node_num; i++) {
        mynode->childs[i] = va_arg(valist, Node);
    }
    va_end(valist);
    return mynode;
}
Node get_child(Node mynode, int child_idx) {
    assert(child_idx < mynode->child_num);
    return mynode->childs[child_idx];
}