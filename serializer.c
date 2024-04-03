#include "serializer.h"


 

CreateStmt_* serialize_cr(CreateStmt* cr){
    CreateStmt_* crt = g_object_new(TYPE_CREATE_STMT_, NULL);
    g_object_set(crt, "tblName", cr->tblName, NULL);
    GPtrArray* cols = g_ptr_array_new();
    ColList* temp = cr->cols;
    do 
    {
        Column_* cl = g_object_new(TYPE_COLUMN_, NULL);
        g_object_set(cl, "column_name", temp->col->column_name,
                         "data_type", temp->col->data_type,
                         "size", temp->col->size, NULL);
        g_ptr_array_add(cols,cl);
        temp = temp->next_col;
    } while (temp);
    g_object_set(crt,"cols",cols, NULL);
    return crt;
}
DropStmt_* serialize_dr(DropStmt* dr){
    DropStmt_* drop = g_object_new(TYPE_DROP_STMT_, NULL);
    g_object_set(drop,"tblName",dr->tblName,NULL);
    return drop;        
}


Cell_* serialize_cell(Val* val){
    Cell_* cl = g_object_new(TYPE_CELL_, NULL);
    switch (val->type)
    {
    case INT_TYPE:
        g_object_set(cl, "integer", val->cell.integer, NULL);
        break;
    case BOOL_TYPE:
        g_object_set(cl, "bl", val->cell.bl, NULL);
        break;
    case FLOAT_TYPE:
        g_object_set(cl, "dbl", val->cell.dbl, NULL);
        break;
    case STRING_TYPE:
        g_object_set(cl, "str", val->cell.str, NULL);
        break;
    default:
        break;
    }
    return cl;
}

InsertStmt_* serialize_ins(InsertStmt* ins) {
    InsertStmt_* insert = g_object_new(TYPE_INSERT_STMT_, NULL);
    g_object_set(insert, "tblName", ins->tblName, NULL);
    GPtrArray* rows = g_ptr_array_new();
    InsertList* il = ins->rows;
    do{
        GPtrArray* vals = g_ptr_array_new();
        ColValList* cvl = il->row;
        do{
            ColVal_* cv = g_object_new(TYPE_COL_VAL_, NULL);
            g_object_set(cv,"col", cvl->colVal->col, 
                            "val", g_object_new(TYPE_VAL_,  "cell", serialize_cell(cvl->colVal->val), 
                                                            "type", cvl->colVal->val->type, 
                                                            NULL),
                            NULL);
            g_ptr_array_add(vals, cv);
            cvl = cvl->next;
        } while (cvl);  
        g_ptr_array_add(rows,vals);
        il = il->nextRow;
    } while (il);
    g_object_set(insert, "rows", rows, NULL);
    return insert;
}

CompoundCondition_* serialize_c_cond(CompoundCondition* c_cond){
    CompoundCondition_* c_cond_ = g_object_new(TYPE_COMPOUND_CONDITION_, NULL);
    g_object_set(c_cond_, "op", c_cond->op, NULL);
    CompoundCondition* temp = c_cond;
    Condition_* cond_ = g_object_new(TYPE_CONDITION_, NULL);
    FullColumnName_* fcn = g_object_new(TYPE_FULL_COLUMN_NAME_, NULL);
    if (c_cond->left->column->row_name) {
        g_object_set(fcn, "row_name", temp->left->column->row_name, NULL);
    }
    else
    {
        c_cond_->left->column->__isset_row_name = 0;
    }
    g_object_set(fcn, "col_name", temp->left->column->col_name, NULL);
    g_object_set(cond_, "column", fcn,
                        "comp_type", temp->left->comp_type,
                        "value", g_object_new(TYPE_VAL_,"cell", serialize_cell(temp->left->value), 
                                                        "type", temp->left->value->type, 
                                                        NULL),
                        NULL);
    g_object_set(c_cond_, "left", cond_, NULL);
    if (temp->right){
        g_ptr_array_add(c_cond_->right, serialize_c_cond(temp->right));
        c_cond_->__isset_right = 1;
    }
    return c_cond_;
}

SelectStmt_* serialize_sel(SelectStmt* sel){
    return NULL;
}
DeleteStmt_* serialize_del(DeleteStmt* del){
    DeleteStmt_* dl = g_object_new(TYPE_DELETE_STMT_, NULL);
    g_object_set(dl,"tblName", del->tblName, NULL);
    if (del->cond){
        g_object_set(dl,"cond", serialize_c_cond(del->cond),NULL);
        dl->__isset_cond = 1;
    }
    return dl;
}
UpdateStmt_* serialize_upd(UpdateStmt* upd){
    UpdateStmt_* update = g_object_new(TYPE_UPDATE_STMT_,NULL);
    g_object_set(update,"tblName", upd->tblName,
                        "cond", serialize_c_cond(upd->cond), 
                        NULL);
    ColValList* cvl = upd->colVals;
    GPtrArray* new_row = g_ptr_array_new();
    do
    {
        ColVal_* cv = g_object_new(TYPE_COL_VAL_, NULL);
        g_object_set(cv,"col", cvl->colVal->col, 
                        "val", g_object_new(TYPE_VAL_,  "cell", serialize_cell(cvl->colVal->val), 
                                                        "type", cvl->colVal->val->type, 
                                                        NULL),
                        NULL);
        g_ptr_array_add(new_row, cv);
        cvl = cvl->next;
    } while (cvl);
    
    return update;
}
Statement_* serialize(Statement* stmt){
    printStatement(stmt);
    Statement_* ser_st = g_object_new(TYPE_STATEMENT_, NULL);
    Stmt_* ser_stmt = g_object_new(TYPE_STMT_, NULL); 
    switch (stmt->st_type)
    {
    case CREATE_STMT:
        g_object_set(ser_stmt, "create_tbl", serialize_cr(stmt->stmt.create_tbl),NULL);
        break;
    case DROP_STMT:
        g_object_set(ser_stmt, "drop_tbl", serialize_dr(stmt->stmt.drop_tbl),NULL);
        break;
    case INSERT_STMT:
        g_object_set(ser_stmt, "insert", serialize_ins(stmt->stmt.insert),NULL);
        break;
    case DELETE_STMT:
        g_object_set(ser_stmt, "dl", serialize_del(stmt->stmt.del),NULL);
        break;
    case SELECT_STMT:
        g_object_set(ser_stmt, "select", serialize_sel(stmt->stmt.select),NULL);
        break;
    case UPDATE_STMT:
        g_object_set(ser_stmt, "update", serialize_upd(stmt->stmt.update),NULL);
        break;
    default:
        break;
    }
    g_object_set(ser_st,"st_type", stmt->st_type,"stmt", ser_stmt, NULL);
    return ser_st;
}