#include "controlflow.h"

int global_bb_count = 0;                    // BB块编号
int fb_number = 0;                          // FB块编号
int global_fb_count = 0;                    // FB块个数
BasicBlockList* bb_array;                   // BB链表数组，便于索引
FunctionBlock* fb_array;                    // FB数组，便于索引
extern int optimizer_debug;                 // optimizer debug mode
extern InterCodeList* label_array;          // 所有编号的数组
extern InterCodeList global_ir_list_head;   // 循环双向链表头
extern BasicBlockList global_bb_list_head;  // 基本块循环双向链表头

void optimize() {
    remove_redundant_label();
    global_bb_list_head = init_bb_list();
    construct_bb_list();
    construct_bb_array();
    init_fb_array();
    construct_fb_array();
    construct_cfg();
    // show_cfg();
    // CP();
    // LVA();
}

void remove_redundant_label() {
    InterCodeList cur = global_ir_list_head->next;
    while (cur != global_ir_list_head) {
        assert(cur->code != NULL);
        if (cur->code->kind == IR_LABEL) {
            assert(cur->code->u.unary_ir.op != NULL);
            int label_num = cur->code->u.unary_ir.op->u.label_no;
            InterCodeList next = cur->next;
            while (next != global_ir_list_head) {
                assert(next->code != NULL);
                if (next->code->kind != IR_LABEL) break;
                delete_ir(next);
                next->code->u.unary_ir.op->u.label_no = label_num;
                next = next->next;
            }
        }
        cur = cur->next;
    }
}

BasicBlockList init_bb_list() {
    BasicBlockList bb_list_head = (BasicBlockList)malloc(sizeof(struct BasicBlockList_));
    assert(bb_list_head != NULL);
    bb_list_head->prev = bb_list_head->next = bb_list_head;
    bb_list_head->bb = NULL;
    return bb_list_head;
}

BasicBlock init_bb() {
    BasicBlock bb = (BasicBlock)malloc(sizeof(struct BasicBlock_));
    assert(bb != NULL);
    bb->fb_no = bb->bb_no = UNKNOWN;
    bb->first = bb->last = NULL;
    bb->pre = init_bb_list();
    bb->suc = init_bb_list();
    return bb;
}

void add_bb(BasicBlockList bb_list_head, BasicBlock bb) {
    if (bb == NULL) return;
    BasicBlockList new_term = (BasicBlockList)malloc(sizeof(struct BasicBlockList_));
    assert(new_term != NULL);
    new_term->bb = bb;
    BasicBlockList tail = bb_list_head->prev;
    tail->next = new_term;
    new_term->prev = tail;
    bb_list_head->prev = new_term;
    new_term->next = bb_list_head;
}

void bb_tag_ir_list() {
    InterCodeList cur = global_ir_list_head->next;
    bb_tag_ir(cur->code);  // step1: 第一条指令标记为BB开始
    while (cur != global_ir_list_head) {
        assert(cur->code != NULL);
        if (cur->code->kind == IR_IF_GOTO || cur->code->kind == IR_GOTO) {
            int label_num = UNKNOWN;
            if (cur->code->kind == IR_GOTO) {  //跳转目标
                assert(cur->code->u.unary_ir.op != NULL);
                label_num = cur->code->u.unary_ir.op->u.label_no;
            } else {
                assert(cur->code->u.if_goto.z != NULL);
                label_num = cur->code->u.if_goto.z->u.label_no;
            }
            assert(label_num != UNKNOWN && label_num < label_number);
            // step2: 跳转语句的目标指令为BB开始
            bb_tag_ir(label_array[label_num]->code);
            // step3: 跳转语句的下一条指令为BB开始
            if (cur->next != global_ir_list_head) {
                bb_tag_ir(cur->next->code);
            }
        } else if (cur->code->kind == IR_FUNC) {
            // step4: 函数定义指令为BB开始
            bb_tag_ir(cur->code);
        } else if (cur->code->kind == IR_RETURN) {
            // step4: 函数返回语句的下一条指令为BB开始
            if (cur->next != global_ir_list_head) {
                bb_tag_ir(cur->next->code);
            }
        }
        cur = cur->next;
    }
}

void bb_tag_ir(InterCode ir) {
    assert(ir != NULL);
    if (!ir->bb_start && ir->kind == IR_FUNC) global_fb_count++;
    ir->bb_start = true;
}

bool is_bb_start(InterCode ir) {
    assert(ir != NULL);
    return ir->bb_start;
}

void construct_bb_list() {
    InterCodeList prev_first = NULL;
    bb_tag_ir_list();  // 进行BB的标记
    InterCodeList cur = global_ir_list_head->next;
    while (cur != global_ir_list_head) {
        assert(cur->code != NULL);
        if (is_bb_start(cur->code)) {
            if (prev_first) {  // 第二次遇到开始指令
                assert(prev_first != cur);
                construct_bb(prev_first, cur->prev);
            }
            prev_first = cur;
        }
        cur = cur->next;
    }
    assert(prev_first != NULL);
    construct_bb(prev_first, global_ir_list_head->prev);  // 最后一个BB块
}

void init_fb_array() {
    fb_array = (FunctionBlock*)calloc(global_fb_count, sizeof(FunctionBlock));
    assert(fb_array != NULL);
    for (int i = 0; i < global_fb_count; i++) {
        fb_array[i] = (FunctionBlock)malloc(sizeof(struct FunctionBlock_));
        fb_array[i]->fb_no = i;
        fb_array[i]->bb_last = fb_array[i]->bb_first = UNKNOWN;
        fb_array[i]->bb_nums = 0;
        BasicBlock bb_entry = init_bb();
        BasicBlock bb_exit = init_bb();
        bb_entry->bb_no = ENTRY;
        bb_exit->bb_no = EXIT;
        fb_array[i]->entry = init_bb_list();
        fb_array[i]->entry->bb = bb_entry;
        fb_array[i]->exit = init_bb_list();
        fb_array[i]->exit->bb = bb_exit;
        fb_array[i]->func_name = NULL;
    }
}

