#include "chart.h"
#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>
#include "matplotlibcpp.h"
#include <iostream>

using namespace std;
namespace plt = matplotlibcpp; // usamos wrapper minimalista del header

void Chart::save_bar(const ResultSetKV& rs, const std::string& title, const std::string& outpath, const std::string& ylabel){
    plt::Plotter p;
    p.clf();
    p.figure(title);

    std::vector<std::string> labels;
    std::vector<double> values;

    for (auto& [k,v] : rs.rows){
        std::string label = k.empty() ? "(vacío)" : k;
        labels.push_back(label);
        values.push_back(v);
    }

    p.bar(labels, values);
    if(!ylabel.empty()) p.ylabel(ylabel);
    p.tight_layout();
    p.savefig(outpath);
    p.clf();
}

void Chart::save_line(const TimeSeries& ts, const string& title, const string& outpath, const string& ylabel){
    plt::Plotter p;
    p.figure(title);

    std::vector<double> x, y;
    for (const auto& [year, total] : ts.points) {
        x.push_back(static_cast<double>(year)); // convertir año a double
        y.push_back(total);
    }

    if (!x.empty()) {
        p.plot(x, y);
        p.xlabel("Año");
        if (!ylabel.empty()) p.ylabel(ylabel);
        p.tight_layout();
        p.savefig(outpath);
    } else {
        std::cerr << "⚠️ Serie de tiempo vacía: no se graficará nada\n";
    }

    p.clf(); // limpiar aunque no haya graficado nada
}