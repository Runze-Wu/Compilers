#include "objectcode.h"
int local_offset;  // 函数内部变量偏移量

void gencode(FILE* code_out) {
    init_registers();
    for (int i = 0; i < fb_number; i++) {
        gencode_fb(fb_array[i], code_out);
    }
}

void gencode_fb(FunctionBlock fb, FILE* code_out) {
    assert(fb && code_out);
    local_offset = 0;
    for (int j = fb->bb_first; j <= fb->bb_last; j++) {
        gencode_bb(bb_array[j], code_out);
    }
}

void gencode_bb(BasicBlock bb, FILE* code_out) {
    assert(bb && code_out);
    InterCodeList cur = bb->first;
    do {
        gencode_ir(cur->code, code_out);
        cur = cur->next;
    } while (cur != bb->last->next);
}

void gencode_ir(InterCode ir, FILE* code_out) {
    assert(ir && code_out);
    switch (ir->kind) {
        case IR_LABEL:
            break;
        case IR_FUNC:
            break;
        case IR_GOTO:
            break;
        case IR_RETURN:
            break;
        case IR_ARG:
            break;
        case IR_PARAM:
            break;
        case IR_READ:
            break;
        case IR_WRITE:
            break;
        case IR_DEC:
            break;
        case IR_ASSIGN:
            break;
        case IR_ADDR:
            break;
        case IR_LOAD:
            break;
        case IR_STORE:
            break;
        case IR_CALL:
            break;
        case IR_ADD:
            break;
        case IR_SUB:
            break;
        case IR_MUL:
            break;
        case IR_DIV:
            break;
        case IR_IF_GOTO:
            break;
        default:
            assert(0);
            break;
    }
}

void init_registers() {
    for (int i = 0; i < REGISTER_NUM; i++) {
        regs[i].name = reg_names[i];
        regs[i].state = FREE;
        regs[i].var = NULL;
    }
}