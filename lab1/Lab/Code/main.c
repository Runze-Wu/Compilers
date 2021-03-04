#include <stdio.h>
int curr_lineno=1;
FILE *yyin; //This is the file pointer from which the lexer reads its input.
extern int yylex();
int main(int argc,char**argv){
    if(argc>1){
        if(!(yyin=fopen(argv[1],"r"))){
            perror(argv[1]);
            return 1;
        }
        printf("open file:%s\n",argv[1]);
        curr_lineno=1;
    }
    while(yylex()!=0);
    fclose(yyin);
    return 0; 
}