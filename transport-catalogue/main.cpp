#include <fstream>
#include <iostream>
#include <sstream>

#include "json_reader.h"

using namespace std;

int main() {
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
}