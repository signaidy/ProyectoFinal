#include "chart.h"
#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>
#include "matplotlibcpp.h"

using namespace std;
namespace plt = matplotlibcpp; // usamos wrapper minimalista del header


void Chart::save_bar(const ResultSetKV& rs, const string& title, const string& outpath, const string& ylabel){
    plt::Plotter p;
    p.figure(title);


    vector<string> labels; labels.reserve(rs.rows.size());
    vector<double> values; values.reserve(rs.rows.size());
    for (auto& [k,v] : rs.rows){ labels.push_back(k); values.push_back(v); }


    p.bar(labels, values);
    if(!ylabel.empty()) p.ylabel(ylabel);
    p.tight_layout();
    p.savefig(outpath);
    p.clf();
}


void Chart::save_line(const TimeSeries& ts, const string& title, const string& outpath, const string& ylabel){
    plt::Plotter p;
    p.figure(title);


    vector<double> x, y; x.reserve(ts.points.size()); y.reserve(ts.points.size());
    for(auto& [year,total] : ts.points){ x.push_back((double)year); y.push_back(total); }


    p.plot(x, y);
    p.xlabel("Año");
    if(!ylabel.empty()) p.ylabel(ylabel);
    p.tight_layout();
    p.savefig(outpath);
    p.clf();
}