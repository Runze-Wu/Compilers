#include "translator.h"

extern int translator_debug;
extern int temp_number;                    // 临时变量编号
extern int label_number;                   // 跳转编号
extern InterCodeList global_ir_list_head;  // 循环双向链表头

void translate_Program(Node root) {
    global_ir_list_head = init_ir_list();
    assert(global_ir_list_head != NULL);
    if (root == NULL) return;
    dump_translator_node(root, "Program");
    // Program -> ExtDefList
    assert(root->child_num == 1);
    translate_ExtDefList(get_child(root, 0));
}

void translate_ExtDefList(Node root) {
    if (root == NULL) return;
    dump_translator_node(root, "ExtDefList");
    // ExtDefList -> ExtDef ExtDefList
    assert(root->child_num == 2);
    translate_ExtDef(get_child(root, 0));
    translate_ExtDefList(get_child(root, 1));
}

void translate_ExtDef(Node root) {
    if (root == NULL) return;
    dump_translator_node(root, "ExtDef");
    assert(root->child_num == 2 || root->child_num == 3);
    if (root->child_num == 3) {
        if (strcmp(get_child(root, 1)->name, "ExtDecList") == 0) {  // ExtDef -> Specifier ExtDecList SEMI
            // 假设4 没有全局变量
        } else if (strcmp(get_child(root, 2)->name, "CompSt") == 0) {  // ExtDef -> Specifier FunDec CompSt
            translate_FunDec(get_child(root, 1));
            translate_CompSt(get_child(root, 2));
        }
    } else if (root->child_num == 2) {
        if (strcmp(get_child(root, 1)->name, "SEMI") == 0) {  // ExtDef -> Specifier SEMI
        }
    }
}

Operand translate_VarDec(Node root) {
    if (root == NULL) return NULL;
    dump_translator_node(root, "VarDec");
    assert(root->child_num == 1 || root->child_num == 4);
    Operand name_op = NULL;
    if (root->child_num == 1) {  // VarDec -> ID
        char* var_name = get_child(root, 0)->val;
        FieldList res = look_up(var_name);
        assert(res != NULL);
        if (res->type->kind == ARRAY) {  // 数组变量定义
            name_op = gen_operand(OP_ARRAY, -1, -1, var_name);
            name_op->type = res->type->u.array.elem;
            name_op->size = res->type->u.array.size;
            int dec_size = get_size(res->type);
            // DEC variable.name size
            gen_ir(global_ir_list_head, IR_DEC, name_op, NULL, NULL, dec_size, NULL);
        } else if (res->type->kind == BASIC) {  // 基础类型变量
            name_op = gen_operand(OP_VARIABLE, -1, -1, var_name);
        } else if (res->type->kind == STRUCTURE) {  // 不应出现结构体变量
            dump_structure_err();
        } else {  // 不应出现函数或者结构体定义类型
            assert(0);
        }
    } else if (root->child_num == 4) {  // VarDec -> VarDec LB INT RB
        return translate_VarDec(get_child(root, 0));
    }
    return name_op;
}

void translate_FunDec(Node root) {
    if (root == NULL) return;
    dump_translator_node(root, "FunDec");
    assert(root->child_num == 3 || root->child_num == 4);
    char* func_name = get_child(root, 0)->val;
    FieldList func_field = look_up(func_name);
    assert(func_field != NULL);
    // FUNCTION func.name
    Operand func_op = gen_operand(OP_FUNCTION, -1, -1, func_name);
    gen_ir(global_ir_list_head, IR_FUNC, func_op, NULL, NULL, -1, NULL);
    if (root->child_num == 3) {         // FunDec -> ID LP RP
    } else if (root->child_num == 4) {  // FunDec -> ID LP VarList RP
        FieldList arg_field = func_field->type->u.function.argv;
        while (arg_field) {
            // PARAM arg.name
            Operand arg_op;
            switch (arg_field->type->kind) {
                case BASIC:  // 参数是普通变量
                    arg_op = gen_operand(OP_VARIABLE, -1, -1, arg_field->name);
                    break;
                case ARRAY:
                    arg_op = gen_operand(OP_ARRAY, -1, -1, arg_field->name);
                    break;
                case STRUCTURE:  // 假设不存在结构变量
                    dump_structure_err();
                    break;
                default:
                    break;
            }
            gen_ir(global_ir_list_head, IR_PARAM, arg_op, NULL, NULL, -1, NULL);
            arg_field = arg_field->tail;
        }
    }
}

