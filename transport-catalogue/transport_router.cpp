#include "transport_router.h"

namespace transport_router {
    TransportRouter::TransportRouter(const catalogue::TransportCatalogue& db, int bus_velocity, int bus_wait_time)
        :db_(db),
        bus_velocity_(bus_velocity * 16.667),
        bus_wait_time_(bus_wait_time * 1.0)
    {
        AddStopsAsVertex();
        FillEdges();
    }

    std::optional<transport_router::Result> TransportRouter::CreateRoute(const graph::Router<double>& router, domain::Stop* from, domain::Stop* to)
    {
        std::optional<graph::Router<double>::RouteInfo> route = router.BuildRoute(db_.GetPortalId(from), db_.GetPortalId(to));

        if (!route) {
            return std::nullopt;
        }

        transport_router::Result result;

        if (route.has_value()) {
            result.total_time = route.value().weight;
            int spans = route.value().edges.size();
            for (const auto& edge_id : route.value().edges) {
                graph::Edge<double> edge = dwg_.GetEdge(edge_id);
                if (edge.from % 2 == 0) {
                    Event wait;
                    wait.state = transport_router::EVENT_STATE::WAIT;
                    wait.stop_name = edge.from_name;
                    wait.time = edge.weight;

                    result.events.push_back(std::move(wait));
                    --spans;
                }
                else {
                    Event bus;
                    bus.state = transport_router::EVENT_STATE::BUS;
                    bus.time = edge.weight;
                    bus.bus_name = edge.bus_name;
                    bus.span_count = spans;

                    result.events.push_back(std::move(bus));
                    --spans;
                }
            }
        }

        return result;
    }

    graph::Router<double> TransportRouter::GetRouter()
    {
        graph::Router<double> r(dwg_);
        return r;
    }

    template<typename It>
    void TransportRouter::ParseBusRouteOnEdges(It begin, It end, const domain::Bus* bus) {
        for (auto from = begin; from != end; ++from) {
            int distance = 0;
            for (auto to = std::next(from); to != end; ++to) {

                auto prev_to = std::prev(to);
                distance += db_.GetDistanceBetweenStops(*prev_to, *to);
                if (db_.GetDistanceBetweenStops(*prev_to, *to) == 0 && !bus->is_roundtrip) {
                    distance += db_.GetDistanceBetweenStops(*to, *prev_to);
                }

                double weight = distance / bus_velocity_;
                auto from_s = *from;
                auto to_s = *to;
                dwg_.AddEdge(graph::Edge<double>{
                    db_.GetHubId(from_s), from_s->stop_name,
                        db_.GetPortalId(to_s), to_s->stop_name, weight, bus->bus_name });
            }
        }
    }

    void TransportRouter::AddStopsAsVertex() {
        dwg_.ResizeIncidenceList(db_.GetStops()->size() * 2);
        for (const auto& [stop_name, stop] : *db_.GetStops()) {
            dwg_.AddEdge(graph::Edge<double>{
                db_.GetPortalId(stop), stop->stop_name, db_.GetHubId(stop), stop->stop_name, bus_wait_time_, {}});
        }
    }

    void TransportRouter::FillEdges() {
        for (const auto& [bus_name, bus] : *db_.GetBuses()) {
            ParseBusRouteOnEdges(bus->stops.begin(), bus->stops.end(), bus);
        }
        
    }
} // namespace transport_router