void construct_fb_array() {
    int prev_fb = UNKNOWN, cur_fb = UNKNOWN;
    char* prev_func = NULL;
    for (int i = 0; i < global_bb_count; i++) {
        BasicBlockList cur = bb_array[i];
        assert(cur->bb->first->code != NULL);
        if (cur->bb->first->code->kind == IR_FUNC) {
            cur_fb = i;
            if (prev_fb != UNKNOWN) {
                construct_fb(fb_array[fb_number++], prev_func, prev_fb, cur_fb - 1);
            }
            prev_fb = i;
            prev_func = cur->bb->first->code->u.unary_ir.op->u.func_name;
        }
        cur->bb->fb_no = fb_number;
    }
    assert(prev_fb != UNKNOWN);
    construct_fb(fb_array[fb_number++], prev_func, prev_fb, global_bb_count - 1);  // 最后一个FB块
    assert(fb_number == global_fb_count);
}

void construct_fb(FunctionBlock fb, char* func_name, int first, int last) {
    assert(first != UNKNOWN && last != UNKNOWN);
    fb->bb_first = first;
    fb->bb_last = last;
    fb->func_name = func_name;
    fb->bb_nums = last - first + 1;
    if (optimizer_debug) {
        printf("new fb:%d\n", fb->fb_no);
        printf("first bb: %d\n", first);
        printf("last bb: %d\n", last);
    }
}

void construct_bb(InterCodeList first, InterCodeList last) {
    assert(first && last);
    BasicBlock bb = init_bb();
    bb->pre = init_bb_list();
    bb->suc = init_bb_list();  // 前继后继设为空的头结点
    bb->first = first;
    bb->last = last;
    bb->bb_no = global_bb_count++;
    InterCodeList cur = bb->first;
    do {
        cur->code->bb_no = global_bb_count - 1;  //记录所在的BB
        cur = cur->next;
    } while (cur != bb->last->next);
    if (optimizer_debug) {
        printf("new bb:%d\n", bb->bb_no);
        printf("first ir:\t");
        show_ir(first->code, stdout);
        printf("next ir:\t");
        show_ir(last->code, stdout);
    }
    add_bb(global_bb_list_head, bb);
}

void construct_bb_array() {
    bb_array = (BasicBlockList*)calloc(global_bb_count, sizeof(BasicBlockList));
    BasicBlockList cur = global_bb_list_head->next;
    int i = 0;
    while (cur != global_bb_list_head) {
        bb_array[i++] = cur;
        cur = cur->next;
    }
}

void construct_cfg() {
    for (int i = 0; i < global_bb_count; i++) {
        BasicBlockList cur = bb_array[i];
        int fb_no = cur->bb->fb_no;
        assert(fb_no != UNKNOWN && fb_no < global_fb_count);
        BasicBlockList entry = fb_array[fb_no]->entry, exit = fb_array[fb_no]->exit;
        int bb_first = fb_array[fb_no]->bb_first, bb_last = fb_array[fb_no]->bb_last;
        assert(bb_first <= i && i <= bb_last);
        int label_num = UNKNOWN;
        assert(cur->bb->last->code != NULL);
        if (i == bb_first) {  // 第一块将Entry设置为其前驱
            add_bb(cur->bb->pre, entry->bb);
            add_bb(entry->bb->suc, cur->bb);
        }
        if (cur->bb->last->code->kind == IR_RETURN) {  // RETURN将Exit设置为其后继
            add_bb(cur->bb->suc, exit->bb);
            add_bb(exit->prev, cur->bb);
        } else if (cur->bb->last->code->kind != IR_GOTO) {
            if (i != bb_last) {
                assert(i < global_bb_count - 1);
                add_bb(cur->bb->suc, bb_array[i + 1]->bb);  // 下一个BB为直接后继
                add_bb(bb_array[i + 1]->bb->pre, cur->bb);  // 设置前驱
            } else {                                        // 最后一块将Exit设置为其后继
                add_bb(cur->bb->suc, exit->bb);
                add_bb(exit->prev, cur->bb);
            }
        }
        if (cur->bb->last->code->kind == IR_GOTO) {  //跳转后继
            assert(cur->bb->last->code->u.unary_ir.op != NULL);
            label_num = cur->bb->last->code->u.unary_ir.op->u.label_no;
        } else if (cur->bb->last->code->kind == IR_IF_GOTO) {
            assert(cur->bb->last->code->u.if_goto.z != NULL);
            label_num = cur->bb->last->code->u.if_goto.z->u.label_no;
        }
        if (label_num != UNKNOWN) {
            int goto_bb = label_array[label_num]->code->bb_no;
            add_bb(cur->bb->suc, bb_array[goto_bb]->bb);  // 设置后继
            add_bb(bb_array[goto_bb]->bb->pre, cur->bb);  // 设置前驱
        }
    }
}

void show_cfg() {
    for (int i = 0; i < global_bb_count; i++) {
        BasicBlockList cur = bb_array[i];
        if (optimizer_debug) printf("-------basic block%d,next:-------\n", cur->bb->bb_no);
        show_bb_list(cur->bb->suc, stdout);
        if (optimizer_debug) printf("-------basic block%d,prev:-------\n", cur->bb->bb_no);
        show_bb_list(cur->bb->pre, stdout);
    }
}

