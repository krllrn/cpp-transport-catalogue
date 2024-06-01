#include <sstream>

#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace json_reader {

    namespace detail {
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

        json::Dict ParseBusAnswer(std::optional<domain::RouteInformation> route_info, int request_id) {
            using namespace std::string_literals;

            json::Dict dict;
            if (route_info) {
                dict.insert({ "curvature"s, json::Node(route_info.value().curvature) });
                dict.insert({ "request_id"s, json::Node(request_id) });
                dict.insert({ "route_length"s, json::Node(route_info.value().route_length) });
                dict.insert({ "stop_count"s, json::Node(route_info.value().all_stops) });
                dict.insert({ "unique_stop_count"s, json::Node(route_info.value().unique_stops) });
            }
            else {
                dict.insert({ "request_id"s, json::Node{request_id} });
                dict.insert({ "error_message"s, json::Node{"not found"s} });
            }
            return dict;
        }

        json::Dict ParseStopAnswer(Stop* stop, [[maybe_unused]]BusesPtr buses, int request_id) {
            using namespace std::string_literals;

            json::Dict dict;
            if (stop) {
                json::Array buses_at_stop;
                for (const auto& bus : buses) {
                    buses_at_stop.push_back(json::Node(bus->bus_name));
                }
                dict.insert({ "buses"s, buses_at_stop });
                dict.insert({ "request_id"s, json::Node(request_id) });
            }
            else {
                dict.insert({ "request_id"s, json::Node{request_id} });
                dict.insert({ "error_message"s, json::Node{"not found"s} });
            }

            return dict;
        }

        json::Dict ParseMapAnswer(std::string map_to_string, int request_id) {
            using namespace std::string_literals;
            json::Dict dict;
            if (!map_to_string.empty()) {
                dict.insert({ "map"s, json::Node{map_to_string} });
                dict.insert({ "request_id"s, json::Node{request_id} });
            }
            else {
                dict.insert({ "request_id"s, json::Node{request_id} });
                dict.insert({ "error_message"s, json::Node{"map is empty"} });
            }
            return dict;
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
    } // namespace detail

    void JsonReader::LoadDictionary(std::istream& strm) {
        json::Dict main_dict = LoadJSON(strm).GetRoot().AsMap();

        json::Array base_requests = main_dict.at("base_requests").AsArray();
        if (main_dict.count("render_settings")) {
            render_settings_ = main_dict.at("render_settings").AsMap();
        }
        json::Array stat_requests = main_dict.at("stat_requests").AsArray();

        LoadBaseRequest(base_requests);
        LoadStatRequest(stat_requests);
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

    void JsonReader::LoadStatRequest(const json::Array& requests) {
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