#pragma once

#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <vector>
#include <set>
#include <string>
#include <string_view>
#include <tuple>

#include "geo.h"

namespace catalogue {
    struct Stop {
        std::string stop_name;
        geo::Coordinates coordinates;
    };

    struct Bus {
        std::string bus_name;
        std::vector<Stop*> stops;
    };

    struct RouteInformation {
        int all_stops;
        int unique_stops;
        double distance;
    };

    class TransportCatalogue {

    public:
        void AddStop(const std::string& name, geo::Coordinates stop_coordinates, std::unordered_map<std::string_view, int> next_stops);
        void AddBus(const std::string& name, std::vector<std::string_view> route_stops);
        Stop* FindStop(std::string_view stop);
        Bus* FindBus(std::string_view bus);
        std::set<Bus*>* GetBusesForStop(Stop* stop);
        RouteInformation GetRouteInformation(Bus& bus);

    private:
        std::unordered_map<Stop*, std::set<Bus*>> stop_and_buses_;
        std::unordered_map<Stop*, std::unordered_map<Stop*, int>> stop_and_distances_;
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;

        void AddStopsToBus(Bus* bus, std::vector<std::string_view> route_stops);
        void AddBusToStops(Bus* bus);
        void AddDistanceToOtherStops(Stop* stop, std::unordered_map<std::string_view, int> next_stops);
        std::unordered_map<Stop*, int> ParseNextStopsAndDistance(std::vector<std::string_view> stops);
    };
}


