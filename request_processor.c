#include "request_processor.h"
#include <string.h>


static bool checkScheme(database* db,const char* table_name,GPtrArray* colVals, query_result* result) {
    table* check_scheme = findTable(db, table_name);
    if (!check_scheme) {
        result->status = BAD_REQUEST;
        result->info = strdup("Table does not exists");
        deallocateTable(check_scheme);
        return false;
    }
    ColVal_* colVal = g_object_new(TYPE_COL_VAL_, NULL);
    if (colVals->len != check_scheme->hdr->number_of_columns) {
        result->status = BAD_REQUEST;
        result->info = getTableScheme(check_scheme);
        deallocateTable(check_scheme);
        return false;
    }
    for (size_t i = 0; i < colVals->len; i++){
        colVal = g_ptr_array_index(colVals, i);
        if (strcmp(check_scheme->columns[i].column_name, colVal->col) != 0) {
            result->status = BAD_REQUEST;
            result->info = getTableScheme(check_scheme);
            deallocateTable(check_scheme);
            return false;
        }
        if (check_scheme->columns[i].data_type != colVal->val->type) {
            result->status = BAD_REQUEST;
            result->info = getTableScheme(check_scheme);
            deallocateTable(check_scheme);
            return false;
        }
    }
    return true;
}

static condition* getCond(database *db,Condition_* cond_,const char* row_name, const char* tbl_name, bool* err,query_result* result){
    table* tbl = findTable(db,tbl_name);
    if (!tbl) {
        *err = false;
        result->status = BAD_REQUEST;
        result->info = strdup("Table does not exists");
        return NULL;
    }
    if (getColumnNumber(tbl, cond_->column->col_name) == -1 || tbl->columns[getColumnNumber(tbl, cond_->column->col_name)].data_type != cond_->value->type) {
        *err = true;
        result->status = BAD_REQUEST;
        result->info = getTableScheme(tbl);
        return NULL;
    }
    condition* cond = malloc(sizeof(condition));
    cond->column_name = strdup(cond_->column->col_name);
    cond->comp_type = cond_->comp_type;
    switch (cond_->value->type)
    {
    case DATA_TYPE__INT_TYPE:
        cond->value.integer = cond_->value->cell->integer;
        break;
    case DATA_TYPE__BOOL_TYPE:
        cond->value.bl = cond_->value->cell->bl;
        break;
    case DATA_TYPE__FLOAT_TYPE:
        cond->value.dbl = cond_->value->cell->dbl;
        break;
    case DATA_TYPE__STRING_TYPE:
        cond->value.str = strdup(cond_->value->cell->str);
        break;
    default:
        break;
    }
    return cond;
}

static compound_condition* getCompCond(database* db, CompoundCondition_* c_cond_, const char* row_name, const char* tbl_name, bool* err, query_result* result) {
    compound_condition* c_cond = malloc(sizeof(compound_condition));
    c_cond->cond = getCond(db,c_cond_->left,row_name,tbl_name, err,result);
    if (!err) {
        free(c_cond);
        return NULL;
    }
    c_cond->op = c_cond_->op;
    if (c_cond_->__isset_right) {
        c_cond->next_c_cond = getCompCond(db,g_ptr_array_index(c_cond_->right, 0),row_name,tbl_name,err, result);
    }
    else {
        c_cond->next_c_cond = NULL;
    }
    return c_cond;
}

