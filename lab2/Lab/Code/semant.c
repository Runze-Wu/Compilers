#include "semant.h"
extern int semantic_errs;
extern int semantic_debug;
void Program(Node root) {
    if (root == NULL) return;
    // print_tree(root, 0);
    // Program -> ExtDefList
    dump_node(root);
    assert(root->child_num == 1);
    ExtDefList(get_child(root, 0));
}
void ExtDefList(Node root) {
    if (root == NULL) return;
    // ExtDefList -> ExtDef ExtDefList
    dump_node(root);
    assert(root->child_num == 2);
    ExtDef(get_child(root, 0));
    ExtDefList(get_child(root, 1));
}
void ExtDef(Node root) {
    if (root == NULL) return;
    dump_node(root);
    assert(root->child_num == 2 || root->child_num == 3);
    Type type = Specifier(get_child(root, 0));
    // if (type == NULL) return;
    if (strcmp(get_child(root, 1)->name, "ExtDecList") == 0) {  // ExtDef -> Specifier ExtDecList SEMI
        ExtDecList(get_child(root, 1), type);
    } else if (strcmp(get_child(root, 1)->name, "FunDec") == 0) {  // ExtDef -> Specifier FunDec CompSt
        FunDec(get_child(root, 1), type);
        CompSt(get_child(root, 2), type);
    } else if (strcmp(get_child(root, 1)->name, "SEMI") == 0) {  // ExtDef -> Specifier SEMI
    }
}
void ExtDecList(Node root, Type type) {
    if (root == NULL) return;
    dump_node(root);
    assert(root->child_num == 1 || root->child_num == 3);
    if (root->child_num == 1) {  // ExtDecList -> VarDec
        VarDec(get_child(root, 0), type, NULL);
    } else if (root->child_num == 3) {  // ExtDecList -> VarDec COMMA ExtDecList
        VarDec(get_child(root, 0), type, NULL);
        ExtDecList(get_child(root, 2), type);
    }
}

Type Specifier(Node root) {
    if (root == NULL) return NULL;
    Type type = NULL;
    dump_node(root);
    assert(root->child_num == 1);
    if (strcmp(get_child(root, 0)->name, "TYPE") == 0) {  // Specifier -> TYPE
        type = (Type)malloc(sizeof(struct Type_));
        type->kind = BASIC;
        if (strcmp(get_child(root, 0)->val, "int") == 0) {
            type->u.basic = NUM_INT;
        } else if (strcmp(get_child(root, 0)->val, "float") == 0) {
            type->u.basic = NUM_FLOAT;
        } else {
            dump_node(get_child(root, 0));
            assert(0);
        }
    } else if (strcmp(get_child(root, 0)->name, "StructSpecifier") == 0) {  // Specifier -> StructSpecifier
        type = StructSpecifier(get_child(root, 0));
    }
    return type;
}
Type StructSpecifier(Node root) {
    if (root == NULL) return NULL;
    Type type = NULL;
    FieldList field = NULL;
    dump_node(root);
    assert(root->child_num == 2 || root->child_num == 5);
    if (root->child_num == 5) {  // StructSpecifier -> STRUCT OptTag LC DefList RC
        char* opt_tag = OptTag(get_child(root, 1));
        if (opt_tag != NULL) {
            if (look_up(opt_tag) != NULL) {
                dump_semantic_error(16, root->line, "Duplicated name", opt_tag);
                return NULL;
            }
        }
        field = (FieldList)malloc(sizeof(struct FieldList_));
        field->name = opt_tag;
        field->type = (Type)malloc(sizeof(struct Type_));
        field->type->kind = STRUCTTAG;
        field->type->u.member = NULL;
        DefList(get_child(root, 3), field);
        if (opt_tag != NULL) insert_field(field);
    } else if (root->child_num == 2) {  // StructSpecifier -> STRUCT TAG
        char* tag = Tag(get_child(root, 1));
        field = look_up(tag);
        if (field == NULL) {
            dump_semantic_error(17, root->line, "Undefined structure", tag);
            return NULL;
        }
    }
    type = (Type)malloc(sizeof(struct Type_));
    type->kind = STRUCTURE;
    type->u.structure = field;
    return type;
}
char* OptTag(Node root) {
    if (root == NULL) return NULL;
    // OptTag -> ID
    dump_node(root);
    assert(root->child_num == 1);
    return get_child(root, 0)->val;
}
char* Tag(Node root) {
    if (root == NULL) return NULL;
    // Tag -> ID
    dump_node(root);
    assert(root->child_num == 1);
    return get_child(root, 0)->val;
}

