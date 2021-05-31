#ifndef __CONTROLFLOW_H__
#define __CONTROLFLOW_H__
/**
 * BasicBlock和ControlFlowGraph建立
 * BB内局部优化：局部公共子表达式消除，消除死代码
 */
#include "intercode.h"

#define UNKNOWN -1
#define ENTRY -10
#define EXIT -100
// #define LVA_DEBUG
// #define CP_DEBUG
#define USE 1
#define DEF 2
#define BOOL 0
#define CPPAIR 1

typedef struct FunctionBlock_* FunctionBlock;
typedef struct BasicBlockList_* BasicBlockList;
typedef struct BasicBlock_* BasicBlock;
typedef struct CPPair_ CPPair;
int global_bb_count;       // BB块个数
int global_fb_count;       // FB块个数
int fb_number;             // FB块编号
int var_nums;              // 变量加临时变量个数
BasicBlockList* bb_array;  // BB链表数组，便于索引
FunctionBlock* fb_array;   // FB数组，便于索引

struct CPPair_ {
    enum { UNDEF, NAC, CONST } kind;
    long long val;
};

struct FunctionBlock_ {          // 函数块
    int fb_no;                   // FB块编号
    int bb_first, bb_last;       // 起始和结尾bb编号
    BasicBlockList entry, exit;  // CFG入口和出口
    char* func_name;             // 函数名便于调试使用
    unsigned int bb_nums;        // 含有的BB块（不包括入口和出口）
};

struct BasicBlockList_ {  // 基本块链表
    BasicBlock bb;
    BasicBlockList prev, next;
};
struct BasicBlock_ {
    int fb_no;                  // 所处的FB块编号
    int bb_no;                  // BB块编号
    BasicBlockList pre, suc;    // 前驱，后继BB链表
    InterCodeList first, last;  // BB的首指令和尾指令
};
void optimize();                // 代码优化接口
void remove_redundant_label();  // 删除冗余的label
/**
 * 建立BB,FB,CFG
 */
BasicBlockList init_bb_list();                                              // 初始化BB双向链表头
BasicBlock init_bb();                                                       // 初始化BB
void add_bb(BasicBlockList bb_list_head, BasicBlock bb);                    // 将bb加到list尾部
void bb_tag_ir_list();                                                      // 给ir_list标记BB开始
void bb_tag_ir(InterCode ir);                                               // 给ir标记为基本块开始
bool is_bb_start(InterCode ir);                                             // 判断是否是BB开始
void construct_bb_list();                                                   // 创建BB链表
void init_fb_array();                                                       // 初始化FB数组
void construct_fb(FunctionBlock fb, char* func_name, int first, int last);  // 创建FB
void construct_bb(InterCodeList first, InterCodeList last);                 // 创建BB
void construct_fb_array();                                                  // 创建FB数组
void construct_bb_array();                                                  // 创建BB数组
void construct_cfg();                                                       // 创建CFG
void show_cfg();                                                            // 输出CFG
void show_bb_list(BasicBlockList bb_list_head, FILE* ir_out);               // 输出BB链表中的指令
void show_bb(BasicBlock bb, FILE* ir_out);                                  // 输出BB中的指令
/**
 * 全局优化部分:
 * 常量传播
 * 活跃变量分析
 * 死赋值消除
 */
bool** allocate_BOOL_matrix(int m, int n);                         // 申请位矩阵
void release_BOOL_matrix(bool** array, int len);                   // 释放位矩阵空间
void bitset_union(bool* a, bool* b, bool* res, int len);           // 位向量合并c=a+b
void bitset_minus(bool* a, bool* b, bool* res, int len);           // 位向量差c=a-b
bool bitset_diff(bool* a, bool* b, int len);                       // 位向量是否不同
void set_bitset(Operand op, bool* bitset, bool val);               // 设置位向量
int get_variable_count();                                          // 获取临时变量和普通变量的个数
int get_variable_id(Operand op);                                   // 获取变量编号
void set_LVA_use_def(Operand op, int kind, bool* use, bool* def);  // 设置use和def的位向量
void LVA_BB_use_def(BasicBlock bb, bool* use, bool* def);          // 分析BB的use和def
void LVA_IR_use_def(InterCode ir, bool* use, bool* def);           // 分析一条指令的use和def
void LVA();                                                        // 活跃变量分析入口
void LVA_meet(int bb_first, int bb_no, bool** IN, bool** OUT);     // 活跃变量分析控制流方程
bool LVA_transfer(int bb_no, bool* in_b, bool* out_b, bool* use, bool* def);  // 活跃变量分析传递方程
void DAE(BasicBlock bb, bool* out);                                           // 死代码消除入口
bool judge_IR_DA(InterCode ir, bool* out);                                    // 修改具有副作用的OUT

void dump_PAIR_matrix(CPPair** array, int m, int n);                // 打印Pair矩阵
CPPair** allocate_PAIR_matrix(int m, int n);                        // 申请Pair矩阵
void release_PAIR_matrix(CPPair** array, int len);                  // 释放Pair矩阵空间
void pairset_union(CPPair* a, CPPair* b, CPPair* res, int len);     // Pair向量合并c=a+b
void pairset_minus(CPPair* a, CPPair* b, CPPair* res, int len);     // Pair向量差c=a-b
bool pairset_diff(CPPair* a, CPPair* b, int len);                   // Pair向量是否不同
void set_CP_out(Operand op, int kind, long long val, CPPair* out);  // 设置对应位置的out
void CP_BB_out(BasicBlock bb, CPPair* in, CPPair* out);             // 分析BB的out
void CP_IR_out(InterCode ir, CPPair* out);                          // 分析一条指令的gen和kill
void CP();                                                          // 常量传播入口
void CP_meet(int bb_first, int bb_no, CPPair** IN, CPPair** OUT);   // 常量传播分析控制流方程
bool CP_transfer(int bb_no, CPPair* in_b, CPPair* out_b);           // 常量传播传递方程
void CP_BB_constant(BasicBlock bb, CPPair* in);
void CP_IR_constant(InterCodeList ir_point, CPPair* out);
#endif