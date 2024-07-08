#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>

#include "json_reader.h"

int main() {
    /*
    json_reader::JsonReader reader;
    std::string str;
    std::string tmp;
    while (std::getline(std::cin, tmp))
    {
        if (std::cin.fail()) {
            break;
        }
        str += tmp;
    }

    std::istringstream strm(str);

    reader.LoadDictionary(strm);

    std::cout << reader.PrintJSON();
    */

    // from/to file
    json_reader::JsonReader reader;
    std::ifstream inputFile(".\\tests\\12\\e4_input.json");
    if (!inputFile.is_open()) {
        std::cerr << "Error opening the file!" << std::endl;
        return 1;
    }

    std::string str;
    if (inputFile) {
        std::ostringstream ss;
        ss << inputFile.rdbuf(); // reading data
        str = ss.str();
    }
    std::istringstream strm(str);
    std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
    reader.LoadDictionary(strm);

    std::ofstream out(".\\tests\\12\\my_out4.json");
    out << reader.PrintJSON();
    std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
    std::chrono::milliseconds diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Execution time: " << diff.count() << " ms" << '\n';

    out.close();
}