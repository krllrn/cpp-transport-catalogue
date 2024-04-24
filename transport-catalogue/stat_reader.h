#pragma once

#include <iosfwd>

#include "transport_catalogue.h"

namespace stat_reader {
    void ParseAndPrintStat(const catalogue::TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output);
}