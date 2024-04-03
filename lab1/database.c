#include "database.h"
#include <string.h>



database *openDatabase(const char* file_name)
{
	FILE* file = fopen(file_name, "r+b");
	if (!file) {
		perror("Error opening db file");
		return 0;
	}
	database* db = malloc(sizeof(database));
	database_header* hdr = malloc(sizeof(database_header));
	if (!db || !hdr) {
		perror("Error allocating memory for database");
		return 0;
	}
	db->hdr = hdr;
	db->file = file;
	db->file_name = strdup(file_name);
	fread(db->hdr,sizeof(database_header),1,db->file);
	return db;
}

database *createDatabase(const char* file_name, uint16_t page_size)
{
	database_header* hdr = calloc(1,sizeof(database_header));
	if (!hdr) {
		perror("Error allocationg memory");
		return 0;
	}
	hdr->page_size = page_size;
	hdr->page_type = DATABASE_MAIN;
	FILE* file = fopen(file_name, "wb");
	if (!file) {
		perror("Error opening file");
		return 0;
	}
	uint8_t* bin_pg = calloc(hdr->page_size, sizeof(uint8_t));
	memcpy(bin_pg, hdr, sizeof(database_header));
	fwrite(bin_pg, hdr->page_size, 1, file);
	fclose(file);
	free(bin_pg);
	free(hdr);
	return openDatabase(file_name);
}
bool saveDatabase(database* db)
{
	uint8_t* bin_page = calloc(1, db->hdr->page_size);
	memcpy(bin_page, db->hdr, sizeof(database_header));
	fseek(db->file, 0, SEEK_SET);
	fwrite(bin_page, db->hdr->page_size, 1, db->file);
	free(bin_page);
	return true;
}
bool closeDatabase(database* db)
{
	FILE* new_file = fopen("new_file.bin", "w+b");
	PAGE_TYPE type;
	uint32_t page_num_old = 0, page_num_new = 0, pages_offset, tmp = 0;
	uint8_t* bin_pg = calloc(1, db->hdr->page_size); 
	fseek(db->file, 0, SEEK_SET);
	while (page_num_old <= db->hdr->number_of_pages) {
		fseek(db->file, db->hdr->page_size*page_num_old, SEEK_SET);
		fread(bin_pg, db->hdr->page_size, 1, db->file);
		memcpy(&type, bin_pg, sizeof(PAGE_TYPE));
		if (type != FREE_PAGE) {
			memcpy(&tmp, bin_pg + sizeof(PAGE_TYPE), sizeof(uint32_t));
			tmp -= pages_offset;
			memcpy(bin_pg + sizeof(PAGE_TYPE), &tmp, sizeof(uint32_t));
			fseek(new_file, db->hdr->page_size * page_num_new, SEEK_SET);
			fwrite(bin_pg, db->hdr->page_size, 1, new_file);
			page_num_new++;
		}
		else{
			db->hdr->number_of_pages--;
			pages_offset++;
		}
		page_num_old++;
	}	
	fclose(db->file);
	db->file = new_file;
	saveDatabase(db);
	fclose(db->file);
	remove(db->file_name);
	rename("new_file.bin", db->file_name);
	free(db->file_name);
	free(db->hdr);
	free(db);
	free(bin_pg);
	return true;
}



uint32_t getFreePage(database* db)
{
	if (db->hdr->first_free_page) {
		return db->hdr->first_free_page;
	}
	return createPage(db);
}

uint32_t createPage(database* db)
{
	uint8_t* bin_pg = calloc(1, db->hdr->page_size);
	if (!bin_pg) {
		perror("Error allocationg memory");
		return 0;
	}
	db->hdr->number_of_pages++;
	fseek(db->file, db->hdr->number_of_pages * db->hdr->page_size, SEEK_SET);
	fwrite(bin_pg, db->hdr->page_size, 1, db->file);
	free(bin_pg);
	return db->hdr->number_of_pages;
}
