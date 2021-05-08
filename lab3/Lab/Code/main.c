#include <stdio.h>

#include "semant.h"
#include "syntax.tab.h"
#include "translator.h"

FILE *yyin;                        // This is the file pointer from which the lexer reads its input.
int lexical_errs = 0;              // 出现的词法错误
int syntax_errs = 0;               // 出现的语法错误
int semantic_errs = 0;             // 出现的语义错误
extern int yydebug;                // bison debug mode
int semantic_debug = 0;            // semantic debug mode
YYSTYPE yylval;                    // 存储终结符的语义值
Node root;                         // AST语法树的根结点
int yylex();                       // 词法分析的接口
int yyparse(void);                 // 语法分析的接口
void yyrestart(FILE *input_file);  // 将yyin指针重置

int main(int argc, char **argv) {
    // yydebug = 1;
    // semantic_debug = 1;
    if (argc == 1) return 1;
    if (argc > 1) {
        if (!(yyin = fopen(argv[1], "r"))) {
            perror(argv[1]);
            return 1;
        }
        if (yydebug) printf("open file:%s\n", argv[1]);
    }
    yyrestart(yyin);
    yyparse();
    fclose(yyin);
    if (lexical_errs || syntax_errs) {
        /* meet erros */
        if (yydebug) printf("totally meet %d lexical errors and %d syntax errors\n", lexical_errs, syntax_errs);
    } else {
        /* Print the AST tree */
        // print_tree(root, 0);
        Program(root);
    }
    if (semantic_errs == 0) {
        translate_Program(root);
    }
    return 0;
}