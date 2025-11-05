#pragma once
#include <string>
#include <semaphore>
#include <sqlite3.h>


struct DbConfig {
std::string db_path {"/app/outputs/incidents.db"};
std::string data_dir {"/app/data"};
};


class DbLoader {
public:
explicit DbLoader(const DbConfig& cfg);
~DbLoader();


void init_schema();
void load_all_csv_parallel();


private:
DbConfig cfg_;
sqlite3* db_ {nullptr};


void load_incidents();
void load_details();
void load_outcomes();
};