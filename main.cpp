#include "json_reader.h"
#include "request_handler.h"

#include <iostream>
#include <random>
#include <fstream>
#include <string_view>

void CreateAndSerializeBase() {
    transport_db::TransportCatalogue catalog;
    map_renderer::MapRenderer renderer;
    transport_router::TransportRouter router(catalog);
    transport_base::TransportSerialization serialization(catalog, renderer, router);
    json_reader::JsonReader json(catalog, renderer, router, serialization);
    transport_db::RequestHandler request_handler(catalog, renderer, router, serialization);
    json.ReadInput(std::cin);
    json.FillCatalogue();
    request_handler.SerializeBase();
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
    json.PrintRequests(std::cout, request_handler);
}

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }


    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        CreateAndSerializeBase();
        // make base here


    }
    else if (mode == "process_requests"sv) {
        DeserializeBaseAndCreate();
        // process requests here

    }
    else {
        PrintUsage();
        return 1;
    }
}




