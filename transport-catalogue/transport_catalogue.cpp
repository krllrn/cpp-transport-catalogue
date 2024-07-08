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
        auto stop = TransportCatalogue::FindStop(name);
        portals_[stop] = portals_.size() + hubs_.size();
        hubs_[stop] = portals_.size() + hubs_.size();
        dwg_.ResizeIncidenceList(portals_.size() + hubs_.size());
    }
}

void TransportCatalogue::AddStopAsVertex(const std::string& name, int bus_wait_time) {
    auto stop = TransportCatalogue::FindStop(name);
    dwg_.AddEdge(graph::Edge<double>{
        portals_.at(stop), stop->stop_name, hubs_.at(stop), stop->stop_name, bus_wait_time * 1.0, {}});
}

graph::DirectedWeightedGraph<double>* catalogue::TransportCatalogue::GetDwg()
{
    return &dwg_;
}

size_t catalogue::TransportCatalogue::GetPortalId(Stop* stop)
{
    return portals_.at(stop);
}

size_t catalogue::TransportCatalogue::GetHubId(Stop* stop)
{
    return hubs_.at(stop);
}

/*
void catalogue::TransportCatalogue::FillEdges(int bus_velocity) {
    for (const auto& bus : buses_) {
        for (size_t i = 0; i < bus.stops.size(); ++i) {
            int passed_distance = 0;
            for (size_t j = i + 1; j < bus.stops.size(); ++j) {
                int distance = TransportCatalogue::GetDistanceBetweenStops(bus.stops[i], bus.stops[j]);
                if (distance == 0) {
                    if (!bus.is_roundtrip) {
                        distance = TransportCatalogue::GetDistanceBetweenStops(bus.stops[j], bus.stops[i]);
                        if (distance == 0) {
                            distance = passed_distance + TransportCatalogue::GetDistanceBetweenStops(bus.stops[j - 1], bus.stops[j]);
                            passed_distance = distance;
                        }
                        else {
                            passed_distance += distance;
                        }
                    }
                    else {
                        distance = passed_distance + TransportCatalogue::GetDistanceBetweenStops(bus.stops[j - 1], bus.stops[j]);
                        passed_distance = distance;
                    }
                }
                else {
                    passed_distance += distance;
                }
                if (distance != 0) {
                    dwg_.AddEdge(graph::Edge<double>{
                        TransportCatalogue::GetHubId(bus.stops[i]),
                            bus.stops[i]->stop_name, TransportCatalogue::GetPortalId(bus.stops[j]),
                            bus.stops[j]->stop_name, distance / (bus_velocity * 16.667), bus.bus_name });
                }
            }
        }
    }
}
*/
template<typename It>
void catalogue::TransportCatalogue::ParseBusRouteOnEdges(It begin, It end, int bus_velocity, const Bus* bus) {
    for (auto from = begin; from != end; ++from) {
        int distance = 0;
        for (auto to = std::next(from); to != end; ++to) {

            auto prev_to = std::prev(to);
            distance += TransportCatalogue::GetDistanceBetweenStops(*prev_to, *to);
            if (TransportCatalogue::GetDistanceBetweenStops(*prev_to, *to) == 0 && !bus->is_roundtrip) {
                distance += TransportCatalogue::GetDistanceBetweenStops(*to, *prev_to);
            }

            double weight = distance / (bus_velocity * 16.667);
            auto from_s = *from;
            auto to_s = *to;
            dwg_.AddEdge(graph::Edge<double>{
                hubs_.at(from_s), from_s->stop_name,
                    portals_.at(to_s), to_s->stop_name, weight, bus->bus_name });
        }
    }
}

void catalogue::TransportCatalogue::FillEdges(int bus_velocity) {
    for (const auto& bus : buses_) {
            ParseBusRouteOnEdges(bus.stops.begin(), bus.stops.end(), bus_velocity, &bus);
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
    for (auto& stop : bus->stops) {
        stop_and_buses_.at(stop).insert(bus);
    }
}

void TransportCatalogue::AddBus(const std::string& name, bool is_roundtrip, std::vector<std::string_view> route_stops, std::vector<std::string_view> route_ends) {
    auto bus = TransportCatalogue::FindBus(name);
    if (bus == nullptr) {
        buses_.emplace_back(std::move(Bus{ name, is_roundtrip, {}, {} }));
        buses_catalogue_[buses_.back().bus_name] = &buses_.back();
    }
    bus = TransportCatalogue::FindBus(name);
    AddStopsToBus(bus, std::move(route_stops));
    AddBusToStops(bus);
    for (const auto& end : route_ends) {
        bus->ends.push_back(FindStop(end));
    }
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

const domain::BusesPtr TransportCatalogue::GetBusesForStop(Stop* stop) {
    if (FindStop(stop->stop_name) != nullptr) {
        return stop_and_buses_.at(FindStop(stop->stop_name));
    }

    return {};
}

const std::unordered_map<std::string_view, Bus*>* TransportCatalogue::GetBuses() const {
    return &buses_catalogue_;
}

const std::unordered_map<std::string_view, Stop*>* TransportCatalogue::GetStops() const {
    return &stops_catalogue_;
}

int TransportCatalogue::GetRouteLength(std::vector<Stop*> bus_stops) const {
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

double TransportCatalogue::GetGeoDistance(std::vector<Stop*> bus_stops) const {
    double geo_distance = .0;
    for (size_t i = 0; i < bus_stops.size() - 1; ++i) {
        geo_distance += geo::ComputeDistance({ bus_stops[i]->coordinates.lat, bus_stops[i]->coordinates.lng },
            { bus_stops[i + 1]->coordinates.lat, bus_stops[i + 1]->coordinates.lng });
    }
    return geo_distance;
}

const RouteInformation TransportCatalogue::GetRouteInformation(Bus* bus){
    int all_stops = static_cast<int>(bus->stops.size());

    std::set<Stop*> set{ bus->stops.begin(), bus->stops.end() };
    int unique_stops = static_cast<int>(set.size());

    std::vector<Stop*> bus_stops = bus->stops;
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

int TransportCatalogue::GetDistanceBetweenStops(Stop* from, Stop* to) {
    if (!stop_and_distances_.count({ from, to })) {
        return 0;
    }
    return stop_and_distances_.at({ from, to });
}
