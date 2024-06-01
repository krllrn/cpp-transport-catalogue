#pragma once

#include <algorithm>
#include <cstdlib>
#include <unordered_map>

#include "domain.h"
#include "geo.h"
#include "svg.h"


/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 */

namespace projector {

    inline const double EPSILON = 1e-6;
    bool IsZero(double value);

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };
} //namespace projector

namespace renderer {

	struct ImageSettings {
		ImageSettings() = default;

		ImageSettings(double width_image, double height_image, double padding)
			: width_image_(width_image),
			height_image_(height_image),
			padding_(padding)
		{}

		double width_image_;
		double height_image_;
		double padding_;
	};

	struct BusSettings {
		BusSettings() = default;

		BusSettings(int bus_label_font_size, std::vector<double> bus_label_offset) 
			: bus_label_font_size_(bus_label_font_size),
			bus_label_offset_(bus_label_offset)
		{}

		int bus_label_font_size_;
		std::vector<double> bus_label_offset_;
	};

	struct StopSettings {
		StopSettings() = default;

		StopSettings(int stop_label_font_size, std::vector<double> stop_label_offset)
			: stop_label_font_size_(stop_label_font_size),
			stop_label_offset_(stop_label_offset)
		{}

		int stop_label_font_size_;
		std::vector<double> stop_label_offset_;
	};

	struct UnderlayerSettings {
		UnderlayerSettings() = default;

		UnderlayerSettings(double underlayer_width, svg::Color underlayer_color)
			: underlayer_width_(underlayer_width),
			underlayer_color_(underlayer_color)
		{}

		double underlayer_width_;
		svg::Color underlayer_color_;
	};

	class MapRenderer
	{
	public:
		MapRenderer() = default;

        MapRenderer(ImageSettings image_settings, double line_width, double stop_radius, BusSettings bus_settings, StopSettings stop_settings,
            UnderlayerSettings underlayer_settings, std::vector<svg::Color> color_palette);

        svg::Document GetRoutesMap(const std::unordered_map<std::string_view, domain::Bus*>* buses) const;

		~MapRenderer() = default;

	private:
        ImageSettings image_settings_;
		double line_width_;
		double stop_radius_;
        BusSettings bus_settings_;
        StopSettings stop_settings_;
        UnderlayerSettings underlayer_settings_;
        std::vector<svg::Color> color_palette_;

        projector::SphereProjector GetSphereProjector(domain::BusesPtr& sorted_buses) const;

        void CreateRouteLines(svg::Document& doc, domain::BusesPtr& sorted_buses, projector::SphereProjector& sphere_projector) const;

        void CreateRouteName(svg::Document& doc, projector::SphereProjector& sphere_projector, domain::BusesPtr& sorted_buses) const;

        void AddNamesToRoundtripRoute(int count, svg::Document& doc, projector::SphereProjector& sphere_projector, domain::Bus* bus) const;

        void AddNamesToNoneRoundtripRoute(int count, svg::Document& doc, projector::SphereProjector& sphere_projector, domain::Bus* bus) const;

        void CreateStopSymbols(svg::Document& doc, projector::SphereProjector& sphere_projector, std::set<domain::Stop*, domain::StopCmp>& sorted_stops) const;

        void CreateStopNames(svg::Document& doc, projector::SphereProjector& sphere_projector, std::set<domain::Stop*, domain::StopCmp>& sorted_stops) const;
	};


}