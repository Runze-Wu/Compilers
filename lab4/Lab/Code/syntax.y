%{
    #include <stdio.h>
    #include "mytree.h"
    extern int syntax_errs;
    extern int yylineno;
    extern char* yytext;
    int yyerr_line=0;
    void yyerror(const char *msg);
    int yylex();
    #define YYDEBUG 1
    extern Node root;
%}
/* declaration part */
%define parse.error verbose
%locations

%union {
    Node node;
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
Program: ExtDefList                                 { $$=nonterminal_node("Program",@$.first_line,1,$1);root=$$; }
;               
ExtDefList: ExtDef ExtDefList                       { $$=nonterminal_node("ExtDefList",@$.first_line,2,$1,$2); }
    | /* empty */                                   { $$=NULL; }
;               
ExtDef: Specifier ExtDecList SEMI                   { $$=nonterminal_node("ExtDef",@$.first_line,3,$1,$2,$3); }
    | Specifier SEMI                                { $$=nonterminal_node("ExtDef",@$.first_line,2,$1,$2); }
    | Specifier FunDec CompSt                       { $$=nonterminal_node("ExtDef",@$.first_line,3,$1,$2,$3); }
    | error ExtDef                                  {} /* 以奇怪的东西开头 */
    | error CompSt                                  { yyerrok; }
    | error SEMI                                    { yyerrok; }
    | Specifier error SEMI                          { yyerrok; }
    | Specifier error CompSt                        { yyerrok; }
;               
ExtDecList: VarDec                                  { $$=nonterminal_node("ExtDecList",@$.first_line,1,$1); }
    | VarDec COMMA ExtDecList                       { $$=nonterminal_node("ExtDecList",@$.first_line,3,$1,$2,$3); }
;
/* Specifiers */
Specifier: TYPE                                     { $$=nonterminal_node("Specifier",@$.first_line,1,$1); }
    | StructSpecifier                               { $$=nonterminal_node("Specifier",@$.first_line,1,$1); }
;
StructSpecifier: STRUCT OptTag LC DefList RC        { $$=nonterminal_node("StructSpecifier",@$.first_line,5,$1,$2,$3,$4,$5); }
    | STRUCT Tag                                    { $$=nonterminal_node("StructSpecifier",@$.first_line,2,$1,$2); }
    | STRUCT OptTag LC DefList error RC             { yyerrok; }
;   
OptTag: ID                                          { $$=nonterminal_node("OptTag",@$.first_line,1,$1); }
    | /* empty */                                   { $$=NULL; }
;
Tag: ID                                             { $$=nonterminal_node("Tag",@$.first_line,1,$1); }
;
/* Declarators */
VarDec: ID                                          { $$=nonterminal_node("VarDec",@$.first_line,1,$1); }
    | VarDec LB INT RB                              { $$=nonterminal_node("VarDec",@$.first_line,4,$1,$2,$3,$4); }
;               
FunDec: ID LP VarList RP                            { $$=nonterminal_node("FunDec",@$.first_line,4,$1,$2,$3,$4); }
    | ID LP RP                                      { $$=nonterminal_node("FunDec",@$.first_line,3,$1,$2,$3); }
;
VarList: ParamDec COMMA VarList                     { $$=nonterminal_node("VarList",@$.first_line,3,$1,$2,$3); }
    | ParamDec                                      { $$=nonterminal_node("VarList",@$.first_line,1,$1); }
;
ParamDec: Specifier VarDec                          { $$=nonterminal_node("ParamDec",@$.first_line,2,$1,$2); }
;
/* Statements */
CompSt: LC DefList StmtList RC                      { $$=nonterminal_node("CompSt",@$.first_line,4,$1,$2,$3,$4); }
    /* |   LC DefList StmtList error                   {} */
;
StmtList: Stmt StmtList                             { $$=nonterminal_node("StmtList",@$.first_line,2,$1,$2); }
    | /* empty */                                   { $$=NULL; }
;
Stmt: Exp SEMI                                      { $$=nonterminal_node("Stmt",@$.first_line,2,$1,$2); }
    | CompSt                                        { $$=nonterminal_node("Stmt",@$.first_line,1,$1); }
    | RETURN Exp SEMI                               { $$=nonterminal_node("Stmt",@$.first_line,3,$1,$2,$3); }
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE       { $$=nonterminal_node("Stmt",@$.first_line,5,$1,$2,$3,$4,$5); }
    | IF LP Exp RP Stmt ELSE Stmt                   { $$=nonterminal_node("Stmt",@$.first_line,7,$1,$2,$3,$4,$5,$6,$7); }
    | WHILE LP Exp RP Stmt                          { $$=nonterminal_node("Stmt",@$.first_line,5,$1,$2,$3,$4,$5); }                       
    | error SEMI                                    { yyerrok; }
    | error Stmt                                    { yyerrok; }
    | Exp error                                     {}
    | IF LP error RP Stmt %prec LOWER_THAN_ELSE     {}
    | IF LP error RP Stmt ELSE Stmt                 {}
    | WHILE LP error RP Stmt                        { yyerrok; }
;
/* Local Definitions */
DefList: Def DefList                                { $$=nonterminal_node("DefList",@$.first_line,2,$1,$2); }
    | /* empty */                                   { $$=NULL; }
;
Def: Specifier DecList SEMI                         { $$=nonterminal_node("Def",@$.first_line,3,$1,$2,$3); }
    | Specifier DecList error SEMI                  { yyerrok; }
    | Specifier error SEMI                          { yyerrok; }
;
DecList: Dec                                        { $$=nonterminal_node("DecList",@$.first_line,1,$1); }
    | Dec COMMA DecList                             { $$=nonterminal_node("DecList",@$.first_line,3,$1,$2,$3); }
;
Dec: VarDec                                         { $$=nonterminal_node("Dec",@$.first_line,1,$1); }
    | VarDec ASSIGNOP Exp                           { $$=nonterminal_node("Dec",@$.first_line,3,$1,$2,$3); }
;
/* Expressions */
Exp: Exp ASSIGNOP Exp                               { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp AND Exp                                   { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp OR Exp                                    { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp RELOP Exp                                 { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp PLUS Exp                                  { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp MINUS Exp                                 { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp STAR Exp                                  { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp DIV Exp                                   { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | LP Exp RP                                     { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | MINUS Exp %prec UMINUS                        { $$=nonterminal_node("Exp",@$.first_line,2,$1,$2); }
    | NOT Exp                                       { $$=nonterminal_node("Exp",@$.first_line,2,$1,$2); }
    | ID LP Args RP                                 { $$=nonterminal_node("Exp",@$.first_line,4,$1,$2,$3,$4); }
    | ID LP RP                                      { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp LB Exp RB                                 { $$=nonterminal_node("Exp",@$.first_line,4,$1,$2,$3,$4); }
    | Exp DOT ID                                    { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | ID                                            { $$=nonterminal_node("Exp",@$.first_line,1,$1); }
    | INT                                           { $$=nonterminal_node("Exp",@$.first_line,1,$1); }
    | FLOAT                                         { $$=nonterminal_node("Exp",@$.first_line,1,$1); }
    | Exp ASSIGNOP error                            {}  
    | Exp AND error                                 {}    
    | Exp OR error                                  {}    
    | Exp RELOP error                               {}    
    | Exp PLUS error                                {}    
    | Exp MINUS error                               {}  
    | Exp STAR error                                {}    
    | Exp DIV error                                 {}     
    | LP error RP                                   { yyerrok; }   
    | LP Exp error                                  {}    
    | MINUS error %prec UMINUS                      {}           
    | NOT error                                     {}  
    | ID LP error RP                                { yyerrok; }  
    | Exp LB error RB                               { yyerrok; }  
;
Args: Exp COMMA Args                                { $$=nonterminal_node("Args",@$.first_line,3,$1,$2,$3); }
    | Exp                                           { $$=nonterminal_node("Args",@$.first_line,1,$1); }
    | error COMMA Exp                               {}
;
%%

#include "lex.yy.c"
void yyerror(const char *msg) {
    if(yyerr_line==yylineno)return;
    yyerr_line=yylineno;
    syntax_errs++;
    // printf("\e[1;31mError Type B\e[0m at Line %d: syntax near \"%s\"\n",yylineno, yytext);
    printf("Error type B at Line %d %s, near \"%s\".\n",yylineno,msg,yytext);
}