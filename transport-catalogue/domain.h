#pragma once

#include <set>
#include <string>
#include <vector>

#include "geo.h"

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки.
 *
 */

namespace domain {

    struct Stop {
        std::string stop_name;
        geo::Coordinates coordinates;

        bool operator==(const Stop& other);
    };

    struct StopCmp {
        bool operator()(const Stop* lhs, const Stop* rhs) const;
    };

    struct Bus {
        std::string bus_name;
        bool is_roundtrip;
        std::vector<Stop*> stops;
        std::vector<Stop*> ends;
    };

    struct RoutePoints {
        Stop* first;
        Stop* second;

        struct EqualTo {
            bool operator()(const RoutePoints& lhs, const RoutePoints& rhs) const;
        };

        struct Hasher {
            size_t operator()(const RoutePoints& stops_point) const;
        };
    };

    struct RouteInformation {
        int all_stops;
        int unique_stops;
        int route_length;
        double curvature;
    };

    struct BusCmp {
        bool operator()(const Bus* lhs, const Bus* rhs) const;
    };

    using BusesPtr = std::set<Bus*, BusCmp>;
}
