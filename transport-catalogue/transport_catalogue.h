#pragma once

#include <deque>
#include <map>
#include <set>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

#include "domain.h"
#include "graph.h"

namespace catalogue {
    using namespace domain;

    class TransportCatalogue {
        
    public:
        void AddStop(const std::string& name, geo::Coordinates stop_coordinates);
        void AddBus(const std::string& name, bool is_roundtrip, std::vector<std::string_view> route_stops, std::vector<std::string_view> route_ends);
        Stop* FindStop(std::string_view stop);
        Bus* FindBus(std::string_view bus);
        void AddDistanceBetweenStops(Stop* from, Stop* to, int distance);
        int GetDistanceBetweenStops(Stop* from, Stop* to);
        const domain::BusesPtr GetBusesForStop(Stop* stop);
        const RouteInformation GetRouteInformation(Bus* bus);
        const std::unordered_map<std::string_view, Bus*>* GetBuses() const;
        const std::unordered_map<std::string_view, Stop*>* GetStops() const;
        void AddStopAsVertex(const std::string& name, int bus_wait_time);
        graph::DirectedWeightedGraph<double>* GetDwg();
        size_t GetPortalId(Stop* stop);
        size_t GetHubId(Stop* stop);
        void FillEdges(int bus_velocity);

    private:
        std::unordered_map<Stop*, domain::BusesPtr> stop_and_buses_;
        std::unordered_map<RoutePoints, int, RoutePoints::Hasher, RoutePoints::EqualTo> stop_and_distances_;

        std::unordered_map<std::string_view, Stop*> stops_catalogue_;
        std::unordered_map<std::string_view, Bus*> buses_catalogue_;

        std::deque<Stop> stops_;
        std::deque<Bus> buses_;

        graph::DirectedWeightedGraph<double> dwg_;
        std::map<domain::Stop*, size_t> portals_;
        std::map<domain::Stop*, size_t> hubs_;

        void AddStopsToBus(Bus* bus, const std::vector<std::string_view>& route_stops);
        void AddBusToStops(Bus* bus);
        int GetRouteLength(std::vector<Stop*> bus_stops) const;
        double GetGeoDistance(std::vector<Stop*> bus_stops) const;

        template<typename It>
        void ParseBusRouteOnEdges(It begin, It end, int bus_velocity, const Bus* bus);

    };
}


