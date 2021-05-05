/*#include "json_builder.h"
#include <iostream>


using namespace std;

int main() {
    json::Print(
        json::Document{
            json::Builder{}
            .StartDict()
                .Key("key1"s).Value(123)
                .Key("key2"s).Value("value2"s)
                .Key("key3"s).StartArray()
                    .Value(456)
                    .StartDict().EndDict()
                    .StartDict()
                        .Key(""s)
                        .Value(nullptr)
                    .EndDict()
                    .Value(""s)
                .EndArray()
            .EndDict()
            .Build()
        },
        cout
    );
    cout << endl;
    
    json::Print(
        json::Document{
            json::Builder{}
            .Value("just a string"s)
            .Build()
        },
        cout
    );
    cout << endl;
}
*/


//#include "transport_catalogue.h"


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
    json.PrintRequests(cout);    //complete queries and write output to json   
    //request.SetRoutesForRender();
    //request.SetStopsForRender();
    //renderer.Render(cout);
    return 0;
}