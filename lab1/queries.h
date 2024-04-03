#ifndef QUERIES_H
#define QUERIES_H
#include <stdbool.h>
#include "database.h"
#include "table.h"
#include "page.h"


typedef enum QUERY_TYPE {
    CREATE,
    DROP,
    SELECT,
    UPDATE,
    INSERT,
    DELETE,
    JOIN
} QUERY_TYPE;

typedef enum LOGICAL_OPERATOR {
    AND,
    OR
} LOGICAL_OPERATOR;

typedef enum COMP_TYPE {
    EQUALS,
    NOT_EQUALS,
    GREATER,
    LESS,
    GREATE_EQUALS,
    LESS_EQUALS,
    LIKE
} COMP_TYPE;


typedef struct condition {
    char* column_name;
    COMP_TYPE comp_type;
    cell value;
} condition;

typedef struct compound_condition {
    condition* cond;
    LOGICAL_OPERATOR op;
    struct compound_condition* next_c_cond;
} compound_condition;

typedef struct {
    char* join_table_name;
    char* join_column;
    char* target_column;
} join;
typedef struct {
    char* table_name;
    compound_condition* where_conditions;
    join* join_tables;
    uint32_t join_number;
} select_query;

typedef struct insert_query {
    row* new_rows;
    uint32_t rows_count;
    char* table_name;
} insert_query;

typedef struct {
    char* table_name;
    row* new_row;
    compound_condition* where_conditions;
} update_query;

typedef struct {
    char* table_name;
    compound_condition* where_conditions;
} delete_query;

typedef struct {
    column* columns;
    uint32_t column_count;
    char* table_name;
} create_query;

typedef struct {
    char* table_name;
} drop_query;

typedef struct query {
    QUERY_TYPE type;
    union {
        select_query* select;
        insert_query* insert;
        update_query* update;
        delete_query* del;
        create_query* create_table;
        drop_query* drop_table;
    } query;
} query;

typedef enum {
    INTERNAL_ERROR,
    BAD_REQUEST,
    OK
} STATUS;

typedef struct query_result {
    char* info;
    row* rows;
    uint32_t rows_number;
    STATUS status;
} query_result;
query_result* executeQuery(database* db, query* qr);

#endif