#pragma once
#include <string>
#include <sqlite3.h>
#include <vector>
#include <utility>


struct ResultSetKV { // para barras: label, value
    std::vector<std::pair<std::string,double>> rows;
};


struct TimeSeries { // para series anuales
    std::vector<std::pair<int,double>> points; // year, total
};


class Analytics {
public:
    Analytics(const std::string& db_path);
    ~Analytics();

    double total_incident_count();
    TimeSeries incident_totals_by_year(int from_year, int to_year);
    ResultSetKV top3_transport_by_intelligence();
    ResultSetKV detection_by_avg_arrests(size_t topN);
    ResultSetKV categories_with_longest_sentences_days(size_t topN);
    TimeSeries fine_totals_per_year();
    void run_diagnostics(std::ostream& out);

private:
    sqlite3* db_ {nullptr};
};