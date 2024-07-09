#pragma once

#include <memory>

#include "router.h"
#include "transport_catalogue.h"

namespace transport_router {

    enum class EVENT_STATE {
        WAIT,
        BUS
    };

    struct Event {
        EVENT_STATE state;
        std::string_view stop_name;
        std::string_view bus_name;
        int span_count;
        double time;
    };

    struct RoutingSettings {
        double bus_velocity;
        double bus_wait_time;
    };

    struct Result {
        double total_time = .0;
        std::vector<Event> events;
    };

    class TransportRouter {
    public:
        explicit TransportRouter(const catalogue::TransportCatalogue& db, RoutingSettings settings);

        std::optional<transport_router::Result> CreateRoute(domain::Stop* from, domain::Stop* to);

    private:
        const double METER_PER_MINUTE = 16.667;
        const catalogue::TransportCatalogue& db_;
        graph::DirectedWeightedGraph<double> dwg_;
        std::unique_ptr<graph::Router<double>> router_ptr_;
        RoutingSettings settings_;

        void FillEdges();

        template<typename It>
        void ParseBusRouteOnEdges(It begin, It end, const domain::Bus* bus);

        void AddStopsAsVertex();
    };
    
} // namespace transport_router