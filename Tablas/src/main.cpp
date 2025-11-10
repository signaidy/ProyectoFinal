#include <iostream>
#include <filesystem>
#include <fstream>
#include "db_loader.h"
#include "analytics.h"
#include "chart.h"

using namespace std;
namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    try {
        bool modo_diagnostico = false;
        if (argc > 1 && std::string(argv[1]) == "--diagnostico") {
            modo_diagnostico = true;
        }

        DbConfig cfg; // usa /app/data y /app/outputs dentro del contenedor
        DbLoader loader(cfg);
        loader.init_schema();
        loader.load_all_csv_parallel();

        Analytics an(cfg.db_path);

        if (modo_diagnostico) {
            std::filesystem::create_directories("/app/outputs");
            std::ofstream diag_out("/app/outputs/diagnostico.txt");
            an.run_diagnostics(std::cout);
            an.run_diagnostics(diag_out);
            std::cout << "\nDiagnóstico exportado a /app/outputs/diagnostico.txt\n";
            return 0;
        }

        // (a)
        double pct = an.pct_incidents_2018_2020();
        cout << "(a) % Incidentes 2018-2020: " << pct << "%\n";
        ResultSetKV a_rs; a_rs.rows.push_back({"2018-2020", pct});
        fs::create_directories("/app/outputs");
        Chart::save_bar(a_rs, "% Incidentes 2018-2020", "/app/outputs/a_pct_incidents_2018_2020.png", "%");


        // (b)
        auto b = an.top3_transport_by_intelligence();
        Chart::save_bar(b, "Top 3 transporte (detection=intelligence)", "/app/outputs/b_top3_transport_intelligence.png", "Conteo");


        // (c)
        auto c = an.detection_by_avg_arrests(10);
        Chart::save_bar(c, "Detección por promedio de arrestos", "/app/outputs/c_detection_avg_arrests.png", "Promedio arrestos");


        // (d)
        auto d = an.categories_with_longest_sentences_days(10);
        Chart::save_bar(d, "Categorías con mayores sentencias (días)", "/app/outputs/d_categories_longest_sentences_days.png", "Días (promedio)");


        // (e)
        auto e = an.fine_totals_per_year();
        Chart::save_line(e, "Serie anual de multas (total)", "/app/outputs/e_fine_totals_per_year.png", "Multa total");


        cout << "Listo. Revisa /outputs para los PNG y /outputs/incidents.db para la BD.\n";
    } catch(const std::exception& e){
        cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
    return 0;
}