void show_bb_list(BasicBlockList bb_list_head, FILE* ir_out) {
    BasicBlockList cur = bb_list_head->next;
    while (cur != bb_list_head) {
        assert(cur->bb && cur->bb->bb_no != UNKNOWN);
        if (optimizer_debug && ir_out) {
            if (cur->bb->bb_no == ENTRY) {
                fprintf(ir_out, "-------ENTRY-------\n");
            } else if (cur->bb->bb_no == EXIT) {
                fprintf(ir_out, "-------EXIT-------\n");
            } else {
                fprintf(ir_out, "-------basic block%d-------\n", cur->bb->bb_no);
            }
        }
        if (cur->bb->bb_no != ENTRY && cur->bb->bb_no != EXIT) show_bb(cur->bb, ir_out);
        cur = cur->next;
    }
}

void show_bb(BasicBlock bb, FILE* ir_out) {
    if (bb == NULL || ir_out == NULL) return;
    InterCodeList cur = bb->first;
    do {
        show_ir(cur->code, ir_out);
        cur = cur->next;
    } while (cur != bb->last->next);
}

void dump_BOOL_matrix(bool** array, int m, int n) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            printf("%d", array[i][j]);
        }
        printf("\n");
    }
}

bool** allocate_BOOL_matrix(int m, int n) {
    bool** array = (bool**)calloc(m, sizeof(bool*));
    for (int i = 0; i < m; i++) {
        array[i] = (bool*)calloc(n, sizeof(bool));
        memset(array[i], false, n);
    }
    return array;
}

void release_BOOL_matrix(bool** array, int len) {
    for (int i = 0; i < len; i++) {
        free(array[i]);
    }
    free(array);
}

void bitset_union(bool* a, bool* b, bool* res, int len) {
    for (int i = 0; i < len; i++) {
        res[i] = a[i] | b[i];
    }
}

void bitset_minus(bool* a, bool* b, bool* res, int len) {
    for (int i = 0; i < len; i++) {
        res[i] = a[i] & (a[i] ^ b[i]);
    }
}

bool bitset_diff(bool* a, bool* b, int len) {
    for (int i = 0; i < len; i++) {
        if (a[i] ^ b[i]) return true;
    }
    return false;
}

void set_bitset(Operand op, bool* bitset, bool val) {
    int id = get_variable_id(op);
    if (id != -1) bitset[id] = val;
#ifdef LVA_DEBUG
    if (id != -1 && op->kind == OP_VARIABLE) printf("var-%d set %d\n", id, val);
#endif
}

int get_variable_count() { return var_number + temp_number; }

int get_variable_id(Operand op) {
    int id = -1;
    if (op->kind == OP_TEMP) {
        id = op->u.temp_no;
    } else if (op->kind == OP_VARIABLE) {
        id = temp_number + op->u.var_no;
    }
    return id;
}

void set_LVA_use_def(Operand op, int kind, bool* use, bool* def) {
    int id = get_variable_id(op);
    if (id != -1) {
        if (kind == DEF) {
            def[id] = true;
            use[id] = false;
        } else if (kind == USE) {
            use[id] = true;
            def[id] = false;
        }
    }
}

void LVA_BB_use_def(BasicBlock bb, bool* use, bool* def) {
    InterCodeList cur = bb->last;
    do {
        LVA_IR_use_def(cur->code, use, def);
        cur = cur->prev;
    } while (cur != bb->first->prev);
}

void LVA_IR_use_def(InterCode ir, bool* use, bool* def) {
    assert(ir != NULL);
    switch (ir->kind) {
        case IR_LABEL:
            break;
        case IR_FUNC:
            break;
        case IR_GOTO:
            break;
        case IR_RETURN:
        case IR_ARG:
        case IR_WRITE:
            set_LVA_use_def(ir->u.unary_ir.op, USE, use, def);
            break;
        case IR_DEC:
            set_LVA_use_def(ir->u.dec.op, USE, use, def);
            break;
        case IR_PARAM:
        case IR_READ:
            set_LVA_use_def(ir->u.unary_ir.op, DEF, use, def);
            break;
        case IR_ASSIGN:
        case IR_ADDR:
        case IR_LOAD:
        case IR_STORE:
        case IR_CALL:
            set_LVA_use_def(ir->u.binary_ir.left, DEF, use, def);
            set_LVA_use_def(ir->u.binary_ir.right, USE, use, def);
            break;
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
            set_LVA_use_def(ir->u.ternary_ir.res, DEF, use, def);
            set_LVA_use_def(ir->u.ternary_ir.op1, USE, use, def);
            set_LVA_use_def(ir->u.ternary_ir.op2, USE, use, def);
            break;
        case IR_IF_GOTO:
            set_LVA_use_def(ir->u.if_goto.x, USE, use, def);
            set_LVA_use_def(ir->u.if_goto.y, USE, use, def);
            break;
        default:
            assert(0);
            break;
    }
}

void LVA() {
    var_nums = get_variable_count();
    for (int i = 0; i < fb_number; i++) {
        int bb_first = fb_array[i]->bb_first, bb_last = fb_array[i]->bb_last;
        int bb_nums = fb_array[i]->bb_nums;
        bool** IN = allocate_BOOL_matrix(bb_nums, var_nums);
        bool** OUT = allocate_BOOL_matrix(bb_nums, var_nums);
        bool** use = allocate_BOOL_matrix(bb_nums, var_nums);
        bool** def = allocate_BOOL_matrix(bb_nums, var_nums);
        // 获取use和def
        for (int j = bb_last; j >= bb_first; j--) {
            int bb_off = j - bb_first;
            LVA_BB_use_def(bb_array[j]->bb, use[bb_off], def[bb_off]);
        }
#ifdef LVA_DEBUG
        printf("--------------use-------------------\n");
        dump_BOOL_matrix(use, bb_nums, var_nums);
        printf("--------------def-------------------\n");
        dump_BOOL_matrix(def, bb_nums, var_nums);
#endif
        // 迭代更新,趋近不动点
        bool change;
        do {
            change = false;
            for (int j = bb_last; j >= bb_first; j--) {
                assert(bb_array[j]->bb->bb_no == j);
                LVA_meet(bb_first, j, IN, OUT);
                int bb_off = j - bb_first;
                change = change | LVA_transfer(j, IN[bb_off], OUT[bb_off], use[bb_off], def[bb_off]);
            }
        } while (change);
#ifdef LVA_DEBUG
        printf("--------------IN-------------------\n");
        dump_BOOL_matrix(IN, bb_nums, var_nums);
        printf("--------------OUT-------------------\n");
        dump_BOOL_matrix(OUT, bb_nums, var_nums);
#endif
        for (int j = bb_last; j >= bb_first; j--) {
            DAE(bb_array[j]->bb, OUT[j - bb_first]);
        }
        release_BOOL_matrix(IN, bb_nums);
        release_BOOL_matrix(OUT, bb_nums);
        release_BOOL_matrix(use, bb_nums);
        release_BOOL_matrix(def, bb_nums);
    }
}

