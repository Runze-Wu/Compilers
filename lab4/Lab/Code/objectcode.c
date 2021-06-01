#include "objectcode.h"
int local_offset;            // 函数内部变量偏移量
int arg_num;                 // 函数调用传递的参数
VariableList local_varlist;  // 函数内部变量链表
char* reg_names[] = {"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
                     "s0",   "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"};
void gencode(FILE* code_out) {
    init_registers();
    init_environment(code_out);
    for (int i = 0; i < fb_number; i++) {
        init_varlist();
        initsymbol_fb(fb_array[i], code_out);
        // show_ir(bb_array[fb_array[i]->bb_first]->bb->first->code, stdout);
        // show_varlist();
        gencode_fb(fb_array[i], code_out);
        fprintf(code_out, "\n");
        release_varlist();
    }
}

void init_environment(FILE* code_out) {
    // head
    fprintf(code_out, ".data\n");
    fprintf(code_out, "_prompt: .asciiz \"Enter an integer:\"\n");
    fprintf(code_out, "_ret: .asciiz \"\\n\"\n");
    fprintf(code_out, ".globl main\n");

    // read
    fprintf(code_out, ".text\n");
    fprintf(code_out, "read:\n");
    fprintf(code_out, "  li $v0, 4\n");
    fprintf(code_out, "  la $a0, _prompt\n");
    fprintf(code_out, "  syscall\n");
    fprintf(code_out, "  li $v0, 5\n");
    fprintf(code_out, "  syscall\n");
    fprintf(code_out, "  jr $ra\n");
    fprintf(code_out, "\n");

    // write
    fprintf(code_out, "write:\n");
    fprintf(code_out, "  li $v0, 1\n");
    fprintf(code_out, "  syscall\n");
    fprintf(code_out, "  li $v0, 4\n");
    fprintf(code_out, "  la $a0, _ret\n");
    fprintf(code_out, "  syscall\n");
    fprintf(code_out, "  move $v0, $0\n");
    fprintf(code_out, "  jr $ra\n");
    fprintf(code_out, "\n");
}

void initsymbol_fb(FunctionBlock fb, FILE* code_out) {
    assert(fb && code_out);
    local_offset = 0;
    arg_num = 0;
    param_num = 0;
    for (int j = fb->bb_first; j <= fb->bb_last; j++) {
        initsymbol_bb(bb_array[j]->bb, code_out);
    }
}

void initsymbol_bb(BasicBlock bb, FILE* code_out) {
    assert(bb && code_out);
    InterCodeList cur = bb->first;
    do {
        initsymbol_ir(cur->code, code_out);
        cur = cur->next;
    } while (cur != bb->last->next);
}

void initsymbol_ir(InterCode ir, FILE* code_out) {
    assert(ir && code_out);
    switch (ir->kind) {
        case IR_LABEL:
        case IR_FUNC:
        case IR_GOTO:
            break;
        case IR_PARAM: {
            Variable var = (Variable)malloc(sizeof(struct Variable_));
            assert(var);
            var->reg_no = -1;
            var->op = ir->u.unary_ir.op;
            var->offset = 8 + 4 * param_num;
            insert_var(var);
            param_num++;
            break;
        }
        case IR_RETURN:
        case IR_ARG:
        case IR_READ:
        case IR_WRITE:
            insert_op(ir->u.unary_ir.op);
            break;
        case IR_DEC:
            local_offset -= (ir->u.dec.size - 4);
            insert_op(ir->u.dec.op);
            break;
        case IR_ASSIGN:
        case IR_ADDR:
        case IR_LOAD:
        case IR_STORE:
            insert_op(ir->u.binary_ir.left);
            insert_op(ir->u.binary_ir.right);
            break;
        case IR_CALL:
            insert_op(ir->u.binary_ir.left);
            break;
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
            insert_op(ir->u.ternary_ir.res);
            insert_op(ir->u.ternary_ir.op1);
            insert_op(ir->u.ternary_ir.op2);
            break;
        case IR_IF_GOTO:
            insert_op(ir->u.if_goto.x);
            insert_op(ir->u.if_goto.y);
            break;
        default:
            assert(0);
            break;
    }
}

