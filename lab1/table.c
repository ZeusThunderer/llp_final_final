#include "table.h"


void deallocateRow(table* tbl,row* rw){
		if (rw->cells) {
			for (size_t y = 0; y < tbl->hdr->number_of_columns; y++) {
				if (tbl->columns[y].data_type == STRING) {
					free(rw->cells[y].str);
					rw->cells[y].str = NULL;
				}
			}
			free(rw->cells);
			rw->cells = NULL;
		}
}
void deallocateRowPage(table* tbl,page* pg) {
	if (pg) {
		if (pg->rows) {
			for (size_t i = 0; i < pg->hdr->rows_amount; i++) {
				deallocateRow(tbl, &pg->rows[i]);
			}
			free(pg->rows);
			pg->rows = NULL;
			free(pg->row_lens);
			pg->row_lens = NULL;
		}
		if (pg->hdr) {
			free(pg->hdr);
			pg->hdr = NULL;
		}
		free(pg);
		pg = NULL;
	}
}

page* createRowPage(database* db, table* tbl)
{
	page_header* hdr = calloc(1, sizeof(page_header));
	page* pg = malloc(sizeof(page));
	if (!pg || !hdr) {
		return 0;
	}
	pg->hdr = hdr;
	pg->hdr->page_number = getFreePage(db);
	pg->hdr->page_type = ROW_PAGE;
	pg->hdr->free_space = db->hdr->page_size - sizeof(page_header);
	pg->rows = malloc(0);
	if (writeRowPage(db, tbl, pg)) {
		return pg;
	}
	else
		return 0;
}

void freeRowPage(database* db, table* tbl, page* pg) {
	pg->hdr->page_type = FREE_PAGE;
	pg->hdr->next_page = db->hdr->first_free_page;
	pg->hdr->rows_amount = 0;
	db->hdr->first_free_page = pg->hdr->page_number;
	writeRowPage(db, tbl, pg);
	deallocateRowPage(tbl,pg);
}



page* readRowPage(database* db, table* tbl, uint32_t page_number)
{
	page_header* hdr = malloc(sizeof(page_header));
	page* pg = malloc(sizeof(page));
	uint8_t* bin_pg = malloc(db->hdr->page_size);
	if (!pg || !hdr || !bin_pg) {
		perror("Error alocating memory");
		return 0;
	}
	pg->hdr = hdr;
	fseek(db->file, db->hdr->page_size * page_number, SEEK_SET);
	if (fread(bin_pg, db->hdr->page_size, 1, db->file) != 1) {
		free(bin_pg);
		free(hdr);
		free(pg);
		perror("Error reading file");
		return 0;
	}
	memcpy(pg->hdr, bin_pg, sizeof(page_header));
	pg->row_iterator = 0;
	uint16_t* row_lens = malloc(sizeof(uint16_t) * pg->hdr->rows_amount);
	row* rows = malloc(pg->hdr->rows_amount * sizeof(row));
	uint8_t* bin_row = calloc(1,tbl->hdr->row_size);
	if (!rows || !bin_row || !row_lens) {
		free(bin_pg);
		free(bin_row);
		deallocateRowPage(tbl,pg);
		perror("Error alocating memory");
		return 0;
	}
	for (size_t i = 0, poff = sizeof(uint16_t)*pg->hdr->rows_amount; i < pg->hdr->rows_amount; i++) {
		rows[i].cells = malloc(sizeof(cell)*tbl->hdr->number_of_columns);
		if (!rows[i].cells) {
			free(bin_pg);
			free(bin_row);
			free(row_lens);
			deallocateRowPage(tbl,pg);
			perror("Error alocating memory");
			return 0;
		}
		memcpy(&row_lens[i], bin_pg + sizeof(page_header) + i * sizeof(uint16_t), sizeof(uint16_t));
		memcpy(bin_row, bin_pg + sizeof(page_header) + poff, row_lens[i]);
		for (size_t y = 0, roff = 0; y < tbl->hdr->number_of_columns; y++) {
			column clm = tbl->columns[y];
			cell cll = rows[i].cells[y];
			switch (tbl->columns[y].data_type) {
			case INT:
				memcpy(&rows[i].cells[y].integer, bin_row + roff, tbl->columns[y].size);
				roff += tbl->columns[y].size;
				break;
			case FLOAT:
				memcpy(&rows[i].cells[y].dbl, bin_row + roff, tbl->columns[y].size);
				roff += tbl->columns[y].size;
				break;
			case BOOL:
				memcpy(&rows[i].cells[y].bl, bin_row + roff, tbl->columns[y].size);
				roff += tbl->columns[y].size;
				break;
			case STRING:
				rows[i].cells[y].str = strdup(bin_row+roff);
				roff += strlen(rows[i].cells[y].str)+1;
				break;
			default:
				break;
			}
			if (roff > row_lens[i] || roff > tbl->hdr->row_size) {
				free(bin_pg);
				free(bin_row);
				free(row_lens);
				deallocateRowPage(tbl,pg);
				return 0;
			}
		}
		poff += row_lens[i];
	}
	pg->row_lens = row_lens;
	pg->rows = rows;
	free(bin_row);
	free(bin_pg);
	return pg;
}