void LVA_meet(int bb_first, int bb_no, bool** IN, bool** OUT) {
    // OUT[B]=U_{S a succ of B}IN[S]
    BasicBlockList succ_head = bb_array[bb_no]->bb->suc;
    BasicBlockList cur = succ_head->next;
    for (BasicBlockList cur = succ_head->next; cur != succ_head; cur = cur->next) {
        int succ_no = cur->bb->bb_no;
        if (succ_no == EXIT) continue;
        assert(succ_no != UNKNOWN || succ_no != ENTRY);  // 每个BB都应有编号，ENTRY不应有前驱
        int bb_off = bb_no - bb_first, succ_off = succ_no - bb_first;
        bitset_union(OUT[bb_off], IN[succ_off], OUT[bb_off], var_nums);
    }
#ifdef LVA_DEBUG
    printf("bb%d-out after union\n", bb_no);
    dump_BOOL_matrix(&OUT[bb_no - bb_first], 1, var_nums);
#endif
}

bool LVA_transfer(int bb_no, bool* in_b, bool* out_b, bool* use, bool* def) {
    // IN[B]=use_B U (OUT[B]-def_B)
    bool* prev_in = (bool*)calloc(var_nums, sizeof(bool));
    for (int i = 0; i < var_nums; i++) prev_in[i] = in_b[i];
    bool* temp = (bool*)calloc(var_nums, sizeof(bool));
    bitset_minus(out_b, def, temp, var_nums);
    bitset_union(use, temp, in_b, var_nums);
    bool change = bitset_diff(prev_in, in_b, var_nums);
    free(prev_in);
    free(temp);
#ifdef LVA_DEBUG
    printf("bb%d-in after transfer\n", bb_no);
    dump_BOOL_matrix(&in_b, 1, var_nums);
#endif
    return change;
}

bool judge_IR_DA(InterCode ir, bool* out) {
    bool result = false;
    int res_id = UNKNOWN;
    assert(ir != NULL);
    switch (ir->kind) {
        case IR_LABEL:
            break;
        case IR_FUNC:
            break;
        case IR_GOTO:
            break;
        case IR_RETURN:
        case IR_ARG:
        case IR_WRITE:
            set_bitset(ir->u.unary_ir.op, out, true);
            break;
        case IR_DEC:
            set_bitset(ir->u.dec.op, out, true);
            break;
        case IR_PARAM:
        case IR_READ:  // READ PARAM不受控
            set_bitset(ir->u.unary_ir.op, out, false);
            break;
        case IR_ASSIGN:
        case IR_ADDR:
        case IR_LOAD:
        case IR_STORE:
            res_id = get_variable_id(ir->u.binary_ir.left);
            if (res_id != -1 && out[res_id] == 0) {
#ifdef LVA_DEBUG
                printf("delete assign %d\n", res_id);
#endif
                result = true;
            } else {
                set_bitset(ir->u.binary_ir.left, out, false);
                set_bitset(ir->u.binary_ir.right, out, true);
            }
            break;
        case IR_CALL:
            // 函数调用有全局副作用 a:= call b,a可以消去
            set_bitset(ir->u.binary_ir.left, out, false);
            break;
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
            res_id = get_variable_id(ir->u.ternary_ir.res);
            if (res_id != -1 && out[res_id] == 0) {
                result = true;
            } else {
                set_bitset(ir->u.ternary_ir.res, out, false);
                set_bitset(ir->u.ternary_ir.op1, out, true);
                set_bitset(ir->u.ternary_ir.op2, out, true);
            }
            break;
        case IR_IF_GOTO:
            set_bitset(ir->u.if_goto.x, out, true);
            set_bitset(ir->u.if_goto.y, out, true);
            break;
        default:
            assert(0);
            break;
    }
    return result;
}

void DAE(BasicBlock bb, bool* out) {
    bool* temp_out = (bool*)calloc(var_nums, sizeof(bool));
    for (int i = 0; i < var_nums; i++) temp_out[i] = out[i];
#ifdef LVA_DEBUG
    printf("bb%d-out\n", bb->bb_no);
    dump_BOOL_matrix(&temp_out, 1, var_nums);
#endif
    InterCodeList cur = bb->last;
    do {
        if (judge_IR_DA(cur->code, temp_out)) {
            delete_ir(cur);
#ifdef LVA_DEBUG
            printf("delete ir:\t");
            show_ir(cur->code, stdout);
#endif
        }
        cur = cur->prev;
    } while (cur != bb->first->prev);
    free(temp_out);
}

void dump_PAIR_matrix(CPPair** array, int m, int n) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            printf("%d-%lld ", array[i][j].kind, array[i][j].val);
        }
        printf("\n");
    }
}

CPPair** allocate_PAIR_matrix(int m, int n) {
    CPPair** array = (CPPair**)calloc(m, sizeof(CPPair*));
    for (int i = 0; i < m; i++) {
        array[i] = (CPPair*)calloc(n, sizeof(CPPair));
        for (int j = 0; j < n; j++) {
            array[i][j].kind = UNDEF;
            array[i][j].val = UNKNOWN;
        }
    }
    return array;
}

