#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

 // MapRenderer понадобится в следующей части итогового проекта
RequestHandler::RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer)
    : db_(db),
    renderer_(renderer)
{
}

// Возвращает информацию о маршруте (запрос Bus)
std::optional<RouteInformation> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    auto bus = const_cast<catalogue::TransportCatalogue&>(db_).FindBus(bus_name);
    if (!bus) {
        return {};
    }
    return const_cast<catalogue::TransportCatalogue&>(db_).GetRouteInformation(bus);
}

// Возвращает маршруты, проходящие через
const BusesPtr RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    auto stop = const_cast<catalogue::TransportCatalogue&>(db_).FindStop(stop_name);
    if (!stop) {
        return {};
    }
    return const_cast<catalogue::TransportCatalogue&>(db_).GetBusesForStop(stop);
}

// Этот метод будет нужен в следующей части итогового проекта
svg::Document RequestHandler::RenderMap() const {
    return renderer_.GetRoutesMap(db_.GetBuses());
}