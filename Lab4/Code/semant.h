#ifndef __SEMANT_H__
#define __SEMANT_H__
#include "hashtab.h"
#include "mytree.h"

void add_READ_WRITE_func();  // add read write function to hashtable

void Program(Node root);
void ExtDefList(Node root);
void ExtDef(Node root);
void ExtDecList(Node root, Type type);  // type: ExtDef's Specifier type

Type Specifier(Node root);
Type StructSpecifier(Node root);
char* OptTag(Node root);
char* Tag(Node root);

FieldList VarDec(Node root, Type type, FieldList field);  // type: VarDec type field: StructTag pointer
void FunDec(Node root, Type type);                        // type: FunDec return type
void VarList(Node root, FieldList field);                 // filed: FunDec argc argv
FieldList ParamDec(Node root);

void CompSt(Node root, Type type);    // type: FunDec return type
void Stmtlist(Node root, Type type);  // type: FunDec return type
void Stmt(Node root, Type type);      // type: FunDec return type

void DefList(Node root, FieldList field);             // field: StructTag pointer
void Def(Node root, FieldList field);                 // field: StructTag pointer
void DecList(Node root, Type type, FieldList field);  // type: VarDec type field: StructTag pointer
void Dec(Node root, Type type, FieldList field);      // type: VarDec type field: StructTag pointer

Type Exp(Node root);        // return exp type
FieldList Args(Node root);  // return arguments

FieldList have_member(FieldList struct_field, char* member);  // check member in filed
bool type_matched(Type a, Type b);                            // check type matched, 1: matched, 0: not matched
bool args_matched(FieldList act_args,
                  FieldList form_args);  // check function arguments matched
void add_struct_member(Node member, Type mem_type, FieldList struct_field);
void add_func_parameter(Node param, FieldList func_field);

void dump_type(Type type, int depth);         // show the type, use for debug
void dump_field(FieldList field, int depth);  // show the field, use for debug
void dump_node(Node node);                    // show the node, use for debug
void dump_semantic_error(int err_type, int err_line, char* err_msg, char* err_elm);
#endif