void translate_CompSt(Node root) {
    if (root == NULL) return;
    dump_translator_node(root, "CompSt");
    // CompSt -> LC DefList StmtList RC
    assert(root->child_num == 4);
    translate_DefList(get_child(root, 1));
    translate_StmtList(get_child(root, 2));
}

void translate_StmtList(Node root) {
    if (root == NULL) return;
    dump_translator_node(root, "StmtList");
    // Stmtlist -> Stmt Stmtlist
    assert(root->child_num == 2);
    translate_Stmt(get_child(root, 0));
    translate_StmtList(get_child(root, 1));
}

void translate_Stmt(Node root) {
    if (root == NULL) return;
    dump_translator_node(root, "Stmt");
    assert(root->child_num == 1 || root->child_num == 2 || root->child_num == 3 || root->child_num == 5 ||
           root->child_num == 7);
    if (root->child_num == 1) {  // Stmt -> CompSt
        translate_CompSt(get_child(root, 0));
    } else if (root->child_num == 2) {  // Stmt -> Exp SEMI
        translate_Exp(get_child(root, 0), new_temp());
    } else if (root->child_num == 3) {  // Stmt -> RETURN Exp SEMI
        Operand t1 = new_temp();
        translate_Exp(get_child(root, 1), t1);
        t1 = load_value(t1);
        // RETURN t1
        gen_ir(global_ir_list_head, IR_RETURN, t1, NULL, NULL, -1, NULL);
    } else if (root->child_num == 5) {
        if (strcmp(get_child(root, 0)->name, "IF") == 0) {  // Stmt -> IF LP Exp RP Stmt
            Operand label1 = new_label();
            Operand label2 = new_label();
            translate_Cond(get_child(root, 2), label1, label2);
            // LABEL label1
            gen_ir(global_ir_list_head, IR_LABEL, label1, NULL, NULL, -1, NULL);
            translate_Stmt(get_child(root, 4));
            // LABEL label2
            gen_ir(global_ir_list_head, IR_LABEL, label2, NULL, NULL, -1, NULL);
        } else if (strcmp(get_child(root, 0)->name, "WHILE") == 0) {  // Stmt -> WHILE LP Exp RP Stmt
            Operand label1 = new_label();
            Operand label2 = new_label();
            Operand label3 = new_label();
            // LABEL label1
            gen_ir(global_ir_list_head, IR_LABEL, label1, NULL, NULL, -1, NULL);
            translate_Cond(get_child(root, 2), label2, label3);
            // LABEL label2
            gen_ir(global_ir_list_head, IR_LABEL, label2, NULL, NULL, -1, NULL);
            translate_Stmt(get_child(root, 4));
            // GOTO label1
            gen_ir(global_ir_list_head, IR_GOTO, label1, NULL, NULL, -1, NULL);
            // LABEL label3
            gen_ir(global_ir_list_head, IR_LABEL, label3, NULL, NULL, -1, NULL);
        }
    } else if (root->child_num == 7) {  // Stmt -> IF LP Exp RP Stmt ELSE Stmt
        Operand label1 = new_label();
        Operand label2 = new_label();
        Operand label3 = new_label();
        // print_tree(get_child(root, 2), 0);
        translate_Cond(get_child(root, 2), label1, label2);
        // LABEL label1
        gen_ir(global_ir_list_head, IR_LABEL, label1, NULL, NULL, -1, NULL);
        translate_Stmt(get_child(root, 4));
        // GOTO label3
        gen_ir(global_ir_list_head, IR_GOTO, label3, NULL, NULL, -1, NULL);
        // LABEL label2
        gen_ir(global_ir_list_head, IR_LABEL, label2, NULL, NULL, -1, NULL);
        translate_Stmt(get_child(root, 6));
        // LABEL label3
        gen_ir(global_ir_list_head, IR_LABEL, label3, NULL, NULL, -1, NULL);
    }
}

void translate_DefList(Node root) {
    if (root == NULL) return;
    dump_translator_node(root, "DefList");
    // DefList -> Def DefList
    assert(root->child_num == 2);
    translate_Def(get_child(root, 0));
    translate_DefList(get_child(root, 1));
}

void translate_Def(Node root) {
    if (root == NULL) return;
    dump_translator_node(root, "Def");
    // Def -> Specifier DecList SEMI
    assert(root->child_num == 3);
    translate_DecList(get_child(root, 1));
}