bool writeRowPage(database* db, table* tbl, page* pg)
{
	uint8_t* bin_pg = calloc(1,db->hdr->page_size);
	if (!bin_pg) {
		perror("Error alocating memory");
		return false;
	}
	memcpy(bin_pg, pg->hdr, sizeof(page_header));
	if (pg->rows) {
		for (size_t i = 0; i < pg->hdr->rows_amount; i++){
			memcpy(bin_pg + sizeof(page_header) + i * sizeof(uint16_t), pg->row_lens + i, sizeof(uint16_t));
		}
		for (size_t i = 0, poff = sizeof(uint16_t)* pg->hdr->rows_amount; i < pg->hdr->rows_amount; i++) {
			for (size_t y = 0, roff = 0; y < tbl->hdr->number_of_columns; y++) {
				switch (tbl->columns[y].data_type) {
				case INT:
					memcpy(bin_pg + sizeof(page_header) + poff + roff, &pg->rows[i].cells[y].integer, tbl->columns[y].size);
					roff += tbl->columns[y].size;
					break;
				case FLOAT:
					memcpy(bin_pg + sizeof(page_header) + poff + roff, &pg->rows[i].cells[y].dbl, tbl->columns[y].size);
					roff += tbl->columns[y].size;
					break;
				case BOOL:
					memcpy(bin_pg + sizeof(page_header) + poff + roff, &pg->rows[i].cells[y].bl, tbl->columns[y].size);
					roff += tbl->columns[y].size;
					break;
				case STRING:
					memcpy(bin_pg + sizeof(page_header) + poff + roff, pg->rows[i].cells[y].str, strlen(pg->rows[i].cells[y].str) + 1);
					roff += strlen(pg->rows[i].cells[y].str) + 1;
					break;
				default:
					break;
				}
				
			}
			poff += pg->row_lens[i];
		}
	}
	fseek(db->file, db->hdr->page_size * pg->hdr->page_number, SEEK_SET);
	if (fwrite(bin_pg, db->hdr->page_size, 1, db->file) != 1) {
		free(bin_pg);
		perror("Error writing file");
		return false;
	}
	else {
		free(bin_pg);
		return true;
	}
}
void deallocateTable(table* tbl){
	if (tbl) {
		if (tbl->columns) {
			free(tbl->columns);
			tbl->columns = NULL;
		}
		if (tbl->hdr) {
			free(tbl->hdr);
			tbl->hdr = NULL;
		}
		free(tbl);
		tbl = NULL;
	}
}
static bool saveTable(database* db, table* tbl)
{
	uint8_t* bin_pg = calloc(db->hdr->page_size,sizeof(uint8_t));
	if (!bin_pg) {
		perror("Error allocating memory");
		return 0;
	}
	memcpy(bin_pg, tbl->hdr, sizeof(table_header));
	for (size_t i = 0; i < tbl->hdr->number_of_columns; i++){
		memcpy(bin_pg + sizeof(table_header) + i * sizeof(column), &tbl->columns[i], sizeof(column));
	}
	fseek(db->file, db->hdr->page_size * tbl->hdr->page_number, SEEK_SET);
	if (fwrite(bin_pg, db->hdr->page_size,1, db->file) == 1) {
		free(bin_pg);
		return true;
	}
	else {
		free(bin_pg);
		return false;
	}
}


table* readTable(database* db, uint32_t page_number)
{
	table* tbl = malloc(sizeof(table));
	table_header* hdr = malloc(sizeof(table_header));
	uint8_t* bin_table = malloc(db->hdr->page_size);
	if (!tbl || !hdr || !bin_table) {
		perror("Error allocating memory");
		return 0;
	}

	tbl->hdr = hdr;
	fseek(db->file, page_number * db->hdr->page_size, SEEK_SET);
	if (fread(bin_table, db->hdr->page_size, 1, db->file) != 1) {
		perror("Error reading a file");
		return 0;
	}
	memcpy(tbl->hdr, bin_table, sizeof(table_header));
	tbl->iterator = tbl->hdr->first_row_page;
	column* columns = malloc(tbl->hdr->number_of_columns * sizeof(column));
	for (size_t i = 0; i < tbl->hdr->number_of_columns; i++){
		memcpy(columns+i, bin_table + sizeof(table_header) + i * sizeof(column), sizeof(column));
	}
	tbl->columns = columns;
	tbl->iterator = tbl->hdr->first_row_page;
	free(bin_table);
	return tbl;
}

