#include <iostream>
#include <filesystem>
#include <fstream>
#include "db_loader.h"
#include "analytics.h"
#include "chart.h"

using namespace std;
namespace fs = std::filesystem;

ResultSetKV convert_ts_to_kv(const TimeSeries& ts) {
    ResultSetKV kv;
    for (const auto& [year, value] : ts.points) {
        kv.rows.emplace_back(std::to_string(year), value);
    }
    return kv;
}

int main(int argc, char* argv[]) {
    try {
        bool modo_diagnostico = false;
        if (argc > 1 && std::string(argv[1]) == "--diagnostico") {
            modo_diagnostico = true;
        }

        DbConfig cfg;
        cfg.db_path = "/app/outputs/db/incidents.db";  // fuera del volumen montado
        DbLoader loader(cfg);
        loader.init_schema();
        loader.load_all_csv_parallel();

        Analytics an(cfg.db_path);

        if (modo_diagnostico) {
            std::filesystem::create_directories("/app/outputs");
            std::ofstream diag_out("/app/outputs/images/diagnostico.txt");
            an.run_diagnostics(std::cout);
            an.run_diagnostics(diag_out);
            std::cout << "\nDiagnóstico exportado a /app/outputs/images/diagnostico.txt\n";
            return 0;
        }

        // (a)
        auto a = an.incident_totals_by_year(2018, 2020);
        double total_historico = an.total_incident_count();

        double subtotal = 0;
        for (auto& [_, count] : a.points) subtotal += count;

        double pct_global = (total_historico > 0) ? (subtotal / total_historico) * 100.0 : 0.0;

        cout << "(a) Incidentes por año (2018–2020):\n";
        for (auto& [year, count] : a.points) {
            cout << "  " << year << ": " << count << "\n";
        }

        std::ostringstream title;
        title << "Incidentes por año (2018–2020), " << std::fixed << std::setprecision(2) << pct_global << "% del total";

        Chart::save_bar(convert_ts_to_kv(a), title.str(), "/app/outputs/images/a_incidents_2018_2020.png", "Incidentes");

        // (b)
        auto b = an.top3_transport_by_intelligence();
        
        cout << "(b) Top 3 transporte (detection=intelligence):\n";
        for (const auto& [transport, count] : b.rows) {
            cout << "  " << transport << ": " << count << "\n";
        }
        Chart::save_bar(b, "Top 3 transporte (detection=intelligence)", "/app/outputs/images/b_top3_transport_intelligence.png", "Conteo");


        // (c)
        auto c = an.detection_by_avg_arrests(10);
        cout << "(c) Detección por promedio de arrestos (top 10):\n";
        for (const auto& [detection, avg_arrests] : c.rows) {
            cout << "  " << detection << ": " << avg_arrests << "\n";
        }
        Chart::save_bar(c, "Detección por promedio de arrestos", "/app/outputs/images/c_detection_avg_arrests.png", "Promedio arrestos");

        // (d)
        auto d = an.categories_with_longest_sentences_days(10);
        cout << "(d) Categorías con mayores sentencias (días):\n";
        for (const auto& [category, avg_days] : d.rows) {
            cout << "  " << category << ": " << avg_days << "\n";
        }
        Chart::save_bar(d, "Categorías con mayores sentencias (días)", "/app/outputs/images/d_categories_longest_sentences_days.png", "Días (promedio)");


        // (e)
        auto e = an.fine_totals_per_year();
        cout << "(e) Multas totales por año:\n";
        for (const auto& [year, total] : e.points) {
            cout << "  " << year << ": " << total << "\n";
        }
        Chart::save_line(e, "Serie anual de multas (total)", "/app/outputs/images/e_fine_totals_per_year.png", "Multa total");


        cout << "Listo. Revisa /outputs para los PNG y /outputs/incidents.db para la BD.\n";
    } catch(const std::exception& e){
        cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
    return 0;
}