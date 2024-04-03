// lab1_llp.cpp : Defines the entry point for the application.
//

#include "database.h"
#include "page.h"
#include "table.h"
#include "queries.h"

int main()
{
	database* db = createDatabase("sus101.bin", DEFAULT_PAGE_SIZE);
	/*table* tbl = malloc(sizeof(table));
	table_header* hdr = malloc(sizeof(table_header));*/
	column* columns = malloc(sizeof(column)*3);
	strcpy(columns[0].column_name,"col1");
	columns[0].data_type = INT;
	columns[0].size = 8;
	strcpy(columns[1].column_name, "col2");
	columns[1].data_type = BOOL;
	columns[1].size = 8;
	strcpy(columns[2].column_name, "col3");
	columns[2].data_type = STRING;
	columns[2].size = 64;
	//tbl->columns = columns;
	//strcpy(hdr->table_name,"Table_test");
	//hdr->number_of_columns = 3;
	//hdr->row_size = 80;
	//tbl->hdr = hdr;
	query* qr = malloc(sizeof(query));
	qr->type = CREATE;
	create_query* create_table = calloc(1, sizeof(create_query));
	create_table->columns = columns;
	create_table->column_count = 3;
	create_table->table_name = strdup("Test_query");
	qr->query.create_table = create_table;
	executeQuery(db, qr);
/*	drop_query* drop = malloc(sizeof(drop_query));
	drop->table_name = strdup("Test_query");
	qr->type = DROP;
	qr->query.drop_table = drop;
	executeQuery(db, qr);
	if (findTable(db, "Test_query")) {
		printf("success");
	}*/
	//createTable(db, tbl);
	row* rw = malloc(sizeof(row));
	rw->cells = malloc(sizeof(cell) * 3);
	rw->cells[0].integer = 7777;
	rw->cells[1].bl = true;
	rw->cells[2].str = strdup("test_string");
	//sus = deleteData(db, tbl, readRowPage(db, tbl, tbl->hdr->first_row_page), 0);
	row* rw1 = malloc(sizeof(row));
	rw1->cells = malloc(sizeof(cell) * 3);
	rw1->cells[0].integer = 666;
	rw1->cells[1].bl = false;
	rw1->cells[2].str = strdup("amogus");
	table* tbl = findTable(db, "Test_query");
	for (size_t i = 10; i < 500; i++)
	{		
		sprintf(rw1->cells[2].str, "aa%daa", i);
		bool sus = insertData(db, tbl, rw1);
		sus = insertData(db, tbl, rw);
	}
	select_query* select = malloc(sizeof(select_query));
	select->join_tables = NULL;
	select->join_number = 0;
	select->table_name = strdup("Test_query");
	compound_condition* where_cond = malloc(sizeof(compound_condition));
	where_cond->next_c_cond = NULL;
	where_cond->cond = malloc(sizeof(condition));
	where_cond->cond->column_name = strdup("col1");
	where_cond->cond->comp_type = GREATER;
	where_cond->cond->value.integer = 700;
	select->where_conditions = where_cond;
	qr->type = SELECT;
	qr->query.select = select;
	query_result* qrr = executeQuery(db,qr);
	/*bool sus = insertData(db, tbl, rw1);
	sus = insertData(db, tbl, rw);*/	
	closeDatabase(db);
	/*database* db1 = openDatabase("sus101.bin");
	table* tbl = findTable(db1, "Table_test");
	dropTable(db1, tbl);
	closeDatabase(db1);*/
	/*db = openDatabase("sus101.bin");
	table* test = findTable(db, "Table_test");
	page* pg = readRowPage(db,test, test->hdr->first_row_page);
	for (size_t i = 0; i < pg->hdr->rows_amount; i++)
	{
		for (size_t y = 0; y < test->hdr->number_of_columns; y++){
			switch (test->columns[y].data_type)
			{
			case INT:
				printf("%d\n", pg->rows[i].cells[y].integer);
				break;
			case BOOL:
				printf("%d\n", pg->rows[i].cells[y].bl);
				break;
			case STRING:
				printf("%s\n", pg->rows[i].cells[y].str);
				break;
			default:
				break;
			}
		}
	}
	page *g2 = readRowPage(db, test, pg->hdr->next_page);
	printf("Free space: %d", pg->hdr->free_space);
	dropTable(db, test);
	saveDatabase(db);
	deallocateRow(tbl, rw);
	deallocateRow(tbl, rw1);
	deallocateTable(tbl);
//	deallocateTable(test);
*/
	return 0;
}