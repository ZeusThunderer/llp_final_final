client: main.c linq.h linq.c lexerLINQ.l parserLINQ.y data.h data.c write.h write.c
	bison -d parserLINQ.y -Wall
	flex lexerLINQ.l
	gcc -g -o $@ main.c linq.c parserLINQ.tab.c lex.yy.c data.c write.c -lfl
.PHONY: clean

clean:
	rm -f client lex.yy.c parserLINQ.tab.c lexerLINQ.tab.h parserLINQ.tab.h