void gencode_fb(FunctionBlock fb, FILE* code_out) {
    assert(fb && code_out);
    arg_num = 0;
    for (int j = fb->bb_first; j <= fb->bb_last; j++) {
        gencode_bb(bb_array[j]->bb, code_out);
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
            gen_ir_LABEL(ir, code_out);
            break;
        case IR_FUNC:
            gen_ir_FUNC(ir, code_out);
            break;
        case IR_GOTO:
            gen_ir_GOTO(ir, code_out);
            break;
        case IR_RETURN:
            gen_ir_RETURN(ir, code_out);
            break;
        case IR_ARG:
            gen_ir_ARG(ir, code_out);
            break;
        case IR_PARAM:
            gen_ir_PARAM(ir, code_out);
            break;
        case IR_READ:
            gen_ir_READ(ir, code_out);
            break;
        case IR_WRITE:
            gen_ir_WRITE(ir, code_out);
            break;
        case IR_DEC:
            gen_ir_DEC(ir, code_out);
            break;
        case IR_ASSIGN:
        case IR_ADDR:
            gen_ir_ASSIGN_ADDR(ir, code_out);
            break;
        case IR_LOAD:
        case IR_STORE:
            gen_ir_LOAD_STORE(ir, code_out);
            break;
        case IR_CALL:
            gen_ir_CALL(ir, code_out);
            break;
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
            gen_ir_ARITH(ir, code_out);
            break;
        case IR_IF_GOTO:
            gen_ir_IF_GOTO(ir, code_out);
            break;
        default:
            assert(0);
            break;
    }
}

void gen_ir_LABEL(InterCode ir, FILE* code_out) {
    assert(ir && code_out);
    assert(ir->kind == IR_LABEL);
    fprintf(code_out, "  label%d:\n", ir->u.unary_ir.op->u.label_no);
}

void gen_ir_FUNC(InterCode ir, FILE* code_out) {
    assert(ir && code_out);
    assert(ir->kind == IR_FUNC);
    fprintf(code_out, "%s:\n", ir->u.unary_ir.op->u.func_name);
    fprintf(code_out, "  move $fp, $sp\n");  // 初始化帧指针
    fprintf(code_out, "  addi $sp, $sp, %d\n", local_offset);
}

void gen_ir_GOTO(InterCode ir, FILE* code_out) {
    assert(ir && code_out);
    assert(ir->kind == IR_GOTO);
    fprintf(code_out, "  j label%d\n", ir->u.unary_ir.op->u.label_no);
}

void gen_ir_RETURN(InterCode ir, FILE* code_out) {
    assert(ir && code_out);
    assert(ir->kind == IR_RETURN);
    int reg = get_reg(ir->u.unary_ir.op, false, code_out);
    fprintf(code_out, "  move $v0, $%s\n", regs[reg].name);
    fprintf(code_out, "  jr $ra\n");
    clear_reg(reg);
}

void gen_ir_ARG(InterCode ir, FILE* code_out) {
    assert(ir && code_out);
    assert(ir->kind == IR_ARG);
    arg_num++;
    int reg = get_reg(ir->u.unary_ir.op, false, code_out);
    fprintf(code_out, "  addi $sp, $sp, -4\n");
    fprintf(code_out, "  sw $%s, 0($sp)\n", regs[reg].name);
    clear_reg(reg);
}

void gen_ir_PARAM(InterCode ir, FILE* code_out) { assert(ir && code_out); }

void gen_ir_READ(InterCode ir, FILE* code_out) {
    assert(ir && code_out);
    assert(ir->kind == IR_READ);
    // 保存返回地址
    fprintf(code_out, "  addi $sp, $sp, -4\n");
    fprintf(code_out, "  sw $ra, 0($sp)\n");
    // 调用read函数
    fprintf(code_out, "  jal read\n");
    // 恢复栈指针和返回地址
    fprintf(code_out, "  lw $ra, 0($sp)\n");
    fprintf(code_out, "  addi $sp, $sp, 4\n");
    int reg = get_reg(ir->u.unary_ir.op, true, code_out);
    fprintf(code_out, "  move $%s, $v0\n", regs[reg].name);
    store_reg(reg, code_out);
}

void gen_ir_WRITE(InterCode ir, FILE* code_out) {
    assert(ir && code_out);
    assert(ir->kind == IR_WRITE);
    int reg = get_reg(ir->u.unary_ir.op, false, code_out);
    fprintf(code_out, "  move $a0, $%s\n", regs[reg].name);
    // 保存返回地址
    fprintf(code_out, "  addi $sp, $sp, -4\n");
    fprintf(code_out, "  sw $ra, 0($sp)\n");
    // 调用write函数
    fprintf(code_out, "  jal write\n");
    // 恢复栈指针和返回地址
    fprintf(code_out, "  lw $ra, 0($sp)\n");
    fprintf(code_out, "  addi $sp, $sp, 4\n");
    clear_reg(reg);
}

