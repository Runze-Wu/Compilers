make clean

flex -o ./lex.yy.c ./lexical.l
bison -o ./syntax.tab.c -d -v ./syntax.y

gcc -g -c ./syntax.tab.c -o ./syntax.tab.o
gcc -g -std=c99   -c -o semant.o semant.c
gcc -g -std=c99   -c -o main.o main.c
gcc -g -std=c99   -c -o controlflow.o controlflow.c
gcc -g -std=c99   -c -o translator.o translator.c
gcc -g -std=c99   -c -o intercode.o intercode.c
gcc -g -std=c99   -c -o mytree.o mytree.c
gcc -g -std=c99   -c -o hashtab.o hashtab.c
gcc -g -std=c99   -c -o objectcode.o objectcode.c
gcc -g -o parser ./semant.o ./main.o  ./objectcode.o ./controlflow.o ./translator.o ./intercode.o ./syntax.tab.o ./mytree.o ./hashtab.o -lfl -ly