table* findTable(database* db, const char* table_name)
{
	table* tbl = NULL;
	uint32_t pg = db->hdr->first_table_page;
	while (pg) {
		tbl = readTable(db, pg);
		if (tbl) {
			if (!strcmp(tbl->hdr->table_name, table_name)) {
				return tbl;
			}
		}
		pg = tbl->hdr->next_table;
		deallocateTable(tbl);
	}
	return tbl;
}

page* getTableFreePage(database* db, table* tbl) {
	page* pg;
	if (!tbl->hdr->free_place) {
		pg = createRowPage(db, tbl);
		if (!pg) {
			deallocateRowPage(tbl, pg);
			return NULL;
		}
		pg->row_lens = malloc(sizeof(uint16_t));
		tbl->hdr->row_size = tbl->hdr->row_size;
		pg->hdr->next_page = tbl->hdr->first_row_page;
		tbl->hdr->first_row_page = pg->hdr->page_number;
		pg->hdr->next_free = tbl->hdr->free_place;
		tbl->hdr->free_place = pg->hdr->page_number;
		tbl->iterator = tbl->hdr->first_row_page;
	}
	else {
		pg = readRowPage(db, tbl, tbl->hdr->free_place);
		if (!pg) {
			deallocateRowPage(tbl, pg);
			return NULL;
		}
	}
	return pg;
}

page* iterateTable(database* db, table* tbl)
{
	if (!tbl->iterator) {
		return NULL;
	}
	page* pg = readRowPage(db,tbl, tbl->iterator);
	if (pg) {
		tbl->iterator = pg->hdr->next_page;
		return pg;
	}
	else{
		return NULL;
	}
}
row* iterateRowPage(page* pg) {
	if (pg->row_iterator < pg->hdr->rows_amount) {
		return &pg->rows[pg->row_iterator];
		pg->row_iterator++;
	}
	else
		return NULL;
}

char* getDataTypeInStr(DATA_TYPE type) {
	switch (type)
	{
	case INT:
		return "INT";
		break;
	case FLOAT:
		return "DOUBLE";
		break;
	case BOOL:
		return "BOOL";
		break;
	case STRING:
		return "VARCHAR";
		break;
	default:
		return "";
		break;
	}
}

char* getTableScheme(table* tbl) {
	char* scheme = "Table scheme: ";
	uint32_t len = strlen(scheme);
	for (size_t i = 0; i < tbl->hdr->number_of_columns; i++){

	}
	return scheme;
}
int getColumnNumber(table* tbl,char* column_name) {
	for (size_t i = 0; i < tbl->hdr->number_of_columns; i++){
		if (strcmp(tbl->columns[i].column_name, column_name) == 0) {
			return i;
		}
	}
	return -1;
}
bool createTable(database* db, table* new_tbl)
{
	new_tbl->hdr->number_of_pages = 0;
	new_tbl->hdr->first_row_page = 0;
	new_tbl->hdr->free_place = 0;
	new_tbl->hdr->page_type = TABLE_PAGE;
	new_tbl->hdr->next_table = db->hdr->first_table_page;
	new_tbl->hdr->page_number = getFreePage(db);
	new_tbl->iterator = new_tbl->hdr->first_row_page;
	new_tbl->hdr->prev_table = db->hdr->first_table_page;
	db->hdr->first_table_page = new_tbl->hdr->page_number;
	return saveTable(db, new_tbl);
}

bool dropTable(database* db, table* tbl)
{
	page* pg = NULL;
	while (tbl->iterator){
		pg = iterateTable(db, tbl);
		if (pg) {
			freeRowPage(db,tbl,pg);
			tbl->hdr->number_of_pages--;
			pg = NULL;
		}
	}
	pg = malloc(sizeof(page));
	page_header* hdr = calloc(1, sizeof(page_header));
	pg->hdr = hdr;
	pg->rows = NULL;
	pg->row_lens = NULL;
	pg->hdr->page_number = tbl->hdr->page_number;
	db->hdr->first_table_page = tbl->hdr->prev_table;
	freeRowPage(db, tbl, pg);
	deallocateTable(tbl);
	return true;
}

uint16_t rowLength(table* tbl, row* rw) {
	uint16_t row_len = 0;
	for (size_t i = 0; i < tbl->hdr->number_of_columns; i++) {
		if (tbl->columns[i].data_type == STRING) {
			row_len += strlen(rw->cells[i].str) + 1;
		}
		else {
			row_len += tbl->columns[i].size;
		}
	}
	return row_len;
}

