#pragma once

#include <memory>

#include "json.h"
#include "request_handler.h"
#include "router.h"
#include "transport_router.h"

namespace json_reader {
    class JsonReader {
    public:
        JsonReader() = default;

        void LoadDictionary(std::istream& strm);

        void PrintSVG(std::ostream& out);

        std::string PrintJSON();

    private:
        catalogue::TransportCatalogue db_;

        json::Dict render_settings_;

        json::Dict routing_settings_;

        json::Array answers_;

        json::Document LoadJSON(std::istream& strm);

        void LoadBaseRequest(const json::Array& requests);

        void LoadStatRequest(const json::Array& requests, transport_router::TransportRouter* ts_router);

        void LoadBus(json::Dict& node);

        void LoadStop(json::Dict& node);

        void GetBus(json::Dict& node);

        void GetStop(json::Dict& node);

        void GetMap(json::Dict& node);

        void GetRoute(json::Dict& node, transport_router::TransportRouter* ts_router);

        renderer::MapRenderer GetRenderer() const;
    };
}