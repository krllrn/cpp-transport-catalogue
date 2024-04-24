#include "stat_reader.h"

#include <iostream>
#include <iomanip>

using namespace std::literals;
using namespace stat_reader;

void stat_reader::ParseAndPrintStat(const catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
    std::ostream& output) {
    std::string_view command = request.substr(0, request.find_first_of(' '));
    std::string_view information = request.substr(request.find_first_of(' ') + 1, request.size() - request.find_first_of(' '));
    if (command == "Stop"s) {
        auto current_stop = const_cast<catalogue::TransportCatalogue&>(transport_catalogue).FindStop(information);
        if (current_stop == nullptr) {
            output << request.data() << ": not found" << std::endl;
        }
        else if (current_stop->buses.size() == 0) {
            output << request.data() << ": no buses" << std::endl;
        }
        else {
            output << request.data() << ": buses";
            for (const auto b : current_stop->buses) {
                output << " "s << b;
            }
            output << std::endl;
        }
    }
    else if (command == "Bus"s) {
        auto [all_stops, unique_stops, distance] = const_cast<catalogue::TransportCatalogue&>(transport_catalogue).GetRouteInformation(information);
        if (all_stops == 0 && unique_stops == 0 && distance == .0) {
            output << request.data() << ": not found"s << std::endl;
        }
        else {
            output << std::setprecision(6) << request.data() << ": "s << std::to_string(all_stops) << " stops on route, "s << std::to_string(unique_stops)
                << " unique stops, "s << std::to_string(distance) << " route length"s << std::endl;
        }
    }
}