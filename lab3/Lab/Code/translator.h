#ifndef __TRANSLATOR_H__
#define __TRANSLATOR_H__

#include "hashtab.h"
#include "intercode.h"
#include "mytree.h"

int temp_number;   // 临时变量编号
int label_number;  // 跳转编号

void translate_Program(Node root);
void translate_ExtDefList(Node root);
void translate_ExtDef(Node root);
void translate_ExtDecList(Node root);

void translate_Specifier(Node root);
void translate_StructSpecifier(Node root);
void translate_OptTag(Node root);
void translate_Tag(Node root);

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
#endif