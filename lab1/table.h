#ifndef TABLE_H
#define TABLE_H
#include "database.h"
#include "page.h"
#define NAME_SIZE 64

typedef struct table_header table_header;
typedef struct table table;
typedef struct column column;
typedef enum DATA_TYPE DATA_TYPE;
typedef struct page page;
typedef struct page_header page_header;
typedef struct row row;
typedef union cell cell;

enum DATA_TYPE
{
	INT,
	FLOAT,
	STRING,
	BOOL
};
struct column
{
	char column_name[NAME_SIZE];
	DATA_TYPE data_type;
	uint16_t size;
};

struct table_header {
	PAGE_TYPE page_type; //here
	uint32_t page_number; //here
	uint32_t next_table; //here
	uint32_t prev_table; //here
	uint32_t first_row_page; //here
	uint32_t number_of_pages; //here
	uint32_t number_of_columns; //outer
	uint32_t row_size; //outer
	uint32_t free_place; //here
	char table_name[NAME_SIZE]; //outer
};


struct table {
	table_header *hdr;
	column* columns;
	uint32_t iterator;
};
union cell {
	int64_t integer;
	double dbl;
	bool bl;
	char* str;
};
struct row
{
	cell* cells;
};

struct page_header {
	PAGE_TYPE page_type; // here
	uint32_t page_number; // here
	uint32_t next_page; //for linked list //outer
	uint32_t prev_page; //for linked list //outer
	uint32_t next_free; //next page with free space //outer
	uint16_t rows_amount; // here
	uint16_t free_space; // here
};

struct page
{
	page_header* hdr;
	uint16_t* row_lens;
	row* rows;
	uint32_t row_iterator;
};

row* allocateRow(table *tbl);
void deallocateRow(table* tbl, row* rw);
page* createRowPage(database* db, table* tbl);
void freeRowPage(database* db, table* tbl, page* pg);
page* readRowPage(database* db, table* tbl, uint32_t page_number);
void deallocateRowPage(table* tbl, page* pg);
bool writeRowPage(database* db, table* tbl, page* pg);

void deallocateTable(table* tbl);
table* readTable(database *db, uint32_t page_number);
table* findTable(database* db, const char* table_name);
char* getTableScheme(table* tbl);
int getColumnNumber(table* tbl, char* column_name);
page* iterateTable(database* db, table* tbl);
row* iterateRowPage(page* pg);

bool createTable(database* db, table* new_tbl);
bool dropTable(database* db, table* tbl);

row* selectSingleRow(database* db, table* tbl, page* pg, uint16_t row_number);
bool insertData(database* db, table* tbl, row* rw);
bool updateData(database* db, table* tbl, page* pg, uint16_t row_number, row* new_row);
bool deleteData(database* db, table* tbl, page* pg, uint16_t row_number);

#endif // !TABLE_H