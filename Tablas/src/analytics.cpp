#include "analytics.h"
#include <stdexcept>
#include <iostream>

// Ejecuta una consulta SQL que devuelve un único valor double
static void exec_scalar_double(sqlite3* db, const std::string& sql, double& out){
    sqlite3_stmt* st=nullptr;
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &st, nullptr)!=SQLITE_OK)
        throw std::runtime_error(sqlite3_errmsg(db));
    int rc = sqlite3_step(st);
    if(rc==SQLITE_ROW){ out = sqlite3_column_double(st, 0); }
    sqlite3_finalize(st);
}

// Ejecuta una consulta SQL que devuelve filas de clave (string) y valor (double)
static void exec_rs_kv(sqlite3* db, const std::string& sql, ResultSetKV& rs){
    sqlite3_stmt* st=nullptr;
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &st, nullptr)!=SQLITE_OK)
        throw std::runtime_error(sqlite3_errmsg(db));
    while(sqlite3_step(st)==SQLITE_ROW){
        const unsigned char* s = sqlite3_column_text(st, 0);
        std::string key = s ? reinterpret_cast<const char*>(s) : "(null)";
        double v = sqlite3_column_double(st, 1);
        rs.rows.emplace_back(key, v);
    }
    sqlite3_finalize(st);
}

// Ejecuta una consulta SQL que devuelve una serie de tiempo (año, valor double)
static void exec_ts(sqlite3* db, const std::string& sql, TimeSeries& ts){
    sqlite3_stmt* st=nullptr;
    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &st, nullptr)!=SQLITE_OK)
        throw std::runtime_error(sqlite3_errmsg(db));
    while(sqlite3_step(st)==SQLITE_ROW){
        int year = sqlite3_column_int(st, 0);
        double total = sqlite3_column_double(st, 1);
        ts.points.emplace_back(year, total);
    }
    sqlite3_finalize(st);
}

// Constructor: abre la base de datos SQLite
Analytics::Analytics(const std::string& db_path){
    if(sqlite3_open(db_path.c_str(), &db_)!=SQLITE_OK)
        throw std::runtime_error("open db: "+std::string(sqlite3_errmsg(db_)));
}

// Destructor: cierra la base de datos SQLite
Analytics::~Analytics(){ if(db_) sqlite3_close(db_); }

// Total de incidentes en la tabla incidents
double Analytics::total_incident_count() {
    double total = 0;
    exec_scalar_double(db_, "SELECT COUNT(*) FROM incidents", total);
    return total;
}

// (a) % de incidentes entre 2018-01-01 y 2020-12-31
// Asumimos formato ISO YYYY-MM-DD en incidents.date
// pct = count(range)/count(total)*100
TimeSeries Analytics::incident_totals_by_year(int from_year, int to_year) {
    TimeSeries ts;
    std::string sql = R"SQL(
        WITH cleaned AS (
            SELECT 
                CAST(SUBSTR(TRIM(REPLACE(REPLACE(date, CHAR(13), ''), CHAR(10), '')), 1, 4) AS INT) AS yr
            FROM incidents
            WHERE TRIM(REPLACE(REPLACE(date, CHAR(13), ''), CHAR(10), '')) 
                  GLOB '[1-2][0-9][0-9][0-9]-[0-1][0-9]-[0-3][0-9]'
        )
        SELECT yr, COUNT(*) AS total
        FROM cleaned
        WHERE yr BETWEEN )SQL" + std::to_string(from_year) + " AND " + std::to_string(to_year) + R"SQL(
        GROUP BY yr
        ORDER BY yr
    )SQL";
    exec_ts(db_, sql, ts);
    return ts;
}

// (b) Top 3 transport_mode con detection='intelligence'
ResultSetKV Analytics::top3_transport_by_intelligence(){
    ResultSetKV rs;
    std::string sql = R"SQL(
        SELECT transport_mode, COUNT(*) AS c
        FROM details
        WHERE LOWER(COALESCE(detection, '')) LIKE '%intelligence%' 
        AND TRIM(COALESCE(transport_mode, '')) != ''
        GROUP BY transport_mode
        ORDER BY c DESC
        LIMIT 3
    )SQL";
    exec_rs_kv(db_, sql, rs);
    return rs;
}

