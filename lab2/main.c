#include <stdio.h>
#include "data.h"
#include "write.h"
#include "parserLINQ.tab.h"
#include "linq.h"
#define BUF_SIZE 1024

extern void yylex_destroy(void);

int main(int argc, char *argv[]){
    char buffer[BUF_SIZE];
    buffer[0] = '\0';
    while (true) {
        printf("stmt > ");
        fgets(buffer, BUF_SIZE, stdin);
        buffer[BUF_SIZE - 1] = '\0';
        if (strcmp(buffer, "quit\n") == 0)
            return 0;
        Statement *s = parse_statement(buffer);
        if (!s) continue;
        printStatement(s);
        freeStatement(s);
    }
}