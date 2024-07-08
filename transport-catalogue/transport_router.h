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

    struct Result {
        double total_time = .0;
        std::vector<Event> events;
    };

    class TransportRouter {
    public:
        explicit TransportRouter(const catalogue::TransportCatalogue& db, int bus_velocity, int bus_wait_time);

        std::optional<transport_router::Result> CreateRoute(const graph::Router<double>& router, domain::Stop* from, domain::Stop* to);

        graph::Router<double> GetRouter();

    private:
        const catalogue::TransportCatalogue& db_;
        graph::DirectedWeightedGraph<double> dwg_;
        double bus_velocity_;
        double bus_wait_time_;

        void FillEdges();

        template<typename It>
        void ParseBusRouteOnEdges(It begin, It end, const domain::Bus* bus);

        void AddStopsAsVertex();
    };
    
} // namespace transport_router