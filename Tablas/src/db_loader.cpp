#include "db_loader.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <thread>
#include <semaphore>

using namespace std;
namespace fs = std::filesystem;


static void exec_sql(sqlite3* db, const string& sql){
    char* err = nullptr;
    if(sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK){
        string msg = err ? err : "unknown";
        sqlite3_free(err);
        throw runtime_error("SQL error: " + msg + "\nQuery: " + sql);
    }
}

DbLoader::DbLoader(const DbConfig& cfg): cfg_(cfg) {
    fs::create_directories(fs::path(cfg_.db_path).parent_path());
    if (sqlite3_open(cfg_.db_path.c_str(), &db_) != SQLITE_OK) {
    throw runtime_error("Cannot open DB: " + string(sqlite3_errmsg(db_)));
    }
    // Mejor rendimiento
    exec_sql(db_, "PRAGMA journal_mode=WAL;");
    exec_sql(db_, "PRAGMA synchronous=NORMAL;");
}

DbLoader::~DbLoader(){ if(db_) sqlite3_close(db_); }


void DbLoader::init_schema(){
    const string incidents = R"SQL(
        CREATE TABLE IF NOT EXISTS incidents (
            report_id TEXT PRIMARY KEY,
            category TEXT,
            date TEXT
        );
        CREATE INDEX IF NOT EXISTS idx_incidents_report_id ON incidents(report_id);
    )SQL";


    const string details = R"SQL(
        CREATE TABLE IF NOT EXISTS details (
            report_id TEXT,
            subject TEXT,
            transport_mode TEXT,
            detection TEXT
        );
        CREATE INDEX IF NOT EXISTS idx_details_report_id ON details(report_id);
    )SQL";


    const string outcomes = R"SQL(
        CREATE TABLE IF NOT EXISTS outcomes (
            report_id TEXT,
            outcome TEXT,
            num_ppl_fined INTEGER,
            fine REAL,
            num_ppl_arrested INTEGER,
            prison_time REAL,
            prison_time_unit TEXT
        );
        CREATE INDEX IF NOT EXISTS idx_outcomes_report_id ON outcomes(report_id);
    )SQL";


    exec_sql(db_, incidents);
    exec_sql(db_, details);
    exec_sql(db_, outcomes);
}

static void bulk_insert_from_csv(sqlite3* db, const fs::path& file, const string& table, const vector<string>& cols){
    ifstream in(file);
    if(!in.is_open()) throw runtime_error("No se pudo abrir: " + file.string());


    string line; // Leer encabezado y descartarlo
    if(!getline(in, line)) return;


    string q = "INSERT INTO "+table+" (";
    for(size_t i=0;i<cols.size();++i){ q += cols[i]; if(i+1<cols.size()) q += ","; }
    q += ") VALUES (" + string(cols.size()-1, '?,') + "?);"; // n placeholders


    sqlite3_stmt* stmt=nullptr;
    if(sqlite3_prepare_v2(db, q.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    throw runtime_error("Prepare failed: "+string(sqlite3_errmsg(db)));


    exec_sql(db, "BEGIN TRANSACTION");


    auto bind_text = [&](int idx, const string& s){
        sqlite3_bind_text(stmt, idx, s.c_str(), (int)s.size(), SQLITE_TRANSIENT);
    };


    while(getline(in, line)){
        vector<string> fields; fields.reserve(cols.size());
        // CSV simple (sin comas escapadas).
        string f; stringstream ss(line);
        while(getline(ss, f, ',')) fields.push_back(f);
        if(fields.size() != cols.size()){
            // intenta reparar campos vacíos al final
            fields.resize(cols.size());
        }
        sqlite3_reset(stmt);
        for(size_t i=0;i<cols.size();++i){
            // Tipado básico: si corresponde a entero/real lo dejamos a SQLite convertir.
            bind_text((int)i+1, fields[i]);
        }
        if(sqlite3_step(stmt) != SQLITE_DONE){
            cerr << "Fila saltada en "<< table <<": "<< sqlite3_errmsg(db) <<"\n";
        }
    }

    exec_sql(db, "COMMIT");
    sqlite3_finalize(stmt);
}


void DbLoader::load_incidents(){
    bulk_insert_from_csv(db_, fs::path(cfg_.data_dir)/"incidents.csv", "incidents",
    {"report_id","category","date"});
}


void DbLoader::load_details(){
    bulk_insert_from_csv(db_, fs::path(cfg_.data_dir)/"details.csv", "details",
    {"report_id","subject","transport_mode","detection"});
}


void DbLoader::load_outcomes(){
    bulk_insert_from_csv(db_, fs::path(cfg_.data_dir)/"outcomes.csv", "outcomes",
    {"report_id","outcome","num_ppl_fined","fine","num_ppl_arrested","prison_time","prison_time_unit"});
}


void DbLoader::load_all_csv_parallel(){
    // Límite de concurrencia (ej. 2 en paralelo)
    static counting_semaphore<3> gate(2);


    auto worker = [&](auto fn){ gate.acquire(); try { fn(); } catch(const exception& e){ cerr << e.what() << "\n"; } gate.release(); };


    thread t1(worker, [&]{ load_incidents(); });
    thread t2(worker, [&]{ load_details(); });
    thread t3(worker, [&]{ load_outcomes(); });


    t1.join(); t2.join(); t3.join();
}