//#include "transport_catalogue.h"


#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"

#include <iostream>
#include <random>
#include <fstream>

using namespace std;
int main() {    
    transport_db::TransportCatalogue catalog;  
    map_renderer::MapRenderer renderer;
    transport_router::TransportRouter router;
    json_reader::JsonReader json(catalog, renderer, router);
    transport_db::RequestHandler request(catalog, renderer, router);	

    json.ReadInput(cin);        //read json
    json.FillCatalogue();    //send data to catalogue
    json.PrintRequests(cout);    //complete queries and write output to json       
    return 0;
}