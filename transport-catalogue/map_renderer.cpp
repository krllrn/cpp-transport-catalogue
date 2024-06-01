#include <set>

#include "map_renderer.h"


namespace projector {
    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }
}
namespace renderer {

    namespace detail {
        domain::BusesPtr GetSortedBuses(const std::unordered_map<std::string_view, domain::Bus*>* buses) {
            domain::BusesPtr sorted_buses;
            for (auto it = buses->begin(); it != buses->end(); ++it) {
                sorted_buses.insert(&*it->second);
            }
            return sorted_buses;
        }

        std::set<domain::Stop*, domain::StopCmp> GetSortedStops(const domain::BusesPtr& sorted_buses) {
            std::set<domain::Stop*, domain::StopCmp> sorted_stops;
            for (const auto& bus : sorted_buses) {
                if (!bus->stops.empty()) {
                    for (const auto& stop : bus->stops) {
                        sorted_stops.insert(stop);
                    }
                }
            }
            return sorted_stops;
        }
    }

    MapRenderer::MapRenderer(ImageSettings image_settings, double line_width, double stop_radius, BusSettings bus_settings, StopSettings stop_settings,
        UnderlayerSettings underlayer_settings, std::vector<svg::Color> color_palette)
        :image_settings_(std::move(image_settings)),
        line_width_(line_width),
        stop_radius_(stop_radius),
        bus_settings_(std::move(bus_settings)),
        stop_settings_(std::move(stop_settings)),
        underlayer_settings_(std::move(underlayer_settings)),
        color_palette_(std::move(color_palette))
    {}

    projector::SphereProjector MapRenderer::GetSphereProjector(domain::BusesPtr& sorted_buses) const {
        std::vector<geo::Coordinates> geo_coords;
        for (const auto& bus : sorted_buses) {
            for (const auto& stop : bus->stops) {
                geo_coords.push_back(stop->coordinates);
            }
        }
        projector::SphereProjector sphere_projector{ geo_coords.begin(), geo_coords.end(), image_settings_.width_image_, image_settings_.height_image_,
            image_settings_.padding_ };

        return sphere_projector;
    }

    void MapRenderer::CreateRouteLines(svg::Document& doc, domain::BusesPtr& sorted_buses, projector::SphereProjector& sphere_projector) const {
        int count = 0;
        for (const auto& bus : sorted_buses) {
            svg::Polyline route;
            route.SetStrokeWidth(line_width_)
                .SetFillColor(svg::NoneColor)
                .SetStrokeWidth(line_width_)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            count %= color_palette_.size();
            if (bus->stops.empty()) {
                continue;
            }
            route.SetStrokeColor(color_palette_.at(count));
            for (const auto& stop : bus->stops) {
                route.AddPoint(sphere_projector(stop->coordinates));
            }
            doc.Add(route);
            ++count;
        }
    }

