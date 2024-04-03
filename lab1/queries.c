#include "queries.h"


static query_result* set_result(STATUS status, const char* info, row* rows, uint32_t rows_number) {
	query_result* res = calloc(1, sizeof(query_result));
	res->status = status;
	res->info = strdup(info);
	res->rows_number = rows_number;
	res->rows = rows;
}

static query_result* executeCreate(database* db, create_query* create) {
	table* tmp = NULL;
	tmp = findTable(db, create->table_name);
	if (tmp){
		deallocateTable(tmp);
		return set_result(BAD_REQUEST,"Table already exists", NULL,0);
	}
	table* tbl = malloc(sizeof(table));
	table_header* hdr = calloc(1,sizeof(table_header));
	column* cols = calloc(create->column_count, sizeof(column));
	for (size_t i = 0; i < create->column_count; i++){
		cols[i].data_type = create->columns[i].data_type;
		cols[i].size = create->columns[i].size;
		strcpy(cols[i].column_name,create->columns[i].column_name);
		if (cols[i].data_type == STRING)
			hdr->row_size++;
		hdr->row_size += create->columns[i].size;
	}
	tbl->hdr = hdr;
	tbl->hdr->number_of_columns = create->column_count;
	strcpy(tbl->hdr->table_name, create->table_name);
	tbl->columns = cols;
	if (createTable(db, tbl)) {
		deallocateTable(tbl);
		return set_result(OK, "Table created", NULL,0);
	}
	else{
		deallocateTable(tbl);
		return set_result(INTERNAL_ERROR, "Error occured creating table", NULL,0);
	}
}

static query_result* executeDrop(database* db, drop_query* drop) {
	table* tbl = NULL;
	tbl = findTable(db, drop->table_name);
	if (!tbl) {
		deallocateTable(tbl);
		return set_result(BAD_REQUEST, "Table not found", NULL,0);
	}
	if (dropTable(db, tbl)) {
		return set_result(OK, "Table dropped", NULL,0);
	}
	else {
		return set_result(INTERNAL_ERROR, "Error occured dropping table", NULL,0);
	}
}

static query_result* executeInsert(database* db, insert_query* insert) {
	table* tbl = NULL;
	tbl = findTable(db, insert->table_name);
	if (!tbl) {
		deallocateTable(tbl);
		return set_result(BAD_REQUEST, "Table not found", NULL,0);
	}
	for (size_t i = 0; i < insert->rows_count; i++){
		if (!insertData(db, tbl, &insert->new_rows[i])) {
			deallocateTable(tbl);
			return set_result(INTERNAL_ERROR, "Error occured inserting table", NULL,0);
		}
	}
	deallocateTable(tbl);
	return set_result(OK, "Data inserted", NULL,0);
}

static bool compareInt(COMP_TYPE c_type, int64_t val1, int64_t val2) {
	switch (c_type)
	{
	case EQUALS: return val1 == val2;
	case NOT_EQUALS: return val1 != val2;
	case GREATER: return val1 > val2;
	case GREATE_EQUALS: return val1 >= val2;
	case LESS: return val1 < val2;
	case LESS_EQUALS:return val1 <= val2;
	default:return false;
	}
}
static bool compareDouble(COMP_TYPE c_type, double val1, double val2) {
	switch (c_type)
	{
	case EQUALS: return val1 == val2;
	case NOT_EQUALS: return val1 != val2;
	case GREATER: return val1 > val2;
	case GREATE_EQUALS: return val1 >= val2;
	case LESS: return val1 < val2;
	case LESS_EQUALS:return val1 <= val2;
	default: return false;
	}
}
static bool compareBool(COMP_TYPE c_type, bool val1, bool val2) {
	switch (c_type)
	{
	case EQUALS: return val1 == val2;
	case NOT_EQUALS: return val1 != val2;
	default:return false;
	}
}
static bool compareStr(COMP_TYPE c_type,const char* str1,const char* str2) {
	int tmp = strcmp(str1, str2);
	switch (c_type)
	{
	case EQUALS: return tmp == 0;
	case NOT_EQUALS: return tmp != 0;
	case GREATER: return tmp > 0;
	case GREATE_EQUALS: return tmp >= 0;
	case LESS: return tmp < 0;
	case LESS_EQUALS:return tmp <= 0;
	case LIKE: return strstr(str1, str2);
	default: return false;
	}
}

static bool checkCondition(table* tbl, condition* cond, row* rw, bool* err) {
	int col_num = getColumnNumber(tbl, cond->column_name);
	if (col_num == -1) {
		*err = true;
		return false;
	}
	switch (tbl->columns[col_num].data_type)
	{
	case INT:
		return compareInt(cond->comp_type, rw->cells[col_num].integer, cond->value.integer);
	case FLOAT:
		return compareDouble(cond->comp_type, rw->cells[col_num].dbl, cond->value.dbl);
	case BOOL:
		return compareBool(cond->comp_type, rw->cells[col_num].bl, cond->value.bl);
	case STRING:
		return compareStr(cond->comp_type, rw->cells[col_num].str, cond->value.str);
	}
	return false;
}
static bool checkCompoundCondition(table* tbl, compound_condition* c_cond, row* rw, bool* err) {
	if (c_cond->next_c_cond) {
		if (c_cond->op == OR) {
			return (checkCondition(tbl, c_cond->cond, rw,err) || checkCompoundCondition(tbl, c_cond->next_c_cond, rw,err));
		}
		else if(c_cond->op == AND) {
			return (checkCondition(tbl, c_cond->cond, rw,err) && checkCompoundCondition(tbl, c_cond->next_c_cond, rw,err));
		}
		else {
			return false;
		}
	}
	else {
		return checkCondition(tbl, c_cond->cond, rw,err);
	}
}


