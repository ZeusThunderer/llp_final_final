 ## Лабораторная работа № 2

---

### Сборка

Requirements: 
* Flex 2.6.4
* Bison 3.5.1
* g++ 9.4.0

``` shell
git clone https://gitlab.se.ifmo.ru/Alkarized/llp_task2.git
cd llp_task2
make
```

Порождает исполняемый файл `ans`, ``` ./ans - для запуска```

--- 
### Цель задания

С помощью выбранного синтаксического анализатора реализовать модуль, который разбирает язык запросов, выданного по варианту, 
к базе данных и строит дерево разбора. В модуле необходимо реализовать описание команд создания, вставки, выборки, 
удаления и обновления элементов данных.

---

### Задачи

1. Выбрать и изучить средство синтаксического анализа, совместивое с языком C/C++
2. Разработать структуры для построения дерева запросов и реализовать работу с ними.
3. Разработать грамматику языка запросов (LINQ) и задать ее в синтаксическом анализаторе с последующим выделением в них структур
4. Разработать интерфейс, который выводит в удобном виде полученные структуры.

---

### Описание работы

Для выполнения задачи был использован лексер Flex и генератор парсеров Bison.

**Структура модуля:**

- **lexerLINQ.l** - правила запросов и токена (лексический анализ)
- **parserLINQ.y** - правила выделения из токенов структур - задается грамматика (синтаксический анализ)
- **data.h** / **data.c** - описание структур / работа с ними
- **write.h** / **write.c** - вывод дерева в виде JSON-объекта.

#### В результате у нас должна быть заполнена данная структура.

```c
typedef struct{
    QUERY_TYPE st_type;
    union{
        CreateStmt* create_tbl;
        DropStmt* drop_tbl;
        InsertStmt* insert;
        SelectStmt* select;
        UpdateStmt* update;
        DeleteStmt* del;
    } stmt;
} Statement;
```
#### Соотвественно подструктуры общей структуры таковы:

```c
typedef enum DATA_TYPE
{
	INT_TYPE,
	FLOAT_TYPE,
	STRING_TYPE,
	BOOL_TYPE
} DATA_TYPE;


typedef union Cell {
	int64_t integer;
	double dbl;
	bool bl;
	char* str;
} Cell;
typedef enum QUERY_TYPE {
    CREATE_STMT,
    DROP_STMT,
    SELECT_STMT,
    UPDATE_STMT,
    INSERT_STMT,
    DELETE_STMT
} QUERY_TYPE;

typedef enum LOGICAL_OPERATOR {
    AND_OP,
    OR_OP
} LOGICAL_OPERATOR;


typedef struct Column
{
	char* column_name;
	DATA_TYPE data_type;
	uint16_t size;
} Column;

typedef struct ColList{
    Column* col;
    struct ColList* next_col;
} ColList;
typedef struct{
    ColList* cols;
    char* tblName;
} CreateStmt;

typedef struct Val{
    Cell cell;
    DATA_TYPE type;
} Val;
typedef struct ColVal{
    char* col;
    Val* val;
} ColVal;
typedef struct ColValList{
    ColVal* colVal;
    struct ColValList* next;
} ColValList;
typedef struct InsertList{
    ColValList* row;
    struct InsertList* nextRow;
} InsertList;
typedef struct InsertStmt{
    char* tblName;
    InsertList* rows;
} InsertStmt;

typedef struct{
    char* tblName;
} DropStmt;

typedef enum COMP_TYPE {
    EQUALS_COMP,
    NOT_EQUALS_COMP,
    GREATER_COMP,
    LESS_COMP,
    GREATER_EQUALS_COMP,
    LESS_EQUALS_COMP,
    LIKE_COMP
} COMP_TYPE;
typedef struct{
    char* row_name;
    char* col_name;
} FullColumnName; 

typedef struct Condition{
    FullColumnName* column;
    COMP_TYPE comp_type;
    Val* value;
} Condition;
typedef struct CompoundCondition {
    Condition* left;
    LOGICAL_OPERATOR op;
    struct CompoundCondition* right;
} CompoundCondition;
typedef struct Projection{
    char* new_col;
    FullColumnName* src;
} Projection;
typedef struct ProjectionList{
    Projection* proj;
    struct ProjectionList* next;
} ProjectionList;
typedef struct SelectList{
    char* rowName;
    ProjectionList* projList;
} SelectList;
typedef struct Join{
    char* rowName;
    char* tblName;
    FullColumnName* left_col;
    FullColumnName* right_col;
} Join;
typedef struct SelectStmt{
    char* rowName;
    char* tblName;
    Join* join;
    CompoundCondition* where;
    SelectList* select;
} SelectStmt;
typedef struct UpdateStmt{
    char* tblName;
    ColValList* colVals;
    CompoundCondition* cond;
} UpdateStmt;
typedef struct DeleteStmt{
    char* tblName;
    CompoundCondition* cond;
} DeleteStmt;
```

