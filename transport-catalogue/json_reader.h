#pragma once

#include <memory>

#include "json.h"
#include "request_handler.h"
#include "router.h"

namespace json_reader {
    class JsonReader {
    public:
        JsonReader() = default;

        void LoadDictionary(std::istream& strm);

        void PrintSVG(std::ostream& out);

        std::string PrintJSON();

    private:
        catalogue::TransportCatalogue db_;

        std::unique_ptr<graph::Router<double>> router_;

        json::Dict render_settings_;

        json::Dict routing_settings_;

        json::Array answers_;

        json::Document LoadJSON(std::istream& strm);

        void LoadBaseRequest(const json::Array& requests);

        void LoadStatRequest(const json::Array& requests);

        void LoadBus(json::Dict& node);

        void LoadStop(json::Dict& node);

        void SetRouter();

        void GetBus(json::Dict& node);

        void GetStop(json::Dict& node);

        void GetMap(json::Dict& node);

        void GetRoute(json::Dict& node);

        renderer::MapRenderer GetRenderer() const;
    };
}