static row* getRowUpdate(table *tbl, update_query* update) {
	query_result* res = calloc(1, sizeof(query_result));
	return res;
}


static query_result* joinSelect(database* db, select_query* select, table* original_table) {
	table* join_tbl = NULL;
	join_tbl = findTable(db, select->join_tables->join_table_name);
	if (!join_tbl) {
		deallocateTable(join_tbl);
		deallocateTable(original_table);
		return set_result(BAD_REQUEST, "Join table does not exists", NULL, 0);
	}
}


static query_result* executeSelect(database* db, select_query* select) {
	table* tbl = NULL;
	tbl = findTable(db, select->table_name);
	if (!tbl) {
		deallocateTable(tbl);
		return set_result(BAD_REQUEST, "Table does not exists", NULL,0);
	}
	if (select->join_tables) {
		return joinSelect(db, select, tbl);
	}
	row* result = NULL;
	uint32_t rows_number = 0;
	page* pg;
	bool* err = malloc(sizeof(bool));
	*err = false;
	while (pg = iterateTable(db, tbl)) {
		for (size_t i = 0; i < pg->hdr->rows_amount; i++){
			if (!select->where_conditions || checkCompoundCondition(tbl, select->where_conditions, &pg->rows[i], err)) {
				rows_number++;
				result = realloc(result, rows_number* sizeof(row));
				result[rows_number - 1].cells = calloc(tbl->hdr->number_of_columns, sizeof(cell));
				for (size_t y = 0; y < tbl->hdr->number_of_columns; y++){
					if (tbl->columns[y].data_type == STRING)
						result[rows_number - 1].cells[y].str = strdup(pg->rows[i].cells[y].str);
					else
						result[rows_number - 1].cells[y] = pg->rows[i].cells[y];
				}
			}
		}
	}
	return set_result(OK, "Data selected", result, rows_number);
}

static query_result* executeUpdate(database* db, update_query* update) {
	table* tbl = NULL;
	tbl = findTable(db, update->table_name);
	if (tbl) {
		deallocateTable(tbl);
		return set_result(BAD_REQUEST, "Table already exists", NULL,0);
	}
	page* pg;
	bool* err = malloc(sizeof(bool));
	row* rw;
	while (pg = iterateTable(db, tbl)) {
		while (rw = iterateRowPage(pg)) {
			if (checkCompoundCondition(tbl, update->where_conditions, rw, err)) {
				if (*err) {
					deallocateRowPage(tbl, pg);
					deallocateTable(tbl);
					return set_result(BAD_REQUEST, "Column not found", NULL,0);
				}
				else{
					updateData(db, tbl, pg, pg->row_iterator - 1, update->new_row);
				}
			}
		}
	}
	set_result(OK, "Data updated", NULL,0);
}
static query_result* executeDelete(database* db, delete_query* del) {
	table* tbl = NULL;
	tbl = findTable(db, del->table_name);
	if (!tbl) {
		deallocateTable(tbl);
		return set_result(BAD_REQUEST, "Table not found", NULL,0);
	}
	page* pg;
	bool* err = malloc(sizeof(bool));
	row* rw;
	while (pg = iterateTable(db, tbl)) {
		while(rw = iterateRowPage(pg)){
			if (!del->where_conditions) {
				if (!deleteData(db, tbl, pg, pg->row_iterator-1)) {
					deallocateRowPage(tbl, pg);
					deallocateTable(tbl);
					return set_result(INTERNAL_ERROR, "Error occured inserting table", NULL,0);
				}
				continue;
			}
			if (checkCompoundCondition(tbl, del->where_conditions,rw,err)) {
				if (*err) {
					deallocateRowPage(tbl, pg);
					deallocateTable(tbl);
					return set_result(BAD_REQUEST, "Column not found", NULL,0);
				}
				if (!deleteData(db, tbl, pg, pg->row_iterator - 1)) {
					deallocateRowPage(tbl, pg);
					deallocateTable(tbl);
					return set_result(INTERNAL_ERROR, "Error occured inserting table", NULL,0);
				}
			}
		}
	}
	return set_result(OK, "Data deleted", NULL,0);
}



query_result* executeQuery(database* db, query* qr)
{
	switch (qr->type)
	{
	case CREATE:
		return executeCreate(db,qr->query.create_table);
	case DROP:
		return executeDrop(db,qr->query.drop_table);
	case SELECT:
		return executeSelect(db, qr->query.select);
	case UPDATE:
		return executeUpdate(db,qr->query.update);
	case DELETE:
		return executeDelete(db,qr->query.del);
	case INSERT:
		return executeInsert(db,qr->query.insert);
	default:
		return NULL;
	}
}
