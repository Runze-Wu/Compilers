/* The scanner definition for C--. */
%{
/* declarations part */
#include "syntax.tab.h"
extern FILE *yyin;
// extern int yylineno;
%}

%option yylineno
/* Define names for regular expressions here. */
DELIM [\t\f\r\v ;\n]
DIGIT [0-9]
SEMI ;
COMMA ,
RELOP >|<|>=|<=|==|!=
ASSIGNOP =
PLUS \+
MINUS -
STAR \*
DIV \/
AND &&
OR \|\|
DOT \.
NOT !
TYPE int|float
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while

INT 0|([1-9]{DIGIT}*)
FLOAT {INT}{DOT}{DIGIT}+
ID [A-Za-z_][A-Za-z0-9_]*

%x COMMENT
%x INLINE_COMMENT
/* Definitions end. */
%%
    /* Multiple Comments */
"/*" {
    char c;
    int meet = 0;
    int flag = 0;
    while((c=input())!='\0'){
        if(c==EOF) break;
        else if(c=='*') meet=1;
        if(meet) {
            if(c=='/') {flag=1; break;}
            else if(c!='*') meet=0;
        }
    }
    if(flag) printf("Line %d comment end\n",yylineno);
    else printf("Error type A at Line %d: unmatched comment\n",yylineno);
}
<<EOF>> {
    printf("EOF end of the file\n");
    return 0;
}
    /*
<COMMENT>[^*\n]* {}
<COMMENT>"*"+[^*\/\n]* {}
<COMMENT>"\n" {
    yylineno++;
}
<COMMENT>"*"+"/" {
    
    BEGIN(INITIAL);
} */

    /* Inline Comments */
"//" {
    BEGIN(INLINE_COMMENT);
}
<INLINE_COMMENT>[^\n]* {}
<INLINE_COMMENT>\n {
    printf("Line %d inline comment end\n",yylineno);
    BEGIN(INITIAL);
}

[\t\f\r\v \n]	{}
{SEMI} {
  printf("SEMI\n");
}
{COMMA} {
  printf("SEMI\n");
}
{RELOP} {
    printf("RELOP: %s\n",yytext);
}
{ASSIGNOP} {
    printf("ASSIGNOP\n");
}
{PLUS} {
    printf("PLUS\n");
}
{MINUS} {
    printf("MINUS\n");
}
{STAR} {
    printf("STAR\n");
}
{DIV} {
    printf("DIV\n");
}
{AND} {
    printf("AND\n");
}
{OR} {
    printf("OR\n");
}
{DOT} {
    printf("DOT\n");
}
{TYPE} {
    printf("TYPE: %s\n",yytext);
}
{LP} {
    printf("LP\n");
}
{RP} {
    printf("RP\n");
}
{LB} {
    printf("LB\n");
}
{RB} {
    printf("RB\n");
}
{LC} {
    printf("LC\n");
}
{RC} {
    printf("RC\n");
}
{STRUCT} {
    printf("STRUCT\n");
}
{RETURN} {
    printf("RETURN\n");
}
{IF} {
    printf("IF\n");
}
{ELSE} {
    printf("ELSE\n");
}
{WHILE} {
    printf("WHILE\n");
}
{INT} {
    printf("INT: %s\n",yytext);
}
0{DIGIT}+ {
    printf("Error type A at Line %d: Decimal with leading zeros:'%s'\n",yylineno,yytext);
}
{FLOAT} {
    printf("FLOAT: %s\n",yytext);
}

({INT}\.)|(\.{INT}) {
    printf("Error type A at Line %d: Float lack right or left part:'%s'\n",yylineno,yytext);
}
    /*
{INT}+[^\t\f\r\v ;\n0-9.]+{INT}* {
    printf("Error type A at Line %d: Number with unexpected  characters:'%s'\n",yylineno,yytext);
} */

{ID} {
    printf("ID: %s\n",yytext);
}
. {
    printf("Error type A at Line %d: Mysterious characters\'%s\'\n",yylineno,yytext);
}
%%