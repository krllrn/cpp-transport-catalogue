#include "domain.h"

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области
 * (domain) вашего приложения и не зависят от транспортного справочника. Например Автобусные
 * маршруты и Остановки.
 *
 */

namespace domain {

    bool StopCmp::operator()(const Stop* lhs, const Stop* rhs) const {
        return std::lexicographical_compare(lhs->stop_name.begin(), lhs->stop_name.end(), rhs->stop_name.begin(), rhs->stop_name.end());
    }

    bool Stop::operator==(const Stop& other) {
        return (stop_name == other.stop_name && coordinates.lat == other.coordinates.lat && coordinates.lng == other.coordinates.lng);
    }

    bool RoutePoints::EqualTo::operator()(const RoutePoints& lhs, const RoutePoints& rhs) const {
        return (lhs.first == rhs.first && lhs.second == rhs.second);
    }

    size_t RoutePoints::Hasher::operator()(const RoutePoints& stops_point) const {
        size_t hash_s = std::hash<std::string>{}(stops_point.first->stop_name) * 37
            + std::hash<std::string>{}(stops_point.second->stop_name) * 37 * 37 * 37;
        return hash_s;
    }

    bool BusCmp::operator()(const Bus* lhs, const Bus* rhs) const {
        return std::lexicographical_compare(lhs->bus_name.begin(), lhs->bus_name.end(), rhs->bus_name.begin(), rhs->bus_name.end());
    }
}