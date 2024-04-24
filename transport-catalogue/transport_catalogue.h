#pragma once

#include <unordered_map>
#include <deque>
#include <vector>
#include <set>
#include <string>
#include <string_view>
#include <tuple>

#include "geo.h"

namespace catalogue {
    struct Stop {
        double lat = .0;
        double lng = .0;
        std::set<std::string_view> buses;
    };

    class TransportCatalogue {

    public:
        void AddStop(std::string id, double lat, double lng);
        std::string_view AddStopFromRoute(std::string stop_name);
        void AddRoute(std::string id, std::vector<std::string_view> route_stops);
        const Stop* FindStop(std::string_view stop);
        const std::pair<std::string_view, std::vector<std::string_view>> FindRoute(std::string_view route);
        std::tuple<int, int, double> GetRouteInformation(std::string_view route);

    private:
        std::unordered_map<std::string_view, Stop> stops_;
        std::unordered_map<std::string_view, std::vector<std::string_view>> routes_;
        std::deque<std::string> stop_list_;
        std::deque<std::string> bus_list_;
    };
}


