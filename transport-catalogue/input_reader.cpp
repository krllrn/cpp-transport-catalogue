#include "input_reader.h"

#include <iterator>

using namespace input_reader;

/**
    * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
    */
geo::Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return { nan, nan };
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return { lat, lng };
}

/**
    * Удаляет пробелы в начале и конце строки
    */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
    * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
    */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
    * Парсит маршрут.
    * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
    * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
    */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return { std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1)) };
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

/**
    * Парсит строку вида "10.123,  -30.1837, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka"
    и передает в TransportCatalogue для добавления дистанции
    */
void ParseAndAddDistance(catalogue::TransportCatalogue& catalogue, const std::string_view& from, std::string_view str) {
    std::vector<std::string_view> split_with_comma = Split(str, ',');
    if (split_with_comma.size() > 2) {
        split_with_comma.erase(split_with_comma.begin(), split_with_comma.begin() + 2);
        for (const auto& s : split_with_comma) {
            auto to = s.substr(s.find_first_of('m') + 5, s.size());
            int distance = std::stoi(std::string(s.substr(0, s.find_first_of('m'))));
            catalogue.AddDistanceBetweenStops(from, to, distance);
        }
    }
}

void InputReader::ApplyCommands([[maybe_unused]] catalogue::TransportCatalogue& catalogue) const {
    for (const CommandDescription& command_description : commands_) {
        if (command_description.command == "Stop") {
            geo::Coordinates stop_coordinates = ParseCoordinates(command_description.description);
            auto stop_name = Trim(command_description.id);
            catalogue.AddStop(std::string(stop_name), stop_coordinates);
            ParseAndAddDistance(catalogue, stop_name, command_description.description);
        }
        else if (command_description.command == "Bus") {
            std::vector<std::string_view> route_stops = ParseRoute(command_description.description);
            auto bus_name = Trim(command_description.id);
            catalogue.AddBus(std::string(bus_name), route_stops);
        }
    }
}