// (c) Métodos de detección con mayor promedio de gente arrestada
// Join details->outcomes por report_id, pero primero filtrar detections frecuentes
ResultSetKV Analytics::detection_by_avg_arrests(size_t topN){
    ResultSetKV rs;
    std::string sql = R"SQL(
        WITH detection_counts AS (
            SELECT 
                LOWER(TRIM(REPLACE(REPLACE(detection, CHAR(13), ''), CHAR(10), ''))) AS detection
            FROM details
            WHERE TRIM(REPLACE(REPLACE(detection, CHAR(13), ''), CHAR(10), '')) != ''
            AND detection IS NOT NULL
            GROUP BY detection
            HAVING COUNT(*) >= 10
        ),
        joined AS (
            SELECT 
                LOWER(TRIM(REPLACE(REPLACE(d.detection, CHAR(13), ''), CHAR(10), ''))) AS label,
                COALESCE(o.num_ppl_arrested, 0) AS arrested
            FROM details d
            INNER JOIN outcomes o ON o.report_id = d.report_id
            INNER JOIN detection_counts dc 
            ON dc.detection = LOWER(TRIM(REPLACE(REPLACE(d.detection, CHAR(13), ''), CHAR(10), '')))
            WHERE TRIM(REPLACE(REPLACE(d.detection, CHAR(13), ''), CHAR(10), '')) != ''
        )
        SELECT label, AVG(arrested) AS avg_arrest
        FROM joined
        GROUP BY label
        HAVING label != ''
        ORDER BY avg_arrest DESC
        LIMIT )SQL" + std::to_string(topN) + ";";
    exec_rs_kv(db_, sql, rs);
    return rs;
}

// (d) Categorías con sentencias más largas (convertir a días)
// Join incidents->outcomes por report_id; convertir unidades a días
ResultSetKV Analytics::categories_with_longest_sentences_days(size_t topN){
    ResultSetKV rs;
    std::string sql = R"SQL(
        WITH norm_outcomes AS (
            SELECT report_id,
                CASE
                    WHEN LOWER(TRIM(COALESCE(prison_time_unit, ''))) IN ('year', 'years') THEN COALESCE(prison_time, 0)*365.0
                    WHEN LOWER(TRIM(COALESCE(prison_time_unit, ''))) IN ('month', 'months') THEN COALESCE(prison_time, 0)*30.0
                    WHEN LOWER(TRIM(COALESCE(prison_time_unit, ''))) IN ('week', 'weeks') THEN COALESCE(prison_time, 0)*7.0
                    WHEN LOWER(TRIM(COALESCE(prison_time_unit, ''))) IN ('day', 'days') THEN COALESCE(prison_time, 0)
                    ELSE 0.0
                END AS days
            FROM outcomes
            WHERE prison_time IS NOT NULL AND prison_time > 0
        )
        SELECT i.category, AVG(o.days) AS avg_days
        FROM incidents i
        JOIN norm_outcomes o 
          ON TRIM(REPLACE(REPLACE(o.report_id, CHAR(13), ''), CHAR(10), '')) = 
             TRIM(REPLACE(REPLACE(i.report_id, CHAR(13), ''), CHAR(10), ''))
        WHERE i.category IS NOT NULL AND o.days > 0
        GROUP BY i.category
        ORDER BY avg_days DESC
        LIMIT )SQL" + std::to_string(topN) + ";";
    exec_rs_kv(db_, sql, rs);
    return rs;
}

// (e) Serie de tiempo anual de total de multas (fine) por año
TimeSeries Analytics::fine_totals_per_year() {
    TimeSeries ts;
    std::string sql = R"SQL(
        WITH cleaned AS (
            SELECT 
                i.report_id,
                CAST(SUBSTR(REPLACE(REPLACE(REPLACE(i.date, CHAR(13), ''), CHAR(10), ''), ' ', ''), 1, 4) AS INT) AS yr
            FROM incidents i
            WHERE REPLACE(REPLACE(REPLACE(i.date, CHAR(13), ''), CHAR(10), ''), ' ', '') 
                GLOB '[1-2][0-9][0-9][0-9]-[0-1][0-9]-[0-3][0-9]'
        )
        SELECT c.yr, SUM(COALESCE(o.fine, 0)) AS total_fine
        FROM cleaned c
        JOIN outcomes o ON o.report_id = c.report_id
        GROUP BY c.yr
        ORDER BY c.yr ASC
    )SQL";
    exec_ts(db_, sql, ts);
    return ts;
}