FieldList VarDec(Node root, Type type, FieldList field) {
    if (root == NULL) return NULL;
    FieldList var_field = NULL;
    dump_node(root);
    assert(root->child_num == 1 || root->child_num == 4);
    if (root->child_num == 1) {  // VarDec -> ID
        var_field = (FieldList)malloc(sizeof(struct FieldList_));
        var_field->type = type;  // 冲突与否，都会返回其Type
        var_field->tail = NULL;
        char* ID = get_child(root, 0)->val;
        var_field->name = ID;
        if (look_up(ID) != NULL) {
            if (field != NULL && field->type->kind == STRUCTTAG) {
                if (have_member(field, ID) != NULL)
                    dump_semantic_error(15, root->line, "Redefined filed", ID);
                else {  // 域内变量与外部变量冲突，暂未处理
                }
            } else {
                dump_semantic_error(3, root->line, "Redefined variable", ID);
            }
        } else {
            insert_field(var_field);
            if (field == NULL) dump_field(var_field, 0);
        }
    } else if (root->child_num == 4) {  // VarDec -> VarDec LB INT RB
        Type array_type = (Type)malloc(sizeof(struct Type_));
        array_type->kind = ARRAY;
        array_type->u.array.size = get_child(root, 2)->data.val_int;
        array_type->u.array.elem = type;
        return VarDec(get_child(root, 0), array_type, field);
    }
    return var_field;
}
void FunDec(Node root, Type type) {
    if (root == NULL) return;
    dump_node(root);
    assert(root->child_num == 3 || root->child_num == 4);
    char* ID = get_child(root, 0)->val;
    FieldList field = (FieldList)malloc(sizeof(struct FieldList_));
    field->name = ID;
    field->type = (Type)malloc(sizeof(struct Type_));
    field->type->kind = FUNCTION;
    field->type->u.function.argc = 0;
    field->type->u.function.argv = NULL;
    field->type->u.function.ret = type;
    if (look_up(ID) != NULL) {
        dump_semantic_error(4, root->line, "Redifined function", ID);
    } else {
        insert_field(field);
        dump_field(field, 0);
    }
    if (root->child_num == 3) {         // FunDec -> ID LP RP
    } else if (root->child_num == 4) {  // FunDec -> ID LP VarList RP
        VarList(get_child(root, 2), field);
    }
}
void VarList(Node root, FieldList field) {
    if (root == NULL) return;
    dump_node(root);
    assert(root->child_num == 1 || root->child_num == 3);
    if (root->child_num == 1) {  // VarList -> ParamDec
        add_func_parameter(get_child(root, 0), field);
    } else if (root->child_num == 3) {  // VarList -> ParamDec COMMA VarList
        add_func_parameter(get_child(root, 0), field);
        VarList(get_child(root, 2), field);
    }
}
FieldList ParamDec(Node root) {
    if (root == NULL) return NULL;
    // ParamDec -> Specifier VarDec
    dump_node(root);
    assert(root->child_num == 2);
    Type type = Specifier(get_child(root, 0));
    return type != NULL ? VarDec(get_child(root, 1), type, NULL) : NULL;
}

void CompSt(Node root, Type type) {
    if (root == NULL) return;
    // CompSt -> LC DefList StmtList RC
    dump_node(root);
    assert(root->child_num == 4);
    DefList(get_child(root, 1), NULL);
    Stmtlist(get_child(root, 2), type);
}
void Stmtlist(Node root, Type type) {
    if (root == NULL) return;
    // Stmtlist -> Stmt Stmtlist
    dump_node(root);
    assert(root->child_num == 2);
    Stmt(get_child(root, 0), type);
    Stmtlist(get_child(root, 1), type);
}
void Stmt(Node root, Type type) {
    if (root == NULL) return;
    dump_node(root);
    assert(root->child_num == 1 || root->child_num == 2 || root->child_num == 3 || root->child_num == 5 ||
           root->child_num == 7);
    if (root->child_num == 1) {  // Stmt -> CompSt
        CompSt(get_child(root, 0), type);
    } else if (root->child_num == 2) {  // Stmt -> Exp SEMI
        Exp(get_child(root, 0));
    } else if (root->child_num == 3) {  // Stmt -> RETURN Exp SEMI
        Type ret_type = Exp(get_child(root, 1));
        if (ret_type != NULL && type_matched(ret_type, type) == 0) {
            dump_semantic_error(8, root->line, "Type mismatched for return", NULL);
        }
        dump_type(ret_type, 0);
    } else if (root->child_num == 5) {
        /**
         * Stmt -> IF LP Exp RP Stmt
         *  Stmt -> WHILE LP Exp RP Stmt
         */
        Exp(get_child(root, 2));
        Stmt(get_child(root, 4), type);
    } else if (root->child_num == 7) {  // Stmt -> IF LP Exp RP Stmt ELSE Stmt
        Exp(get_child(root, 2));
        Stmt(get_child(root, 4), type);
        Stmt(get_child(root, 6), type);
    }
}

