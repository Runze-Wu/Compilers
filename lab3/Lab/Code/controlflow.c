#include "controlflow.h"

unsigned int bb_number = 0;                // BB块编号
extern int optimizer_debug;                // optimizer debug mode
extern InterCodeList global_ir_list_head;  // 循环双向链表头
extern BasicBlockList global_bb_list;      // 基本块循环双向链表头

void optimize() {
    global_bb_list = init_bb_list();
    construct_bb_list(global_bb_list, global_ir_list_head);
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

void construct_bb_list(BasicBlockList bb_list_head, InterCodeList ir_list_head) {
    InterCodeList prev_first = NULL;
    bb_tag_ir_list(ir_list_head);  // 进行BB的标记
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
    construct_bb(bb_list_head, prev_first, ir_list_head->prev);
}

void bb_tag_ir_list(InterCodeList ir_list_head) {
    InterCodeList cur = ir_list_head->next;
    bb_tag_ir(cur->code);  // step1: 第一条指令标记为BB开始
    while (cur != ir_list_head) {
        assert(cur->code != NULL);
        if (cur->code->kind == IR_IF_GOTO || cur->code->kind == IR_GOTO) {
            // step3: 跳转语句的下一条指令为BB开始
            if (cur != ir_list_head) {
                bb_tag_ir(cur->next->code);
            }
        } else if (cur->code->kind == IR_LABEL) {
            // step2: 跳转语句的目标指令为BB开始
            bb_tag_ir(cur->code);
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
    bb->pre = bb->suc = NULL;  // 前继后继设为NULL
    bb->first = first;
    bb->last = last;
    bb->bb_no = bb_number++;
    if (optimizer_debug) {
        printf("new bb:%d\n", bb->bb_no);
        printf("first ir:\t");
        show_ir(first->code, stdout);
        printf("next ir:\t");
        show_ir(last->code, stdout);
    }
    add_bb(bb_list_head, bb);
}

void show_bb_list(BasicBlockList bb_list_head, FILE* ir_out) {
    BasicBlockList cur = bb_list_head->next;
    while (cur != bb_list_head) {
        assert(cur->bb);
        if (optimizer_debug) fprintf(ir_out, "-------basic block%d-------\n", cur->bb->bb_no);
        show_bb(cur->bb, ir_out);
        cur = cur->next;
    }
}

void show_bb(BasicBlock bb, FILE* ir_out) {
    if (bb == NULL) return;
    InterCodeList cur = bb->first;
    do {
        show_ir(cur->code, ir_out);
        cur = cur->next;
    } while (cur != bb->last->next);
}
