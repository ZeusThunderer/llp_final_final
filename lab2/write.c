#include "write.h"

#define tab() for (uint32_t i = 0; i < tabs; i++){ printf("   "); }

uint32_t tabs = 0;

 
void printType(DATA_TYPE type){
    switch (type)
    {
    case INT_TYPE:
        tab();
        printf("type: \"INTEGER\",\n");
        break;
    case STRING_TYPE:
        tab();
        printf("type: \"STRING\",\n");
        break;
    case FLOAT_TYPE:
        tab();
        printf("type: \"FLOAT\",\n");
        break;
    case BOOL_TYPE:
        tab();
        printf("type: \"BOOL\",\n");
        break;
    default:
        break;
    }
}

void printCreate(CreateStmt* cr){
    tab();
    printf("Table: \"%s\",\n", cr->tblName);
    tab();
    printf("Columns:\n");
    tab();
    printf("[\n");
    tabs++;
    ColList* temp = cr->cols;
    do 
    {
        tab();
        printf("{\n");
        tabs++;
        tab();
        printf("Column: \"%s\",\n", temp->col->column_name);
        printType(temp->col->data_type);
        tab();
        printf("size: %d\n", temp->col->size);  
        tabs--;
        tab();
        temp = temp->next_col;
        if (temp)
            printf("},\n");
        else
            printf("}\n");
    } while (temp);
    tabs--;
    tab();
    printf("]\n");
    
}
void printDrop(DropStmt* dr){
    tab();
    printf("Table: \"%s\"\n", dr->tblName);
    tabs--;
}
void printInsert(InsertStmt* ins) {
    tab();
    printf("Table: \"%s\",\n", ins->tblName);
    tab();
    printf("Rows:\n");
    tab();
    printf("[\n");
    tabs++;
    InsertList* il = ins->rows;
    ColValList* cells;
    do
    {
        tab()
        printf("Vals:\n");
        tab();
        printf("[\n");
        tabs++;
        cells = il->row;
        do
        {
            tab();
            printf("{\n");
            tabs++;
            tab();
            printf("Column: \"%s\"\n", cells->colVal->col);
            printType(cells->colVal->val->type);
            switch (cells->colVal->val->type)
            {
            case INT_TYPE:
                tab();
                printf("val:%ld\n", cells->colVal->val->cell.integer);
                break;
            case STRING_TYPE:
                tab();
                printf("val:\"%s\"\n", cells->colVal->val->cell.str);
                break;
            case FLOAT_TYPE:
                tab();
                printf("val:%f\n", cells->colVal->val->cell.dbl);
                break;
            case BOOL_TYPE:
                tab();
                printf("val:%d\n", cells->colVal->val->cell.bl);
                break;
            default:
                break;
            }
            cells = cells->next;
            tabs--;
            tab()
            if (cells)
                printf("},\n");
            else
                printf("}\n");
        } while (cells);
        il = il->nextRow;
        tabs--;
        tab()
        if (il)
            printf("],\n");
        else
            printf("]\n");
    } while (il);
    tabs--;
    tab();
    printf("]\n");
}