void release_PAIR_matrix(CPPair** array, int len) {
    for (int i = 0; i < len; i++) {
        free(array[i]);
    }
    free(array);
}

void pairset_union(CPPair* a, CPPair* b, CPPair* res, int len) {
    for (int i = 0; i < len; i++) {
        if (a[i].kind == NAC || b[i].kind == NAC) {
            res[i].kind = NAC;
        } else if (a[i].kind == UNDEF && b[i].kind == UNDEF) {
            res[i].kind = UNDEF;
        } else if (a[i].kind == UNDEF && b[i].kind == CONST) {
            res[i].kind = b[i].kind;
            res[i].val = b[i].val;
        } else if (b[i].kind == UNDEF && a[i].kind == CONST) {
            res[i].kind = a[i].kind;
            res[i].val = a[i].val;
        } else {  // CONST CONST
            assert(a[i].kind == CONST && b[i].kind == CONST);
            if (a[i].val != b[i].val) {
                res[i].kind = NAC;
            } else {
                res[i].kind = CONST;
                res[i].val = a[i].val;
            }
        }
    }
}

bool pairset_diff(CPPair* a, CPPair* b, int len) {
    for (int i = 0; i < len; i++) {
        if (a[i].kind != b[i].kind) return true;
        if (a[i].kind == CONST && a[i].val != b[i].val) return true;
    }
    return false;
}

void set_CP_out(Operand op, int kind, long long val, CPPair* out) {
    int id = get_variable_id(op);
    if (id != -1) {
        out[id].kind = kind;
        out[id].val = val;
    }
}

void CP_BB_out(BasicBlock bb, CPPair* in, CPPair* out) {
    for (int i = 0; i < var_nums; i++) {
        out[i].kind = in[i].kind;
        out[i].val = in[i].val;
    }
    InterCodeList cur = bb->first;
    do {
        CP_IR_out(cur->code, out);
        cur = cur->next;
    } while (cur != bb->last->next);
}

