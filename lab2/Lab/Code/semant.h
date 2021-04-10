#ifndef __SEMANT_H__
#define __SEMANT_H__
#include "hashtab.h"
#include "mytree.h"

void Program(Node root);
void ExtDefList(Node root);
void ExtDef(Node root);
void ExtDecList(Node root, Type type);

Type Specifier(Node root);
Type StructSpecifier(Node root);
char* OptTag(Node root);
char* Tag(Node root);

void VarDec(Node root, Type type);  // type: VarDec type
void FunDec(Node root);
void VarList(Node root, FieldList filed);  // filed: FunDec argc argv
void ParamDec(Node root);

void CompSt(Node root, Type type);  // type: FunDec return type
void Stmtlist(Node root, Type type);
void Stmt(Node root, Type type);

void DefList(Node root);
void Def(Node root);
void DecList(Node root, Type type);
void Dec(Node root, Type type);

void Exp(Node root);
void Args(Node root);

#endif