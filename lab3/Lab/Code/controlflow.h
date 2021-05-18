#ifndef __CONTROLFLOW_H__
#define __CONTROLFLOW_H__
/**
 * BasicBlock和ControlFlowGraph建立
 * BB内局部优化：局部公共子表达式消除，消除死代码
 */
#include "intercode.h"

typedef struct BasicBlockList_* BasicBlockList;
typedef struct BasicBlock_* BasicBlock;
unsigned int bb_number;    // BB块编号
BasicBlockList* bb_array;  // BB链表数组，便于索引

struct BasicBlockList_ {  // 基本块链表
    BasicBlock bb;
    BasicBlockList prev, next;
};
struct BasicBlock_ {
    unsigned int bb_no;         // BB块编号
    BasicBlockList pre, suc;    // 前继，后继BB链表
    InterCodeList first, last;  // BB的首指令和尾指令
};
void optimize();                                                                  // 代码优化接口
void remove_redundant_label(InterCodeList ir_list_head, InterCodeList* labels);   // 删除冗余的label
BasicBlockList init_bb_list();                                                    //初始化BB双向链表头
void add_bb(BasicBlockList bb_list_head, BasicBlock bb);                          // 将bb加到list尾部
void bb_tag_ir_list(InterCodeList ir_list_head);                                  // 给ir_list标记BB开始
void bb_tag_ir(InterCode ir);                                                     // 给ir标记为基本块开始
bool is_bb_start(InterCode ir);                                                   // 判断是否是BB开始
void construct_bb_list(BasicBlockList bb_list_head, InterCodeList ir_list_head);  // 创建BB链表
void construct_bb(BasicBlockList bb_list_head, InterCodeList first, InterCodeList last);  // 创建BB
void construct_bb_array(BasicBlockList bb_list_head);                                     //创建BB数组
void construct_cfg(BasicBlockList* bbs, InterCodeList* labels);                           //创建CFG
void show_cfg(BasicBlockList* bbs);                                                       // 输出CFG
void show_bb_list(BasicBlockList bb_list_head, FILE* ir_out);                             // 输出BB链表中的指令
void show_bb(BasicBlock bb, FILE* ir_out);                                                // 输出BB中的指令
#endif