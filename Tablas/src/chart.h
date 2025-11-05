#pragma once
#include <string>
#include "analytics.h"


class Chart {
public:
    static void save_bar(const ResultSetKV& rs, const std::string& title, const std::string& outpath, const std::string& ylabel="");
    static void save_line(const TimeSeries& ts, const std::string& title, const std::string& outpath, const std::string& ylabel="");
};