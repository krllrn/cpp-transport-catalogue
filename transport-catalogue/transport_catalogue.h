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

        bool operator==(const Stop& other) {
            return (stop_name == other.stop_name && coordinates.lat == other.coordinates.lat && coordinates.lng == other.coordinates.lng);
        }
    };

    struct Bus {
        std::string bus_name;
        std::vector<Stop*> stops;
    };

    struct RoutePoints {
        Stop* first;
        Stop* second;

        struct EqualTo {
            bool operator()(const RoutePoints& lhs, const RoutePoints& rhs) const {
                return (lhs.first == rhs.first && lhs.second == rhs.second);
            }
        };

        struct Hasher {
            size_t operator()(const RoutePoints& stops_point) const {
                size_t hash_s = std::hash<std::string>{}(stops_point.first->stop_name) * 37
                    + std::hash<std::string>{}(stops_point.second->stop_name) * 37 * 37 * 37;
                return hash_s;
            }
        };
    };

    struct RouteInformation {
        int all_stops;
        int unique_stops;
        int route_length;
        double curvature;
    };

    struct BusCmp {
        bool operator()(const Bus* lhs, const Bus* rhs) const
        {
            return std::lexicographical_compare(lhs->bus_name.begin(), lhs->bus_name.end(), rhs->bus_name.begin(), rhs->bus_name.end());
        }
    };

    class TransportCatalogue {

    public:
        void AddStop(const std::string& name, geo::Coordinates stop_coordinates);
        void AddBus(const std::string& name, std::vector<std::string_view> route_stops);
        Stop* FindStop(std::string_view stop);
        Bus* FindBus(std::string_view bus);
        void AddDistanceBetweenStops(const std::string_view& from, const std::string_view& to, int distance);
        std::set<Bus*, BusCmp>* GetBusesForStop(Stop* stop);
        RouteInformation GetRouteInformation(Bus& bus);

    private:
        std::unordered_map<Stop*, std::set<Bus*, BusCmp>> stop_and_buses_;
        std::unordered_map<RoutePoints, int, RoutePoints::Hasher, RoutePoints::EqualTo> stop_and_distances_;
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;

        void AddStopsToBus(Bus* bus, const std::vector<std::string_view>& route_stops);
        void AddBusToStops(Bus* bus);
        int GetRouteLength(std::vector<Stop*> bus_stops);
        double GetGeoDistance(std::vector<Stop*> bus_stops);
    };
}


