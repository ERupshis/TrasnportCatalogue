//#include "input_reader.h"
//#include "transport_catalogue.h"
//#include "stat_reader.h"
//#include "log_duration.h"
#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"

#include <iostream>
#include <random>

using namespace std;
int main() {
    //system("chcp 1251");
    transport_db::TransportCatalogue catalog;  
    map_renderer::MapRenderer renderer;
    json_reader::JsonReader json(catalog, renderer);
    transport_db::RequestHandler request(catalog, renderer);

    json.ReadInput(cin);        //read json
    json.FillCatalogue();    //send data to catalogue
    //json.PrintRequests(cout);    //complete queries and write output to json   
    request.SetRoutesForRender();
    request.SetStopsForRender();
    renderer.Render(cout);
    return 0;
}