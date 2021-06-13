#include "intercode.h"

int temp_number = 0;   // 临时变量编号
int label_number = 0;  // 跳转编号
int addr_number = 0;   // 地址编号
extern int translator_debug;
extern int optimizer_debug;
extern InterCodeList* label_array;

InterCodeList init_ir_list() {
    InterCodeList ir_list_head = (InterCodeList)malloc(sizeof(struct InterCodeList_));
    assert(ir_list_head != NULL);
    ir_list_head->code = NULL;
    ir_list_head->prev = ir_list_head->next = ir_list_head;
    return ir_list_head;
}

void add_ir(InterCodeList ir_list_head, InterCode ir) {
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

void delete_ir(InterCodeList ir) {
    if (ir == NULL) return;
    InterCodeList prev = ir->prev;
    InterCodeList next = ir->next;
    assert(prev != next && ir != next && ir != prev);
    // 不应出现只有一条指令或者只有头部的情况
    prev->next = next;
    next->prev = prev;
}

void show_ir_list(InterCodeList ir_list_head, FILE* ir_out) {
    label_array = (InterCodeList*)malloc(sizeof(InterCodeList) * label_number);
    InterCodeList cur = ir_list_head->next;
    for (int i = 0; i < label_number; i++) label_array[i] = NULL;
    while (cur != ir_list_head) {
        assert(cur->code != NULL);
        show_ir(cur->code, ir_out);
        if (cur->code->kind == IR_LABEL) label_array[cur->code->u.unary_ir.op->u.label_no] = cur;
        cur = cur->next;
    }
    for (int i = 0; i < label_number; i++) {
        if (label_array[i]) {
            if (optimizer_debug) show_ir(label_array[i]->code, stdout);
        }
    }
}

void show_ir(InterCode ir, FILE* ir_out) {
    if (ir == NULL || ir_out == NULL) return;
    switch (ir->kind) {
        case IR_LABEL:
            fprintf(ir_out, "LABEL ");
            show_op(ir->u.unary_ir.op, ir_out);
            fprintf(ir_out, ": ");
            break;
        case IR_FUNC:
            fprintf(ir_out, "FUNCTION ");
            show_op(ir->u.unary_ir.op, ir_out);
            fprintf(ir_out, ": ");
            break;
        case IR_GOTO:
            fprintf(ir_out, "GOTO ");
            show_op(ir->u.unary_ir.op, ir_out);
            break;
        case IR_RETURN:
            fprintf(ir_out, "RETURN ");
            show_op(ir->u.unary_ir.op, ir_out);
            break;
        case IR_ARG:
            fprintf(ir_out, "ARG ");
            show_op(ir->u.unary_ir.op, ir_out);
            break;
        case IR_PARAM:
            fprintf(ir_out, "PARAM ");
            show_op(ir->u.unary_ir.op, ir_out);
            break;
        case IR_READ:
            fprintf(ir_out, "READ ");
            show_op(ir->u.unary_ir.op, ir_out);
            break;
        case IR_WRITE:
            fprintf(ir_out, "WRITE ");
            show_op(ir->u.unary_ir.op, ir_out);
            break;
        case IR_DEC:
            fprintf(ir_out, "DEC ");
            show_op(ir->u.dec.op, ir_out);
            fprintf(ir_out, "%d ", ir->u.dec.size);
            break;
        case IR_ASSIGN:
            show_op(ir->u.binary_ir.left, ir_out);
            fprintf(ir_out, ":= ");
            show_op(ir->u.binary_ir.right, ir_out);
            break;
        case IR_ADDR:
            show_op(ir->u.binary_ir.left, ir_out);
            fprintf(ir_out, ":= &");
            show_op(ir->u.binary_ir.right, ir_out);
            break;
        case IR_LOAD:
            show_op(ir->u.binary_ir.left, ir_out);
            fprintf(ir_out, ":= *");
            show_op(ir->u.binary_ir.right, ir_out);
            break;
        case IR_STORE:
            fprintf(ir_out, "*");
            show_op(ir->u.binary_ir.left, ir_out);
            fprintf(ir_out, ":= ");
            show_op(ir->u.binary_ir.right, ir_out);
            break;
        case IR_CALL:
            show_op(ir->u.binary_ir.left, ir_out);
            fprintf(ir_out, ":= CALL ");
            show_op(ir->u.binary_ir.right, ir_out);
            break;
        case IR_ADD:
            show_op(ir->u.ternary_ir.res, ir_out);
            fprintf(ir_out, ":= ");
            show_op(ir->u.ternary_ir.op1, ir_out);
            fprintf(ir_out, "+ ");
            show_op(ir->u.ternary_ir.op2, ir_out);
            break;
        case IR_SUB:
            show_op(ir->u.ternary_ir.res, ir_out);
            fprintf(ir_out, ":= ");
            show_op(ir->u.ternary_ir.op1, ir_out);
            fprintf(ir_out, "- ");
            show_op(ir->u.ternary_ir.op2, ir_out);
            break;
        case IR_MUL:
            show_op(ir->u.ternary_ir.res, ir_out);
            fprintf(ir_out, ":= ");
            show_op(ir->u.ternary_ir.op1, ir_out);
            fprintf(ir_out, "* ");
            show_op(ir->u.ternary_ir.op2, ir_out);
            break;
        case IR_DIV:
            show_op(ir->u.ternary_ir.res, ir_out);
            fprintf(ir_out, ":= ");
            show_op(ir->u.ternary_ir.op1, ir_out);
            fprintf(ir_out, "/ ");
            show_op(ir->u.ternary_ir.op2, ir_out);
            break;
        case IR_IF_GOTO:
            fprintf(ir_out, "IF ");
            show_op(ir->u.if_goto.x, ir_out);
            fprintf(ir_out, "%s ", ir->u.if_goto.relop);
            show_op(ir->u.if_goto.y, ir_out);
            fprintf(ir_out, "GOTO ");
            show_op(ir->u.if_goto.z, ir_out);
            break;
        default:
            assert(0);
            break;
    }
    fprintf(ir_out, "\n");
}

void show_op(Operand op, FILE* ir_out) {
    if (op == NULL) return;
    switch (op->kind) {
        case OP_VARIABLE:
            fprintf(ir_out, "var%d ", op->u.var_no);
            break;
        case OP_ADDRESS:
            fprintf(ir_out, "addr%d ", op->u.addr_no);
            break;
        case OP_FUNCTION:
            fprintf(ir_out, "%s ", op->u.func_name);
            break;
        case OP_ARRAY:
            fprintf(ir_out, "array%d ", op->u.array_no);
            break;
        case OP_STRUCTURE:
            dump_structure_err();
            break;
        case OP_LABEL:
            fprintf(ir_out, "label%d ", op->u.label_no);
            break;
        case OP_TEMP:
            fprintf(ir_out, "t%d ", op->u.temp_no);
            break;
        case OP_CONSTANT:
            fprintf(ir_out, "#%lld ", op->u.const_val);
            break;
        default:
            assert(0);
            break;
    }
}

InterCode gen_ir(InterCodeList ir_list_head, int ir_kind, Operand op1, Operand op2, Operand op3, int dec_size,
                 char* relop) {
    InterCode res_ir = (InterCode)malloc(sizeof(struct InterCode_));
    assert(res_ir != NULL);
    res_ir->kind = ir_kind;
    res_ir->bb_start = false;
    switch (ir_kind) {
        case IR_LABEL:
        case IR_FUNC:
        case IR_GOTO:
        case IR_RETURN:
        case IR_ARG:
        case IR_PARAM:
        case IR_READ:
        case IR_WRITE:
            if (op1 == NULL) {
                free(res_ir);
                res_ir = NULL;
            } else {
                res_ir->u.unary_ir.op = op1;
            }
            break;
        case IR_DEC:
            if (op1 == NULL) {
                free(res_ir);
                res_ir = NULL;
            } else {
                res_ir->u.dec.op = op1;
                res_ir->u.dec.size = dec_size;
            }
            break;
        case IR_ASSIGN:
        case IR_ADDR:
        case IR_LOAD:
        case IR_STORE:
        case IR_CALL:
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
        case IR_IF_GOTO:
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
    if (translator_debug) show_ir(res_ir, stdout);
    if (ir_list_head) add_ir(ir_list_head, res_ir);
    return res_ir;
}

Operand gen_operand(int operand_kind, long long val, int number, char* name) {
    Operand res_op = (Operand)malloc(sizeof(struct Operand_));
    assert(res_op != NULL);
    res_op->kind = operand_kind;
    FieldList result;
    switch (operand_kind) {
        case OP_ADDRESS:
            res_op->u.addr_no = number;
            break;
        case OP_FUNCTION:
            result = look_up(name);
            assert(result != NULL && result->type->kind == FUNCTION);
            if (strcmp("main", name) == 0) {
                res_op->u.func_name = name;
            } else {
                res_op->u.func_name = (char*)malloc(strlen(name) + 6);
                sprintf(res_op->u.func_name, "func_%s", name);
            }

            break;
        case OP_ARRAY:
            result = look_up(name);
            assert(result != NULL && result->type->kind == ARRAY);
            res_op->u.array_no = result->id;
            break;
        case OP_STRUCTURE:
            dump_structure_err();
            break;
        case OP_VARIABLE:
            result = look_up(name);
            assert(result != NULL && result->type->kind == BASIC);
            res_op->u.var_no = result->id;
            break;
        case OP_LABEL:
            res_op->u.label_no = number;
            break;
        case OP_TEMP:
            res_op->u.temp_no = number;
            break;
        case OP_CONSTANT:
            res_op->u.const_val = val;
            break;
        default:
            assert(0);
            break;
    }
    return res_op;
}

ArgList init_arg_list() {
    ArgList arg_list_head = (ArgList)malloc(sizeof(struct ArgList_));
    assert(arg_list_head != NULL);
    arg_list_head->arg = NULL;
    arg_list_head->prev = arg_list_head->next = arg_list_head;
    return arg_list_head;
}

void add_arg(ArgList head, Operand arg) {
    if (arg == NULL) return;
    ArgList new_term = (ArgList)malloc(sizeof(struct ArgList_));
    assert(new_term != NULL);
    new_term->arg = arg;
    ArgList tail = head->prev;
    tail->next = new_term;
    new_term->prev = tail;
    head->prev = new_term;
    new_term->next = head;
}

Operand new_temp() { return gen_operand(OP_TEMP, -1, temp_number++, NULL); }

Operand new_addr() { return gen_operand(OP_ADDRESS, -1, addr_number++, NULL); }

Operand new_label() { return gen_operand(OP_LABEL, -1, label_number++, NULL); }