#include "stat_reader.h"

#include <iostream>
#include <iomanip>

using namespace std::literals;
using namespace stat_reader;

void ParseAndPrintStatAboutStop(const catalogue::TransportCatalogue& transport_catalogue, std::string_view request, std::string_view information,
    std::ostream& output) {
    auto current_stop = const_cast<catalogue::TransportCatalogue&>(transport_catalogue).FindStop(information);
    if (current_stop == nullptr) {
        output << request.data() << ": not found" << std::endl;
    }
    else {
        auto current_bus = const_cast<catalogue::TransportCatalogue&>(transport_catalogue).GetBusesForStop(current_stop);
        if (current_bus->size() == 0) {
            output << request.data() << ": no buses" << std::endl;
        }
        else {
            output << request.data() << ": buses";
            for (const auto b : *current_bus) {
                output << " "s << b->bus_name;
            }
            output << std::endl;
        }
    }
}

void ParseAndPrintStatAboutBus(const catalogue::TransportCatalogue& transport_catalogue, std::string_view request, std::string_view information,
    std::ostream& output) {
    auto bus = const_cast<catalogue::TransportCatalogue&>(transport_catalogue).FindBus(information);
    if (bus == nullptr) {
        output << request.data() << ": not found"s << std::endl;
    }
    else {
        catalogue::RouteInformation route_info = const_cast<catalogue::TransportCatalogue&>(transport_catalogue).GetRouteInformation(*bus);
        output << std::setprecision(6) << request.data() << ": "s
            << std::to_string(route_info.all_stops) << " stops on route, "s
            << std::to_string(route_info.unique_stops) << " unique stops, "s
            << std::to_string(route_info.distance) << " route length"s << std::endl;
    }
}

void stat_reader::ParseAndPrintStat(const catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
    std::ostream& output) {
    std::string_view command = request.substr(0, request.find_first_of(' '));
    std::string_view information = request.substr(request.find_first_of(' ') + 1, request.size() - request.find_first_of(' '));
    if (command == "Stop"s) {
        ParseAndPrintStatAboutStop(transport_catalogue, request, information, output);
    }
    else if (command == "Bus"s) {
        ParseAndPrintStatAboutBus(transport_catalogue, request, information, output);
    }
}