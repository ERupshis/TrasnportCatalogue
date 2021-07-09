#include "json_reader.h"
#include "request_handler.h"

#include <iostream>
#include <random>
#include <fstream>
#include <string_view>
#include "log_duration.h"

using namespace std::literals;

void CreateAndSerializeBase() {    
    transport_db::TransportCatalogue catalog;
    map_renderer::MapRenderer renderer;
    transport_router::TransportRouter router(catalog);
    transport_base::TransportSerialization serialization(catalog, renderer, router); 
    json_reader::JsonReader json(catalog, renderer, router, serialization);
    transport_db::RequestHandler request_handler(catalog, renderer, router, serialization);
    json.ReadInput(std::cin);        
    json.FillCatalogue();   
    request_handler.GenerateRouter(); 
    serialization.SerializeBase();
}

void DeserializeBaseAndCreate() {
    transport_db::TransportCatalogue catalog;
    map_renderer::MapRenderer renderer;
    transport_router::TransportRouter router(catalog);
    transport_base::TransportSerialization serialization(catalog, renderer, router); 
    json_reader::JsonReader json(catalog, renderer, router, serialization);
    transport_db::RequestHandler request_handler(catalog, renderer, router, serialization);
    json.ReadInput(std::cin);
    serialization.DeserializeBase();

    //json1.PrintRequests(std::cout, request_handler1);

    std::ofstream result_file("result_7"s);
    json.PrintRequests(result_file, request_handler);
    result_file.close();    
}

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}
// /*
int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        {
            LogDuration test("make_base");
            CreateAndSerializeBase(); 
            // make base here
        } 
    }
    else if (mode == "process_requests"sv) {
        {
            LogDuration test("process_requests");
            DeserializeBaseAndCreate();
            // process requests here
        }
    }
    else {
        PrintUsage();
        return 1;
    }
}
// */
 /*
int main() {    
    {
        LogDuration test("SERIALIZATION");
        transport_db::TransportCatalogue catalog;
        map_renderer::MapRenderer renderer;
        transport_router::TransportRouter router(catalog);
        transport_base::TransportSerialization serialization(catalog, renderer, router); // SPRINT 14
        json_reader::JsonReader json(catalog, renderer, router, serialization);
        transport_db::RequestHandler request_handler(catalog, renderer, router, serialization);
        //std::ifstream base_file("optimization.input");
        std::ifstream base_file("base_7.input");
        json.ReadInput(base_file);        //read json
        base_file.close();
        //  json.ReadInput(std::cin);        //read json
        json.FillCatalogue();    //send data to catalogue
        request_handler.GenerateRouter();
        serialization.SerializeBase();
    }
    {
        LogDuration test("DESERIALIZATION");
        transport_db::TransportCatalogue catalog1;
        map_renderer::MapRenderer renderer1;
        transport_router::TransportRouter router1(catalog1);
        transport_base::TransportSerialization serialization1(catalog1, renderer1, router1); // SPRINT 14
        json_reader::JsonReader json1(catalog1, renderer1, router1, serialization1);
        transport_db::RequestHandler request_handler1(catalog1, renderer1, router1, serialization1);
        std::ifstream request_file("requests_7.input");
        //std::ifstream request_file("optimization_req.input");
        json1.ReadInput(request_file);
        request_file.close();        
        {
            LogDuration test("DESERIALIZATION_PROCESS_REQUESTS");
            serialization1.DeserializeBase();
            std::ofstream result_file("result_7.txt");
            //std::ofstream result_file("optimization_res.txt");
            json1.PrintRequests(result_file, request_handler1);
            result_file.close();
        }
    }
    {      
        LogDuration test("ONE FILE");
        transport_db::TransportCatalogue catalog;
        map_renderer::MapRenderer renderer;
        transport_router::TransportRouter router(catalog);
        transport_base::TransportSerialization serialization(catalog, renderer, router); // SPRINT 14
        json_reader::JsonReader json(catalog, renderer, router, serialization);
        transport_db::RequestHandler request_handler(catalog, renderer, router, serialization);
        
        std::ifstream base_file("base+req_7.input");
        json.ReadInput(base_file);        //read json
        base_file.close();
        {
            LogDuration test("ONE FILE_PROCESS_REQUESTS");
            json.FillCatalogue();    //send data to catalogue
            request_handler.GenerateRouter();

            std::ofstream result_file("simple result_7.txt");
            //std::ofstream result_file("optimization_res.txt");
            json.PrintRequests(result_file, request_handler);
            result_file.close();
        }
    }
    return 1;
}
 */


