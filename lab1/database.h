#ifndef DATABASE_H
#define DATABASE_H
#define DEFAULT_PAGE_SIZE 4096
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct database_header database_header;
typedef struct database database;

typedef enum PAGE_TYPE
{
	DATABASE_MAIN,
	TABLE_PAGE,
	ROW_PAGE,
	FREE_PAGE
} PAGE_TYPE;

struct database_header
{
	PAGE_TYPE page_type;
	uint32_t number_of_pages;
	uint32_t number_of_tables;
	uint32_t first_free_page; //stack of free pages
	uint32_t first_table_page;
	uint16_t page_size;
};

struct database
{
	FILE* file;
	database_header *hdr;
	char* file_name;
};


database *openDatabase(const char* file_name);
database *createDatabase(const char* file_name, uint16_t page_size);
bool closeDatabase(database* database);
bool saveDatabase(database* database);
uint32_t getFreePage(database* db);
uint32_t createPage(database* db);

#endif // !DATABASE_H