void Analytics::run_diagnostics(std::ostream& out) {
    out << "\n--- Diagnóstico de la base de datos ---\n";

    auto run_scalar = [&](const std::string& label, const std::string& sql){
        double val = 0;
        try {
            exec_scalar_double(db_, sql, val);
            out << label << ": " << val << "\n";
        } catch (const std::exception& e) {
            out << "Error en " << label << ": " << e.what() << "\n";
        }
    };

    run_scalar("Total incidents", "SELECT COUNT(*) FROM incidents");
    run_scalar("Total details", "SELECT COUNT(*) FROM details");
    run_scalar("Total outcomes", "SELECT COUNT(*) FROM outcomes");

    run_scalar("Primer año", R"SQL(
        SELECT MIN(CAST(substr(REPLACE(REPLACE(REPLACE(date, CHAR(13), ''), CHAR(10), ''), ' ', ''),1,4) AS INT)) 
        FROM incidents 
        WHERE REPLACE(REPLACE(REPLACE(date, CHAR(13), ''), CHAR(10), ''), ' ', '') GLOB '[1-2][0-9][0-9][0-9]-[0-1][0-9]-[0-3][0-9]'
    )SQL");

    run_scalar("Último año", R"SQL(
        SELECT MAX(CAST(substr(REPLACE(REPLACE(REPLACE(date, CHAR(13), ''), CHAR(10), ''), ' ', ''),1,4) AS INT)) 
        FROM incidents 
        WHERE REPLACE(REPLACE(REPLACE(date, CHAR(13), ''), CHAR(10), ''), ' ', '') GLOB '[1-2][0-9][0-9][0-9]-[0-1][0-9]-[0-3][0-9]'
    )SQL");

    out << "\nMultas totales por año:\n";
    auto fines = fine_totals_per_year();
    for (const auto& [year, total] : fines.points) {
        out << "  " << year << ": " << total << "\n";
    }

    out << "\nDistribución de detection (top 5):\n";
    ResultSetKV det;
    exec_rs_kv(db_, R"SQL(
        SELECT detection, COUNT(*) AS c 
        FROM details 
        GROUP BY detection 
        ORDER BY c DESC 
        LIMIT 5
    )SQL", det);
    for (auto& [k,v] : det.rows) out << "  " << k << ": " << v << "\n";

    out << "\nDistribución de transport_mode (top 5):\n";
    ResultSetKV tr;    

    exec_rs_kv(db_, R"SQL(
        SELECT transport_mode, COUNT(*) AS c 
        FROM details 
        GROUP BY transport_mode 
        ORDER BY c DESC 
        LIMIT 5
    )SQL", tr);
    for (auto& [k,v] : tr.rows) out << "  " << k << ": " << v << "\n";

    run_scalar("Total personas arrestadas", "SELECT SUM(COALESCE(num_ppl_arrested,0)) FROM outcomes");
    run_scalar("Total multas", "SELECT SUM(COALESCE(fine,0)) FROM outcomes");

    out << "\nPrimeras 3 filas de incidents:\n";
    sqlite3_stmt* stmt=nullptr;
    if (sqlite3_prepare_v2(db_, "SELECT * FROM incidents LIMIT 3", -1, &stmt, nullptr)==SQLITE_OK) {
        while(sqlite3_step(stmt)==SQLITE_ROW) {
            out << "  " << sqlite3_column_text(stmt,0) << " | "
                << sqlite3_column_text(stmt,1) << " | "
                << sqlite3_column_text(stmt,2) << "\n";
        }
        sqlite3_finalize(stmt);
    }

    out << "\nDiagnóstico de sentencias (d):\n";    
    ResultSetKV rs;
    exec_rs_kv(db_, R"SQL(
        SELECT LOWER(TRIM(prison_time_unit)) AS unit, COUNT(*) 
        FROM outcomes 
        WHERE prison_time IS NOT NULL AND prison_time > 0
        GROUP BY unit
        ORDER BY COUNT(*) DESC
    )SQL", rs);
    for (const auto& [unit, count] : rs.rows) {
        out << "  " << unit << ": " << count << "\n";
    }
}