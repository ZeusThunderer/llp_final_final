#ifndef PROCESSOR_H
#define PROCESSOR_H
#include "gen-c_glib/nt_structs_types.h"
#include "lab1/queries.h"
#include "lab1/database.h"

ServerResponse* process_client_request(const Statement_* stmt, database* db);

#endif