void DefList(Node root, FieldList field) {
    if (root == NULL) return;
    // DefList -> Def DefList
    dump_node(root);
    assert(root->child_num == 2);
    Def(get_child(root, 0), field);
    DefList(get_child(root, 1), field);
}
void Def(Node root, FieldList field) {
    if (root == NULL) return;
    // Def -> Specifier DecList SEMI
    dump_node(root);
    assert(root->child_num == 3);
    Type type = Specifier(get_child(root, 0));
    if (type != NULL) DecList(get_child(root, 1), type, field);
}
void DecList(Node root, Type type, FieldList field) {
    if (root == NULL) return;
    dump_node(root);
    assert(root->child_num == 1 || root->child_num == 3);
    if (root->child_num == 1) {  // DecList -> Dec
        Dec(get_child(root, 0), type, field);
    } else if (root->child_num == 3) {  // DecList -> Dec COMMA DecList
        Dec(get_child(root, 0), type, field);
        DecList(get_child(root, 2), type, field);
    }
}
void Dec(Node root, Type type, FieldList field) {
    if (root == NULL) return;
    dump_node(root);
    assert(root->child_num == 1 || root->child_num == 3);
    if (root->child_num == 1) {  // Dec -> VarDec
        if (field != NULL && field->type->kind == STRUCTTAG) {
            add_struct_member(get_child(root, 0), type, field);
        } else {
            VarDec(get_child(root, 0), type, field);
        }
    } else if (root->child_num == 3) {  // VarDec ASSIGNOP Exp
        if (field != NULL && field->type->kind == STRUCTTAG) {
            add_struct_member(get_child(root, 0), type, field);
            dump_semantic_error(15, root->line, "Initialized struct field in definition", NULL);
            return;
        }
        FieldList var_dec = VarDec(get_child(root, 0), type, field);
        Type assign_type = Exp(get_child(root, 2));
        if (var_dec != NULL && assign_type != NULL && type_matched(var_dec->type, assign_type) == 0) {
            dump_semantic_error(5, root->line, "Type mismatched for assignment", NULL);
        }
    }
}