void CP_IR_out(InterCode ir, CPPair* out) {
    assert(ir != NULL);
    switch (ir->kind) {
        case IR_LABEL:
        case IR_FUNC:
        case IR_GOTO:
        case IR_RETURN:
        case IR_ARG:
        case IR_WRITE:
        case IR_DEC:
        case IR_IF_GOTO:
            break;
        case IR_PARAM:
        case IR_READ:
            set_CP_out(ir->u.unary_ir.op, NAC, UNKNOWN, out);
            break;
        case IR_ASSIGN:
        case IR_CALL: {
            int left_id = get_variable_id(ir->u.binary_ir.left);
            int right_id = get_variable_id(ir->u.binary_ir.right);
            int right_kind = ir->u.binary_ir.right->kind;
            if (left_id != -1) {
                if (right_id != -1) {
                    if (out[right_id].kind == CONST) {
                        set_CP_out(ir->u.binary_ir.left, CONST, out[right_id].val, out);
                    } else if (out[right_id].kind == NAC) {
                        set_CP_out(ir->u.binary_ir.left, NAC, UNKNOWN, out);
                    } else {
                        set_CP_out(ir->u.binary_ir.left, UNDEF, UNKNOWN, out);
                    }
                } else if (right_kind == OP_CONSTANT) {
                    set_CP_out(ir->u.binary_ir.left, CONST, ir->u.binary_ir.right->u.const_val, out);
                } else {
                    set_CP_out(ir->u.binary_ir.left, NAC, UNKNOWN, out);
                }
            }
            break;
        }
        case IR_ADDR:
        case IR_LOAD:
        case IR_STORE:
            set_CP_out(ir->u.binary_ir.left, NAC, UNKNOWN, out);
            break;
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV: {
            int res_id = get_variable_id(ir->u.ternary_ir.res);
            int op1_id = get_variable_id(ir->u.ternary_ir.op1);
            int op2_id = get_variable_id(ir->u.ternary_ir.op2);
            int op1_kind = ir->u.ternary_ir.op1->kind;
            int op2_kind = ir->u.ternary_ir.op2->kind;
            if (res_id != -1) {
                if (op1_id != -1 && op2_id != -1) {  //操作数均为变量
                    if (out[op1_id].kind == CONST && out[op2_id].kind == CONST) {
                        long long val;
                        long long op1_val = out[op1_id].val, op2_val = out[op2_id].val;
                        if (ir->kind == IR_ADD) {
                            val = op1_val + op2_val;
                        } else if (ir->kind == IR_SUB) {
                            val = op1_val - op2_val;
                        } else if (ir->kind == IR_MUL) {
                            val = op1_val * op2_val;
                        } else if (ir->kind == IR_DIV) {
                            long long lhsVal = op1_val;
                            long long rhsVal = op2_val;
                            if (lhsVal < 0 && rhsVal > 0) {
                                val = (lhsVal - rhsVal + 1) / rhsVal;
                            } else if (lhsVal > 0 && rhsVal < 0) {
                                val = (lhsVal - rhsVal - 1) / rhsVal;
                            } else {
                                val = rhsVal ? lhsVal / rhsVal : 0;
                            }
                        }
                        set_CP_out(ir->u.ternary_ir.res, CONST, val, out);
                    } else if (out[op1_id].kind == NAC || out[op2_id].kind == NAC) {
                        set_CP_out(ir->u.ternary_ir.res, NAC, UNKNOWN, out);
                    } else {
                        set_CP_out(ir->u.ternary_ir.res, UNDEF, UNKNOWN, out);
                    }
                } else if (op1_id != -1 && op2_kind == OP_CONSTANT) {
                    if (out[op1_id].kind == CONST) {
                        long long val;
                        long long op1_val = out[op1_id].val, op2_val = ir->u.ternary_ir.op2->u.const_val;
                        if (ir->kind == IR_ADD) {
                            val = op1_val + op2_val;
                        } else if (ir->kind == IR_SUB) {
                            val = op1_val - op2_val;
                        } else if (ir->kind == IR_MUL) {
                            val = op1_val * op2_val;
                        } else if (ir->kind == IR_DIV) {
                            long long lhsVal = op1_val;
                            long long rhsVal = op2_val;
                            if (lhsVal < 0 && rhsVal > 0) {
                                val = (lhsVal - rhsVal + 1) / rhsVal;
                            } else if (lhsVal > 0 && rhsVal < 0) {
                                val = (lhsVal - rhsVal - 1) / rhsVal;
                            } else {
                                val = rhsVal ? lhsVal / rhsVal : 0;
                            }
                        }
                        set_CP_out(ir->u.ternary_ir.res, CONST, val, out);
                    } else if (out[op1_id].kind == UNDEF) {
                        set_CP_out(ir->u.ternary_ir.res, UNDEF, UNKNOWN, out);
                    } else {
                        set_CP_out(ir->u.ternary_ir.res, NAC, UNKNOWN, out);
                    }
                } else if (op2_id != -1 && op1_kind == OP_CONSTANT) {
                    if (out[op2_id].kind == CONST) {
                        long long val;
                        long long op1_val = ir->u.ternary_ir.op1->u.const_val, op2_val = out[op2_id].val;
                        if (ir->kind == IR_ADD) {
                            val = op1_val + op2_val;
                        } else if (ir->kind == IR_SUB) {
                            val = op1_val - op2_val;
                        } else if (ir->kind == IR_MUL) {
                            val = op1_val * op2_val;
                        } else if (ir->kind == IR_DIV) {
                            long long lhsVal = op1_val;
                            long long rhsVal = op2_val;
                            if (lhsVal < 0 && rhsVal > 0) {
                                val = (lhsVal - rhsVal + 1) / rhsVal;
                            } else if (lhsVal > 0 && rhsVal < 0) {
                                val = (lhsVal - rhsVal - 1) / rhsVal;
                            } else {
                                val = rhsVal ? lhsVal / rhsVal : 0;
                            }
                        }
                        set_CP_out(ir->u.ternary_ir.res, CONST, val, out);
                    } else if (out[op2_id].kind == UNDEF) {
                        set_CP_out(ir->u.ternary_ir.res, UNDEF, UNKNOWN, out);
                    } else {
                        set_CP_out(ir->u.ternary_ir.res, NAC, UNKNOWN, out);
                    }
                } else if (op1_kind == OP_CONSTANT && op2_kind == OP_CONSTANT) {
                    long long val;
                    long long op1_val = ir->u.ternary_ir.op1->u.const_val, op2_val = ir->u.ternary_ir.op2->u.const_val;
                    if (ir->kind == IR_ADD) {
                        val = op1_val + op2_val;
                    } else if (ir->kind == IR_SUB) {
                        val = op1_val - op2_val;
                    } else if (ir->kind == IR_MUL) {
                        val = op1_val * op2_val;
                    } else if (ir->kind == IR_DIV) {
                        long long lhsVal = op1_val;
                        long long rhsVal = op2_val;
                        if (lhsVal < 0 && rhsVal > 0) {
                            val = (lhsVal - rhsVal + 1) / rhsVal;
                        } else if (lhsVal > 0 && rhsVal < 0) {
                            val = (lhsVal - rhsVal - 1) / rhsVal;
                        } else {
                            val = rhsVal ? lhsVal / rhsVal : 0;
                        }
                    }
                    set_CP_out(ir->u.ternary_ir.res, CONST, val, out);
                } else {
                    set_CP_out(ir->u.ternary_ir.res, NAC, UNKNOWN, out);
                }
            }
            break;
        }
        default:
            assert(0);
            break;
    }
}

void CP() {
    var_nums = get_variable_count();
    for (int i = 0; i < fb_number; i++) {
        int bb_first = fb_array[i]->bb_first, bb_last = fb_array[i]->bb_last;
        int bb_nums = fb_array[i]->bb_nums;
        CPPair** IN = allocate_PAIR_matrix(bb_nums, var_nums);
        CPPair** OUT = allocate_PAIR_matrix(bb_nums, var_nums);
        // 迭代更新,趋近不动点
        bool change;
        do {
            change = false;
            for (int j = bb_first; j <= bb_last; j++) {
                assert(bb_array[j]->bb->bb_no == j);
                CP_meet(bb_first, j, IN, OUT);
                int bb_off = j - bb_first;
                change = change | CP_transfer(j, IN[bb_off], OUT[bb_off]);
            }
        } while (change);
#ifdef CP_DEBUG
        printf("--------------IN-------------------\n");
        dump_PAIR_matrix(IN, bb_nums, var_nums);
        printf("--------------OUT-------------------\n");
        dump_PAIR_matrix(OUT, bb_nums, var_nums);
#endif
        for (int j = bb_first; j <= bb_last; j++) {
            CP_BB_constant(bb_array[j]->bb, IN[j - bb_first]);
        }
        release_PAIR_matrix(IN, bb_nums);
        release_PAIR_matrix(OUT, bb_nums);
    }
}

