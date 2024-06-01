#include "stat_reader.h"

#include <iostream>
#include <iomanip>

using namespace stat_reader;

std::string_view TrimReader(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

void ParseAndPrintStatAboutStop(const catalogue::TransportCatalogue& transport_catalogue, std::string_view request, std::string_view stop_name, std::ostream& output) {
    auto current_stop = const_cast<catalogue::TransportCatalogue&>(transport_catalogue).FindStop(stop_name);
    if (current_stop == nullptr) {
        output << request.data() << ": not found\n";
    }
    else {
        auto current_bus = const_cast<catalogue::TransportCatalogue&>(transport_catalogue).GetBusesForStop(current_stop);
        if (current_bus.empty()) {
            output << request.data() << ": no buses\n";
        }
        else {
            output << request.data() << ": buses";
            for (const auto b : current_bus) {
                output << " " << b->bus_name;
            }
            output << "\n";
        }
    }
}

void ParseAndPrintStatAboutBus(const catalogue::TransportCatalogue& transport_catalogue, std::string_view request, std::string_view bus_name,
    std::ostream& output) {
    auto bus = const_cast<catalogue::TransportCatalogue&>(transport_catalogue).FindBus(bus_name);
    if (bus == nullptr) {
        output << request.data() << ": not found\n";
    }
    else {
        catalogue::RouteInformation route_info = const_cast<catalogue::TransportCatalogue&>(transport_catalogue).GetRouteInformation(bus);
        output << std::setprecision(6) << request.data() << ": "
            << route_info.all_stops << " stops on route, "
            << route_info.unique_stops << " unique stops, "
            << route_info.route_length << " route length, "
            << route_info.curvature << " curvature\n";
    }
}

void stat_reader::ParseAndPrintStat(const catalogue::TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output) {
    std::string_view command = request.substr(0, request.find_first_of(' '));
    std::string_view information = TrimReader(request.substr(request.find_first_of(' ') + 1, request.size() - request.find_first_of(' ')));
    if (command == "Stop") {
        ParseAndPrintStatAboutStop(transport_catalogue, request, information, output);
    }
    else if (command == "Bus") {
        ParseAndPrintStatAboutBus(transport_catalogue, request, information, output);
    }
}