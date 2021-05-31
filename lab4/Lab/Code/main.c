#include <stdio.h>

#include "controlflow.h"
#include "semant.h"
#include "syntax.tab.h"
#include "translator.h"

FILE* yyin;                          // This is the file pointer from which the lexer reads its input.
FILE* ir_out;                        // the file pointer from which the translator writes its output.
int lexical_errs = 0;                // 出现的词法错误
int syntax_errs = 0;                 // 出现的语法错误
int semantic_errs = 0;               // 出现的语义错误
extern int yydebug;                  // bison debug mode
int semantic_debug = 0;              // semantic debug mode
int translator_debug = 0;            // translator debug mode
int translator_struct = 0;           // 是否处理结构体
int optimizer_debug = 0;             // optimizer debug mode
YYSTYPE yylval;                      // 存储终结符的语义值
Node root;                           // AST语法树的根结点
InterCodeList* label_array;          // 所有编号的数组
InterCodeList global_ir_list_head;   // IR循环双向链表头
BasicBlockList global_bb_list_head;  // 基本块循环双向链表头
int yylex();                         // 词法分析的接口
int yyparse(void);                   // 语法分析的接口
void yyrestart(FILE* input_file);    // 将yyin指针重置

int main(int argc, char** argv) {
    // yydebug = 1;
    // semantic_debug = 1;
    // translator_debug = 1;
    // translator_struct = 1;
    // optimizer_debug = 1;
    if (argc <= 2) return 1;
    if (argc > 2) {
        if (!(yyin = fopen(argv[1], "r"))) {
            perror(argv[1]);
            return 1;
        }
        if (!(ir_out = fopen(argv[2], "w"))) {
            perror(argv[2]);
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
        if (semantic_errs == 0) {
            translate_Program(root);
            // show_ir_list(global_ir_list_head, ir_out);

            if (optimizer_debug) {
                show_ir_list(global_ir_list_head, ir_out);
                optimize();
                show_bb_list(global_bb_list_head, NULL);
            } else {
                show_ir_list(global_ir_list_head, NULL);
                optimize();
                show_bb_list(global_bb_list_head, ir_out);
            }
        }
    }
    return 0;
}