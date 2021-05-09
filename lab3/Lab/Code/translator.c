#include "translator.h"

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
            translate_FunDec(root);
            translate_CompSt(root);
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
    if (root->child_num == 1) {  // Stmt -> CompSt
        translate_CompSt(get_child(root, 0));
    } else if (root->child_num == 2) {  // Stmt -> Exp SEMI
        translate_Exp(get_child(root, 0), NULL);
    } else if (root->child_num == 3) {  // Stmt -> RETURN Exp SEMI
        Operand t1 = new_temp();
        translate_Exp(root, t1);
        // RETURN t1
        gen_ir(IR_RETURN, t1, NULL, NULL, -1, NULL);
    } else if (root->child_num == 5) {
        if (strcmp(get_child(root, 0)->name, "IF") == 0) {  // Stmt -> IF LP Exp RP Stmt
            Operand label1 = new_label();
            Operand label2 = new_label();
            translate_Cond(get_child(root, 2), label1, label2);
            // LABEL label1
            gen_ir(LABEL, label1, NULL, NULL, -1, NULL);
            translate_Stmt(get_child(root, 4));
            // LABEL label2
            gen_ir(LABEL, label2, NULL, NULL, -1, NULL);
        } else if (strcmp(get_child(root, 0)->name, "WHILE") == 0) {  // Stmt -> WHILE LP Exp RP Stmt
            Operand label1 = new_label();
            Operand label2 = new_label();
            Operand label3 = new_label();
            // LABEL label1
            gen_ir(LABEL, label1, NULL, NULL, -1, NULL);
            translate_Cond(get_child(root, 2), label2, label3);
            // LABEL label2
            gen_ir(LABEL, label2, NULL, NULL, -1, NULL);
            translate_Stmt(get_child(root, 4));
            // GOTO label1
            gen_ir(GOTO, label1, NULL, NULL, -1, NULL);
            // LABEL label3
            gen_ir(LABEL, label3, NULL, NULL, -1, NULL);
        }
    } else if (root->child_num == 7) {  // Stmt -> IF LP Exp RP Stmt ELSE Stmt
        Operand label1 = new_label();
        Operand label2 = new_label();
        Operand label3 = new_label();
        translate_Cond(get_child(root, 2), label1, label2);
        // LABEL label1
        translate_Stmt(get_child(root, 4));
        // GOTO label3
        gen_ir(GOTO, label3, NULL, NULL, -1, NULL);
        // LABEL label2
        gen_ir(LABEL, label2, NULL, NULL, -1, NULL);
        translate_Stmt(get_child(root, 6));
        // LABEL label3
        gen_ir(LABEL, label3, NULL, NULL, -1, NULL);
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

void translate_Exp(Node root, Operand place) {
    if (root == NULL) return;
    assert(root->child_num == 1 || root->child_num == 2 || root->child_num == 3 || root->child_num == 4);
    if ((root->child_num == 2 && strcmp(get_child(root, 0)->name, "NOT") == 0) ||
        (root->child_num == 3 &&
         (strcmp(get_child(root, 1)->name, "AND") == 0 || strcmp(get_child(root, 1)->name, "OR") == 0 ||
          strcmp(get_child(root, 1)->name, "RELOP") == 0))) {
        /**
         * Exp -> NOT Exp
         * Exp -> Exp RELOP Exp
         * Exp -> Exp AND Exp
         * Exp -> Exp OR Exp
         */
        Operand label1 = new_label();
        Operand label2 = new_label();
        // place := #0
        Operand const_op = gen_operand(CONSTANT, 0, -1, NULL);
        gen_ir(ASSIGN, place, const_op, NULL, -1, NULL);
        translate_Cond(root, label1, label2);
        // LABEL label1
        gen_ir(LABEL, label1, NULL, NULL, -1, NULL);
        // place := #1
        const_op->u.const_val = 1;
        gen_ir(ASSIGN, place, const_op, NULL, -1, NULL);
        // LABEL label2
        gen_ir(LABEL, label2, NULL, NULL, -1, NULL);
    } else if (root->child_num == 1) {
        if (strcmp(get_child(root, 0)->name, "ID") == 0) {  // Exp -> ID
            FieldList result = look_up(get_child(root, 0)->val);
            assert(result != NULL);
            if (result->type->kind == BASIC) {
                // place := variable.name
                Operand var_op = gen_operand(VARIABLE, -1, -1, get_child(root, 0)->val);
                gen_ir(ASSIGN, place, var_op, NULL, -1, NULL);
            } else if (result->type->kind == ARRAY) {
                // ID type : ARRAY TODO
            } else if (result->type->kind == STRUCTURE) {  // 假设2 不存在结构体变量
            }
        } else if (strcmp(get_child(root, 0)->name, "INT") == 0) {  // Exp -> INT
            // place := #value
            Operand const_op = gen_operand(CONSTANT, get_child(root, 0)->data.val_int, -1, NULL);
            gen_ir(ASSIGN, place, const_op, NULL, -1, NULL);
        } else {  // 假设1 不存在浮点型常量
            assert(0);
        }
    } else if (root->child_num == 2) {
        if (strcmp(get_child(root, 0)->name, "MINUS") == 0) {  // Exp -> MINUS Exp
            Operand t1 = new_temp();
            translate_Exp(get_child(root, 1), t1);
            // place := #0 - t1
            Operand const_op = gen_operand(CONSTANT, 0, -1, NULL);
            gen_ir(IR_SUB, place, const_op, t1, -1, NULL);
        }
    } else if (root->child_num == 3) {
        if (strcmp(get_child(root, 0)->name, "LP") == 0) {  // Exp -> LP Exp RP
            translate_Exp(get_child(root, 1), place);
        } else if (strcmp(get_child(root, 0)->name, "ID") == 0) {  // Exp -> ID LP RP
            FieldList function = look_up(get_child(root, 0)->val);
            assert(function != NULL);
            if (function->name == "read") {  // READ place
                gen_ir(READ, place, NULL, NULL, -1, NULL);
            } else {
                // place := CALL function.name
                Operand func_op = gen_operand(FUNCTION, -1, -1, function->name);
                gen_ir(CALL, place, func_op, NULL, -1, NULL);
            }
        } else if (strcmp(get_child(root, 1)->name, "DOT") == 0) {  // Exp -> Exp DOT ID
            dump_structure_err();
        } else if (strcmp(get_child(root, 1)->name, "ASSIGNOP") == 0) {  // Exp ASSIGNOP Exp
            Node node_left = get_child(root, 0);
            if (node_left->child_num == 1 && strcmp(get_child(node_left, 0)->name, "ID") == 0) {  // Exp1 -> ID
                FieldList result = look_up(get_child(node_left, 0)->val);
                assert(result != NULL);
                if (result->type->kind == BASIC) {
                    Operand t1 = new_temp();
                    translate_Exp(root, t1);
                    // variable.name := t1
                    Operand var_op = gen_operand(VARIABLE, -1, -1, get_child(root, 0)->val);
                    gen_ir(ASSIGN, var_op, t1, NULL, -1, NULL);
                    // place := variable.name
                    gen_ir(ASSIGN, place, var_op, NULL, -1, NULL);
                } else if (result->type->kind == ARRAY) {
                    // 数组互相赋值 TODO
                } else if (result->type->kind == STRUCTURE) {  // 假设2 不存在结构体变量
                    dump_structure_err();
                }
            } else if (node_left->child_num == 3 &&
                       strcmp(get_child(node_left, 1)->name, "DOT") == 0) {  // 假设2 不存在结构体变量
                dump_structure_err();
            } else if (node_left->child_num == 4 &&
                       strcmp(get_child(node_left, 0)->name, "Exp") == 0) {  // Exp1 -> Exp2 LB Exp3 RB
            }
        } else {
            /**
             * Exp -> Exp PLUS Exp
             * Exp -> Exp MINUS Exp
             * Exp -> Exp STAR Exp
             * Exp -> Exp DIV Exp
             */
            Operand t1 = new_temp();
            Operand t2 = new_temp();
            translate_Exp(get_child(root, 0), t1);
            translate_Exp(get_child(root, 2), t2);
            int ir_kind = -1;
            if (strcmp(get_child(root, 1)->name, "PLUS") == 0) {
                ir_kind = IR_ADD;
            } else if (strcmp(get_child(root, 1)->name, "MINUS") == 0) {
                ir_kind = IR_SUB;
            } else if (strcmp(get_child(root, 1)->name, "STAR") == 0) {
                ir_kind = IR_MUL;
            } else if (strcmp(get_child(root, 1)->name, "DIV") == 0) {
                ir_kind = IR_DIV;
            }
            // place := t1 op t2
            gen_ir(ir_kind, place, t1, t2, -1, NULL);
        }
    } else if (root->child_num == 4) {
        if (strcmp(get_child(root, 0)->name, "ID") == 0) {  // Exp -> ID LP Args RP
            FieldList function = look_up(get_child(root, 0)->val);
            assert(function != NULL);
        } else if (strcmp(get_child(root, 0)->name, "Exp") == 0) {  // Exp -> Exp LB Exp RB
        }
    }
}

void translate_Args(Node root) {
    if (root == NULL) return;
}

void translate_Cond(Node root, Operand label_true, Operand label_false) {
    if (root == NULL) return;
    assert(root->child_num == 1 || root->child_num == 2 || root->child_num == 3 || root->child_num == 4);
    if (strcmp(get_child(root, 0)->name, "NOT") == 0) {  // NOT Exp
        translate_Cond(root, label_false, label_true);
    } else if (strcmp(get_child(root, 1)->name, "AND") == 0) {  // Exp AND Exp
        Operand label1 = new_label();
        translate_Cond(get_child(root, 0), label1, label_false);
        gen_ir(LABEL, label1, NULL, NULL, -1, NULL);
        translate_Cond(get_child(root, 2), label_true, label_false);
    } else if (strcmp(get_child(root, 1)->name, "OR") == 0) {  // Exp OR Exp
        Operand label1 = new_label();
        translate_Cond(get_child(root, 0), label_true, label1);
        gen_ir(LABEL, label1, NULL, NULL, -1, NULL);
        translate_Cond(get_child(root, 2), label_true, label_false);
    } else if (strcmp(get_child(root, 1)->name, "RELOP") == 0) {  // Exp RELOP Exp
        Operand t1 = new_temp();
        Operand t2 = new_temp();
        char* relop = get_child(root, 1)->val;
        translate_Exp(get_child(root, 0), t1);
        translate_Exp(get_child(root, 2), t2);
        // IF t1 op t2 GOTO label_true
        gen_ir(IF_GOTO, t1, t2, label_true, -1, relop);
        // GOTO label_false
        gen_ir(GOTO, label_false, NULL, NULL, -1, NULL);
    } else {
        Operand t1 = new_temp();
        translate_Exp(root, t1);
        // IF t1 != #0 GOTO label_true
        gen_ir(IF_GOTO, t1, gen_operand(TEMP, 0, -1, NULL), label_true, -1, "!=");
        // GOTO label_false
        gen_ir(GOTO, label_false, NULL, NULL, -1, NULL);
    }
}

void dump_structure_err() {
    printf("Cannot translate: Code contains variables or parameters of structure type.");
    exit(-1);
}