#pragma once

#include "json.h"
#include "request_handler.h"

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

        json::Array answers_;

        json::Document LoadJSON(std::istream& strm);

        void LoadBaseRequest(const json::Array& requests);

        void LoadStatRequest(const json::Array& requests);

        void LoadBus(json::Dict& node);

        void LoadStop(json::Dict& node);

        void GetBus(json::Dict& node);

        void GetStop(json::Dict& node);

        void GetMap(json::Dict& node);

        renderer::MapRenderer GetRenderer() const;
    };
}