void translate_DecList(Node root) {
    if (root == NULL) return;
    dump_translator_node(root, "DecList");
    assert(root->child_num == 1 || root->child_num == 3);
    translate_Dec(get_child(root, 0));
    if (root->child_num == 1) {         // DecList -> Dec
    } else if (root->child_num == 3) {  // DecList -> Dec COMMA DecList
        translate_DecList(get_child(root, 2));
    }
}

void translate_Dec(Node root) {
    if (root == NULL) return;
    dump_translator_node(root, "Dec");
    assert(root->child_num == 1 || root->child_num == 3);
    Operand name_op = translate_VarDec(get_child(root, 0));
    if (root->child_num == 1) {         // Dec -> VarDec
    } else if (root->child_num == 3) {  // VarDec ASSIGNOP Exp
        Operand t1 = new_temp();
        translate_Exp(get_child(root, 2), t1);
        if (name_op->kind == OP_ARRAY) {  // 给数组初始化
            array_deep_copy(name_op, t1);
        } else if (name_op->kind == OP_VARIABLE) {  // 基础类型变量初始化
            t1 = load_value(t1);
            // variable.name := t1
            gen_ir(global_ir_list_head, IR_ASSIGN, name_op, t1, NULL, -1, NULL);
        }
    }
}

void translate_Exp(Node root, Operand place) {
    if (root == NULL) return;
    dump_translator_node(root, "Exp");
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
        gen_ir(global_ir_list_head, IR_ASSIGN, place, gen_operand(OP_CONSTANT, 0, -1, NULL), NULL, -1, NULL);
        translate_Cond(root, label1, label2);
        // LABEL label1
        gen_ir(global_ir_list_head, IR_LABEL, label1, NULL, NULL, -1, NULL);
        // place := #1
        gen_ir(global_ir_list_head, IR_ASSIGN, place, gen_operand(OP_CONSTANT, 1, -1, NULL), NULL, -1, NULL);
        // LABEL label2
        gen_ir(global_ir_list_head, IR_LABEL, label2, NULL, NULL, -1, NULL);
    } else if (root->child_num == 1) {
        // 优化： 不再生成 t := v,而是将t改成v
        if (strcmp(get_child(root, 0)->name, "ID") == 0) {  // Exp -> ID
            FieldList result = look_up(get_child(root, 0)->val);
            assert(result != NULL);
            if (result->type->kind == BASIC) {
                // place := variable.name
                place->kind = OP_VARIABLE;
                place->u.var_no = result->id;
            } else if (result->type->kind == ARRAY) {
                // ID type : ARRAY TODO
                Operand array = gen_operand(OP_ARRAY, -1, -1, result->name);
                // 是否是函数的形式参数，此时ID代表的是其数组地址
                if (result->arg == true) {
                    place->kind = OP_ADDRESS;
                    place->u.addr_no = addr_number++;
                    // place := array
                    gen_ir(global_ir_list_head, IR_ASSIGN, place, array, NULL, -1, NULL);
                } else {
                    place->kind = OP_ARRAY;
                    place->u.array_no = result->id;
                }
                place->type = result->type->u.array.elem;
                place->size = result->type->u.array.size;
            } else if (result->type->kind == STRUCTURE) {  // 假设2 不存在结构体变量
                dump_structure_err();
            }
        } else if (strcmp(get_child(root, 0)->name, "INT") == 0) {  // Exp -> INT
            // place := #value
            place->kind = OP_CONSTANT;
            place->u.const_val = get_child(root, 0)->data.val_int;
        } else {  // 假设1 不存在浮点型常量
            assert(0);
        }
    } else if (root->child_num == 2) {
        if (strcmp(get_child(root, 0)->name, "MINUS") == 0) {  // Exp -> MINUS Exp
            Operand t1 = new_temp();
            translate_Exp(get_child(root, 1), t1);
            t1 = load_value(t1);
            if (t1->kind == OP_CONSTANT) {
                place->kind = OP_CONSTANT;
                place->u.const_val = -1 * t1->u.const_val;
            } else {  // place := #0 - t1
                gen_ir(global_ir_list_head, IR_SUB, place, gen_operand(OP_CONSTANT, 0, -1, NULL), t1, -1, NULL);
            }
        }
    } else if (root->child_num == 3) {
        if (strcmp(get_child(root, 0)->name, "LP") == 0) {  // Exp -> LP Exp RP
            translate_Exp(get_child(root, 1), place);
        } else if (strcmp(get_child(root, 0)->name, "ID") == 0) {  // Exp -> ID LP RP
            FieldList function = look_up(get_child(root, 0)->val);
            assert(function != NULL);
            if (strcmp(function->name, "read") == 0) {
                // READ place
                gen_ir(global_ir_list_head, IR_READ, place, NULL, NULL, -1, NULL);
            } else {
                // place := CALL function.name
                Operand func_op = gen_operand(OP_FUNCTION, -1, -1, function->name);
                gen_ir(global_ir_list_head, IR_CALL, place, func_op, NULL, -1, NULL);
            }
        } else if (strcmp(get_child(root, 1)->name, "DOT") == 0) {  // Exp -> Exp DOT ID
            dump_structure_err();
        } else if (strcmp(get_child(root, 1)->name, "ASSIGNOP") == 0) {  // Exp ASSIGNOP Exp
            Operand op_left = new_temp();
            translate_Exp(get_child(root, 0), op_left);
            Operand op_right = new_temp();
            translate_Exp(get_child(root, 2), op_right);
            if (op_left->kind == OP_ADDRESS || op_left->kind == OP_ARRAY) {
                // 左值为数组或者地址 arr_a=arr_b||*addr_a=val_b||*addr_a=*addr_b;
                if (op_right->kind == OP_ADDRESS || op_right->kind == OP_ARRAY) {  // 数组赋值
                    array_deep_copy(op_left, op_right);
                } else {  // 单值写地址
                    assert(op_left->kind == OP_ADDRESS);
                    // *op_left := op_right
                    gen_ir(global_ir_list_head, IR_STORE, op_left, op_right, NULL, -1, NULL);
                }
            } else {  // 左值为普通变量
                op_right = load_value(op_right);
                // op_left := op_right
                gen_ir(global_ir_list_head, IR_ASSIGN, op_left, op_right, NULL, -1, NULL);
            }
            //优化：不再生成place:=op_right，而是直接将右值赋给返回值
            // place := op_right
            place->kind = op_right->kind;
            place->u = op_right->u;
        } else {
            /**
             * Exp -> Exp PLUS Exp
             * Exp -> Exp MINUS Exp
             * Exp -> Exp STAR Exp
             * Exp -> Exp DIV Exp
             */
            Operand t1 = new_temp();
            translate_Exp(get_child(root, 0), t1);
            t1 = load_value(t1);
            Operand t2 = new_temp();
            translate_Exp(get_child(root, 2), t2);
            t2 = load_value(t2);
            int ir_kind = -1;
            long long val;
            if (strcmp(get_child(root, 1)->name, "PLUS") == 0) {
                ir_kind = IR_ADD;
                val = t1->u.const_val + t2->u.const_val;
            } else if (strcmp(get_child(root, 1)->name, "MINUS") == 0) {
                ir_kind = IR_SUB;
                val = t1->u.const_val - t2->u.const_val;
            } else if (strcmp(get_child(root, 1)->name, "STAR") == 0) {
                ir_kind = IR_MUL;
                val = t1->u.const_val * t2->u.const_val;
            } else if (strcmp(get_child(root, 1)->name, "DIV") == 0) {
                ir_kind = IR_DIV;
                // 除零溢出和向下取整
                long long lhsVal = t1->u.const_val;
                long long rhsVal = t2->u.const_val;
                if (lhsVal < 0 && rhsVal > 0) {
                    val = (lhsVal - rhsVal + 1) / rhsVal;
                } else if (lhsVal > 0 && rhsVal < 0) {
                    val = (lhsVal - rhsVal - 1) / rhsVal;
                } else {
                    val = rhsVal ? lhsVal / rhsVal : 0;
                }
            } else {
                assert(0);
            }
            if (t1->kind == OP_CONSTANT && t2->kind == OP_CONSTANT) {
                // 常量合并
                place->kind = OP_CONSTANT;
                place->u.const_val = val;
            } else {
                // place := t1 op t2
                gen_ir(global_ir_list_head, ir_kind, place, t1, t2, -1, NULL);
            }
        }
    } else if (root->child_num == 4) {
        if (strcmp(get_child(root, 0)->name, "ID") == 0) {  // Exp -> ID LP Args RP
            FieldList function = look_up(get_child(root, 0)->val);
            assert(function != NULL);
            if (strcmp(function->name, "write") == 0) {
                translate_Args(get_child(root, 2), true);
                // place := #0
                place->kind = OP_CONSTANT;
                place->u.const_val = 0;
            } else {
                translate_Args(get_child(root, 2), false);
                // place := CALL function.name
                Operand func_op = gen_operand(OP_FUNCTION, -1, -1, function->name);
                gen_ir(global_ir_list_head, IR_CALL, place, func_op, NULL, -1, NULL);
            }
        } else if (strcmp(get_child(root, 0)->name, "Exp") == 0) {  // Exp -> Exp LB Exp RB
            Operand t1 = new_temp();
            translate_Exp(get_child(root, 0), t1);
            assert(t1->kind == OP_ARRAY || t1->kind == OP_ADDRESS);
            Operand t2 = new_temp();
            translate_Exp(get_child(root, 2), t2);
            t2 = load_value(t2);
            Operand offset = new_temp();
            int width = get_size(t1->type);
            if (t2->kind == OP_CONSTANT) {
                offset->kind = OP_CONSTANT;
                offset->u.const_val = width * t2->u.const_val;
            } else {
                // offset :=  t2 * width
                gen_ir(global_ir_list_head, IR_MUL, offset, t2, gen_operand(OP_CONSTANT, width, -1, NULL), -1, NULL);
            }

            // 将place设置为ADDRESS类型，名字为临时变量编号
            place->kind = OP_ADDRESS;
            place->u.addr_no = addr_number++;

            if (t1->kind == OP_ARRAY) {  // Exp1-> ID
                if (offset->kind == OP_CONSTANT && offset->u.const_val == 0) {
                    gen_ir(global_ir_list_head, IR_ADDR, place, t1, NULL, -1, NULL);
                } else {
                    Operand base = new_addr();
                    // base := &addr
                    gen_ir(global_ir_list_head, IR_ADDR, base, t1, NULL, -1, NULL);
                    // place := base + offset
                    gen_ir(global_ir_list_head, IR_ADD, place, base, offset, -1, NULL);
                }
            } else if (t1->kind == OP_ADDRESS) {  // Exp1 -> Exp LB Exp RB
                // place := t1 + offset
                gen_ir(global_ir_list_head, IR_ADD, place, t1, offset, -1, NULL);
            } else {
                assert(0);
            }

            if (t1->type->kind == BASIC) {  // 数组解析完毕
                place->type = NULL;
                place->size = 0;
            } else if (t1->type->kind == ARRAY) {
                place->type = t1->type->u.array.elem;
                place->size = t1->type->u.array.size;
            }
        }
    }
}

