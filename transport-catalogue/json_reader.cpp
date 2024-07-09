#include <sstream>

#include "json_reader.h"
#include "json_builder.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace json_reader {

    namespace detail {
        using namespace std::string_literals;

        /**
        * Парсит маршрут.
        * Для кольцевого маршрута ("is_roundtrip": true) возвращает массив названий остановок [A,B,C,A] - ">"
        * Для некольцевого маршрута ("is_roundtrip": false) возвращает массив названий остановок [A,B,C,D,C,B,A] - "-"
        */
        std::vector<std::string_view> ParseRoute(bool is_roundtrip, std::vector<std::string_view> stops) {
            if (is_roundtrip) {
                //stops.push_back(*stops.begin());
                return stops;
            }

            std::vector<std::string_view> results(stops.begin(), stops.end());
            results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

            return results;
        }

        json::Dict ParseNotFoundAnswer(int request_id) {

            return json::Builder{}.StartDict()
                .Key("request_id"s).Value(json::Node{ request_id })
                .Key("error_message"s).Value(json::Node{ "not found"s })
                .EndDict()
                .Build().AsMap();
        }

        json::Dict ParseBusAnswer(std::optional<domain::RouteInformation> route_info, int request_id) {
            if (!route_info) {
                return ParseNotFoundAnswer(request_id);
            }

            return json::Builder{}.StartDict()
                        .Key("curvature"s).Value(json::Node(route_info.value().curvature))
                        .Key("request_id"s).Value(json::Node(request_id))
                        .Key("route_length"s).Value(json::Node(route_info.value().route_length))
                        .Key("stop_count"s).Value(json::Node(route_info.value().all_stops))
                        .Key("unique_stop_count"s).Value(json::Node(route_info.value().unique_stops))
                    .EndDict()
                .Build().AsMap();
        }

        json::Dict ParseStopAnswer(Stop* stop, [[maybe_unused]]BusesPtr buses, int request_id) {
            
            if (!stop) {
                return ParseNotFoundAnswer(request_id);
            }

            json::Array buses_at_stop;
            for (const auto& bus : buses) {
                buses_at_stop.push_back(json::Node(bus->bus_name));
            }

            return json::Builder{}.StartDict()
                        .Key("buses"s).Value(buses_at_stop)
                        .Key("request_id"s).Value(json::Node(request_id))
                    .EndDict()
                .Build().AsMap();
        }

        json::Dict ParseMapAnswer(std::string map_to_string, int request_id) {
            if (map_to_string.empty()) {
                return ParseNotFoundAnswer(request_id);
            }

            return json::Builder{}.StartDict()
                .Key("map"s).Value(json::Node{ map_to_string })
                .Key("request_id"s).Value(json::Node(request_id))
                .EndDict()
                .Build().AsMap();
        }

        svg::Color ParseColor(const json::Node& to_parse) {
            if (to_parse.IsString()) {
                return svg::Color{ to_parse.AsString() };
            }
            else if (to_parse.IsArray()) {
                if (to_parse.AsArray().size() == 3) {
                    return svg::Rgb(to_parse.AsArray()[0].AsInt(), 
                        to_parse.AsArray()[1].AsInt(), 
                        to_parse.AsArray()[2].AsInt());
                }
                else if (to_parse.AsArray().size() == 4) {
                    return svg::Rgba(to_parse.AsArray()[0].AsInt(), 
                        to_parse.AsArray()[1].AsInt(), 
                        to_parse.AsArray()[2].AsInt(), 
                        to_parse.AsArray()[3].AsDouble());
                }
            }
            return svg::NoneColor;
        }

        json::Dict ParseRouteAnswer(int request_id, transport_router::Result route) {
            std::vector<json::Node> items;
            for (const auto& event : route.events) {
                if (event.state == transport_router::EVENT_STATE::WAIT) {
                    json::Node wait = json::Builder{}.StartDict()
                        .Key("type").Value(std::string("Wait"))
                        .Key("stop_name").Value(json::Node{ std::string(event.stop_name) })
                        .Key("time").Value(event.time)
                        .EndDict().Build();

                    items.push_back(wait);
                }
                else if (event.state == transport_router::EVENT_STATE::BUS) {
                    json::Node bus = json::Builder{}.StartDict()
                        .Key("type").Value(std::string("Bus"))
                        .Key("bus").Value(json::Node{ std::string(event.bus_name) })
                        .Key("span_count").Value(event.span_count)
                        .Key("time").Value(event.time)
                        .EndDict().Build();

                    items.push_back(bus);
                }
            }

            return json::Builder{}.StartDict().Key("request_id").Value(request_id)
                .Key("total_time").Value(route.total_time)
                .Key("items").Value(items)
                .EndDict().Build().AsMap();
        }
    } // namespace detail  

    void JsonReader::LoadDictionary(std::istream& strm) {
        json::Dict main_dict = LoadJSON(strm).GetRoot().AsMap();
        
        if (main_dict.count("render_settings")) {
            render_settings_ = main_dict.at("render_settings").AsMap();
        }
        if (main_dict.count("routing_settings")) {
            routing_settings_ = main_dict.at("routing_settings").AsMap();
        }

        json::Array base_requests = main_dict.at("base_requests").AsArray();
        json::Array stat_requests = main_dict.at("stat_requests").AsArray();

        LoadBaseRequest(base_requests);
        transport_router::TransportRouter ts_router(db_, transport_router::RoutingSettings{ routing_settings_.at("bus_velocity").AsDouble(),
            routing_settings_.at("bus_wait_time").AsDouble() });
        LoadStatRequest(stat_requests, &ts_router);
    }

    void JsonReader::LoadBus(json::Dict& node) {
        std::vector<std::string_view> stops;
        json::Array stops_from_array = node.at("stops").AsArray();
        for (const auto& s : stops_from_array) {
            stops.push_back(s.AsString());
        }
        std::vector<std::string_view> ends;
        ends.push_back(stops[0]);
        if (stops[0] != stops[stops.size() - 1]) {
            ends.push_back(stops[stops.size() - 1]);
        }
        db_.AddBus(node.at("name").AsString(), node.at("is_roundtrip").AsBool(), detail::ParseRoute(node.at("is_roundtrip").AsBool(), stops), ends);
    }

    void JsonReader::LoadStop(json::Dict& node) {
        geo::Coordinates stop_coordinates{ node.at("latitude").AsDouble(), node.at("longitude").AsDouble() };
        db_.AddStop(node.at("name").AsString(), stop_coordinates);
        if (node.at("road_distances").IsMap()) {
            json::Dict road_distances = node.at("road_distances").AsMap();
            if (!road_distances.empty()) {
                for (const auto& [stop_name, distance] : road_distances) {
                    if (!db_.FindStop(stop_name)) {
                        db_.AddStop(stop_name, geo::Coordinates{});
                    }
                    db_.AddDistanceBetweenStops(db_.FindStop(node.at("name").AsString()), db_.FindStop(stop_name), distance.AsInt());
                }
            }
        }
    }

    void JsonReader::LoadBaseRequest(const json::Array& requests) {
        for (const auto& request : requests) {
            json::Dict node = request.AsMap();
            if (node.at("type").AsString() == "Bus") {
                LoadBus(node);
            }
            else if (node.at("type").AsString() == "Stop") {
                LoadStop(node);
            }
        }
    }

    renderer::MapRenderer JsonReader::GetRenderer() const {
        if (render_settings_.empty()) {
            return {};
        }

        renderer::ImageSettings image_settings(render_settings_.at("width").AsDouble(), render_settings_.at("height").AsDouble(), render_settings_.at("padding").AsDouble());

        double line_width = render_settings_.at("line_width").AsDouble();
        double stop_radius = render_settings_.at("stop_radius").AsDouble();

        std::vector<double> bus_label_offset;
        for (const auto& value : render_settings_.at("bus_label_offset").AsArray()) {
            bus_label_offset.emplace_back(value.AsDouble());
        }
        renderer::BusSettings bus_settings(render_settings_.at("bus_label_font_size").AsInt(), std::move(bus_label_offset));

        std::vector<double> stop_label_offset;
        for (const auto& value : render_settings_.at("stop_label_offset").AsArray()) {
            stop_label_offset.emplace_back(value.AsDouble());
        }
        renderer::StopSettings stop_settings(render_settings_.at("stop_label_font_size").AsInt(), stop_label_offset);

        svg::Color color = detail::ParseColor(render_settings_.at("underlayer_color"));
        renderer::UnderlayerSettings underlayer_settings(render_settings_.at("underlayer_width").AsDouble(), std::move(color));

        std::vector<svg::Color> color_palette;
        for (const auto& color : render_settings_.at("color_palette").AsArray()) {
            color_palette.emplace_back(detail::ParseColor(color));
        }

        return renderer::MapRenderer(image_settings, line_width, stop_radius, bus_settings, stop_settings, underlayer_settings, color_palette);
    }

    void JsonReader::GetBus(json::Dict& node) {
        if (!db_.FindBus(node.at("name").AsString())) {
            answers_.emplace_back(detail::ParseBusAnswer(std::nullopt, node.at("id").AsInt()));
        }
        else {
            std::optional<domain::RouteInformation> route_info = db_.GetRouteInformation(db_.FindBus(node.at("name").AsString()));
            answers_.emplace_back(detail::ParseBusAnswer(route_info, node.at("id").AsInt()));
        }
    }

    void JsonReader::GetStop(json::Dict& node) {
        Stop* stop = db_.FindStop(node.at("name").AsString());
        if (!stop) {
            answers_.emplace_back(detail::ParseStopAnswer(stop, {}, node.at("id").AsInt()));
        }
        else {
            BusesPtr buses = db_.GetBusesForStop(stop);
            answers_.emplace_back(detail::ParseStopAnswer(stop, buses, node.at("id").AsInt()));
        }
    }

    void JsonReader::GetMap(json::Dict& node) {
        std::ostringstream str_strm;
        PrintSVG(str_strm);

        answers_.emplace_back(detail::ParseMapAnswer(str_strm.str(), node.at("id").AsInt()));
    }

    void JsonReader::GetRoute(json::Dict& node, transport_router::TransportRouter* ts_router) {
        Stop* from = db_.FindStop(node.at("from").AsString());
        Stop* to = db_.FindStop(node.at("to").AsString());

        std::optional<transport_router::Result> route = ts_router->CreateRoute(from, to);
        if (!route.has_value()) {
            answers_.emplace_back(detail::ParseNotFoundAnswer(node.at("id").AsInt()));
        }
        else {
            answers_.emplace_back(detail::ParseRouteAnswer(node.at("id").AsInt(), route.value()));
        }
    }

    void JsonReader::LoadStatRequest(const json::Array& requests, transport_router::TransportRouter* ts_router) {
        for (const auto& request : requests) {
            json::Dict node = request.AsMap();
            if (node.at("type").AsString() == "Bus") {
                GetBus(node);
            }
            else if (node.at("type").AsString() == "Stop") {
                GetStop(node);
            }
            else if (node.at("type").AsString() == "Map") {
                GetMap(node);
            }
            else if (node.at("type").AsString() == "Route") {
                GetRoute(node, ts_router);
            }
        }
    }

    json::Document JsonReader::LoadJSON(std::istream& strm) {
        return json::Load(strm);
    }

    std::string JsonReader::PrintJSON() {
        std::ostringstream out;
        
        json::Print(json::Document{ answers_ }, out);
        
        return out.str();
    }

    void JsonReader::PrintSVG(std::ostream& out) {
        renderer::MapRenderer renderer = GetRenderer();
        RequestHandler handler(db_, renderer);
        svg::Document doc = handler.RenderMap();
        doc.Render(out);
    }

} // namespace json_reader