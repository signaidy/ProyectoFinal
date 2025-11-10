#include "db_loader.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <thread>
#include <semaphore>
#include <regex>

using namespace std;
namespace fs = std::filesystem;

vector<string> parse_csv_line(const string& line) {
    vector<string> result;
    regex csv_re(R"((?:^|,)(\"(?:[^\"]|\"\")*\"|[^,]*))");
    auto begin = sregex_iterator(line.begin(), line.end(), csv_re);
    auto end = sregex_iterator();

    for (auto i = begin; i != end; ++i) {
        string match = (*i)[1].str();
        if (!match.empty() && match[0] == '"') {
            // Remove enclosing quotes and unescape double quotes
            match = match.substr(1, match.size() - 2);
            size_t pos = 0;
            while ((pos = match.find("\"\"", pos)) != string::npos) {
                match.replace(pos, 2, "\"");
                ++pos;
            }
        }
        result.push_back(match);
    }
    return result;
}

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

    // Crear placeholders e instrucción SQL
    string placeholders;
    for (size_t i = 0; i < cols.size(); ++i) {
        placeholders += "?";
        if (i + 1 < cols.size()) placeholders += ",";
    }

    string q = "INSERT OR IGNORE INTO " + table + " (";
    for (size_t i = 0; i < cols.size(); ++i) {
        q += cols[i];
        if (i + 1 < cols.size()) q += ",";
    }
    q += ") VALUES (" + placeholders + ");";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, q.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        throw runtime_error(string("Prepare failed: ") + sqlite3_errmsg(db) + "\nSQL: " + q);

    exec_sql(db, "BEGIN TRANSACTION");

    while (getline(in, line)) {
        vector<string> fields = parse_csv_line(line);
        if (fields.size() != cols.size()) {
            fields.resize(cols.size()); // Rellenar si faltan
        }

        sqlite3_reset(stmt);
        for (size_t i = 0; i < cols.size(); ++i) {
            const string& val = fields[i];
            int col_idx = static_cast<int>(i + 1);

            if (val.empty()) {
                sqlite3_bind_null(stmt, col_idx);
            } else {
                // Si parece un entero
                char* end = nullptr;
                long lval = strtol(val.c_str(), &end, 10);
                if (*end == '\0') {
                    sqlite3_bind_int64(stmt, col_idx, lval);
                } else {
                    // Si parece un número real
                    double dval = strtod(val.c_str(), &end);
                    if (*end == '\0') {
                        sqlite3_bind_double(stmt, col_idx, dval);
                    } else {
                        // Si no es número, lo tratamos como texto
                        string cleaned = regex_replace(val, regex(R"([\r\n\t])"), "");
                        cleaned = regex_replace(cleaned, regex(R"(^\s+|\s+$)"), "");
                        sqlite3_bind_text(stmt, col_idx, cleaned.c_str(), static_cast<int>(cleaned.size()), SQLITE_TRANSIENT);
                    }
                }
            }
        }

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cerr << "Fila saltada en " << table << ": " << sqlite3_errmsg(db) << "\n";
        }
    }

    exec_sql(db, "COMMIT");
    sqlite3_finalize(stmt);
}

void DbLoader::load_incidents(){
    sqlite3* dbc = nullptr;
    if (sqlite3_open(cfg_.db_path.c_str(), &dbc) != SQLITE_OK) {
        throw std::runtime_error(std::string("Cannot open DB (incidents): ") + sqlite3_errmsg(dbc));
    }
    sqlite3_busy_timeout(dbc, 30000);
    exec_sql(dbc, "PRAGMA journal_mode=WAL;");
    exec_sql(dbc, "PRAGMA synchronous=NORMAL;");

    bulk_insert_from_csv(
        dbc,
        std::filesystem::path(cfg_.data_dir) / "incidents.csv",
        "incidents",
        {"report_id","category","date"}
    );

    sqlite3_close(dbc);
}

void DbLoader::load_details(){
    sqlite3* dbc = nullptr;
    if (sqlite3_open(cfg_.db_path.c_str(), &dbc) != SQLITE_OK) {
        throw std::runtime_error(std::string("Cannot open DB (details): ") + sqlite3_errmsg(dbc));
    }
    sqlite3_busy_timeout(dbc, 30000);
    exec_sql(dbc, "PRAGMA journal_mode=WAL;");
    exec_sql(dbc, "PRAGMA synchronous=NORMAL;");

    bulk_insert_from_csv(
        dbc,
        std::filesystem::path(cfg_.data_dir) / "details.csv",
        "details",
        {"report_id","subject","transport_mode","detection"}
    );

    sqlite3_close(dbc);
}

void DbLoader::load_outcomes(){
    sqlite3* dbc = nullptr;
    if (sqlite3_open(cfg_.db_path.c_str(), &dbc) != SQLITE_OK) {
        throw std::runtime_error(std::string("Cannot open DB (outcomes): ") + sqlite3_errmsg(dbc));
    }
    sqlite3_busy_timeout(dbc, 30000);
    exec_sql(dbc, "PRAGMA journal_mode=WAL;");
    exec_sql(dbc, "PRAGMA synchronous=NORMAL;");

    bulk_insert_from_csv(
        dbc,
        std::filesystem::path(cfg_.data_dir) / "outcomes.csv",
        "outcomes",
        {"report_id","outcome","num_ppl_fined","fine","num_ppl_arrested","prison_time","prison_time_unit"}
    );

    sqlite3_close(dbc);
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