void gen_ir_DEC(InterCode ir, FILE* code_out) { assert(ir && code_out); }

void gen_ir_ASSIGN_ADDR(InterCode ir, FILE* code_out) {
    assert(ir && code_out);
    int left = get_reg(ir->u.binary_ir.left, true, code_out);
    int right = get_reg(ir->u.binary_ir.right, false, code_out);
    assert(ir->kind == IR_ASSIGN || ir->kind == IR_ADDR);
    fprintf(code_out, "  move $%s, $%s\n", regs[left].name, regs[right].name);
    store_reg(left, code_out);
    clear_reg(right);
}

void gen_ir_LOAD_STORE(InterCode ir, FILE* code_out) {
    assert(ir && code_out);
    assert(ir->kind == IR_LOAD || ir->kind == IR_STORE);
    int left, right;
    if (ir->kind == IR_LOAD) {  // x = *y
        left = get_reg(ir->u.binary_ir.left, true, code_out);
        right = get_reg(ir->u.binary_ir.right, false, code_out);
        fprintf(code_out, "  lw $%s, 0($%s)\n", regs[left].name, regs[right].name);
        store_reg(left, code_out);
        clear_reg(right);
    } else {  // *x = y
        left = get_reg(ir->u.binary_ir.left, false, code_out);
        right = get_reg(ir->u.binary_ir.right, false, code_out);
        fprintf(code_out, "  sw $%s, 0($%s)\n", regs[right].name, regs[left].name);
        clear_reg(left);
        clear_reg(right);
    }
}

void gen_ir_CALL(InterCode ir, FILE* code_out) {
    assert(ir && code_out);
    assert(ir->kind == IR_CALL);
    // 保存帧指针和返回地址
    fprintf(code_out, "  addi $sp, $sp, -8\n");
    fprintf(code_out, "  sw $fp, 0($sp)\n");
    fprintf(code_out, "  sw $ra, 4($sp)\n");
    // 调用read函数
    fprintf(code_out, "  jal %s\n", ir->u.unary_ir.op->u.func_name);
    // 恢复栈帧指针和返回地址
    fprintf(code_out, "  move $sp, $fp\n");
    fprintf(code_out, "  lw $ra, 4($sp)\n");
    fprintf(code_out, "  lw $fp, 0($sp)\n");

    fprintf(code_out, "  addi $sp, $sp, %d\n", 8 + arg_num * 4);
    arg_num = 0;  //函数调用结束，传递参数个数清零
    int reg = get_reg(ir->u.binary_ir.left, true, code_out);
    fprintf(code_out, "  move $%s, $v0\n", regs[reg].name);
    store_reg(reg, code_out);
}

void gen_ir_ARITH(InterCode ir, FILE* code_out) {
    assert(ir && code_out);
    int res = get_reg(ir->u.ternary_ir.res, true, code_out);
    int op1 = get_reg(ir->u.ternary_ir.op1, false, code_out);
    int op2 = get_reg(ir->u.ternary_ir.op2, false, code_out);
    char* arith_op = NULL;
    switch (ir->kind) {
        case IR_ADD:
            arith_op = "add";
            break;
        case IR_SUB:
            arith_op = "sub";
            break;
        case IR_MUL:
            arith_op = "mul";
            break;
        case IR_DIV:
            arith_op = "div";
            break;
        default:
            assert(0);
            break;
    }
    if (ir->kind == IR_DIV) {
        fprintf(code_out, "  %s $%s,$%s\n", arith_op, regs[op1].name, regs[op2].name);
        fprintf(code_out, "  mflo $%s\n", regs[res].name);
    } else {
        fprintf(code_out, "  %s $%s, $%s, $%s\n", arith_op, regs[res].name, regs[op1].name, regs[op2].name);
    }
    store_reg(res, code_out);
    clear_reg(op1);
    clear_reg(op2);
}

