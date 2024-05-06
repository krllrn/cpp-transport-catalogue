#include <iostream>
#include <fstream>


#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

void TestWithLocalFiles() {
    catalogue::TransportCatalogue catalogue;

    std::string line;

    std::ifstream in(".\\test\\1in.txt");

    if (in.is_open())
    {
        std::getline(in, line);
        int base_request_count = std::stoi(line);
        {
            input_reader::InputReader reader;
            for (int i = 0; i < base_request_count; ++i) {
                getline(in, line);
                reader.ParseLine(line);
            }
            reader.ApplyCommands(catalogue);
        }
        std::ofstream out(".\\test\\my_out.txt");
        std::getline(in, line);
        int stat_request_count = std::stoi(line);
        for (int i = 0; i < stat_request_count; ++i) {
            getline(in, line);
            stat_reader::ParseAndPrintStat(catalogue, line, out);
        }
        out.close();
    }
    in.close();
}

int main() {
    TestWithLocalFiles();
    /*
    catalogue::TransportCatalogue catalogue;

    int base_request_count;
    cin >> base_request_count >> ws;

    {
        input_reader::InputReader reader;
        for (int i = 0; i < base_request_count; ++i) {
            string line;
            getline(cin, line);
            reader.ParseLine(line);
        }
        reader.ApplyCommands(catalogue);
    }

    int stat_request_count;
    cin >> stat_request_count >> ws;
    for (int i = 0; i < stat_request_count; ++i) {
        string line;
        getline(cin, line);
        stat_reader::ParseAndPrintStat(catalogue, line, cout);
    }*/
}