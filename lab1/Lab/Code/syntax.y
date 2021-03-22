%{
    #include <stdio.h>
    #include "mytree.h"
    extern int syntax_errs;
    extern int yylineno;
    extern char* yytext;
    void yyerror(const char *msg);
    void my_yyerror(const char *msg);
    int yylex();
    #define YYDEBUG 1
    extern struct treenode* root;
%}
/* declaration part */
%define parse.error simple
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
%type <node> error
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

%nonassoc LOWER_THAN_CORRECT
%nonassoc CORRECT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%%
/* High-level Definitions */
Program: ExtDefList {
        $$=nonterminal_node("Program",@1.first_line);
        root=$$;
        set_parent_brother($$,1,$1);
    }
;
ExtDefList: ExtDef ExtDefList {
        $$=nonterminal_node("ExtDefList",@1.first_line);
        set_parent_brother($$,2,$1,$2);
    }
    | /* empty */ {
        $$=NULL;
    }
;
ExtDef: Specifier ExtDecList SEMI {
        $$=nonterminal_node("ExtDef",@1.first_line);
        set_parent_brother($$,3,$1,$2,$3);
    }
    | Specifier SEMI {    
        $$=nonterminal_node("ExtDef",@1.first_line);
        set_parent_brother($$,2,$1,$2);
    }
    | Specifier FunDec CompSt {
        $$=nonterminal_node("ExtDef",@1.first_line);
        set_parent_brother($$,3,$1,$2,$3);
    }
    // | Specifier ExtDecList error SEMI
    // | error Specifier SEMI 
    // | error ExtDecList SEMI 
    // | Specifier FunDec error 
    | error SEMI {yyerrok;}
    | Specifier error SEMI {yyerrok;}
    // | Specifier error {}
    // | Specifier error CompSt {}
;
ExtDecList: VarDec {
        $$=nonterminal_node("ExtDecList",@1.first_line);
        set_parent_brother($$,1,$1);
    }
    | VarDec COMMA ExtDecList {
        $$=nonterminal_node("ExtDecList",@1.first_line);
        set_parent_brother($$,3,$1,$2,$3);
    }
    | VarDec COMMA error {}
    | error COMMA ExtDecList {}
;
/* Specifiers */
Specifier: TYPE {
        $$=nonterminal_node("Specifier",@1.first_line);
        set_parent_brother($$,1,$1);
    }
    | StructSpecifier {
        $$=nonterminal_node("Specifier",@1.first_line);
        set_parent_brother($$,1,$1);
    }
;
StructSpecifier: STRUCT OptTag LC DefList RC {
        $$=nonterminal_node("StructSpecifier",@1.first_line);
        set_parent_brother($$,5,$1,$2,$3,$4,$5);
    }
    | STRUCT Tag {
        $$=nonterminal_node("StructSpecifier",@1.first_line);
        set_parent_brother($$,2,$1,$2);
    }
;
OptTag: ID {
        $$=nonterminal_node("OptTag",@1.first_line);
        set_parent_brother($$,1,$1);
    }
    | /* empty */ {
        $$=NULL;
    }
;
Tag: ID {
        $$=nonterminal_node("Tag",@1.first_line);
        set_parent_brother($$,1,$1);
    }
;
/* Declarators */
VarDec: ID {
        $$=nonterminal_node("VarDec",@1.first_line);
        set_parent_brother($$,1,$1);
    }
    | VarDec LB INT RB {
        $$=nonterminal_node("VarDec",@1.first_line);
        set_parent_brother($$,4,$1,$2,$3,$4);
    }
    | VarDec LB error RB {my_yyerror(NULL);yyerrok;}
;
FunDec: ID LP VarList RP {
        $$=nonterminal_node("FunDec",@1.first_line);
        set_parent_brother($$,4,$1,$2,$3,$4);
    }
    | ID LP RP {
        $$=nonterminal_node("FunDec",@1.first_line);
        set_parent_brother($$,3,$1,$2,$3);
    }
    | error LP RP {yyerrok;}
    | error LP VarList RP {yyerrok;}
    | ID LP error RP {yyerrok;}
    