void translate_Args(Node root, bool write_func) {
    if (root == NULL) return;
    dump_translator_node(root, "Args");
    assert(root->child_num == 1 || root->child_num == 3);

    if (root->child_num == 1) {         // Args -> Exp
    } else if (root->child_num == 3) {  // Args -> Exp COMMA Args
        assert(!write_func);            // WRITE 只有一个参数
        translate_Args(get_child(root, 2), write_func);
    }
    Operand arg = new_temp();
    translate_Exp(get_child(root, 0), arg);
    if (arg->kind == OP_STRUCTURE) {  // 不存在结构体类型的变量作为参数
        dump_structure_err();
    }
    if (write_func) {  // WRITE arg
        arg = load_value(arg);
        gen_ir(global_ir_list_head, IR_WRITE, arg, NULL, NULL, -1, NULL);
    } else {
        if (arg->kind == OP_ARRAY) {
            arg = get_addr(arg, true);
        } else if (arg->kind == OP_ADDRESS) {
            if (arg->type == NULL) {  // 参数是数值
                arg = load_value(arg);
            } else if (arg->type->kind == BASIC) {  // 参数是一维数组
            } else {                                // 高维数组不会作为参数
                assert(0);
            }
        }
        gen_ir(global_ir_list_head, IR_ARG, arg, NULL, NULL, -1, NULL);
    }
}

