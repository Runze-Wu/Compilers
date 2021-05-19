#include "controlflow.h"

unsigned int bb_number = 0;                // BB块编号
BasicBlockList* bb_array;                  // BB链表数组，便于索引
extern int optimizer_debug;                // optimizer debug mode
extern InterCodeList* label_array;         // 所有编号的数组
extern InterCodeList global_ir_list_head;  // 循环双向链表头
extern BasicBlockList global_bb_list;      // 基本块循环双向链表头

void optimize() {
    remove_redundant_label(global_ir_list_head, label_array);
    global_bb_list = init_bb_list();
    construct_bb_list(global_bb_list, global_ir_list_head, label_array);
}

void remove_redundant_label(InterCodeList ir_list_head, InterCodeList* labels) {
    InterCodeList cur = ir_list_head->next;
    while (cur != ir_list_head) {
        assert(cur->code != NULL);
        if (cur->code->kind == IR_LABEL) {
            assert(cur->code->u.unary_ir.op != NULL);
            unsigned int label_num = cur->code->u.unary_ir.op->u.number;
            InterCodeList next = cur->next;
            while (next != ir_list_head) {
                assert(next->code != NULL);
                if (next->code->kind != IR_LABEL) break;
                delete_ir(next);
                next->code->u.unary_ir.op->u.number = label_num;
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
    return bb_list_head;
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

void construct_bb_list(BasicBlockList bb_list_head, InterCodeList ir_list_head, InterCodeList* labels) {
    InterCodeList prev_first = NULL;
    bb_tag_ir_list(ir_list_head, labels);  // 进行BB的标记
    InterCodeList cur = ir_list_head->next;
    while (cur != ir_list_head) {
        assert(cur->code != NULL);
        if (is_bb_start(cur->code)) {
            if (prev_first) {  // 第二次遇到开始指令
                assert(prev_first != cur);
                construct_bb(bb_list_head, prev_first, cur->prev);
            }
            prev_first = cur;
        }
        cur = cur->next;
    }
    assert(prev_first != NULL);
    construct_bb(bb_list_head, prev_first, ir_list_head->prev);  // 最后一个BB块

    construct_bb_array(bb_list_head);  // 构建CFG
    construct_cfg(bb_array, label_array);
    show_cfg(bb_array);
}

void bb_tag_ir_list(InterCodeList ir_list_head, InterCodeList* labels) {
    InterCodeList cur = ir_list_head->next;
    bb_tag_ir(cur->code);  // step1: 第一条指令标记为BB开始
    while (cur != ir_list_head) {
        assert(cur->code != NULL);
        if (cur->code->kind == IR_IF_GOTO || cur->code->kind == IR_GOTO) {
            unsigned int label_num = -1;
            if (cur->code->kind == IR_GOTO) {  //跳转目标
                assert(cur->code->u.unary_ir.op != NULL);
                label_num = cur->code->u.unary_ir.op->u.number;
            } else {
                assert(cur->code->u.if_goto.z != NULL);
                label_num = cur->code->u.if_goto.z->u.number;
            }
            assert(label_num != -1 && label_num < label_number);
            // step2: 跳转语句的目标指令为BB开始
            bb_tag_ir(labels[label_num]->code);
            // step3: 跳转语句的下一条指令为BB开始
            if (cur != ir_list_head) {
                bb_tag_ir(cur->next->code);
            }
        } else if (cur->code->kind == IR_FUNC) {
            // step4: 函数定义指令为BB开始
            bb_tag_ir(cur->code);
        }
        cur = cur->next;
    }
}

void bb_tag_ir(InterCode ir) {
    assert(ir != NULL);
    ir->bb_start = true;
}

bool is_bb_start(InterCode ir) {
    assert(ir != NULL);
    return ir->bb_start;
}

void construct_bb(BasicBlockList bb_list_head, InterCodeList first, InterCodeList last) {
    assert(first && last);
    BasicBlock bb = (BasicBlock)malloc(sizeof(struct BasicBlock_));
    bb->pre = init_bb_list();
    bb->suc = init_bb_list();  // 前继后继设为空的头结点
    bb->first = first;
    bb->last = last;
    bb->bb_no = bb_number++;
    InterCodeList cur = bb->first;
    do {
        cur->code->bb_no = bb_number - 1;  //记录所在的BB
        cur = cur->next;
    } while (cur != bb->last->next);
    if (optimizer_debug) {
        printf("new bb:%d\n", bb->bb_no);
        printf("first ir:\t");
        show_ir(first->code, stdout);
        printf("next ir:\t");
        show_ir(last->code, stdout);
    }
    add_bb(bb_list_head, bb);
}

void construct_bb_array(BasicBlockList bb_list_head) {
    bb_array = (BasicBlockList*)malloc(sizeof(BasicBlockList) * bb_number);
    BasicBlockList cur = bb_list_head->next;
    int i = 0;
    while (cur != bb_list_head) {
        bb_array[i++] = cur;
        cur = cur->next;
    }
}

void construct_cfg(BasicBlockList* bbs, InterCodeList* labels) {
    for (int i = 0; i < bb_number; i++) {
        BasicBlockList cur = bbs[i];
        assert(cur->bb->last->code != NULL);
        unsigned int label_num = -1;
        if (cur->bb->last->code->kind != IR_GOTO) {  // 直接后继
            if (i < bb_number - 1) {
                add_bb(cur->bb->suc, cur->next->bb);  // 最后一块没有直接后继
                add_bb(cur->next->bb->pre, cur->bb);  // 设置前驱
            }
        }
        if (cur->bb->last->code->kind == IR_GOTO) {  //跳转后继
            assert(cur->bb->last->code->u.unary_ir.op != NULL);
            label_num = cur->bb->last->code->u.unary_ir.op->u.number;
        } else if (cur->bb->last->code->kind == IR_IF_GOTO) {
            assert(cur->bb->last->code->u.if_goto.z != NULL);
            label_num = cur->bb->last->code->u.if_goto.z->u.number;
        }
        if (label_num != -1) {
            unsigned int goto_bb = labels[label_num]->code->bb_no;
            add_bb(cur->bb->suc, bbs[goto_bb]->bb);
            add_bb(bbs[goto_bb]->bb->pre, cur->bb);
        }
    }
}

void show_cfg(BasicBlockList* bbs) {
    for (int i = 0; i < bb_number; i++) {
        BasicBlockList cur = bbs[i];
        if (optimizer_debug) printf("-------basic block%d,next:-------\n", cur->bb->bb_no);
        show_bb_list(cur->bb->suc, stdout);
        if (optimizer_debug) printf("-------basic block%d,prev:-------\n", cur->bb->bb_no);
        show_bb_list(cur->bb->pre, stdout);
    }
}

void show_bb_list(BasicBlockList bb_list_head, FILE* ir_out) {
    BasicBlockList cur = bb_list_head->next;
    while (cur != bb_list_head) {
        assert(cur->bb);
        if (optimizer_debug && ir_out) fprintf(ir_out, "-------basic block%d-------\n", cur->bb->bb_no);
        show_bb(cur->bb, ir_out);
        cur = cur->next;
    }
}

void show_bb(BasicBlock bb, FILE* ir_out) {
    if (bb == NULL || ir_out == NULL) return;
    InterCodeList cur = bb->first;
    do {
        if (optimizer_debug || ir_out != stdout) show_ir(cur->code, ir_out);
        cur = cur->next;
    } while (cur != bb->last->next);
}
