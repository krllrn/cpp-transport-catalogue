#include "transport_router.h"

namespace transport_router {
    TransportRouter::TransportRouter(catalogue::TransportCatalogue* db, graph::Router<double>* router)
        :db_(db),
        router_(router)
    {
    }

    std::optional<transport_router::Result> TransportRouter::CreateRoute(domain::Stop* from, domain::Stop* to)
    {

        std::optional<graph::Router<double>::RouteInfo> route = router_->BuildRoute(db_->GetPortalId(from), db_->GetPortalId(to));

        if (!route) {
            return std::nullopt;
        }

        transport_router::Result result;

        if (route.has_value()) {
            result.total_time = route.value().weight;
            int spans = route.value().edges.size();
            for (const auto& edge_id : route.value().edges) {
                graph::Edge<double> edge = db_->GetDwg()->GetEdge(edge_id);
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
} // namespace transport_router