void translate_Cond(Node root, Operand label_true, Operand label_false) {
    if (root == NULL) return;
    assert(root->child_num == 1 || root->child_num == 2 || root->child_num == 3 || root->child_num == 4);
    if (translator_debug) printf("Translate \e[1;31mCond\e[0m\n");

    if (root->child_num == 2 && strcmp(get_child(root, 0)->name, "NOT") == 0) {  // NOT Exp
        translate_Cond(get_child(root, 1), label_false, label_true);
    } else if (root->child_num == 3 && strcmp(get_child(root, 1)->name, "AND") == 0) {  // Exp AND Exp
        Operand label1 = new_label();
        translate_Cond(get_child(root, 0), label1, label_false);
        gen_ir(global_ir_list_head, IR_LABEL, label1, NULL, NULL, -1, NULL);
        translate_Cond(get_child(root, 2), label_true, label_false);
    } else if (root->child_num == 3 && strcmp(get_child(root, 1)->name, "OR") == 0) {  // Exp OR Exp
        Operand label1 = new_label();
        translate_Cond(get_child(root, 0), label_true, label1);
        gen_ir(global_ir_list_head, IR_LABEL, label1, NULL, NULL, -1, NULL);
        translate_Cond(get_child(root, 2), label_true, label_false);
    } else if (root->child_num == 3 && strcmp(get_child(root, 1)->name, "RELOP") == 0) {  // Exp RELOP Exp
        Operand t1 = new_temp();
        translate_Exp(get_child(root, 0), t1);
        t1 = load_value(t1);
        Operand t2 = new_temp();
        translate_Exp(get_child(root, 2), t2);
        t2 = load_value(t2);
        char* relop = get_child(root, 1)->val;
        // IF t1 op t2 GOTO label_true
        gen_ir(global_ir_list_head, IR_IF_GOTO, t1, t2, label_true, -1, relop);
        // GOTO label_false
        gen_ir(global_ir_list_head, IR_GOTO, label_false, NULL, NULL, -1, NULL);
    } else {
        Operand t1 = new_temp();
        translate_Exp(root, t1);
        t1 = load_value(t1);
        // IF t1 != #0 GOTO label_true
        gen_ir(global_ir_list_head, IR_IF_GOTO, t1, gen_operand(OP_CONSTANT, 0, -1, NULL), label_true, -1, "!=");
        // GOTO label_false
        gen_ir(global_ir_list_head, IR_GOTO, label_false, NULL, NULL, -1, NULL);
    }
}

