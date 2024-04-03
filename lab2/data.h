#ifndef LAB2_DATA_H
#define LAB2_DATA_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#define NAME_SIZE 64

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


Column* createColumn(const char* col_name, DATA_TYPE type);
Column* createStringColumn(const char* col_name, uint32_t str_len);
ColList* createColumnList(Column* col, ColList* nextCol);
Statement* createCreateStatement(const char* table_name, ColList* colList);

Statement* createDropStatement(const char* tbl_name);

ColVal* createColumnValue(const char* col_name, Val* val);
ColValList* createColumnValueList(ColVal* colVal, ColValList* colValList);
InsertList* createInsertList(ColValList* row, InsertList* next);
Statement* createInsertStatement(const char* tbl_name,InsertList* rows);

Val *createIntValue(uint32_t intValue);
Val *createBoolValue(bool bl);
Val *createString(const char* str);
Val *createFloatValue(double dbl);
FullColumnName* createTableColumn(const char* row_name,const char* col_name);
Condition* createCondition(FullColumnName* col_name, COMP_TYPE type, Val* val);
CompoundCondition* createCompoundCondition(Condition* left, LOGICAL_OPERATOR op, CompoundCondition* right);
Projection* createProjection(const char* new_col, FullColumnName* src);
ProjectionList* createProjectionList(Projection* proj, ProjectionList* next);
SelectList* createSelectList(const char* rowName, ProjectionList* projList);
Join* createJoin(const char* rowName, const char* tblName, FullColumnName* left_col, FullColumnName* right_col);
Statement* createSelectStatement(const char* rowName, const char* tblName, Join* join, CompoundCondition* where, SelectList* select);
Statement* createUpdateStatement(const char* tblName, ColValList* colVals, CompoundCondition* cond);
Statement* createDeleteStatement(const char* tblName, CompoundCondition* cond);


void freeStatement(Statement* st);
void passResult(Statement** stmt, Statement* res);

//add free functions
#endif