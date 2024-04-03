#include "data.h"





Column* createColumn(const char* col_name, DATA_TYPE type){
    Column* col = (Column*)malloc(sizeof(Column));
    col->data_type = type;
    col->size = sizeof(Cell);
    col->column_name = strdup(col_name);
    return col;
}  
Column* createStringColumn(const char* col_name, uint32_t str_len){
    Column* col = (Column*)malloc(sizeof(Column));
    col->data_type = STRING_TYPE;
    col->size = str_len;
    col->column_name = strdup(col_name);
    return col;
}
ColList* createColumnList(Column* col, ColList* nextCol){
    ColList* colList = (ColList*)malloc(sizeof(ColList));
    colList->col = col;
    colList->next_col = nextCol;
    return colList;
}
Statement* createCreateStatement(const char* table_name, ColList* colList){
    Statement* stmt = (Statement*)malloc(sizeof(Statement));
    stmt->st_type = CREATE_STMT;
    stmt->stmt.create_tbl = (CreateStmt*)malloc(sizeof(CreateStmt));
    stmt->stmt.create_tbl->tblName = strdup(table_name);
    stmt->stmt.create_tbl->cols = colList;
    return stmt;
}

Statement* createDropStatement(const char* tbl_name){
    DropStmt* dr = (DropStmt*)malloc(sizeof(DropStmt));
    dr->tblName = strdup(tbl_name);
    Statement* stmt = (Statement*)malloc(sizeof(Statement));
    stmt->st_type = DROP_STMT;
    stmt->stmt.drop_tbl = dr;
    return stmt;
}

ColVal* createColumnValue(const char* col_name, Val* val){
    ColVal* colVal = (ColVal*)malloc(sizeof(ColVal));
    colVal->col = strdup(col_name);
    colVal->val = val;
    return colVal;
}
ColValList* createColumnValueList(ColVal* colVal, ColValList* colValList){
    ColValList *cvl = (ColValList*)malloc(sizeof(ColValList));
    cvl->colVal = colVal;
    cvl->next = colValList;
    return cvl;
}
InsertList* createInsertList(ColValList* row, InsertList* next){
    InsertList* insList = (InsertList*)malloc(sizeof(InsertList));
    insList->row = row;
    insList->nextRow = next;
    return insList;
}
Statement* createInsertStatement(const char* tbl_name,InsertList* rows){
    InsertStmt* ins = (InsertStmt*)malloc(sizeof(InsertStmt));
    ins->tblName = strdup(tbl_name);
    ins->rows = rows;
    Statement* stmt = (Statement*)malloc(sizeof(Statement));
    stmt->st_type = INSERT_STMT;
    stmt->stmt.insert = ins;
    return stmt;
}

Val *createIntValue(uint32_t intValue){
    Val *value = (Val*)(calloc(1, sizeof(Val)));
    value->type = INT_TYPE;
    value->cell.integer = intValue;
    return value;
}
Val *createBoolValue(bool bl){
    Val *value = (Val*)(calloc(1, sizeof(Val)));
    value->type = BOOL_TYPE;
    value->cell.bl = bl;
    return value;
}
Val *createString(const char* str){
    Val *value = (Val*)(calloc(1, sizeof(Val)));
    value->type = STRING_TYPE;
    value->cell.str = strdup(str);
    return value;
}
Val *createFloatValue(double dbl){
    Val *value = (Val*)(calloc(1, sizeof(Val)));
    value->type = FLOAT_TYPE;
    value->cell.dbl = dbl;
    return value;
}
FullColumnName* createTableColumn(const char* row_name,const char* col_name){
    FullColumnName* fcn = (FullColumnName*)malloc(sizeof(FullColumnName));
    if (row_name)
        fcn->row_name = strdup(row_name);
    else
        fcn->row_name = NULL;
    fcn->col_name = strdup(col_name);
    return fcn;    
}
Condition* createCondition(FullColumnName* col_name, COMP_TYPE type, Val* val){
    Condition* cond = (Condition*)malloc(sizeof(Condition));
    cond->column = col_name;
    cond->comp_type = type;
    cond->value = val;
    return cond;
}
CompoundCondition* createCompoundCondition(Condition* left, LOGICAL_OPERATOR op, CompoundCondition* right){
    CompoundCondition* cc = (CompoundCondition*)malloc(sizeof(CompoundCondition));
    cc->left = left;
    cc->op = op;
    cc->right = right;
    return cc;

}
Projection* createProjection(const char* new_col, FullColumnName* src){
    Projection* proj = (Projection*)malloc(sizeof(Projection));
    proj->new_col = strdup(new_col);
    proj->src = src;
    return proj;
}
ProjectionList* createProjectionList(Projection* proj, ProjectionList* next){
    ProjectionList* pl = (ProjectionList*)malloc(sizeof(ProjectionList));
    pl->proj = proj;
    pl->next = next;
    return pl;
}
SelectList* createSelectList(const char* rowName, ProjectionList* projList){
    SelectList* sl = (SelectList*)malloc(sizeof(SelectList));
    if (rowName)
        sl->rowName = strdup(rowName);
    else 
        sl->rowName = NULL;
    sl->projList = projList;
    return sl;
}
Join* createJoin(const char* rowName, const char* tblName, FullColumnName* left_col, FullColumnName* right_col){
    Join* join = (Join*)malloc(sizeof(Join));
    join->rowName = strdup(rowName);
    join->tblName = strdup(tblName);
    join->left_col = left_col;
    join->right_col = right_col;
    return join;    
}
Statement* createSelectStatement(const char* rowName, const char* tblName, Join* join, CompoundCondition* where, SelectList* select){
    SelectStmt* sel = (SelectStmt*)malloc(sizeof(SelectStmt));
    sel->rowName = strdup(rowName);
    sel->tblName = strdup(tblName);
    sel->join = join;
    sel->where = where;
    sel->select = select;
    Statement* stmt = (Statement*)malloc(sizeof(Statement));
    stmt->st_type = SELECT_STMT;
    stmt->stmt.select = sel;
    return stmt; 
}
Statement* createUpdateStatement(const char* tblName, ColValList* colVals, CompoundCondition* cond){
    UpdateStmt* ud = (UpdateStmt*)malloc(sizeof(UpdateStmt));
    ud->tblName = strdup(tblName);
    ud->colVals = colVals;
    ud->cond = cond;
    Statement* stmt = (Statement*)malloc(sizeof(Statement));
    stmt->st_type = UPDATE_STMT;
    stmt->stmt.update = ud;
    return stmt; 
}
Statement* createDeleteStatement(const char* tblName, CompoundCondition* cond){
    DeleteStmt* del = (DeleteStmt*)malloc(sizeof(DeleteStmt));
    del->tblName = strdup(tblName);
    del->cond = cond;
    Statement* stmt = (Statement*)malloc(sizeof(Statement));
    stmt->st_type = DELETE_STMT;
    stmt->stmt.del = del;
    return stmt; 
}


void freeStatement(Statement* st){
    free(st);
    st = NULL;
}

void passResult(Statement** stmt, Statement* res){
    if (res) {
        *stmt = res;
    } else {
        *stmt = NULL;
    }
}