;
VarList: ParamDec COMMA VarList {
        $$=nonterminal_node("VarList",@1.first_line);
        set_parent_brother($$,3,$1,$2,$3);
    }
    | ParamDec {
        $$=nonterminal_node("VarList",@1.first_line);
        set_parent_brother($$,1,$1);
    }
;
ParamDec: Specifier VarDec {
        $$=nonterminal_node("ParamDec",@1.first_line);
        set_parent_brother($$,2,$1,$2);
    }
;
/* Statements */
CompSt: LC DefList StmtList RC {
        $$=nonterminal_node("CompSt",@1.first_line);
        set_parent_brother($$,4,$1,$2,$3,$4);
    }
;
StmtList: Stmt StmtList {
        $$=nonterminal_node("StmtList",@1.first_line);
        set_parent_brother($$,2,$1,$2);
    }
    | /* empty */ {
        $$=NULL;
    }
;
Stmt: Exp SEMI {
        $$=nonterminal_node("Stmt",@1.first_line);
        set_parent_brother($$,2,$1,$2);
    }
    | CompSt {
        $$=nonterminal_node("Stmt",@1.first_line);
        set_parent_brother($$,1,$1);
    }
    | RETURN Exp SEMI {
        $$=nonterminal_node("Stmt",@1.first_line);
        set_parent_brother($$,3,$1,$2,$3);
    }
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {
        $$=nonterminal_node("Stmt",@1.first_line);
        set_parent_brother($$,5,$1,$2,$3,$4,$5);
    }
    | IF LP Exp RP Stmt ELSE Stmt {
        $$=nonterminal_node("Stmt",@1.first_line);
        set_parent_brother($$,7,$1,$2,$3,$4,$5,$6,$7);
    }
    | WHILE LP Exp RP Stmt {
        $$=nonterminal_node("Stmt",@1.first_line);
        set_parent_brother($$,5,$1,$2,$3,$4,$5);
    }
    | error 
    | Exp error SEMI {yyerrok;}
    | error SEMI {yyerrok;}
    // | Exp error {}
    | RETURN Exp error SEMI
    | IF LP error RP Stmt  %prec LOWER_THAN_ELSE
    | IF LP error RP Stmt ELSE Stmt
    | WHILE LP error RP Stmt
;
/* Local Definitions */
DefList: Def DefList {
        $$=nonterminal_node("DefList",@1.first_line);
        set_parent_brother($$,2,$1,$2);
    }
    | /* empty */ {
        $$=NULL;
    }
;
Def: Specifier DecList SEMI {
        $$=nonterminal_node("Def",@1.first_line);
        set_parent_brother($$,3,$1,$2,$3);
    }
    | Specifier DecList error
    | Specifier error SEMI {yyerrok;}
;
DecList: Dec {
        $$=nonterminal_node("DecList",@1.first_line);
        set_parent_brother($$,1,$1);
    }
    | Dec COMMA DecList {
        $$=nonterminal_node("DecList",@1.first_line);
        set_parent_brother($$,3,$1,$2,$3);
    }
    // | Dec COMMA error DecList {yyerrok;}
    | error COMMA DecList {yyerrok;}
;
Dec: VarDec {
        $$=nonterminal_node("Dec",@1.first_line);
        set_parent_brother($$,1,$1);
    }
    | VarDec ASSIGNOP Exp {
        $$=nonterminal_node("Dec",@1.first_line);
        set_parent_brother($$,3,$1,$2,$3);
    }
    | error Dec {yyerrok;}
    | error ASSIGNOP Exp {yyerrok;}
    | VarDec ASSIGNOP error