void gen_ir_IF_GOTO(InterCode ir, FILE* code_out) {
    assert(ir && code_out);
    assert(ir->kind == IR_IF_GOTO);
    int x = get_reg(ir->u.if_goto.x, false, code_out);
    int y = get_reg(ir->u.if_goto.y, false, code_out);
    char* rel_op = NULL;
    if (strcmp(ir->u.if_goto.relop, "==") == 0) {
        rel_op = "beq";
    } else if (strcmp(ir->u.if_goto.relop, "!=") == 0) {
        rel_op = "bne";
    } else if (strcmp(ir->u.if_goto.relop, ">") == 0) {
        rel_op = "bgt";
    } else if (strcmp(ir->u.if_goto.relop, "<") == 0) {
        rel_op = "blt";
    } else if (strcmp(ir->u.if_goto.relop, ">=") == 0) {
        rel_op = "bge";
    } else if (strcmp(ir->u.if_goto.relop, "<=") == 0) {
        rel_op = "ble";
    }
    fprintf(code_out, "  %s $%s, $%s, label%d\n", rel_op, regs[x].name, regs[y].name, ir->u.if_goto.z->u.label_no);
    clear_reg(x);
    clear_reg(y);
}

void init_registers() {
    for (int i = 0; i < REGISTER_NUM; i++) {
        regs[i].name = reg_names[i];
        regs[i].state = FREE;
        regs[i].var = NULL;
    }
}

int get_reg(Operand op, bool left, FILE* code_out) {
    assert(op);
    int i;
    for (i = 8; i <= 15; i++) {  // 调用者保存的t0-t7寄存器随意使用
        if (regs[i].state == FREE) break;
    }
    assert(i != 16);  // 使用后立即写会内存，不应该找不到空闲寄存器
    regs[i].state = BUSY;
    if (op->kind == OP_CONSTANT) {
        fprintf(code_out, "  li $%s, %lld\n", regs[i].name, op->u.const_val);
    } else {
        Variable var = find_var(op);
        assert(var);
        var->reg_no = i;
        regs[i].var = var;
        switch (op->kind) {
            case OP_TEMP:
            case OP_VARIABLE:
            case OP_ADDRESS:
                if (!left) fprintf(code_out, "  lw $%s, %d($fp)\n", regs[i].name, var->offset);
                break;
            case OP_ARRAY:
                fprintf(code_out, "  addi $%s, $fp, %d\n", regs[i].name, var->offset);
                break;
        }
    }
    return i;
}

void init_varlist() {
    local_offset = 0;
    local_varlist = NULL;
}

void release_varlist() {
    VariableList cur = local_varlist;
    while (cur) {
        VariableList temp = cur;
        cur = cur->next;
        free(temp->var);
        free(temp);
    }
}

void show_varlist() {
    for (VariableList cur = local_varlist; cur; cur = cur->next) {
        printf("offset:%d\top:", cur->var->offset);
        show_op(cur->var->op, stdout);
        printf("\n");
    }
}

Variable find_var(Operand op) {
    assert(op);
    if (op->kind == OP_CONSTANT) return NULL;
    assert(op->kind != OP_FUNCTION && op->kind != OP_LABEL);
    for (VariableList cur = local_varlist; cur; cur = cur->next) {
        if (cur->var->op->kind != op->kind) continue;
        switch (op->kind) {
            case OP_TEMP:
                if (cur->var->op->u.temp_no == op->u.temp_no) return cur->var;
                break;
            case OP_ARRAY:
                if (cur->var->op->u.array_no == op->u.array_no) return cur->var;
                break;
            case OP_VARIABLE:
                if (cur->var->op->u.var_no == op->u.var_no) return cur->var;
                break;
            case OP_ADDRESS:
                if (cur->var->op->u.addr_no == op->u.addr_no) return cur->var;
                break;
            default:
                assert(0);
                break;
        }
    }
    return NULL;
}

void insert_var(Variable var) {
    VariableList temp = (VariableList)malloc(sizeof(struct VariableList_));
    assert(temp);
    temp->var = var;
    temp->next = local_varlist;
    local_varlist = temp;
}

void insert_op(Operand op) {
    assert(op);
    if (op->kind == OP_CONSTANT) return;
    if (!find_var(op)) {  // 不在符号表中
        local_offset -= 4;
        Variable var = (Variable)malloc(sizeof(struct Variable_));
        assert(var);
        var->reg_no = -1;
        var->op = op;
        var->offset = local_offset;
        insert_var(var);
    }
}

void store_reg(int reg_no, FILE* code_out) {
    assert(reg_no != -1);
    if (regs[reg_no].var) {  // 寄存器存有变量，否则寄存器可能存放的为立即数
        assert(regs[reg_no].var->offset != -1);
        fprintf(code_out, "  sw $%s, %d($fp)\n", regs[reg_no].name, regs[reg_no].var->offset);
    }
    clear_reg(reg_no);
}

void clear_reg(int reg_no) {
    regs[reg_no].state = FREE;
    regs[reg_no].var = NULL;
}