%{
    #include <stdio.h>
    #include "mytree.h"
    extern int syntax_errs;
    extern int yylineno;
    void yyerror(const char *msg);
    int yylex();
    #define YYDEBUG 1
%}
/* declaration part */
%define parse.error verbose
%locations

%union {
    struct treenode* node;
}
/* High-level Definitions */
%type <node> Program
%type <node> ExtDefList
%type <node> ExtDef
%type <node> ExtDecList
/* Specifiers */
%type <node> Specifier
%type <node> StructSpecifier
%type <node> OptTag
%type <node> Tag
/* Declarators */
%type <node> VarDec
%type <node> FunDec
%type <node> VarList
%type <node> ParamDec
/* Statements */
%type <node> CompSt
%type <node> StmtList
%type <node> Stmt
/* Local Definitions */
%type <node> DefList
%type <node> Def
%type <node> DecList
%type <node> Dec
/* Expressions */
%type <node> Exp
%type <node> Args
%start Program
/* declared tokens */
%token <node> INT FLOAT ID
%token <node> SEMI COMMA DOT
%token <node> ASSIGNOP RELOP PLUS MINUS STAR DIV
%token <node> AND OR NOT
%token <node> LP RP LB RB LC RC
%token <node> TYPE STRUCT RETURN
%token <node> IF ELSE WHILE

/* 结合性与优先级 */
%right ASSIGNOP
%left  OR
%left  AND
%left  RELOP
%left  PLUS MINUS
%left  STAR DIV
%right NOT
%left  UMINUS /* 处理负号 */
%left  DOT LP RP LB RB
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%%
/* High-level Definitions */
Program: ExtDefList {}
;
ExtDefList: ExtDef ExtDefList {}
    | /* empty */ {}
;
ExtDef: Specifier ExtDecList SEMI {}
    | Specifier SEMI {}
    | Specifier FunDec CompSt {}
    | error SEMI {}
    | error Specifier SEMI {}
    | Specifier error SEMI {}
;
ExtDecList: VarDec {}
    | VarDec COMMA ExtDecList {}
;
/* Specifiers */
Specifier: TYPE {}
    | StructSpecifier {}
;
StructSpecifier: STRUCT OptTag LC DefList RC {}
    | STRUCT Tag {}
    | STRUCT OptTag LC error RC {}
;
OptTag: ID {}
    | /* empty */ {}
;
Tag: ID {}
;
/* Declarators */
VarDec: ID {}
    | VarDec LB INT RB {}
    | VarDec LB error RB {}
;
FunDec: ID LP VarList RP {}
    | ID LP RP {}
    | ID LP error RP {}
    
;
VarList: ParamDec COMMA VarList {}
    | ParamDec {}
;
ParamDec: Specifier VarDec {}
;
/* Statements */
CompSt: LC DefList StmtList RC {}
    | LC DefList error RC {}
;
StmtList: Stmt StmtList {}
    | /* empty */ {}
;
Stmt: Exp SEMI {}
    | CompSt {}
    | RETURN Exp SEMI {}
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {}
    | IF LP Exp RP Stmt ELSE Stmt {}
    | WHILE LP Exp RP Stmt {}
    | error SEMI {}
    | RETURN error SEMI {}
    | RETURN Exp error {}
;
/* Local Definitions */
DefList: Def DefList {}
    | /* empty */ {}
;
Def: Specifier DecList SEMI {}
;
DecList: Dec {}
    | Dec COMMA DecList {}
;
Dec: VarDec {}
    | VarDec ASSIGNOP Exp {}
;
/* Expressions */
Exp: Exp ASSIGNOP Exp {}
    | Exp AND Exp {}
    | Exp OR Exp {}
    | Exp RELOP Exp {}
    | Exp PLUS Exp {}
    | Exp MINUS Exp {}
    | Exp STAR Exp {}
    | Exp DIV Exp {}
    | LP Exp RP {}
    | MINUS Exp %prec UMINUS{}
    | NOT Exp {}
    | ID LP Args RP {}
    | ID LP RP {}
    | Exp LB Exp RB {}
    | Exp DOT ID {}
    | ID {}
    | INT {}
    | FLOAT {}
    | Exp ASSIGNOP error {}
    | Exp AND error {}
    | Exp OR error {}
    | Exp RELOP error {}
    | Exp PLUS error {}
    | Exp MINUS error {}
    | Exp STAR error {}
    | Exp DIV error {}
    | LP error RP {}
    | MINUS error {}
    | NOT error {}
    | ID LP error RP {}
    | Exp LB error RB {}
;
Args: Exp COMMA Args {}
    | Exp {}
;
%%

#include "lex.yy.c"
void yyerror(const char *msg) {
    syntax_errs++;
    printf("Error type B at Line %d: %s\n",yylineno, msg);
}