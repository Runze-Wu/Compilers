#ifndef __TRANSLATOR_H__
#define __TRANSLATOR_H__

#include "hashtab.h"
#include "intercode.h"
#include "mytree.h"

void translate_Program(Node root);
void translate_ExtDefList(Node root);
void translate_ExtDef(Node root);
/**
 * 假设4 没有全局变量,无需考虑ExtDecList
 * 变量类型在语义分析部分全部完成,无需考虑Specifier
 */

void translate_VarDec(Node root);
void translate_FunDec(Node root);
void translate_VarList(Node root);
void translate_ParamDec(Node root);

void translate_CompSt(Node root);
void translate_StmtList(Node root);
void translate_Stmt(Node root);

void translate_DefList(Node root);
void translate_Def(Node root);
void translate_DecList(Node root);
void translate_Dec(Node root);

void translate_Exp(Node root);
void translate_Args(Node root);

void translate_Cond(Node root, Operand true_label, Operand false_label);
#endif