Operand array_deep_copy(Operand op_left, Operand op_right) {
    Operand left_base = get_addr(op_left, false);
    Operand right_base = get_addr(op_right, false);
    // 拷贝较少数组的元素
    int size_left = get_size(op_left->type) * op_left->size;
    int size_right = get_size(op_right->type) * op_right->size;
    int size = size_left < size_right ? size_left : size_right;
    assert(size % 4 == 0);
    Operand left = new_addr();
    Operand right = new_addr();
    Operand val = new_temp();
    // val := *right
    gen_ir(global_ir_list_head, IR_LOAD, val, right_base, NULL, -1, NULL);
    // *left := val
    gen_ir(global_ir_list_head, IR_STORE, left_base, val, NULL, -1, NULL);
    for (int i = 4; i < size; i += 4) {
        Operand offset = gen_operand(OP_CONSTANT, i, -1, NULL);
        // left := base + offset
        gen_ir(global_ir_list_head, IR_ADD, left, left_base, offset, -1, NULL);
        // right := base + offset
        gen_ir(global_ir_list_head, IR_ADD, right, right_base, offset, -1, NULL);
        // val := *right
        gen_ir(global_ir_list_head, IR_LOAD, val, right, NULL, -1, NULL);
        // *left := val
        gen_ir(global_ir_list_head, IR_STORE, left, val, NULL, -1, NULL);
    }
    return left_base;
}

Operand load_value(Operand addr) {
    if (addr->kind != OP_ADDRESS) return addr;
    Operand place = new_temp();
    // place := *addr
    gen_ir(global_ir_list_head, IR_LOAD, place, addr, NULL, -1, NULL);
    return place;
}

Operand get_addr(Operand addr, bool is_arg) {
    if (addr->kind != OP_ARRAY) return addr;
    Operand place = new_addr();
    // place := &addr
    gen_ir(global_ir_list_head, IR_ADDR, place, addr, NULL, -1, NULL);
    return place;
}

int get_size(Type type) {
    if (type == NULL) return 0;
    if (type->kind == BASIC) {
        return 4;
    } else if (type->kind == ARRAY) {
        return type->u.array.size * get_size(type->u.array.elem);
    } else if (type->kind == STRUCTURE) {
        dump_structure_err();
        int size = 0;
        FieldList iter = type->u.structure->type->u.member;
        while (iter) {
            size += get_size(iter->type);
            iter = iter->tail;
        }
        return size;

    } else {
        assert(0);
    }
    return 0;
}

void dump_translator_node(Node node, char* translator_name) {
    if (translator_debug == 0) return;
    printf("Translate \e[1;31m%-15s\e[0m", translator_name);
    if (node->tokenFlag == 0) {
        printf("Node \e[1;31m%-15s\e[0m\tchild_num %d\n", node->name, node->child_num);
        assert(strcmp(node->name, translator_name) == 0);
    } else
        print_tree(node, 0);
}