#### Список самих токенов написанных благодря `Flex`

```c++
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


```

#### Пример грамматика написанной на `Bison`:

```c++
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
Таким образом, мы получаем готовые структуры, который и выводится в виде JSON в стандартный поток вывода.

---

### Примеры работы модуля:

#### create: 

``` tableName.create(a int, b bool, c string, d double);``` 

#### result: 
``` json
CREATE TABLE
{
   Table: "sus",
   Columns:
   [
      {
         Column: "a",
         type: "INTEGER",
         size: 8
      },
      {
         Column: "b",
         type: "FLOAT",
         size: 8
      },
      {
         Column: "c",
         type: "STRING",
         size: 255
      }
   ]
}
```

#### insert:

``` tableName.add(new {a1 = 12, a2 = "asd"}, new {a1 = true, a2 = 12131.123});  ```

#### result: 

``` json
INSERT
{
   Table: "tableName",
   Rows:
   [
      Vals:
      [
         {
            Column: "a1"
            type: "INTEGER",
            val:12
         },
         {
            Column: "a2"
            type: "STRING",
            val:"asd"
         }
      ],
      Vals:
      [
         {
            Column: "a1"
            type: "BOOL",
            val:1
         },
         {
            Column: "a2"
            type: "FLOAT",
            val:12131.123000
         }
      ]
   ]
}
```

#### delete:

```  tableName.remove().where(a > 2 AND b = 3 OR c LIKE "sad"); ```

#### result: 

``` json
DELETE
{
   Table: "tableName",
   compound_condition:
   {
      condition:
      {
         Column:
         {
            col_name: "a",
         },
         comp_type: ">",
         val:
         {
            type: "INTEGER",
            val:2
         }
      },
      operation: AND,
      compound_condition:
      {
         condition:
         {
            Column:
            {
               col_name: "b",
            },
            comp_type: "=",
            val:
            {
               type: "INTEGER",
               val:3
            }
         },
         operation: OR,
         compound_condition:
         {
            condition:
            {
               Column:
               {
                  col_name: "c",
               },
               val:
               {
                  type: "STRING",
                  val:"sad"
               }
            }
         }
      }
   }
}

```

#### update:

```  tableName.update(a = "a", b = 12, c = false, d = 21.21).where(a = "b" OR c = true);  ```

#### result:

``` json
UPDATE
{
   Table: "tableName",
   Row:
   [
      {
         Column: "a"
         type: "STRING",
         val:"a"
      },
      {
         Column: "b"
         type: "INTEGER",
         val:12
      },
      {
         Column: "c"
         type: "BOOL",
         val:0
      },
      {
         Column: "d"
         type: "FLOAT",
         val:21.210000
      }
   ]
   compound_condition:
   {
      condition:
      {
         Column:
         {
            col_name: "a",
         },
         comp_type: "=",
         val:
         {
            type: "STRING",
            val:"b"
         }
      },
      operation: OR,
      compound_condition:
      {
         condition:
         {
            Column:
            {
               col_name: "c",
            },
            comp_type: "=",
            val:
            {
               type: "BOOL",
               val:1
            }
         }
      }
   }
}
``` 

#### select:

```  from x in tableName where x.a1 = "asd" select x; ```

#### result:

``` json
SELECT
{
      row_name: "x",
      Table: "tableName",
      compound_condition:
      {
         condition:
         {
            Column:
            {
               row_name: "x",
               col_name: "a1",
            },
            comp_type: "=",
            val:
            {
               type: "STRING",
               val:"asd"
            }
         }
      }
}
```

---

### Вывод:

Выполняя данную лабораторную работу я познакомился с такими мощными инструментами как Flex и Bison, 
написал можно сказать свой парсер языка запросов. Достаточно полезное знание, а так же и не трудное в усвоение. 