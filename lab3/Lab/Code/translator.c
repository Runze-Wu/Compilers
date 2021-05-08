#include "translator.h"

int temp_number = 0;   // 临时变量编号
int label_number = 0;  // 跳转编号

void translate_Program(Node root) {
    init_ir_list();
    if (root == NULL) return;
    // Program -> ExtDefList
    assert(root->child_num == 1);
    translate_ExtDefList(get_child(root, 0));
    show_ir_list(ir_list_head);
}

void translate_ExtDefList(Node root) {
    if (root == NULL) return;
    // ExtDefList -> ExtDef ExtDefList
    assert(root->child_num == 2);
    translate_ExtDef(get_child(root, 0));
    translate_ExtDefList(get_child(root, 1));
}

void translate_ExtDef(Node root) {
    if (root == NULL) return;
    assert(root->child_num == 2 || root->child_num == 3);
    if (root->child_num == 3) {
        if (strcmp(get_child(root, 1)->name, "ExtDecList") == 0) {  // ExtDef -> Specifier ExtDecList SEMI
            // 假设4 没有全局变量
        } else if (strcmp(get_child(root, 2)->name, "CompSt") == 0) {  // ExtDef -> Specifier FunDec CompSt
        }
    } else if (root->child_num == 2) {
        if (strcmp(get_child(root, 1)->name, "SEMI") == 0) {  // ExtDef -> Specifier SEMI
        }
    }
}

void translate_VarDec(Node root) {
    if (root == NULL) return;
    assert(root->child_num == 1 || root->child_num == 4);
    if (root->child_num == 1) {         // VarDec -> ID
    } else if (root->child_num == 4) {  // VarDec -> VarDec LB INT RB
    }
}

void translate_FunDec(Node root) {
    if (root == NULL) return;
    assert(root->child_num == 3 || root->child_num == 4);
    if (root->child_num == 3) {         // FunDec -> ID LP RP
    } else if (root->child_num == 4) {  // FunDec -> ID LP VarList RP
    }
}

void translate_VarList(Node root) {
    if (root == NULL) return;
    assert(root->child_num == 1 || root->child_num == 3);
    if (root->child_num == 1) {         // VarList -> ParamDec
    } else if (root->child_num == 3) {  // VarList -> ParamDec COMMA VarList
    }
}

void translate_ParamDec(Node root) {
    if (root == NULL) return;
    // ParamDec -> Specifier VarDec
    assert(root->child_num == 2);
}

void translate_CompSt(Node root) {
    if (root == NULL) return;
    // CompSt -> LC DefList StmtList RC
    assert(root->child_num == 4);
}

void translate_StmtList(Node root) {
    if (root == NULL) return;
    // Stmtlist -> Stmt Stmtlist
    assert(root->child_num == 2);
}

void translate_Stmt(Node root) {
    if (root == NULL) return;
    assert(root->child_num == 1 || root->child_num == 2 || root->child_num == 3 || root->child_num == 5 ||
           root->child_num == 7);
    if (root->child_num == 1) {         // Stmt -> CompSt
    } else if (root->child_num == 2) {  // Stmt -> Exp SEMI
    } else if (root->child_num == 3) {  // Stmt -> RETURN Exp SEMI
    } else if (root->child_num == 5) {
        /**
         * Stmt -> IF LP Exp RP Stmt
         *  Stmt -> WHILE LP Exp RP Stmt
         */
    } else if (root->child_num == 7) {  // Stmt -> IF LP Exp RP Stmt ELSE Stmt
    }
}

void translate_DefList(Node root) {
    if (root == NULL) return;
    // DefList -> Def DefList
    assert(root->child_num == 2);
}

void translate_Def(Node root) {
    if (root == NULL) return;
    // Def -> Specifier DecList SEMI
    assert(root->child_num == 3);
}

void translate_DecList(Node root) {
    if (root == NULL) return;
    assert(root->child_num == 1 || root->child_num == 3);
    if (root->child_num == 1) {         // DecList -> Dec
    } else if (root->child_num == 3) {  // DecList -> Dec COMMA DecList
    }
}

void translate_Dec(Node root) {
    if (root == NULL) return;
    assert(root->child_num == 1 || root->child_num == 3);
    if (root->child_num == 1) {         // Dec -> VarDec
    } else if (root->child_num == 3) {  // VarDec ASSIGNOP Exp
    }
}

void translate_Exp(Node root) {
    if (root == NULL) return;
    assert(root->child_num == 1 || root->child_num == 2 || root->child_num == 3 || root->child_num == 4);
    if (root->child_num == 1) {
        if (strcmp(get_child(root, 0)->name, "ID") == 0) {  // Exp -> ID
        } else {
        }
    } else if (root->child_num == 2) {
        /**
         * Exp -> NOT Exp
         * Exp -> MINUS Exp
         */
        if (strcmp(get_child(root, 0)->name, "NOT") == 0) {
        } else if (strcmp(get_child(root, 0)->name, "MINUS") == 0) {
        }
    } else if (root->child_num == 3) {
        if (strcmp(get_child(root, 0)->name, "LP") == 0) {  // Exp -> LP Exp RP

        } else if (strcmp(get_child(root, 0)->name, "ID") == 0) {  // Exp -> ID LP RP

        } else if (strcmp(get_child(root, 1)->name, "DOT") == 0) {  // Exp -> Exp DOT ID

        } else if (strcmp(get_child(root, 1)->name, "ASSIGNOP") == 0) {  // Exp ASSIGNOP Exp
            Node node_left = get_child(root, 0);
            if ((node_left->child_num == 1 && strcmp(get_child(node_left, 0)->name, "ID") == 0) ||
                (node_left->child_num == 3 && strcmp(get_child(node_left, 1)->name, "DOT") == 0) ||
                (node_left->child_num == 4 && strcmp(get_child(node_left, 0)->name, "Exp") == 0)) {
            }
        } else {
            /**
             * Exp -> Exp AND Exp
             * Exp -> Exp OR Exp
             * Exp -> Exp RELOP Exp
             * Exp -> Exp PLUS Exp
             * Exp -> Exp MINUS Exp
             * Exp -> Exp STAR Exp
             * Exp -> Exp DIV Exp
             */
            if (strcmp(get_child(root, 1)->name, "AND") == 0 || strcmp(get_child(root, 1)->name, "OR") == 0) {
            } else if (strcmp(get_child(root, 1)->name, "RELOP") == 0) {
            }
        }
    } else if (root->child_num == 4) {
        if (strcmp(get_child(root, 0)->name, "ID") == 0) {          // Exp -> ID LP Args RP
        } else if (strcmp(get_child(root, 0)->name, "Exp") == 0) {  // Exp -> Exp LB Exp RB
        }
    }
}

void translate_Args(Node root) {
    if (root == NULL) return;
}

void translate_Cond(Node root, Operand true_label, Operand false_label) {
    if (root == NULL) return;
}