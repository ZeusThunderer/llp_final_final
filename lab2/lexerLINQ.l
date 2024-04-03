%option caseless
%option noyywrap
%option yylineno

%{
#include <stdio.h>
#include <string.h>
#include "data.h"
#include "parserLINQ.tab.h"
void showError(char *token);
#define YYERROR_VERBOSE 1
%}

INTEGERS    [-+]?[0-9]+
FLOATS      [+-]?([0-9]+[.][0-9]*|[.][0-9]+)
NAME        [a-zA-Z_][a-zA-Z0-9_]{0,63}
STRING      \"([^\\\"]|\\.)*\"

%%

"SELECT"    {return SELECT;}
"ADD"       {return ADD;}
"UPDATE"    {return UPDATE;}
"CREATE"    {return CREATE;}
"REMOVE"    {return REMOVE;}
"DROP"      {return DROP;}
"ON"        {return ON;}
"IN"        {return IN;}
"INTO"      {return INTO;}
"NEW"       {return NEW;}
"FROM"      {return FROM;}
"WHERE"     {return WHERE;}
"JOIN"      {return JOIN;}
"AND"       {yylval.logicOp = AND_OP; return AND;}
"OR"        {yylval.logicOp = OR_OP; return OR;}
"="         {yylval.comp_type = EQUALS_COMP; return EQUALS;}
"!="        {yylval.comp_type = NOT_EQUALS_COMP; return COMPARISON;}
">"         {yylval.comp_type = GREATER_COMP; return COMPARISON;}
"<"         {yylval.comp_type = LESS_COMP; return COMPARISON;}
">="        {yylval.comp_type = GREATER_EQUALS_COMP; return COMPARISON;}
"<="        {yylval.comp_type = LESS_EQUALS_COMP; return COMPARISON;}
"LIKE"      {yylval.comp_type = LIKE_COMP; return CONTAINS;}
"INT"       {yylval.valType = INT_TYPE; return TYPE;}
"FLOAT"     {yylval.valType = FLOAT_TYPE; return TYPE;}
"VARCHAR"   {yylval.valType = STRING_TYPE; return VARCHAR;}
"BOOL"      {yylval.valType = BOOL_TYPE; return TYPE;}
"FALSE"     {yylval.boolVal = false; return BOOL;}
"TRUE"      {yylval.boolVal = true; return BOOL;}
"("         {return LB;}
")"         {return RB;}
"{"         {return LCB;}
"}"         {return RCB;}
";"         {return SEMICOLON;}
"."         {return DOT;}
","         {return COMMA;}

{STRING}    {yylval.str = strdup(yytext + 1);
             yylval.str[strlen(yylval.str)-1] = '\0';
             return STRING;}
{NAME}      {strcpy(yylval.name,yytext); return NAME; }
{INTEGERS}  {
                yylval.intVal = atoi(yytext);
                if(yylval.intVal > 0) {
                    return POSITIVE_INT;
                } else {
                    return INTEGER;
                }
            }
{FLOATS}    {yylval.floatVal = atof(yytext); return FLOAT; }
[ \t\n]+    ;
.           {showError(yytext);}

%%

void showError(char* token) {
    printf("Unknown token: %s\n", token);
}