;
/* Expressions */
Exp: Exp ASSIGNOP Exp {
        $$=nonterminal_node("Exp",@1.first_line);
        set_parent_brother($$,3,$1,$2,$3);
    }
    | Exp AND Exp {
        $$=nonterminal_node("Exp",@1.first_line);
        set_parent_brother($$,3,$1,$2,$3);
    }
    | Exp OR Exp {
        $$=nonterminal_node("Exp",@1.first_line);
        set_parent_brother($$,3,$1,$2,$3);
    }
    | Exp RELOP Exp {
        $$=nonterminal_node("Exp",@1.first_line);
        set_parent_brother($$,3,$1,$2,$3);
    }
    | Exp PLUS Exp {
        $$=nonterminal_node("Exp",@1.first_line);
        set_parent_brother($$,3,$1,$2,$3);
    }
    | Exp MINUS Exp {
        $$=nonterminal_node("Exp",@1.first_line);
        set_parent_brother($$,3,$1,$2,$3);
    }
    | Exp STAR Exp {
        $$=nonterminal_node("Exp",@1.first_line);
        set_parent_brother($$,3,$1,$2,$3);
    }
    | Exp DIV Exp {
        $$=nonterminal_node("Exp",@1.first_line);
        set_parent_brother($$,3,$1,$2,$3);
    }
    | LP Exp RP {
        $$=nonterminal_node("Exp",@1.first_line);
        set_parent_brother($$,3,$1,$2,$3);
    }
    | MINUS Exp %prec UMINUS{
        $$=nonterminal_node("Exp",@1.first_line);
        set_parent_brother($$,2,$1,$2);
    }
    | NOT Exp {
        $$=nonterminal_node("Exp",@1.first_line);
        set_parent_brother($$,2,$1,$2);
    }
    | ID LP Args RP {
        $$=nonterminal_node("Exp",@1.first_line);
        set_parent_brother($$,4,$1,$2,$3,$4);
    }
    | ID LP RP {
        $$=nonterminal_node("Exp",@1.first_line);
        set_parent_brother($$,3,$1,$2,$3);
    }
    | Exp LB Exp RB {
        $$=nonterminal_node("Exp",@1.first_line);
        set_parent_brother($$,4,$1,$2,$3,$4);
    }
    | Exp DOT ID {
        $$=nonterminal_node("Exp",@1.first_line);
        set_parent_brother($$,3,$1,$2,$3);
    }
    | ID {
        $$=nonterminal_node("Exp",@1.first_line);
        set_parent_brother($$,1,$1);
    }
    | INT {
        $$=nonterminal_node("Exp",@1.first_line);
        set_parent_brother($$,1,$1);
    }
    | FLOAT {
        $$=nonterminal_node("Exp",@1.first_line);
        set_parent_brother($$,1,$1);
    }
    | Exp ASSIGNOP error    
    | Exp AND error         
    | Exp OR error          
    | Exp RELOP error       
    | Exp PLUS error        
    | Exp MINUS error      
    | Exp STAR error        
    | Exp DIV error         
    | LP error RP          
    | LP Exp error          
    | MINUS error %prec UMINUS         
    | NOT error        
    | ID LP error RP        {yyerrok;}
    | Exp LB error RB       {yyerrok;}
;
Args: Exp COMMA Args {
        $$=nonterminal_node("Args",@1.first_line);
        set_parent_brother($$,3,$1,$2,$3);
    }
    | Exp {
        $$=nonterminal_node("Args",@1.first_line);
        set_parent_brother($$,1,$1);
    }
    | Exp COMMA error {my_yyerror(NULL);yyerrok;}
;
%%

#include "lex.yy.c"
void yyerror(const char *msg) {
    syntax_errs++;
    // printf("\e[1;31mError Type B\e[0m at Line %d: syntax near \"%s\"\n",yylineno, yytext);
    printf("Error Type B at Line %d: syntax error near \"%s\"\n",yylineno, yytext);
}
void my_yyerror(const char *msg) {
    // syntax_errs++;
    // printf("\e[1;32mError Type B\e[0m at Line %d: syntax near \"%s\"\n",yylineno, yytext);
}