ServerResponse* process_client_request(const Statement_* stmt, database* db) {
    query* from_client = malloc(sizeof(query));
    ServerResponse* response = g_object_new(TYPE_SERVER_RESPONSE, NULL);
    query_result* result = calloc(1, sizeof(query_result));
    bool checkStmt = true;
    switch (stmt->st_type)
    {
    case QUERY_TYPE__CREATE_STMT:
        create_query* create = malloc(sizeof(create_query));
        create->table_name = strdup(stmt->stmt->create_tbl->tblName);
        create->column_count = stmt->stmt->create_tbl->cols->len;
        create->columns = calloc(create->column_count, sizeof(column));
        Column_* col = g_object_new(TYPE_COLUMN_, NULL);
        for (size_t i = 0; i < create->column_count; i++) {
            col = g_ptr_array_index(stmt->stmt->create_tbl->cols, i);
            strcpy(create->columns[i].column_name, col->column_name);
            create->columns[i].data_type = col->data_type;
            create->columns[i].size = col->size;
        }
        from_client->type = CREATE;
        from_client->query.create_table = create;
        break;
    case QUERY_TYPE__DROP_STMT:
        drop_query* drop = malloc(sizeof(drop_query));
        drop->table_name = strdup(stmt->stmt->drop_tbl->tblName);
        from_client->type = DROP;
        from_client->query.drop_table = drop;
        break;
    case QUERY_TYPE__SELECT_STMT:
        g_object_set(response, "info", strdup(stmt->stmt->select->tblName), "status", STATUS__OK, NULL);
        break;
    case QUERY_TYPE__UPDATE_STMT:
        update_query* update = malloc(sizeof(update_query));
        update->table_name = strdup(stmt->stmt->update->tblName);
        update->where_conditions = getCompCond(db, stmt->stmt->update->cond, NULL, update->table_name, &checkStmt, result);
        if (!checkStmt) {
            break;
        }
        update->new_row = malloc(sizeof(cell) * stmt->stmt->update->new_row->len);
        ColVal_* colVal = g_object_new(TYPE_COL_VAL_, NULL);
        for (size_t y = 0; y < stmt->stmt->update->new_row->len; y++) {
            colVal = g_ptr_array_index(stmt->stmt->update->new_row, y);
            switch (colVal->val->type)
            {
            case DATA_TYPE__INT_TYPE:
                update->new_row->cells[y].integer = colVal->val->cell->integer;
                break;
            case DATA_TYPE__BOOL_TYPE:
                update->new_row->cells[y].bl = colVal->val->cell->bl;
                break;
            case DATA_TYPE__FLOAT_TYPE:
                update->new_row->cells[y].dbl = colVal->val->cell->dbl;
                break;
            case DATA_TYPE__STRING_TYPE:
                update->new_row->cells[y].str = strdup(colVal->val->cell->str);
                break;
            default:
                break;
            }
        }
        break;
    case QUERY_TYPE__INSERT_STMT:
        GPtrArray* arr;
        for (size_t i = 0; i < stmt->stmt->insert->rows->len; i++) {
            arr = g_ptr_array_index(stmt->stmt->insert->rows, i);
            checkStmt = checkStmt && checkScheme(db, stmt->stmt->insert->tblName, arr, result);
        }
        if (!checkStmt) {
            break;
        }
        insert_query* insert = malloc(sizeof(insert_query));
        insert->rows_count = stmt->stmt->insert->rows->len;
        insert->table_name = stmt->stmt->insert->tblName;
        insert->new_rows = malloc(sizeof(row) * insert->rows_count);
        for (size_t i = 0; i < insert->rows_count; i++) {
            arr = g_ptr_array_index(stmt->stmt->insert->rows, i);
            insert->new_rows[i].cells = malloc(sizeof(cell) * arr->len);
            ColVal_* colVal = g_object_new(TYPE_COL_VAL_, NULL);
            for (size_t y = 0; y < arr->len; y++) {
                colVal = g_ptr_array_index(arr, y);
                switch (colVal->val->type)
                {
                case DATA_TYPE__INT_TYPE:
                    insert->new_rows[i].cells[y].integer = colVal->val->cell->integer;
                    break;
                case DATA_TYPE__BOOL_TYPE:
                    insert->new_rows[i].cells[y].bl = colVal->val->cell->bl;
                    break;
                case DATA_TYPE__FLOAT_TYPE:
                    insert->new_rows[i].cells[y].dbl = colVal->val->cell->dbl;
                    break;
                case DATA_TYPE__STRING_TYPE:
                    insert->new_rows[i].cells[y].str = strdup(colVal->val->cell->str);
                    break;
                default:
                    break;
                }
            }
        }
        from_client->type = INSERT;
        from_client->query.insert = insert;
        break;
    case QUERY_TYPE__DELETE_STMT:
        delete_query* del = malloc(sizeof(delete_query));
        del->table_name = strdup(stmt->stmt->dl->tblName);
        if (stmt->stmt->dl->__isset_cond){
            del->where_conditions = getCompCond(db, stmt->stmt->dl->cond, NULL, del->table_name, &checkStmt, result);
            if (!checkStmt) {
                break;
            }
        }
        else
            del->where_conditions = NULL;
        from_client->type = DELETE;
        from_client->query.del = del;
        break;
    default:
        g_object_set(response, "info", "unknown statement", "status", STATUS__OK, NULL);
        break;
    }
    if (checkStmt)
        result = executeQuery(db, from_client);
    g_object_set(response, "info", strdup(result->info), "status", result->status, NULL);
    return response;
}