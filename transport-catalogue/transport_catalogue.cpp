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
        stop_and_buses_[TransportCatalogue::FindStop(name)] = {};
    }
}

void TransportCatalogue::AddStopsToBus(Bus* bus, const std::vector<std::string_view>& route_stops) {
    for (const auto& stop_at_route : route_stops) {
        const Stop* current_stop = TransportCatalogue::FindStop(stop_at_route);
        if (current_stop == nullptr) {
            AddStop(std::string(stop_at_route), geo::Coordinates{});
        }
    }
    for (const auto& stop_at_route : route_stops) {
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
    }
    bus = TransportCatalogue::FindBus(name);
    AddStopsToBus(bus, std::move(route_stops));
    AddBusToStops(bus);
}

Stop* TransportCatalogue::FindStop(std::string_view stop_name) {
    auto it = std::find_if(stops_.begin(), stops_.end(), [&](Stop& s) {
        return s.stop_name == stop_name;
        });
    if (it == stops_.end()) {
        return nullptr;
    }
    return &*it;
}

Bus* TransportCatalogue::FindBus(std::string_view bus_name) {
    auto it = std::find_if(buses_.begin(), buses_.end(), [&](Bus& b) {
        return b.bus_name == bus_name;
        });
    if (it == buses_.end()) {
        return nullptr;
    }
    return &*it;
}

std::set<Bus*, BusCmp>* TransportCatalogue::GetBusesForStop(Stop* stop) {
    if (stop_and_buses_.count(stop) == 0) {
        return nullptr;
    }

    return &stop_and_buses_.at(stop);
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

RouteInformation TransportCatalogue::GetRouteInformation(Bus& bus) {
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

void TransportCatalogue::AddDistanceBetweenStops(const std::string_view& from, const std::string_view& to, int distance) {
    auto from_stop = FindStop(from);
    auto to_stop = FindStop(to);
    if (to_stop != nullptr) {
        RoutePoints stops_point = { from_stop, to_stop };
        stop_and_distances_[stops_point] = distance;
    }
    else {
        AddStop(std::string(to), geo::Coordinates{});
        stop_and_distances_[{ from_stop, FindStop(to) }] = distance;
    }
}
