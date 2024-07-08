#pragma once

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
        TransportRouter(catalogue::TransportCatalogue* db, graph::Router<double>* router);

        std::optional<transport_router::Result> CreateRoute(domain::Stop* from, domain::Stop* to);

    private:
        catalogue::TransportCatalogue* db_;
        graph::Router<double>* router_;
    };
    
} // namespace transport_router