void printCompoundCondition(CompoundCondition* c_cond){
    tab();
    printf("compound_condition:\n");
    tab();
    printf("{\n");
    tabs++;
    tab();
    printf("condition:\n");
    tab();
    printf("{\n");
    tabs++;
    tab();
    printf("Column:\n");
    tab();
    printf("{\n");
    tabs++;
    if(c_cond->left->column->row_name){
    tab();
    printf("row_name: \"%s\",\n",c_cond->left->column->row_name);
    }
    tab();
    printf("col_name: \"%s\",\n",c_cond->left->column->col_name);
    tabs--;
    tab();
    printf("},\n");
    switch (c_cond->left->comp_type)
    {
    case EQUALS_COMP:
        tab();
        printf("comp_type: \"=\",\n");
        break;
    case NOT_EQUALS_COMP:
        tab();
        printf("comp_type: \"!=\",\n");
        break;
    case GREATER_COMP:
        tab();
        printf("comp_type: \">\",\n");
        break;
    case GREATER_EQUALS_COMP:
        tab();
        printf("comp_type: \">=\",\n");
        break;
    case LESS_COMP:
        tab();
        printf("comp_type: \"<\",\n");
        break;
    case LESS_EQUALS_COMP:
        tab();
        printf("comp_type: \"<=\",\n");
        break;
    default:
        break;
    }
    tab();
    printf("val:\n");
    tab();
    printf("{\n");
    tabs++;
    printType(c_cond->left->value->type);
    switch (c_cond->left->value->type)
    {
    case INT_TYPE:
        tab();
        printf("val:%ld\n", c_cond->left->value->cell.integer);
        break;
    case STRING_TYPE:
        tab();
        printf("val:\"%s\"\n", c_cond->left->value->cell.str);
        break;
    case FLOAT_TYPE:
        tab();
        printf("val:%f\n", c_cond->left->value->cell.dbl);
        break;
    case BOOL_TYPE:
        tab();
        printf("val:%d\n", c_cond->left->value->cell.bl);
        break;
    default:
        break;
    }
    tabs--;
    tab()
    printf("}\n");
    tabs--;
    tab()
    if (c_cond->right){
        printf("},\n");   
        switch (c_cond->op)
        {
        case OR_OP:
            tab();
            printf("operation: OR,\n");
            break;
        case AND_OP:
            tab();
            printf("operation: AND,\n");
            break;
        default:
            break;
        }

        printCompoundCondition(c_cond->right);
    }
    else{
        printf("}\n");
    }
    tabs--;
    tab()
    printf("}\n");
}

void printSelect(SelectStmt* sel){
    tab();
    printf("Table: \"%s\",\n", sel->tblName);
}
void printDelete(DeleteStmt* del){
    tab();
    printf("Table: \"%s\",\n", del->tblName);
}
void printUpdate(UpdateStmt* upd){
    tab();
    printf("Table: \"%s\",\n", upd->tblName);
    tab();
    ColValList* cells = upd->colVals;
    printf("Row:\n");
    tab();
    printf("[\n");
    tabs++;
    do
    {
        tab();
        printf("{\n");
        tabs++;
        tab();
        printf("Column: \"%s\"\n", cells->colVal->col);
        printType(cells->colVal->val->type);
        switch (cells->colVal->val->type)
        {
        case INT_TYPE:
            tab();
            printf("val:%ld\n", cells->colVal->val->cell.integer);
            break;
        case STRING_TYPE:
            tab();
            printf("val:\"%s\"\n", cells->colVal->val->cell.str);
            break;
        case FLOAT_TYPE:
            tab();
            printf("val:%f\n", cells->colVal->val->cell.dbl);
            break;
        case BOOL_TYPE:
            tab();
            printf("val:%d\n", cells->colVal->val->cell.bl);
            break;
        default:
            break;
        }
        cells = cells->next;
        tabs--;
        tab()
        if (cells)
            printf("},\n");
        else
            printf("}\n");
    } while (cells);
        tabs--;
    tab();
    printf("]\n");
    printCompoundCondition(upd->cond);
}
void printStatement(Statement* stmt){
    switch (stmt->st_type)
    {
    case CREATE_STMT:
        printf("CREATE TABLE\n{\n");
        tabs++;
        printCreate(stmt->stmt.create_tbl);
        break;
    case DROP_STMT:
        printf("DROP TABLE\n{\n");
        tabs++;
        printDrop(stmt->stmt.drop_tbl);
        break;
    case INSERT_STMT:
        printf("INSERT\n{\n");
        tabs++;
        printInsert(stmt->stmt.insert);
        break;
    case DELETE_STMT:
        printf("DELETE\n{\n");
        tabs++;
        printDelete(stmt->stmt.del);
        break;
    case SELECT_STMT:
        printf("SELECT\n{\n");
        tabs++;
        printSelect(stmt->stmt.select);
        break;
    case UPDATE_STMT:
        printf("UPDATE\n{\n");
        tabs++;
        printUpdate(stmt->stmt.update);
        break;
    default:
        printf("\n%d\n", stmt->st_type);
        break;
    }
    printf("}\n");
}