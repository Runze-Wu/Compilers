#include "intercode.h"

unsigned int temp_number = 0;   // 临时变量编号
unsigned int label_number = 0;  // 跳转编号

void init_ir_list() {
    ir_list_head = (InterCodeList)malloc(sizeof(struct InterCodeList_));
    assert(ir_list_head != NULL);
    ir_list_head->prev = ir_list_head->next = ir_list_head;
}

void add_ir(InterCode ir) {
    if (ir == NULL) return;
    InterCodeList new_term = (InterCodeList)malloc(sizeof(struct InterCodeList_));
    assert(new_term != NULL);
    new_term->code = ir;
    InterCodeList tail = ir_list_head->prev;
    tail->next = new_term;
    new_term->prev = tail;
    ir_list_head->prev = new_term;
    new_term->next = ir_list_head;
}

void show_ir_list() {
    InterCodeList cur = ir_list_head->next;
    while (cur != ir_list_head) {
        show_ir(cur->code);
        cur = cur->next;
    }
}

void show_ir(InterCode ir) {
    if (ir == NULL) return;
    switch (ir->kind) {
        case DEF_LABEL:
        case DEF_FUNC:
        case GOTO:
        case IR_RETURN:
        case ARG:
        case PARAM:
        case READ:
        case WRITE:
            show_op(ir->u.unary_ir.op);
            break;
        case DEC:
            show_op(ir->u.dec.op);
            // size
            break;
        case ASSIGN:
        case GET_ADDR:
        case LOAD:
        case STORE:
            show_op(ir->u.binary_ir.left);
            show_op(ir->u.binary_ir.right);
            break;
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
            show_op(ir->u.ternary_ir.res);
            show_op(ir->u.ternary_ir.op1);
            show_op(ir->u.ternary_ir.op2);
            break;
        case IF_GOTO:
            show_op(ir->u.if_goto.x);
            show_op(ir->u.if_goto.y);
            show_op(ir->u.if_goto.z);
            // relop
            break;
        default:
            assert(0);
            break;
    }
}

void show_op(Operand op) {
    if (op == NULL) return;
    switch (op->kind) {
        case VARIABLE:
        case ADDRESS:
        case OP_FUNCTION:
        case OP_ARRAY:
        case OP_STRUCTURE:
            // name
            break;
        case LABEL:
        case TEMP:
            // number
            break;
        case CONSTANT:
            // val
            break;
        default:
            assert(0);
            break;
    }
}

void gen_ir(int ir_kind, Operand op1, Operand op2, Operand op3, int dec_size, char* relop) {
    InterCode res_ir = (InterCode)malloc(sizeof(struct InterCode_));
    assert(res_ir != NULL);
    res_ir->kind = ir_kind;
    switch (ir_kind) {
        case DEF_LABEL:
        case DEF_FUNC:
        case GOTO:
        case IR_RETURN:
        case ARG:
        case PARAM:
        case READ:
        case WRITE:
            if (op1 == NULL) {
                free(res_ir);
                res_ir = NULL;
            } else {
                res_ir->u.unary_ir.op = op1;
            }
            break;
        case DEC:
            if (op1 == NULL) {
                free(res_ir);
                res_ir = NULL;
            } else {
                res_ir->u.dec.op = op1;
                res_ir->u.dec.size = dec_size;
            }
            break;
        case ASSIGN:
        case GET_ADDR:
        case LOAD:
        case STORE:
        case CALL:
            if (op1 == NULL || op2 == NULL) {
                free(res_ir);
                res_ir = NULL;
            } else {
                res_ir->u.binary_ir.left = op1;
                res_ir->u.binary_ir.right = op2;
            }
            break;
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
            if (op1 == NULL || op2 == NULL || op3 == NULL) {
                free(res_ir);
                res_ir = NULL;
            } else {
                res_ir->u.ternary_ir.res = op1;
                res_ir->u.ternary_ir.op1 = op2;
                res_ir->u.ternary_ir.op2 = op3;
            }
            break;
        case IF_GOTO:
            if (op1 == NULL || op2 == NULL || op3 == NULL) {
                free(res_ir);
                res_ir = NULL;
            } else {
                res_ir->u.if_goto.x = op1;
                res_ir->u.if_goto.y = op2;
                res_ir->u.if_goto.z = op3;
                strcpy(res_ir->u.if_goto.relop, relop);
            }
            break;
        default:
            assert(0);
            break;
    }
    add_ir(res_ir);
}

Operand gen_operand(int operand_kind, int val, int number, char* name) {
    Operand res_op = (Operand)malloc(sizeof(struct Operand_));
    assert(res_op != NULL);
    res_op->kind = operand_kind;
    switch (operand_kind) {
        case VARIABLE:
        case ADDRESS:
        case OP_FUNCTION:
        case OP_ARRAY:
        case OP_STRUCTURE:
            res_op->u.name = name;
            break;
        case LABEL:
        case TEMP:
            res_op->u.number = number;
            break;
        case CONSTANT:
            res_op->u.const_val = val;
            break;
        default:
            assert(0);
            break;
    }
    return res_op;
}

Operand new_temp() { return gen_operand(TEMP, -1, temp_number++, NULL); }

Operand new_label() { return gen_operand(LABEL, -1, label_number++, NULL); }