#include "transport_catalogue.h"

#include <algorithm>
#include <cassert>
#include <iostream>

using namespace catalogue;

void TransportCatalogue::AddStop(const std::string& name, geo::Coordinates stop_coordinates, std::unordered_map<std::string_view, int> next_stops) {
    auto stop = TransportCatalogue::FindStop(name);
    if (stop != nullptr) {
        stop->coordinates = stop_coordinates;
    }
    else {
        stops_.emplace_back(std::move(Stop{  name, stop_coordinates }));
        stop_and_buses_[TransportCatalogue::FindStop(name)] = {};
    }
    AddDistanceToOtherStops(FindStop(name), next_stops);
}

void TransportCatalogue::AddStopsToBus(Bus* bus, std::vector<std::string_view> route_stops) {
    for (const auto s : route_stops) {
        const Stop* stop = TransportCatalogue::FindStop(s);
        if (stop == nullptr) {
            AddStop(std::move(std::string(s)), geo::Coordinates{}, {});
        }
    }
    for (const auto& s : route_stops) {
        bus->stops.push_back(FindStop(s));
    }
}

void TransportCatalogue::AddBusToStops(Bus* bus) {
    for (auto s : bus->stops) {
        stop_and_buses_.at(s).insert(bus);
    }
}

void TransportCatalogue::AddBus(const std::string& name, std::vector<std::string_view> route_stops) {
    auto bus = TransportCatalogue::FindBus(name);
    if (bus == nullptr) {
        buses_.emplace_back(std::move(Bus{ name, {} }));
    }
    bus = TransportCatalogue::FindBus(name);
    AddStopsToBus(bus, route_stops);
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

std::set<Bus*>* TransportCatalogue::GetBusesForStop(Stop* stop) {
    if (stop_and_buses_.count(stop) == 0) {
        return nullptr;
    }
    return &stop_and_buses_.at(stop);
}

RouteInformation TransportCatalogue::GetRouteInformation(Bus& bus) {
    int all_stops = static_cast<int>(bus.stops.size());

    std::set<Stop*> set{ bus.stops.begin(), bus.stops.end() };
    int unique_stops = static_cast<int>(set.size());

    std::vector<Stop*> bus_stops = bus.stops;
    double distance = .0;
    for (size_t i = 0; i < bus_stops.size() - 1; ++i) {
        distance += geo::ComputeDistance({ bus_stops[i]->coordinates.lat, bus_stops[i]->coordinates.lng },
            { bus_stops[i + 1]->coordinates.lat, bus_stops[i + 1]->coordinates.lng });
    }
    RouteInformation info_to_return{ all_stops, unique_stops, distance };
    return info_to_return;
}

void TransportCatalogue::AddDistanceToOtherStops(Stop* stop, std::unordered_map<std::string_view, int> next_stops) {
    if (next_stops.size() != 0) {
        for (const auto [stop_name, dist] : next_stops) {
            auto route_stop = FindStop(stop_name);
            if (route_stop != nullptr) {
                stop_and_distances_[stop].insert({ route_stop, dist });
            }
            else {
                AddStop(std::string(stop_name), geo::Coordinates{}, {});
                stop_and_distances_[stop].insert({ FindStop(stop_name), dist});
            }
        }
    }
}
