#include "transport_catalogue.h"

#include <algorithm>

using namespace catalogue;

void TransportCatalogue::AddStop(const std::string& name, geo::Coordinates stop_coordinates) {
    auto stop = TransportCatalogue::FindStop(name);
    if (stop != nullptr) {
        stop->coordinates = stop_coordinates;
    }
    else {
        stops_.emplace_back(std::move(Stop{ name, stop_coordinates }));
        stops_catalogue_[stops_.back().stop_name] = &stops_.back();
        stop_and_buses_[TransportCatalogue::FindStop(name)] = {};
    }
}

void TransportCatalogue::AddStopsToBus(Bus* bus, const std::vector<std::string_view>& route_stops) {
    for (const auto& stop_at_route : route_stops) {
        if (TransportCatalogue::FindStop(stop_at_route) == nullptr) {
            AddStop(std::string(stop_at_route), geo::Coordinates{});
        }
        bus->stops.push_back(FindStop(stop_at_route));
    }
}

void TransportCatalogue::AddBusToStops(Bus* bus) {
    for (auto stop : bus->stops) {
        stop_and_buses_.at(stop).insert(bus);
    }
}

void TransportCatalogue::AddBus(const std::string& name, std::vector<std::string_view> route_stops) {
    auto bus = TransportCatalogue::FindBus(name);
    if (bus == nullptr) {
        buses_.emplace_back(std::move(Bus{ name, {} }));
        buses_catalogue_[buses_.back().bus_name] = &buses_.back();
    }
    bus = TransportCatalogue::FindBus(name);
    AddStopsToBus(bus, std::move(route_stops));
    AddBusToStops(bus);
}

Stop* TransportCatalogue::FindStop(std::string_view stop_name) {
    if (auto stop_find = stops_catalogue_.find(stop_name); stop_find != stops_catalogue_.end()) {
        return stop_find->second;
    }
    return nullptr;
}

Bus* TransportCatalogue::FindBus(std::string_view bus_name) {
    if (auto bus_find = buses_catalogue_.find(bus_name); bus_find != buses_catalogue_.end()) {
        return bus_find->second;
    }
    return nullptr;
}

std::set<Bus*, BusCmp>* TransportCatalogue::GetBusesForStop(const Stop* stop) {
    if (FindStop(stop->stop_name) != nullptr) {
        return &stop_and_buses_.at(FindStop(stop->stop_name));
    }

    return nullptr;
}

int TransportCatalogue::GetRouteLength(std::vector<Stop*> bus_stops) {
    int route_length = 0;
    for (size_t i = 0; i < bus_stops.size() - 1; ++i) {
        Stop* current = bus_stops[i];
        Stop* next = bus_stops[i + 1];
        RoutePoints first = { current, next };
        RoutePoints second = { next, current };
        if (stop_and_distances_.find(first) != stop_and_distances_.end()) {
            route_length += stop_and_distances_.at(first);
        }
        else {
            if (stop_and_distances_.find(second) != stop_and_distances_.end()) {
                route_length += stop_and_distances_.at(second);
            }
        }
    }
    return route_length;
}

double TransportCatalogue::GetGeoDistance(std::vector<Stop*> bus_stops) {
    double geo_distance = .0;
    for (size_t i = 0; i < bus_stops.size() - 1; ++i) {
        geo_distance += geo::ComputeDistance({ bus_stops[i]->coordinates.lat, bus_stops[i]->coordinates.lng },
            { bus_stops[i + 1]->coordinates.lat, bus_stops[i + 1]->coordinates.lng });
    }
    return geo_distance;
}

RouteInformation TransportCatalogue::GetRouteInformation(const Bus& bus) {
    int all_stops = static_cast<int>(bus.stops.size());

    std::set<Stop*> set{ bus.stops.begin(), bus.stops.end() };
    int unique_stops = static_cast<int>(set.size());

    std::vector<Stop*> bus_stops = bus.stops;
    int route_length = GetRouteLength(bus_stops);
    double geo_distance = GetGeoDistance(bus_stops);
    double curvature = route_length == 0 ? .0 : route_length / geo_distance;

    RouteInformation info_to_return{ all_stops, unique_stops, route_length, curvature };
    return info_to_return;
}

void TransportCatalogue::AddDistanceBetweenStops(Stop* from, Stop* to, int distance) {
    RoutePoints stops_point = { from, to };
    stop_and_distances_[stops_point] = distance;
}
