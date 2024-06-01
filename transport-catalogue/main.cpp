#include <fstream>
#include <iostream>
#include <sstream>

#include "json_reader.h"

namespace {
    /*
    Тест № 1 не прошёл проверку
причина: Неправильный ответ
подсказка: Некорректно обрабатывается некольцевой маршрут из двух остановок
*/
    std::string NoneRoundtripTwoStops() {
        return R"({
            "base_requests": [
        {
            "type": "Bus",
                "name" : "114",
                "stops" : ["Ocean staition", "Rivera bridge"] ,
                "is_roundtrip" : false
        },
        {
          "type": "Stop",
          "name": "Rivera bridge",
          "latitude": 43.587795,
          "longitude": 39.716901,
          "road_distances": {"Ocean staition": 850}
        },
        {
          "type": "Stop",
          "name": "Ocean staition",
          "latitude": 43.581969,
          "longitude": 39.719848,
          "road_distances": {"Rivera bridge": 850}
        }
            ],
            "stat_requests": [
        { "id": 1, "type" : "Bus", "name" : "114" }
            ]
        } )";
    }

    /*
Тест № 2 не прошёл проверку
причина: Неправильный ответ
подсказка: Некорректно обрабатывается кольцевой маршрут из трёх остановок
*/
    std::string RoundtripThreeStops() {
        return R"({
            "base_requests": [
        {
            "type": "Bus",
                "name" : "114",
                "stops" : ["Ocean staition", "Rivera bridge", "Myachino", "Ocean staition"] ,
                "is_roundtrip" : true
        },
        {
          "type": "Stop",
          "name": "Rivera bridge",
          "latitude": 43.587795,
          "longitude": 39.716901,
          "road_distances": {"Ocean staition": 850}
        },
        {
          "type": "Stop",
          "name": "Ocean staition",
          "latitude": 43.581969,
          "longitude": 39.719848,
          "road_distances": {"Myachino": 850}
        },
        {
          "type": "Stop",
          "name": "Myachino",
          "latitude": 43.581969,
          "longitude": 39.719848,
          "road_distances": {"Rivera bridge": 1700}
        }
            ],
            "stat_requests": [
        { "id": 1, "type" : "Bus", "name" : "114" }
            ]
        } )";
    }

    std::string StopNotInRoute() {
        return R"({
            "base_requests": [
        {
            "type": "Bus",
                "name" : "114",
                "stops" : ["Ocean staition", "Rivera bridge", "Ocean staition"] ,
                "is_roundtrip" : true
        },
        {
          "type": "Stop",
          "name": "Rivera bridge",
          "latitude": 43.587795,
          "longitude": 39.716901,
          "road_distances": {"Ocean staition": 850}
        },
        {
          "type": "Stop",
          "name": "Ocean staition",
          "latitude": 43.581969,
          "longitude": 39.719848,
          "road_distances": {"Myachino": 850}
        },
        {
          "type": "Stop",
          "name": "Myachino",
          "latitude": 43.581969,
          "longitude": 39.719848,
          "road_distances": {"Rivera bridge": 1700}
        }
            ],
            "stat_requests": [
        { "id": 1, "type": "Stop", "name": "Myachino" }
            ]
        } )";
    }

    /*Test from pachka 1*/
    std::string Pachka1() {
        return R"({
    "base_requests": 
        [{"type": "Stop", "name": "A", "latitude": 55.611087, "longitude": 37.20829, "road_distances": {"B": 1000}}, 
        {"type": "Stop", "name": "B", "latitude": 55.595884, "longitude": 37.209755, "road_distances": {}}, 
        {"type": "Bus", "name": "B1", "stops": ["A", "B"], "is_roundtrip": false}], 
    "stat_requests": [{"id": 585791080, "type": "Bus", "name": "B1"}]})";
    }
    /*
    Тест № 8 не прошёл проверку
        причина : Неправильный ответ
        подсказка : Неверно обрабатывается запрос Stop к остановке, которая не входит ни в один из маршрутов
        */
    std::string StopNotInRoutes() {
        return R"({
            "base_requests": [
        {
          "type": "Stop",
          "name": "Rivera bridge",
          "latitude": 43.587795,
          "longitude": 39.716901,
          "road_distances": {"Ocean staition": 850}
        },
        {
          "type": "Stop",
          "name": "Ocean staition",
          "latitude": 43.581969,
          "longitude": 39.719848,
          "road_distances": {"Myachino": 850}
        },
        {
          "type": "Stop",
          "name": "Myachino",
          "latitude": 43.581969,
          "longitude": 39.719848,
          "road_distances": {"Rivera bridge": 1700}
        },
        {
          "type": "Bus",
          "name": "14",
          "stops": [
            "Ulitsa Lizy Chaikinoi",
            "Elektroseti",
            "Ulitsa Dokuchaeva",
            "Ulitsa Lizy Chaikinoi"
          ],
          "is_roundtrip": true
        },
        {
            "type": "Bus",
                "name" : "114",
                "stops" : ["Ocean staition", "Rivera bridge", "Ocean staition"] ,
                "is_roundtrip" : true
        }

            ],
            "stat_requests": [
        { "id": 1, "type": "Stop", "name": "Myachino" }
            ]
        } )";
    }
    /*
        Тест № 12 не прошёл проверку
причина: Неправильный ответ
подсказка: Некорректно обрабатывается некольцевой маршрут, расстояния между остановками заданы в обоих направлениях
        */
    std::string NoneRoundtripAllDistance() {
        return R"({
            "base_requests": [
        {
          "type": "Stop",
          "name": "A",
          "latitude": 43.587795,
          "longitude": 39.716901,
          "road_distances": {"B": 500}
        },
        {
          "type": "Stop",
          "name": "B",
          "latitude": 43.581969,
          "longitude": 39.719848,
          "road_distances": {"C": 500, "A": 100}
        },
        {
          "type": "Stop",
          "name": "C",
          "latitude": 43.581969,
          "longitude": 39.719848,
          "road_distances": {"B": 100}
        },
        {
          "type": "Bus",
          "name": "14",
          "stops": [
            "A",
            "B",
            "C"
          ],
          "is_roundtrip": false
        }
            ],
            "stat_requests": [
        { "id": 1, "type": "Bus", "name": "14" }
            ]
        } )";
    }
    /*
Тест № 13 не прошёл проверку
причина: Неправильный ответ
подсказка: Некорректно обрабатывается некольцевой маршрут, расстояния между остановками заданы только в прямом направлении
        */

    std::string NoneRoundtripForwardDistance() {
        return R"({
            "base_requests": [
        {
          "type": "Stop",
          "name": "A",
          "latitude": 43.587795,
          "longitude": 39.716901,
          "road_distances": {"B": 500}
        },
        {
          "type": "Stop",
          "name": "B",
          "latitude": 43.581969,
          "longitude": 39.719848,
          "road_distances": {"C": 500}
        },
        {
          "type": "Stop",
          "name": "C",
          "latitude": 43.581969,
          "longitude": 39.719848,
          "road_distances": {}
        },
        {
          "type": "Bus",
          "name": "14",
          "stops": [
            "A",
            "B",
            "C"
          ],
          "is_roundtrip": false
        }
            ],
            "stat_requests": [
        { "id": 1, "type": "Bus", "name": "14" }
            ]
        } )";
    }
    /*
    Тест № 14 не прошёл проверку
причина: Неправильный ответ
подсказка: Некорректно обрабатывается некольцевой маршрут, расстояния между остановками заданы только в обратном направлении
    */
    std::string NoneRoundtripBackwardDistance() {
        return R"({
            "base_requests": [
        {
          "type": "Stop",
          "name": "A",
          "latitude": 43.587795,
          "longitude": 39.716901,
          "road_distances": {}
        },
        {
          "type": "Stop",
          "name": "B",
          "latitude": 43.581969,
          "longitude": 39.719848,
          "road_distances": {"A": 500}
        },
        {
          "type": "Stop",
          "name": "C",
          "latitude": 43.581969,
          "longitude": 39.719848,
          "road_distances": {"B": 500}
        },
        {
          "type": "Bus",
          "name": "14",
          "stops": [
            "A",
            "B",
            "C"
          ],
          "is_roundtrip": false
        }
            ],
            "stat_requests": [
        { "id": 1, "type": "Bus", "name": "14" }
            ]
        } )";
    }

    /*Test from pachka 2*/
    std::string Pachka2() {
            return R"({"render_settings": {"width": 91.83864308189214, "height": 33.30624155116589, "padding": 2.063614988828813, 
    "stop_radius": 40.36610532844539, "line_width": 0.5624490017929631, "stop_label_font_size": 6, 
    "stop_label_offset": [50.554188820040764, 76.74709744719402], "underlayer_color": [162, 186, 189, 0.017767846496926865], 
    "underlayer_width": 19.55259129677881, "color_palette": ["brown", "orange", "coral", [23, 171, 123], "olive", [131, 170, 232, 0.1921269924875878], 
    "navy", [96, 160, 67], "khaki", "lime", "khaki", [33, 35, 87], [228, 176, 67], 
    "aqua", "green", [50, 202, 20], [174, 140, 101], [144, 43, 152], [38, 31, 243, 0.8425428691903316], [146, 239, 130, 0.5749606346044288], 
    "bisque", [36, 191, 105, 0.4361961711576685], [162, 108, 41], [156, 5, 236, 0.5489670808671056], "cyan", [20, 252, 90, 0.6244917537536336], 
    "peru", "black", [159, 22, 8], [238, 202, 120], "green", "bisque", "black", "thistle", [31, 133, 15], [150, 238, 210, 0.03479792807040927], "brown"], 
    "bus_label_font_size": 1, "bus_label_offset": [-82.51719587275346, 77.72860351242304]}, 
    "base_requests": [{"type": "Stop", "name": "A", "latitude": 55.611087, "longitude": 37.20829, "road_distances": {"B": 4000}}, 
                    {"type": "Stop", "name": "B", "latitude": 55.595884, "longitude": 37.209755, "road_distances": {"A": 4500, "C": 9050}}, 
                    {"type": "Stop", "name": "C", "latitude": 55.632761, "longitude": 37.333324, "road_distances": {"B": 9300}}, 
                    {"type": "Bus", "name": "ABC", "stops": ["A", "B", "C"], "is_roundtrip": false}], 
    "stat_requests": [{"id": 100435477, "type": "Bus", "name": "ABC"}]})";
    }
}


