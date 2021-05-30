#include "objectcode.h"

void init_registers() {
    for (int i = 0; i < REGISTER_NUM; i++) {
        regs[i].name = reg_names[i];
        regs[i].state = FREE;
        regs[i].var = NULL;
    }
}