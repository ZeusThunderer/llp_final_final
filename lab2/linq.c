#include <stdio.h>
#include "data.h"
#include "write.h"
#include "parserLINQ.tab.h"


extern int yy_scan_string(const char* str);
extern int yylex_destroy(void);


Statement* parse_statement(const char* buffer){
    Statement* statement = (Statement*)malloc(sizeof(Statement));
    yy_scan_string(buffer);
    yyparse(&statement);    
    yylex_destroy();
    return statement;
}