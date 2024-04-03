%{
#include <stdio.h>
#include "data.h"
#include "write.h"
#define YYERROR_VERBOSE 1
extern int yylex(void);
extern int yylineno;
void yyerror(Statement **st,char const *s);
%}

%define parse.error verbose

%token SELECT ADD UPDATE CREATE DROP REMOVE ON IN INTO NEW EQUALS FROM WHERE JOIN
%token STRING INTEGER FLOAT BOOL TYPE VARCHAR NAME POSITIVE_INT
%token LB RB LCB RCB QUOTE SEMICOLON DOT COMMA OTHER
%token COMPARISON CONTAINS

%left OR
%left AND

%type <name> NAME
%type <str> STRING
%type <intVal> INTEGER
%type <intVal> POSITIVE_INT
%type <boolVal> BOOL
%type <floatVal> FLOAT
%type <comp_type> EQUALS
%type <comp_type> COMPARISON
%type <comp_type> CONTAINS
%type <valType> TYPE
%type <logicOp> AND
%type <logicOp> OR

%type <statement> statement
%type <statement> createStatement
%type <statement> dropStatement
%type <statement> insertStatement
%type <statement> deleteStatement
%type <statement> updateStatement
%type <statement> selectStatement
%type <join> joinStatement
%type <compound_condition> whereStatement
%type <colValList> columnValueList
%type <colVal> columnValue
%type <selectList> selectList
%type <projectionList> projectionList
%type <projection> projection
%type <colList> columnDefinitions
%type <col> columnDefinition
%type <insertList> insertList
%type <compound_condition> compound_condition
%type <condition> condition
%type <compound_condition> compound_condition_select
%type <condition> condition_select
%type <value> value
%type <fullColumnName> fullColumnName
%type <name> columnName
%type <name> tableRowName
%type <name> newColumnName
%type <name> tableName

%union {
    char name[NAME_SIZE];
    char* str; 
    int64_t intVal;
    double floatVal;
    bool boolVal; 
    DATA_TYPE valType;
    COMP_TYPE comp_type;
    LOGICAL_OPERATOR logicOp; 
    Condition* condition;
    CompoundCondition* compound_condition;
    Join* join;
    Projection* projection;
    ProjectionList* projectionList;
    Statement* statement;
    Val* value;
    ColVal* colVal;
    ColValList* colValList; 
    ColList* colList;
    Column* col;
    FullColumnName* fullColumnName;
    InsertList* insertList;
    SelectList* selectList;
}


%parse-param {Statement **st}

%start input

%%

input:  %empty
|	    input statement SEMICOLON {passResult(st, $2); return 0;}
;

statement: 	    createStatement
|           dropStatement
|           insertStatement
|           deleteStatement
|           updateStatement
|           selectStatement
;

createStatement:    tableName DOT CREATE LB columnDefinitions RB {$$ = createCreateStatement($1, $5);}
;

columnDefinitions:  columnDefinition COMMA columnDefinitions {$$ = createColumnList($1, $3);}
|                   columnDefinition {$$ = createColumnList($1, NULL);}
;

columnDefinition:   columnName TYPE {$$ = createColumn($1, $2);}
|                   columnName VARCHAR LB POSITIVE_INT RB {$$ = createStringColumn($1,$4);}
;

dropStatement:      tableName DOT DROP LB RB {$$ = createDropStatement($1);}
;

insertStatement:    tableName DOT ADD LB insertList RB {$$ = createInsertStatement($1, $5);}
;

insertList:         NEW LCB columnValueList RCB COMMA insertList {$$ = createInsertList($3, $6);}
|                   NEW LCB columnValueList RCB {$$ = createInsertList($3, NULL);}
;

columnValueList:    columnValue COMMA columnValueList {$$ = createColumnValueList($1, $3);}
|                   columnValue {$$ = createColumnValueList($1, NULL);}
;

columnValue:       columnName EQUALS value {$$ = createColumnValue($1, $3);}
;

deleteStatement:    tableName DOT REMOVE LB RB {$$ = createDeleteStatement($1, NULL);}
|                   tableName DOT REMOVE LB RB DOT WHERE LB compound_condition RB {$$ = createDeleteStatement($1, $9);}
;

updateStatement:    tableName DOT UPDATE LB columnValueList RB {$$ = createUpdateStatement($1, $5, NULL);}
|                   tableName DOT UPDATE LB columnValueList RB DOT WHERE LB compound_condition RB {$$ = createUpdateStatement($1, $5, $10);}
;

selectStatement:    FROM tableRowName IN tableName joinStatement whereStatement SELECT selectList {$$ = createSelectStatement($4, $2, $5, $6, $8);}
;

joinStatement:  %empty {$$ = NULL;}
|               JOIN tableRowName IN tableName ON fullColumnName EQUALS fullColumnName {$$ = createJoin($2, $4, $6, $8);}
;

whereStatement: %empty {$$ = NULL;}
|               WHERE compound_condition_select {$$ = $2;}
;
 
selectList:     tableRowName {$$ = createSelectList($1, NULL);}
|               NEW LCB projectionList RCB {$$ = createSelectList(NULL, $3);}
;

projectionList:         projection COMMA projectionList     {$$ = createProjectionList($1, $3);}
|                       projection                          {$$ = createProjectionList($1, NULL);}
;

projection:             newColumnName EQUALS fullColumnName {$$ = createProjection($1, $3);}
;

compound_condition:     condition OR compound_condition  {$$ = createCompoundCondition($1, $2, $3);}
|                       condition AND compound_condition {$$ = createCompoundCondition($1, $2, $3);}
|                       condition                        {$$ = createCompoundCondition($1,OR_OP, NULL);}
;

condition:              columnName COMPARISON value {$$ = createCondition(createTableColumn(NULL,$1), $2, $3);}
|                       columnName EQUALS value {$$ = createCondition(createTableColumn(NULL,$1), $2, $3);}
|                       columnName CONTAINS STRING {$$ = createCondition(createTableColumn(NULL,$1), $2, createString($3));}
;

compound_condition_select:  condition_select OR compound_condition_select  {$$ = createCompoundCondition($1, $2, $3);}
|                           condition_select AND compound_condition_select {$$ = createCompoundCondition($1, $2, $3);}
|                           condition_select                        {$$ = createCompoundCondition($1,OR_OP, NULL);}
;

condition_select:   fullColumnName COMPARISON value {$$ = createCondition($1, $2, $3);}
|                   fullColumnName EQUALS value {$$ = createCondition($1, $2, $3);}
|                   fullColumnName CONTAINS STRING {$$ = createCondition($1, $2, createString($3));}
;


fullColumnName: tableRowName DOT columnName {$$ = createTableColumn($1, $3);}
;

value:      INTEGER {$$ = createIntValue($1);}
|           POSITIVE_INT {$$ = createIntValue($1);}
|           FLOAT {$$ = createFloatValue($1);}
|           BOOL {$$ = createBoolValue($1);}
|           STRING {$$ = createString($1);}
;

columnName:     NAME
;

tableRowName:   NAME
;

newColumnName:  NAME
;

tableName:      NAME
;

%%

void yyerror (Statement **st,char const *s) {
    fprintf (stderr, "got %s on line number %d\n", s, yylineno);
    *st = NULL;
}