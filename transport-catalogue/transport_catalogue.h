#pragma once

#include <deque>
#include <map>
#include <set>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

#include "domain.h"

namespace catalogue {
    using namespace domain;

    class TransportCatalogue {
        
    public:
        void AddStop(const std::string& name, geo::Coordinates stop_coordinates);
        void AddBus(const std::string& name, bool is_roundtrip, std::vector<std::string_view> route_stops, std::vector<std::string_view> route_ends);
        Stop* FindStop(std::string_view stop);
        Bus* FindBus(std::string_view bus);
        void AddDistanceBetweenStops(Stop* from, Stop* to, int distance);
        const int GetDistanceBetweenStops(Stop* from, Stop* to) const;
        const domain::BusesPtr GetBusesForStop(Stop* stop);
        const RouteInformation GetRouteInformation(Bus* bus);
        const std::unordered_map<std::string_view, Bus*>* GetBuses() const;
        const std::unordered_map<std::string_view, Stop*>* GetStops() const;
        const size_t GetPortalId(Stop* stop) const;
        const size_t GetHubId(Stop* stop) const;

    private:
        std::unordered_map<Stop*, domain::BusesPtr> stop_and_buses_;
        std::unordered_map<RoutePoints, int, RoutePoints::Hasher, RoutePoints::EqualTo> stop_and_distances_;

        std::unordered_map<std::string_view, Stop*> stops_catalogue_;
        std::unordered_map<std::string_view, Bus*> buses_catalogue_;

        std::deque<Stop> stops_;
        std::deque<Bus> buses_;

        std::map<domain::Stop*, size_t> portals_;
        std::map<domain::Stop*, size_t> hubs_;

        void AddStopsToBus(Bus* bus, const std::vector<std::string_view>& route_stops);
        void AddBusToStops(Bus* bus);
        int GetRouteLength(std::vector<Stop*> bus_stops) const;
        double GetGeoDistance(std::vector<Stop*> bus_stops) const;

    };
}