int main() {

     //from file to 
    {
        json_reader::JsonReader reader;
        std::string str;
        std::ifstream in_file;
        in_file.open(".\\tests\\case_input_23.txt");

        std::stringstream buffer;
        if (in_file.is_open()) {
            buffer << in_file.rdbuf();
        }
        in_file.close();

        str = buffer.str();
        std::istringstream strm(str);

        reader.LoadDictionary(strm);

        std::ofstream out(".\\tests\\my_author_output_23.json");
        //out << reader.PrintJSON();
        reader.PrintSVG(out);
    }
     
    /*
     std::cout << "None roundtrip with two stops:" << std::endl;
     // некольцевой маршрут из двух остановок
     {
         json_reader::JsonReader reader;

         std::istringstream strm(NoneRoundtripTwoStops());

         reader.LoadDictionary(strm);

         std::cout << reader.PrintJSON();
     }
     std::cout << std::endl;
     std::cout << "Roundtrip with three stops:" << std::endl;
     // кольцевой маршрут из трёх остановок
     {
         json_reader::JsonReader reader;

         std::istringstream strm(RoundtripThreeStops());

         reader.LoadDictionary(strm);

         std::cout << reader.PrintJSON();
     }

     std::cout << std::endl;
     std::cout << "Test from Pachka 1:" << std::endl;
     // кольцевой маршрут из трёх остановок
     {
         json_reader::JsonReader reader;

         std::istringstream strm(Pachka1());

         reader.LoadDictionary(strm);

         std::cout << reader.PrintJSON();
     }

     std::cout << std::endl;
     std::cout << "Stop not in route:" << std::endl;
     // кольцевой маршрут из трёх остановок
     {
         json_reader::JsonReader reader;

         std::istringstream strm(StopNotInRoute());

         reader.LoadDictionary(strm);

         std::cout << reader.PrintJSON();
     }

     std::cout << std::endl;
     std::cout << "Stop not in routes:" << std::endl;
     // кольцевой маршрут из трёх остановок
     {
         json_reader::JsonReader reader;

         std::istringstream strm(StopNotInRoutes());

         reader.LoadDictionary(strm);

         std::cout << reader.PrintJSON();
     }

     std::cout << std::endl;
     std::cout << "All stops with distance: " << std::endl;
     // кольцевой маршрут из трёх остановок
     {
         json_reader::JsonReader reader;

         std::istringstream strm(NoneRoundtripAllDistance());

         reader.LoadDictionary(strm);

         std::cout << reader.PrintJSON();
     }

     std::cout << std::endl;
     std::cout << "All stops only forward distance: " << std::endl;
     // кольцевой маршрут из трёх остановок
     {
         json_reader::JsonReader reader;

         std::istringstream strm(NoneRoundtripForwardDistance());

         reader.LoadDictionary(strm);

         std::cout << reader.PrintJSON();
     }

     std::cout << std::endl;
     std::cout << "All stops only backward distance: " << std::endl;
     // кольцевой маршрут из трёх остановок
     {
         json_reader::JsonReader reader;

         std::istringstream strm(NoneRoundtripBackwardDistance());

         reader.LoadDictionary(strm);

         std::cout << reader.PrintJSON();
     }
     
          std::cout << std::endl;
     std::cout << "Stop from Pachka2:" << std::endl;
     // кольцевой маршрут из трёх остановок
     {
         json_reader::JsonReader reader;

         std::istringstream strm(Pachka2());

         reader.LoadDictionary(strm);

         reader.PrintSVG(std::cout);
         std::ofstream out(".\\tests\\output_pachka2.svg");
         reader.PrintSVG(out);
     }

     std::cout << std::endl;
     std::cout << "23:" << std::endl;
     {
         json_reader::JsonReader reader;
         std::string str;
         std::ifstream in_file;
         in_file.open(".\\tests\\case_input_23.txt");

         std::stringstream buffer;
         if (in_file.is_open()) {
             buffer << in_file.rdbuf();
         }
         in_file.close();

         str = buffer.str();
         std::istringstream strm(str);

         reader.LoadDictionary(strm);

         reader.PrintSVG(std::cout);
         //std::ofstream out(".\\tests\\author_output_23.svg");
         //reader.PrintSVG(out);
     }
     
     std::cout << std::endl;
     std::cout << "Parse SVG:" << std::endl;
     {
         json_reader::JsonReader reader;
         std::string str;
         std::ifstream in_file;
         in_file.open(".\\tests\\input1_circles_names.json");

         std::stringstream buffer;
         if (in_file.is_open()) {
             buffer << in_file.rdbuf();
         }
         in_file.close();

         str = buffer.str();
         std::istringstream strm(str);

         reader.LoadDictionary(strm);

         std::ofstream out(".\\tests\\output_bus_names.svg");
         reader.PrintSVG(out);
     }
     
*/
}