void CP_meet(int bb_first, int bb_no, CPPair** IN, CPPair** OUT) {
    // IN[B]=U_{P a pre of B}OUT[P]
    BasicBlockList pre_head = bb_array[bb_no]->bb->pre;
    BasicBlockList cur = pre_head->next;
    for (BasicBlockList cur = pre_head->next; cur != pre_head; cur = cur->next) {
        int pre_no = cur->bb->bb_no;
        if (pre_no == ENTRY) continue;
        assert(pre_no != UNKNOWN || pre_no != EXIT);  // 每个BB都应有编号，EXIT不应有后继
        int bb_off = bb_no - bb_first, pre_off = pre_no - bb_first;
        pairset_union(IN[bb_off], OUT[pre_off], IN[bb_off], var_nums);
    }
#ifdef CP_DEBUG
    printf("bb%d-in after union\n", bb_no);
    dump_PAIR_matrix(&IN[bb_no - bb_first], 1, var_nums);
#endif
}

bool CP_transfer(int bb_no, CPPair* in_b, CPPair* out_b) {
    // OUT[B]=gen_B U (IN[B]-kill_B)
    CPPair* prev_out = (CPPair*)calloc(var_nums, sizeof(CPPair));
    for (int i = 0; i < var_nums; i++) {
        prev_out[i].kind = out_b[i].kind;
        prev_out[i].val = out_b[i].val;
    }
    CP_BB_out(bb_array[bb_no]->bb, in_b, out_b);
    bool change = pairset_diff(prev_out, out_b, var_nums);
    free(prev_out);
#ifdef CP_DEBUG
    printf("bb%d-out after transfer\n", bb_no);
    dump_PAIR_matrix(&out_b, 1, var_nums);
#endif
    return change;
}