bool updateData(database* db, table* tbl, page* pg, uint16_t row_number, row* new_row)
{
	uint16_t row_len_old = rowLength(tbl, &pg->rows[row_number]);
	uint16_t row_len_new = rowLength(tbl, new_row);
	if (pg->hdr->free_space < (row_len_new - row_len_old)) {   
		deleteData(db, tbl, pg, row_number);
		insertData(db, tbl, new_row);
	}
	else {
		for (size_t i = 0; i < tbl->hdr->number_of_columns; i++){
			if (tbl->columns[i].data_type == STRING) {
				pg->rows[row_number].cells[i].str = strdup(new_row->cells[i].str);
			}
			else {
				pg->rows[row_number].cells[i] = new_row->cells[i];
			}
		}
		pg->row_lens[row_number] = row_len_new;
		pg->hdr->free_space += (row_len_new - row_len_old);
		if (pg->hdr->free_space < tbl->hdr->row_size){
			tbl->hdr->free_place = pg->hdr->next_free;
		}
	}
}

bool deleteData(database* db, table* tbl, page* pg, uint16_t row_number)
{
	if (row_number > pg->hdr->rows_amount) {
		return false;
	}
	deallocateRow(tbl,&pg->rows[row_number]);
	if (pg->hdr->free_space < tbl->hdr->row_size) {
		pg->hdr->free_space += pg->row_lens[row_number];
		if (pg->hdr->free_space > tbl->hdr->row_size) {
			pg->hdr->next_free = tbl->hdr->free_place;
			tbl->hdr->free_place = pg->hdr->page_number;
		}
	}
	else{
		pg->hdr->free_space += pg->row_lens[row_number];
	}
	if (pg->hdr->free_space < tbl->hdr->row_size) {
		tbl->hdr->free_place = pg->hdr->next_free;
		pg->hdr->next_free = 0;
	}
	for (size_t i = row_number; i < pg->hdr->rows_amount - 1; i++){
		pg->row_lens[i] = pg->row_lens[i + 1];
		pg->rows[i] = pg->rows[i + 1];
	}
	pg->hdr->rows_amount--;
	pg->row_iterator--;
	if (!pg->hdr->rows_amount) {
		page* pg_prev, *pg_next;
		if (pg->hdr->prev_page) {
			pg_prev = readRowPage(db, tbl, pg->hdr->prev_page);
			pg_prev->hdr->next_free = pg->hdr->next_page;
			writeRowPage(db, tbl, pg_prev);
			deallocateRowPage(tbl, pg_prev);
		}
		if (pg->hdr->next_page) {
			pg_next = readRowPage(db, tbl, pg->hdr->next_page);
			pg_next->hdr->prev_page = pg->hdr->prev_page;
			writeRowPage(db, tbl, pg_next);
			deallocateRowPage(tbl,pg_next);
		}
		freeRowPage(db, tbl, pg);
		return true;
	}
	pg->rows = realloc(pg->rows, pg->hdr->rows_amount * sizeof(row));
	if (!writeRowPage(db, tbl, pg)) {
		return false;
	}
	return saveTable(db, tbl);
}



bool insertData(database* db, table* tbl, row* rw)
{
	uint16_t row_len = rowLength(tbl,rw);
	page* pg = getTableFreePage(db,tbl);
	pg->row_lens = realloc(pg->row_lens,(pg->hdr->rows_amount + 1)* sizeof(uint16_t));
	pg->rows = realloc(pg->rows, (pg->hdr->rows_amount + 1)* sizeof(row));
	if (!pg->rows) {
		return false;
	}
	pg->rows[pg->hdr->rows_amount].cells = malloc(tbl->hdr->number_of_columns*sizeof(cell));
	if (!pg->rows) {
		perror("Error allocating memory");
		return false;
	
	}
	pg->row_lens[pg->hdr->rows_amount] = row_len;
	for (size_t i = 0; i < tbl->hdr->number_of_columns; i++){
		if (tbl->columns[i].data_type == STRING) {
			pg->rows[pg->hdr->rows_amount].cells[i].str = strdup(rw->cells[i].str);
		}
		else {
			pg->rows[pg->hdr->rows_amount].cells[i] = rw->cells[i];
		}
	}
	pg->hdr->free_space -= (pg->row_lens[pg->hdr->rows_amount] + sizeof(uint16_t));
	pg->hdr->rows_amount++;
	if (pg->hdr->free_space < tbl->hdr->row_size) {
		tbl->hdr->free_place = pg->hdr->next_free;
		pg->hdr->next_free = 0;
	}
	if (!writeRowPage(db,tbl, pg)) {
		return false;
	}
	return saveTable(db,tbl);
}



