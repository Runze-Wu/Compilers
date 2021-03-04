/* The scanner definition for C--. */
%{
    /* declarations part */
    extern FILE *fin;
    extern int curr_lineno;
%}

/* Define names for regular expressions here. */

DIGIT [0-9]
SEMI ;
COMMA ,
RELOP > | < | >= | <= | == | !=
ASSIGNOP =
PLUS +
MINUS -
STAR \*
DIV /
AND &&
OR ||
DOT \.
NOT !
TYPE int | float
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
FLOAT INT{DOT}INT
ID [A-Za-z_][A-Za-z0-9_]*

%x COMMENT
%x INLINE_COMMENT
/* Definitions end. */
%%
/* Multiple Comments */
<COMMENT,INITIAL>"/*" {
    BEGIN(COMMENT);
}
<COMMENT>[^{STAR}\n]* {}
<COMMENT>"*"+[^*/\n]* {}
<COMMENT>"*/" {
    BEGIN(INITIAL)
}
<COMMENT>"\n" {
    curr_lineno++;
}
/* Inline Comments */
"//"{
    BEGIN(INLINE_COMMENT);
}
<INLINE_COMMENT>[^\n]* {}
<INLINE_COMMENT>\n {
    curr_lineno++;
    BEGIN(INITIAL);
}

%%