    void MapRenderer::AddNamesToNoneRoundtripRoute(int count, svg::Document& doc, projector::SphereProjector& sphere_projector, domain::Bus* bus) const {
        count %= color_palette_.size();

        svg::Text stop_name;
        stop_name.SetOffset(svg::Point{ bus_settings_.bus_label_offset_[0], bus_settings_.bus_label_offset_[1] })
            .SetFontSize(bus_settings_.bus_label_font_size_)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetFillColor(color_palette_.at(count));
        svg::Text stop_name_stroke;
        stop_name_stroke.SetOffset(svg::Point{ bus_settings_.bus_label_offset_[0], bus_settings_.bus_label_offset_[1] })
            .SetFontSize(bus_settings_.bus_label_font_size_)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetFillColor(underlayer_settings_.underlayer_color_)
            .SetStrokeColor(underlayer_settings_.underlayer_color_)
            .SetStrokeWidth(underlayer_settings_.underlayer_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        if (!bus->ends.empty()) {
            for (size_t i = 0; i < bus->ends.size(); ++i) {
                doc.Add(stop_name_stroke.SetPosition(sphere_projector(bus->ends[i]->coordinates)).SetData(bus->bus_name));
                doc.Add(stop_name.SetPosition(sphere_projector(bus->ends[i]->coordinates)).SetData(bus->bus_name));
            }
        }
    }

    void MapRenderer::AddNamesToRoundtripRoute(int count, svg::Document& doc, projector::SphereProjector& sphere_projector, domain::Bus* bus) const {
        count %= color_palette_.size();

        svg::Text stop_name;
        stop_name.SetOffset(svg::Point{ bus_settings_.bus_label_offset_[0], bus_settings_.bus_label_offset_[1] })
            .SetFontSize(bus_settings_.bus_label_font_size_)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetFillColor(color_palette_.at(count));
        svg::Text stop_name_stroke;
        stop_name_stroke.SetOffset(svg::Point{ bus_settings_.bus_label_offset_[0], bus_settings_.bus_label_offset_[1] })
            .SetFontSize(bus_settings_.bus_label_font_size_)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetFillColor(underlayer_settings_.underlayer_color_)
            .SetStrokeColor(underlayer_settings_.underlayer_color_)
            .SetStrokeWidth(underlayer_settings_.underlayer_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        if (!bus->stops.empty()) {
            for (size_t i = 0; i < bus->ends.size(); ++i) {
                doc.Add(stop_name_stroke.SetPosition(sphere_projector(bus->ends[i]->coordinates)).SetData(bus->bus_name));
                doc.Add(stop_name.SetPosition(sphere_projector(bus->ends[i]->coordinates)).SetData(bus->bus_name));
            }
        }
    }

    void MapRenderer::CreateRouteName(svg::Document& doc, projector::SphereProjector& sphere_projector, domain::BusesPtr& sorted_buses) const {
        int count = 0;
        for (const auto& bus : sorted_buses) {
            if (bus->is_roundtrip) {
                AddNamesToRoundtripRoute(count, doc, sphere_projector, bus);
            }
            else if (!bus->is_roundtrip) {
                AddNamesToNoneRoundtripRoute(count, doc, sphere_projector, bus);
            }
            ++count;
        }
    }

    void MapRenderer::CreateStopSymbols(svg::Document& doc, projector::SphereProjector& sphere_projector, std::set<domain::Stop*, domain::StopCmp>& sorted_stops) const {
        svg::Circle stop_symbol;
        for (const auto& stop : sorted_stops) {
            stop_symbol.SetCenter(sphere_projector(stop->coordinates))
                .SetRadius(stop_radius_)
                .SetFillColor("white");
            doc.Add(stop_symbol);
        }
    }

    void MapRenderer::CreateStopNames(svg::Document& doc, projector::SphereProjector& sphere_projector, std::set<domain::Stop*, domain::StopCmp>& sorted_stops) const {
        svg::Text stop_name;
        svg::Text stop_name_stroke;
        for (const auto& stop : sorted_stops) {
            stop_name_stroke.SetPosition(sphere_projector(stop->coordinates))
                .SetOffset(svg::Point{ stop_settings_.stop_label_offset_[0], stop_settings_.stop_label_offset_[1] })
                .SetFontSize(stop_settings_.stop_label_font_size_)
                .SetFontFamily("Verdana")
                .SetFillColor(underlayer_settings_.underlayer_color_)
                .SetStrokeColor(underlayer_settings_.underlayer_color_)
                .SetStrokeWidth(underlayer_settings_.underlayer_width_)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetData(stop->stop_name);
            doc.Add(stop_name_stroke);

            stop_name.SetPosition(sphere_projector(stop->coordinates))
                .SetOffset(svg::Point{ stop_settings_.stop_label_offset_[0], stop_settings_.stop_label_offset_[1] })
                .SetFontSize(stop_settings_.stop_label_font_size_)
                .SetFillColor("black")
                .SetFontFamily("Verdana")
                .SetData(stop->stop_name);
            doc.Add(stop_name);
        }
    }

    svg::Document MapRenderer::GetRoutesMap(const std::unordered_map<std::string_view, domain::Bus*>* buses) const {
        svg::Document doc;
        auto sorted_buses = detail::GetSortedBuses(buses);
        auto sphere_projector = GetSphereProjector(sorted_buses);
        //create lines
        CreateRouteLines(doc, sorted_buses, sphere_projector);

        // create route names
        CreateRouteName(doc, sphere_projector, sorted_buses);

        // stop symbols
        std::set<domain::Stop*, domain::StopCmp> sorted_stops = detail::GetSortedStops(sorted_buses);
        CreateStopSymbols(doc, sphere_projector, sorted_stops);

        // stop names
        CreateStopNames(doc, sphere_projector, sorted_stops);

        return doc;
    }
}