Type Exp(Node root) {
    if (root == NULL) return NULL;
    Type type = NULL;
    FieldList result = NULL;
    dump_node(root);
    assert(root->child_num == 1 || root->child_num == 2 || root->child_num == 3 || root->child_num == 4);
    if (root->child_num == 1) {
        if (strcmp(get_child(root, 0)->name, "ID") == 0) {  // Exp -> ID
            result = look_up(get_child(root, 0)->val);
            if (result == NULL) {
                dump_semantic_error(1, root->line, "Undefined variable", get_child(root, 0)->val);
            } else {
                type = result->type;
            }
        } else {
            type = (Type)malloc(sizeof(struct Type_));
            type->kind = BASIC;
            if (get_child(root, 0)->datatype == TYPE_INT) {  // Exp -> INT
                type->u.basic = NUM_INT;
            } else if (get_child(root, 0)->datatype == TYPE_FLOAT) {  // Exp -> FLOAT
                type->u.basic = NUM_FLOAT;
            } else {
                dump_node(get_child(root, 0));
                assert(0);
            }
        }
    } else if (root->child_num == 2) {
        /**
         * Exp -> NOT Exp
         * Exp -> MINUS Exp
         */
        type = Exp(get_child(root, 1));
    } else if (root->child_num == 3) {
        if (strcmp(get_child(root, 0)->name, "LP") == 0) {  // Exp -> LP Exp RP
            type = Exp(get_child(root, 1));
        } else if (strcmp(get_child(root, 0)->name, "ID") == 0) {  // Exp -> ID LP RP
            result = look_up(get_child(root, 0)->val);
            if (result == NULL) {
                dump_semantic_error(2, root->line, "Undefined function", get_child(root, 0)->val);
            } else if (result != NULL && result->type->kind != FUNCTION) {
                dump_semantic_error(11, root->line, "Not a function", get_child(root, 0)->val);
            } else if (args_matched(NULL, result->type->u.function.argv) == 0) {
                type = result->type->u.function.ret;
                dump_semantic_error(9, root->line, "Function is not appicable for arguments", get_child(root, 0)->val);
            } else {
                type = result->type->u.function.ret;
            }
        } else if (strcmp(get_child(root, 1)->name, "DOT") == 0) {  // Exp -> Exp DOT ID
            Type type1 = Exp(get_child(root, 0));
            if (type1 == NULL) {
            } else if (type1->kind != STRUCTURE) {
                dump_semantic_error(13, root->line, "Illegal use of", ".");
            } else {
                char* mem_name = get_child(root, 2)->val;
                FieldList member_point = have_member(type1->u.structure, mem_name);
                if (member_point == NULL) {
                    dump_semantic_error(14, root->line, "Non-existent field", mem_name);
                } else {
                    type = member_point->type;
                }
            }
        } else if (strcmp(get_child(root, 1)->name, "ASSIGNOP") == 0) {  // Exp ASSIGNOP Exp
            Node node_left = get_child(root, 0);
            type = Exp(node_left);
            Type type_right = Exp(get_child(root, 2));
            dump_type(type, 0);
            dump_type(type_right, 0);
            if (type == NULL) {
            } else if ((node_left->child_num == 1 && strcmp(get_child(node_left, 0)->name, "ID") == 0) ||
                       (node_left->child_num == 3 && strcmp(get_child(node_left, 1)->name, "DOT") == 0) ||
                       (node_left->child_num == 4 && strcmp(get_child(node_left, 0)->name, "Exp") == 0)) {
                if (type_matched(type, type_right) == 0) {
                    dump_semantic_error(5, root->line, "Type mismatched for assignment", NULL);
                }
            } else {
                dump_semantic_error(6, root->line, "The left-hand side of an assignment must be a variable", NULL);
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
            type = Exp(get_child(root, 0));
            Type type_right = Exp(get_child(root, 2));

            if (type != NULL && type_right != NULL && (type->kind != BASIC || type_matched(type, type_right) == 0)) {
                dump_semantic_error(7, root->line, "Type mismatched for operands", NULL);
                dump_type(type, 0);
                dump_type(type_right, 0);
            }
        }
    } else if (root->child_num == 4) {
        if (strcmp(get_child(root, 0)->name, "ID") == 0) {  // Exp -> ID LP Args RP
            result = look_up(get_child(root, 0)->val);
            if (result == NULL) {
                dump_semantic_error(2, root->line, "Undefined function", get_child(root, 0)->val);
            } else if (result->type->kind != FUNCTION) {
                dump_semantic_error(11, root->line, "Not a function", get_child(root, 0)->val);
            } else {
                FieldList act_args = Args(get_child(root, 2));
                if (act_args == NULL) {
                } else if (args_matched(act_args, result->type->u.function.argv) == 0) {
                    dump_semantic_error(9, root->line, "Function is not appicable for arguments",
                                        get_child(root, 0)->val);
                    dump_field(result, 0);
                } else {
                    type = result->type->u.function.ret;
                }
            }
        } else if (strcmp(get_child(root, 0)->name, "Exp") == 0) {  // Exp -> Exp LB Exp RB
            Type type1 = Exp(get_child(root, 0));
            if (type1 == NULL) {
            } else if (type1->kind != ARRAY) {
                dump_semantic_error(10, root->line, "Not an array", NULL);
            } else {
                type = type1->u.array.elem;
            }
            Type type2 = Exp(get_child(root, 2));
            if (type2 != NULL && (type2->kind != BASIC || type2->u.basic != NUM_INT)) {
                dump_semantic_error(12, root->line, "Not an integer", NULL);
            }
        }
    }
    return type;
}
FieldList Args(Node root) {
    if (root == NULL) return NULL;
    dump_node(root);
    assert(root->child_num == 1 || root->child_num == 3);
    Type args_type = Exp(get_child(root, 0));
    if (args_type == NULL) return NULL;
    FieldList args = (FieldList)malloc(sizeof(struct FieldList_));
    args->name = "arg";
    args->type = args_type;
    args->tail = NULL;
    if (root->child_num == 1) {         // Args -> Exp
    } else if (root->child_num == 3) {  // Args -> Exp COMMA Args
        args->tail = Args(get_child(root, 2));
    }
    return args;
}

FieldList have_member(FieldList struct_field, char* member) {
    if (struct_field == NULL || struct_field->type->kind != STRUCTTAG) return NULL;
    FieldList member_point = struct_field->type->u.member;
    while (member_point != NULL) {
        if (strcmp(member_point->name, member) == 0) {
            return member_point;
        }
        member_point = member_point->tail;
    }
    return NULL;
}

bool type_matched(Type a, Type b) {
    if (a == NULL && b == NULL) return true;
    if (a == NULL || b == NULL) return false;
    if (a->kind != b->kind) return false;
    FieldList a_member = a->u.member;
    FieldList b_member = b->u.member;
    switch (a->kind) {
        case BASIC:
            return a->u.basic == b->u.basic;
            break;
        case ARRAY:
            return type_matched(a->u.array.elem, b->u.array.elem);
            break;
        case STRUCTURE:
            return type_matched(a->u.structure->type, b->u.structure->type);
            break;
        case STRUCTTAG:
            while (a_member != NULL || b_member != NULL) {
                if (a_member == NULL || b_member == NULL) return false;
                if (type_matched(a_member->type, b_member->type) == 0) return false;
                a_member = a_member->tail;
                b_member = b_member->tail;
            }
            break;
        case FUNCTION:
            assert(0);
            break;
    }
    return true;
}

bool args_matched(FieldList act_args, FieldList form_args) {
    if (act_args == NULL && form_args == NULL) return true;
    if (act_args == NULL || form_args == NULL) return false;
    while (act_args != NULL || form_args != NULL) {
        if (act_args == NULL || form_args == NULL) return false;
        if (type_matched(act_args->type, form_args->type) == 0) return false;
        act_args = act_args->tail;
        form_args = form_args->tail;
    }
    return true;
}

void add_struct_member(Node member, Type mem_type, FieldList struct_field) {
    assert(struct_field != NULL);
    FieldList mem_field = VarDec(member, mem_type, struct_field);
    FieldList temp_field = struct_field->type->u.member;
    if (mem_field == NULL) return;
    if (struct_field->type->u.member == NULL) {
        struct_field->type->u.member = mem_field;
    } else {
        while (temp_field->tail != NULL) temp_field = temp_field->tail;
        temp_field->tail = mem_field;
    }
}

void add_func_parameter(Node param, FieldList func_field) {
    assert(func_field != NULL);
    func_field->type->u.function.argc++;
    FieldList arg_field = ParamDec(param);
    FieldList temp_field = func_field->type->u.function.argv;
    if (arg_field == NULL) return;
    if (func_field->type->u.function.argv == NULL) {
        func_field->type->u.function.argv = arg_field;
    } else {
        while (temp_field->tail != NULL) temp_field = temp_field->tail;
        temp_field->tail = arg_field;
    }
}

void dump_type(Type type, int depth) {
    if (semantic_debug == 0 || type == NULL) return;
    FieldList field;
    for (int i = 0; i < depth; i++) printf("  ");
    switch (type->kind) {
        case BASIC:
            printf(" BASIC kind: ");
            if (type->u.basic == NUM_INT)
                printf("NUM_INT\n");
            else if (type->u.basic == NUM_FLOAT)
                printf("NUM_FLOAT\n");
            break;
        case ARRAY:
            printf(" ARRAY size: %d\n", type->u.array.size);
            dump_type(type->u.array.elem, depth + 1);
            break;
        case STRUCTURE:
            printf(" structure:\n");
            dump_field(type->u.structure, depth + 1);
            break;
        case STRUCTTAG:
            printf(" STRUCTTAG\n");
            field = type->u.member;
            while (field != NULL) {
                dump_field(field, depth + 1);
                field = field->tail;
            }
            break;
        case FUNCTION:
            printf(" FUNCTION\n");
            field = type->u.function.argv;
            if (field != NULL)
                printf("  argc:%d\n  argv:", type->u.function.argc);
            else
                printf("  argv is null\n");
            while (field != NULL) {
                dump_field(field, depth + 1);
                field = field->tail;
            }
            printf("  ret type:");
            dump_type(type->u.function.ret, depth);
            break;
        default:
            printf("\n");
            break;
    }
}

void dump_field(FieldList field, int depth) {
    if (semantic_debug == 0 || field == NULL) return;
    for (int i = 0; i < depth; i++) printf("  ");
    printf("field: %s type:", field->name);
    dump_type(field->type, depth);
}

void dump_node(Node node) {
    if (semantic_debug == 0) return;
    if (node->tokenFlag == 0)
        printf("Node \e[1;31m%s\e[0m child_num %d\n", node->name, node->child_num);
    else
        print_tree(node, 0);
}

void dump_semantic_error(int err_type, int err_line, char* err_msg, char* err_elm) {
    printf("Error type %d at Line %d: %s", err_type, err_line, err_msg);
    if (err_elm != NULL) {
        printf(" \"%s\"", err_elm);
    }
    printf(".\n");
}