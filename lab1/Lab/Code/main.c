#include <stdio.h>
int curr_lineno=1;
FILE *fin; //This is the file pointer from which the lexer reads its input.

int main(int argc,char**argv){
    if(argc>1){
        if(!(fin=fopen(argv[1],"r"))){
            perror(argv[1]);
            return 1;
        }
        curr_lineno=1;
    }
    while (yylex()!=0);
    fclose(fin);
    return 0; 
}