void CP_BB_constant(BasicBlock bb, CPPair* in) {
    CPPair* prev_in = (CPPair*)calloc(var_nums, sizeof(CPPair));
    for (int i = 0; i < var_nums; i++) {
        prev_in[i].kind = in[i].kind;
        prev_in[i].val = in[i].val;
    }
    InterCodeList cur = bb->first;
    do {
        CP_IR_constant(cur, prev_in);
        cur = cur->next;
    } while (cur != bb->last->next);
    free(prev_in);
}
void CP_IR_constant(InterCodeList ir_point, CPPair* out) {
    assert(ir_point != NULL);
    InterCode ir = ir_point->code;
    switch (ir->kind) {
        case IR_LABEL:
        case IR_FUNC:
        case IR_GOTO:
        case IR_DEC:
        case IR_IF_GOTO:
            break;
        case IR_RETURN:
        case IR_ARG:
        case IR_WRITE: {
            int id = get_variable_id(ir->u.unary_ir.op);
            if (id != -1 && out[id].kind == CONST) {
                ir->u.unary_ir.op = gen_operand(OP_CONSTANT, out[id].val, -1, NULL);
            }
            break;
        }
        case IR_PARAM:
        case IR_READ:
            set_CP_out(ir->u.unary_ir.op, NAC, UNKNOWN, out);
            break;
        case IR_ASSIGN:
        case IR_CALL: {
            int left_id = get_variable_id(ir->u.binary_ir.left);
            int right_id = get_variable_id(ir->u.binary_ir.right);
            int right_kind = ir->u.binary_ir.right->kind;
            if (left_id != -1) {
                if (right_id != -1) {
                    if (out[right_id].kind == CONST) {
                        set_CP_out(ir->u.binary_ir.left, CONST, out[right_id].val, out);
                        Operand right_val = gen_operand(OP_CONSTANT, out[right_id].val, -1, NULL);
                        ir_point->code = gen_ir(NULL, IR_ASSIGN, ir->u.binary_ir.left, right_val, NULL, -1, NULL);
                    } else if (out[right_id].kind == NAC) {
                        set_CP_out(ir->u.binary_ir.left, NAC, UNKNOWN, out);
                    } else {
                        set_CP_out(ir->u.binary_ir.left, UNDEF, UNKNOWN, out);
                    }
                } else if (right_kind == OP_CONSTANT) {
                    set_CP_out(ir->u.binary_ir.left, CONST, ir->u.binary_ir.right->u.const_val, out);
                    Operand right_val = gen_operand(OP_CONSTANT, ir->u.binary_ir.right->u.const_val, -1, NULL);
                    ir_point->code = gen_ir(NULL, IR_ASSIGN, ir->u.binary_ir.left, right_val, NULL, -1, NULL);
                } else {
                    set_CP_out(ir->u.binary_ir.left, NAC, UNKNOWN, out);
                }
            }
            break;
        }
        case IR_ADDR:
        case IR_LOAD:
        case IR_STORE:
            set_CP_out(ir->u.binary_ir.left, NAC, UNKNOWN, out);
            break;
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV: {
            int res_id = get_variable_id(ir->u.ternary_ir.res);
            int op1_id = get_variable_id(ir->u.ternary_ir.op1);
            int op2_id = get_variable_id(ir->u.ternary_ir.op2);
            int op1_kind = ir->u.ternary_ir.op1->kind;
            int op2_kind = ir->u.ternary_ir.op2->kind;
            if (op1_id != -1 && out[op1_id].kind == CONST) {
                long long op1_val = out[op1_id].val;
                ir->u.ternary_ir.op1 = gen_operand(OP_CONSTANT, op1_val, -1, NULL);
            }
            if (op2_id != -1 && out[op2_id].kind == CONST) {
                long long op2_val = out[op2_id].val;
                ir->u.ternary_ir.op2 = gen_operand(OP_CONSTANT, op2_val, -1, NULL);
            }
            if (res_id != -1) {
                if (op1_id != -1 && op2_id != -1) {  //操作数均为变量
                    if (out[op1_id].kind == CONST && out[op2_id].kind == CONST) {
                        long long val;
                        long long op1_val = out[op1_id].val, op2_val = out[op2_id].val;
                        if (ir->kind == IR_ADD) {
                            val = op1_val + op2_val;
                        } else if (ir->kind == IR_SUB) {
                            val = op1_val - op2_val;
                        } else if (ir->kind == IR_MUL) {
                            val = op1_val * op2_val;
                        } else if (ir->kind == IR_DIV) {
                            long long lhsVal = op1_val;
                            long long rhsVal = op2_val;
                            if (lhsVal < 0 && rhsVal > 0) {
                                val = (lhsVal - rhsVal + 1) / rhsVal;
                            } else if (lhsVal > 0 && rhsVal < 0) {
                                val = (lhsVal - rhsVal - 1) / rhsVal;
                            } else {
                                val = rhsVal ? lhsVal / rhsVal : 0;
                            }
                        }
                        set_CP_out(ir->u.ternary_ir.res, CONST, val, out);
                        Operand right_val = gen_operand(OP_CONSTANT, val, -1, NULL);
                        ir_point->code = gen_ir(NULL, IR_ASSIGN, ir->u.ternary_ir.res, right_val, NULL, -1, NULL);
                    } else if (out[op1_id].kind == NAC || out[op2_id].kind == NAC) {
                        set_CP_out(ir->u.ternary_ir.res, NAC, UNKNOWN, out);
                    } else {
                        set_CP_out(ir->u.ternary_ir.res, UNDEF, UNKNOWN, out);
                    }
                } else if (op1_id != -1 && op2_kind == OP_CONSTANT) {
                    if (out[op1_id].kind == CONST) {
                        long long val;
                        long long op1_val = out[op1_id].val, op2_val = ir->u.ternary_ir.op2->u.const_val;
                        if (ir->kind == IR_ADD) {
                            val = op1_val + op2_val;
                        } else if (ir->kind == IR_SUB) {
                            val = op1_val - op2_val;
                        } else if (ir->kind == IR_MUL) {
                            val = op1_val * op2_val;
                        } else if (ir->kind == IR_DIV) {
                            long long lhsVal = op1_val;
                            long long rhsVal = op2_val;
                            if (lhsVal < 0 && rhsVal > 0) {
                                val = (lhsVal - rhsVal + 1) / rhsVal;
                            } else if (lhsVal > 0 && rhsVal < 0) {
                                val = (lhsVal - rhsVal - 1) / rhsVal;
                            } else {
                                val = rhsVal ? lhsVal / rhsVal : 0;
                            }
                        }
                        set_CP_out(ir->u.ternary_ir.res, CONST, val, out);
                        Operand right_val = gen_operand(OP_CONSTANT, val, -1, NULL);
                        ir_point->code = gen_ir(NULL, IR_ASSIGN, ir->u.ternary_ir.res, right_val, NULL, -1, NULL);
                    } else if (out[op1_id].kind == UNDEF) {
                        set_CP_out(ir->u.ternary_ir.res, UNDEF, UNKNOWN, out);
                    } else {
                        set_CP_out(ir->u.ternary_ir.res, NAC, UNKNOWN, out);
                    }
                } else if (op2_id != -1 && op1_kind == OP_CONSTANT) {
                    if (out[op2_id].kind == CONST) {
                        long long val;
                        long long op1_val = ir->u.ternary_ir.op1->u.const_val, op2_val = out[op2_id].val;
                        if (ir->kind == IR_ADD) {
                            val = op1_val + op2_val;
                        } else if (ir->kind == IR_SUB) {
                            val = op1_val - op2_val;
                        } else if (ir->kind == IR_MUL) {
                            val = op1_val * op2_val;
                        } else if (ir->kind == IR_DIV) {
                            long long lhsVal = op1_val;
                            long long rhsVal = op2_val;
                            if (lhsVal < 0 && rhsVal > 0) {
                                val = (lhsVal - rhsVal + 1) / rhsVal;
                            } else if (lhsVal > 0 && rhsVal < 0) {
                                val = (lhsVal - rhsVal - 1) / rhsVal;
                            } else {
                                val = rhsVal ? lhsVal / rhsVal : 0;
                            }
                        }
                        set_CP_out(ir->u.ternary_ir.res, CONST, val, out);
                        Operand right_val = gen_operand(OP_CONSTANT, val, -1, NULL);
                        ir_point->code = gen_ir(NULL, IR_ASSIGN, ir->u.ternary_ir.res, right_val, NULL, -1, NULL);
                    } else if (out[op2_id].kind == UNDEF) {
                        set_CP_out(ir->u.ternary_ir.res, UNDEF, UNKNOWN, out);
                    } else {
                        set_CP_out(ir->u.ternary_ir.res, NAC, UNKNOWN, out);
                    }
                } else if (op1_kind == OP_CONSTANT && op2_kind == OP_CONSTANT) {
                    long long val;
                    long long op1_val = ir->u.ternary_ir.op1->u.const_val, op2_val = ir->u.ternary_ir.op2->u.const_val;
                    if (ir->kind == IR_ADD) {
                        val = op1_val + op2_val;
                    } else if (ir->kind == IR_SUB) {
                        val = op1_val - op2_val;
                    } else if (ir->kind == IR_MUL) {
                        val = op1_val * op2_val;
                    } else if (ir->kind == IR_DIV) {
                        long long lhsVal = op1_val;
                        long long rhsVal = op2_val;
                        if (lhsVal < 0 && rhsVal > 0) {
                            val = (lhsVal - rhsVal + 1) / rhsVal;
                        } else if (lhsVal > 0 && rhsVal < 0) {
                            val = (lhsVal - rhsVal - 1) / rhsVal;
                        } else {
                            val = rhsVal ? lhsVal / rhsVal : 0;
                        }
                    }
                    set_CP_out(ir->u.ternary_ir.res, CONST, val, out);
                    Operand right_val = gen_operand(OP_CONSTANT, val, -1, NULL);
                    ir_point->code = gen_ir(NULL, IR_ASSIGN, ir->u.ternary_ir.res, right_val, NULL, -1, NULL);
                } else {
                    set_CP_out(ir->u.ternary_ir.res, NAC, UNKNOWN, out);
                }
            }
            break;
        }
        default:
            assert(0);
            break;
    }
}