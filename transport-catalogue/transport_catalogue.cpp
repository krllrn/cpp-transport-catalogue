#include "transport_catalogue.h"

#include <algorithm>
#include <cassert>
#include <iostream>

using namespace catalogue;

void TransportCatalogue::AddStop(std::string id, double lat, double lng) {
    auto it = std::find(stop_list_.begin(), stop_list_.end(), id);
    if (it != stop_list_.end()) {
        Stop& current_stop = stops_.at(stop_list_.at(it - stop_list_.begin()));
        current_stop.lat = lat;
        current_stop.lng = lng;
    }
    else {
        stop_list_.emplace_back(id);
        stops_[stop_list_.back()] = { lat, lng, {} };
    }
}

std::string_view TransportCatalogue::AddStopFromRoute(std::string stop_name) {
    auto it = std::find(stop_list_.begin(), stop_list_.end(), stop_name);
    if (it != stop_list_.end()) {
        stops_.at(stop_list_.at(it - stop_list_.begin())).buses.insert(bus_list_.back());
    }
    else {
        stop_list_.emplace_back(stop_name);
        stops_[stop_list_.back()] = { .0, .0, {bus_list_.back()} };
    }
    auto it2 = std::find(stop_list_.begin(), stop_list_.end(), stop_name);
    return stop_list_.at(it2 - stop_list_.begin());
}

void TransportCatalogue::AddRoute(std::string id, std::vector<std::string_view> route_stops) {
    auto it = std::find(bus_list_.begin(), bus_list_.end(), id);
    if (it == bus_list_.end()) {
        bus_list_.emplace_back(move(id));
        std::vector<std::string_view> new_route;
        for (const auto s : route_stops) {
            std::string tmp_stop(s);
            new_route.push_back(AddStopFromRoute(tmp_stop));
        }
        routes_[bus_list_.back()] = new_route;
    }
}

const Stop* TransportCatalogue::FindStop(std::string_view stop_name) {
    if (stops_.count(stop_name) == 0) {
        return nullptr;
    }
    return &stops_.at(stop_name);
}

const std::pair<std::string_view, std::vector<std::string_view>> TransportCatalogue::FindRoute(std::string_view route_name) {
    assert(routes_.count(route_name) == 0);
    return { route_name, routes_.at(route_name) };
}

std::tuple<int, int, double> TransportCatalogue::GetRouteInformation(std::string_view route_name) {
    if (routes_.count(route_name) == 0) {
        return {0 ,0, .0};
    }
    std::vector<std::string_view> route = routes_.at(route_name);
    int all_stops = static_cast<int>(route.size());
    std::set<std::string_view> set{ route.begin(), route.end() };
    int unique_stops = static_cast<int>(set.size());

    double distance = .0;
    for (size_t i = 0; i < route.size() - 1; ++i) {
        distance += geo::ComputeDistance({ stops_.at(route[i]).lat, stops_.at(route[i]).lng },
            { stops_.at(route[i + 1]).lat, stops_.at(route[i + 1]).lng });
    }

    return { all_stops, unique_stops, distance };
}
