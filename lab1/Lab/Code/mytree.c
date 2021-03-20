#include "mytree.h"
void print_tree(struct treenode* mynode, int depth) {
    if (mynode == NULL) return;
    for (int i = 0; i <= depth; i++) printf(" ");
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
                printf(": %d", mynode->data.val_int);
                break;
            default:
                break;
        }
    } else {
        printf(" (%d)", mynode->column);
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
            sscanf(val, "%d", &mynode->data.val_int);
            break;
        default:
            break;
    }
    return mynode;
}
/*
int main() {
    Node* node1 = add_token_node("ID", TYPE_ID, "abc");
    Node* node2 = add_token_node("INT", TYPE_INT, "123");
    Node* node3 = add_token_node("FLOAT", TYPE_FLOAT, "123.3");
    print_tree(node1, -1);
    print_tree(node2, -1);
    print_tree(node3, -1);
    return 0;
}
*/
