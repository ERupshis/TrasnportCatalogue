#include "json_reader.h"

namespace json_reader {
    using namespace std::literals;
    using namespace transport_db;
    using namespace transport_base;

    namespace detail {        
        /////Add data to catalogue Area///////////////////////////////////////////////////////////
        std::map<std::string, int> DistBtwStops(const json::Dict& dic) { // Handle stops distances for BaseStop()
            std::map<std::string, int> res;
            for (const auto& elem : dic) {
                res[elem.first] = elem.second.AsInt();
            }
            return res;
        }

        RequestStop BaseStop(const json::Dict& dic) {
            RequestStop stop;
            stop.e_type = RequestType::BASE;
            stop.type = request_type::STOP;
            stop.name = dic.at("name"s).AsString();
            stop.latitude = dic.at("latitude"s).AsDouble();
            stop.longitude = dic.at("longitude"s).AsDouble();
            if (dic.count("road_distances"s)) {
                stop.road_distances = std::move(DistBtwStops(dic.at("road_distances"s).AsMap())); // read road_distances as map and then convert Node in this map
            }
            return stop;
        }

        std::vector<std::string> StopsForBus(const json::Array& arr) { // Handle array of stops for BaseBus()
            std::vector<std::string> res;
            for (const auto& elem : arr) {
                res.emplace_back(elem.AsString());
            }
            return res;
        }

        RequestBus BaseBus(const json::Dict& dic) {
            RequestBus bus;
            bus.e_type = RequestType::BASE;
            bus.type = request_type::BUS;
            bus.name = dic.at("name"s).AsString();
            bus.stops = std::move(StopsForBus(dic.at("stops"s).AsArray()));
            bus.is_roundtrip = dic.at("is_roundtrip"s).AsBool();
            return bus;
        }

        /////Get data from catalogue Area///////////////////////////////////////////////////////////
        RequestStat Stat(const json::Dict& dic) {
            RequestStat stat;               
            stat.e_type = RequestType::STAT;
            stat.id = dic.at("id"s).AsInt();
            if (dic.at("type"s).AsString() == "Bus"s) { // STAT request type?
                stat.type = request_type::BUS;
                stat.name = dic.at("name"s).AsString();
            }
            else if (dic.at("type"s).AsString() == "Stop"s) {
                stat.type = request_type::STOP;
                stat.name = dic.at("name"s).AsString();
            }
            else if (dic.at("type"s).AsString() == "Map"s) {
                stat.type = request_type::MAP;
            }
            else if (dic.at("type"s).AsString() == "Route"s) { ///SPRINT12
                stat.type = request_type::ROUTE;
                stat.route.from = dic.at("from"s).AsString();
                stat.route.to = dic.at("to"s).AsString();
            }
            return stat;
        }

        svg::Color ColorFromNode(const json::Node& node) {
            if (node.IsArray()) {//holds rgb/rgba
                if (node.AsArray().size() == 3) {
                    svg::Rgb rgb;
                    rgb.red = node.AsArray()[0].AsInt();
                    rgb.green = node.AsArray()[1].AsInt();
                    rgb.blue = node.AsArray()[2].AsInt();
                    return rgb;
                }
                else {
                    svg::Rgba rgba;
                    rgba.red = node.AsArray()[0].AsInt();
                    rgba.green = node.AsArray()[1].AsInt();
                    rgba.blue = node.AsArray()[2].AsInt();
                    rgba.opacity = node.AsArray()[3].AsDouble();
                    return rgba;
                }
            }
            else {//holds string
                return node.AsString();
            }
        }

        transport_db::RenderSettings RenderMap(const json::Dict& dic) {
            using namespace detail;
            transport_db::RenderSettings res;
            res.width = dic.at("width"s).AsDouble();
            res.height = dic.at("height"s).AsDouble();
            res.padding = dic.at("padding"s).AsDouble();
            res.line_width = dic.at("line_width"s).AsDouble();
            res.stop_radius = dic.at("stop_radius"s).AsDouble();
            res.bus_label_font_size = dic.at("bus_label_font_size"s).AsInt();
            res.bus_label_offset[0] = dic.at("bus_label_offset"s).AsArray()[0].AsDouble();
            res.bus_label_offset[1] = dic.at("bus_label_offset"s).AsArray()[1].AsDouble();
            res.stop_label_font_size = dic.at("stop_label_font_size"s).AsInt();
            res.stop_label_offset[0] = dic.at("stop_label_offset"s).AsArray()[0].AsDouble();
            res.stop_label_offset[1] = dic.at("stop_label_offset"s).AsArray()[1].AsDouble();
            res.underlayer_color = ColorFromNode(dic.at("underlayer_color"s));
            res.underlayer_width = dic.at("underlayer_width"s).AsDouble();
            for (const auto& node : dic.at("color_palette"s).AsArray()) {
                res.color_palette.push_back(ColorFromNode(node));
            }
            return res;
        }

        transport_db::RouterSettings RouterMap(const json::Dict& dic) { ///SPRINT12
            using namespace detail;
            transport_db::RouterSettings res;
            res.bus_velocity_kmh = dic.at("bus_velocity"s).AsInt();
            res.bus_wait_time = dic.at("bus_wait_time"s).AsInt();            
            return res;
        }

        SerializationSettings SerializationCatalogue(const json::Dict& dic) { ///SPRINT14          
            SerializationSettings res;
            res.file_name = dic.at("file"s).AsString();            
            return res;
        }
    }
    using namespace detail;
    /////JsonReader class part/////////////////////////////////////////////////////////////////  
    void JsonReader::ReadInput(std::istream& input) {
        FillDoc(input); // fill json doc
        for (const auto& elem : document_.GetRoot().AsMap()) { //fill all requests
            if (elem.first == "base_requests"s) {
                FillBase(elem.second.AsArray()); //add data request
            }
            else if (elem.first == "stat_requests"s) {
                FillStat(elem.second.AsArray()); //get data request
            }
            else if (elem.first == "render_settings"s) {
                FillRender(elem.second.AsMap()); //set render settings
            }
            else if (elem.first == "routing_settings"s) { ///SPRINT 12
                FillRouting(elem.second.AsMap()); //set render settings
            }
            else if (elem.first == "serialization_settings"s) { ///SPRINT 14
                FillSerialization(elem.second.AsMap()); //set serialization_settings
            }
        }
    }

    void JsonReader::FillCatalogue() {
        std::vector<std::pair<std::string, std::map<std::string, int>>> stops_w_dist; //vector of all distances between stops
        for (const auto& elem : requests_) { // 1st stage - Add Stops
            if (RequestStop* s = dynamic_cast<RequestStop*>(elem.get())) {
                catalogue_.AddStop(s->name, s->latitude, s->longitude);
                if (!s->road_distances.empty()) {
                    stops_w_dist.emplace_back(std::make_pair(s->name, s->road_distances));
                }
            }
        }
        for (const auto& elem : requests_) { // 2nd stage - Add Buses
            if (RequestBus* b = dynamic_cast<RequestBus*>(elem.get())) {
                std::vector <std::string> string_stops = std::move(b->stops);
                std::vector<std::string_view> stops;
                for (const auto& stop : string_stops) {
                    stops.push_back(stop);
                }
                std::string_view end_stop = stops.back();
                if (!b->is_roundtrip) {
                    stops.reserve(2 * stops.size());
                    stops.insert(stops.end(), next(stops.rbegin()), stops.rend()); // all stops of route are adding beffore 
                                                                                    // adding it in catalogue
                }
                catalogue_.AddRoute(b->name, stops, b->is_roundtrip, end_stop); // Set end_stop name from request for detecting 
                                                                                // condition with wrong input of b->is_roundtrip 
            }
        }
        for (auto& [main_stop, des_stops] : stops_w_dist) { // 3rd stage - Add distances between Stops
            for (auto& [des_stop, dist] : des_stops) {
                catalogue_.SetDistBtwStops(main_stop, des_stop, dist);
            }
        }
    }  

    void JsonReader::PrintRequests(std::ostream& out, RequestHandler& request_handler) {
        //transport_db::RequestHandler request_handler(catalogue_, renderer_, router_, serialization_); ///SPRINT14       
        out << "["s << std::endl;
        bool first = true;
        for (const auto& elem : requests_) {
            if (RequestStat* s = dynamic_cast<RequestStat*>(elem.get())) { 
                if (!first) {
                    out << ',' << std::endl;
                }
                if (s->type == request_type::STOP) {
                    Builder request{};
                    request.StartDict().Key("request_id"s).Value(s->id);
                    ProcessStopStatRequest(catalogue_.GetStop(s->name), request);
                    request.EndDict();
                    Print(Document{ request.Build() }, out);
                }
                else if (s->type == request_type::BUS) {
                    Builder request{};
                    request.StartDict().Key("request_id"s).Value(s->id);
                    ProcessBusStatRequest(catalogue_.GetRoute(s->name), request); // fill dic with Stop data     
                    request.EndDict();
                    Print(Document{ request.Build() }, out);
                }
                else if (s->type == request_type::MAP) {
                    Builder request{};
                    request.StartDict().Key("request_id"s).Value(s->id);
                    //transport_db::RequestHandler request_handler(catalogue_, renderer_, router_, serialization_); ///SPRINT14
                    request_handler.SetCatalogueDataToRender();                    
                    std::stringstream strm;
                    renderer_.Render(strm);
                    request.Key("map"s).Value(strm.str());
                    request.EndDict();
                    Print(Document{ request.Build() }, out);
                }
                else if (s->type == request_type::ROUTE) { ///SPRINT12
                    Builder request{};                    
                    const std::vector<transport_router::Edges>* edges_data = router_.GetEdgesData();
                    auto route_data = router_.GetRoute(s->route.from, s->route.to);                    
                    request.StartDict().Key("request_id"s).Value(s->id);                        
                        if (route_data && route_data->edges.size() > 0) { // route is generated
                            request.Key("total_time"s).Value(route_data->weight)
                                .Key("items")
                                .StartArray();
                            for (size_t edge_id : route_data->edges) {
                                std::string name{ edges_data->at(edge_id).name };
                                if (edges_data->at(edge_id).type == edge_type::WAIT) { // graph edge belongs to Stop
                                    request.StartDict()
                                        .Key("stop_name"s).Value(name)
                                        .Key("time"s).Value(edges_data->at(edge_id).time)
                                        .Key("type"s).Value("Wait"s)
                                        .EndDict();
                                }
                                else { // graph edge belongs to bus
                                    request.StartDict()
                                        .Key("bus"s).Value(name)
                                        .Key("time"s).Value(edges_data->at(edge_id).time)
                                        .Key("type"s).Value("Bus"s)
                                        .Key("span_count"s).Value(static_cast<int>(edges_data->at(edge_id).span_count))
                                        .EndDict();
                                }
                                    
                            }
                            request.EndArray();
                        }
                        else if (!route_data) { // failed to create route
                            request.Key("error_message"s).Value("not found"s);
                        }
                        else { // stop FROM = stop TO
                            request.Key("total_time"s).Value(0)
                                .Key("items")
                                .StartArray()
                                .EndArray();
                        }
                        request.EndDict();  
                    Print(Document{ request.Build() }, out);
                }
                first = false;                
            }
        }
        out << std::endl << "]"s << std::endl;        
    }


    void JsonReader::FillDoc(std::istream& strm) { // read JSON doc
        document_ = json::Load(strm);
    }

    void JsonReader::FillBase(const std::vector <Node>& vec) { // add Add request
        for (const auto& elem : vec) {
            if (elem.AsMap().count("type"s)) {
                if (elem.AsMap().at("type"s).AsString() == "Stop"s) {
                    requests_.push_back(std::make_unique<RequestStop>(detail::BaseStop(elem.AsMap()))); // fill stop
                }
                else if (elem.AsMap().at("type"s).AsString() == "Bus"s) {
                    requests_.push_back(std::make_unique<RequestBus>(detail::BaseBus(elem.AsMap()))); // fill stop
                }
            }
        }
    }

    void JsonReader::FillStat(const std::vector<Node>& vec) { // add Get request
        for (const auto& elem : vec) {
            if (elem.AsMap().count("type"s)) {
                if (elem.AsMap().at("type"s).AsString() == "Bus"s 
                    || elem.AsMap().at("type"s).AsString() == "Stop"s 
                    || elem.AsMap().at("type"s).AsString() == "Map"s
                    || elem.AsMap().at("type"s).AsString() == "Route"s ///SPRINT12
                ) {
                    requests_.emplace_back(std::make_unique<RequestStat>(detail::Stat(elem.AsMap())));
                }
            }
        }
    }

    void JsonReader::FillRender(const std::map<std::string, Node>& dic) { // add Add Render settings request
        renderer_.SetSettings(RenderMap(dic));
    }

    void JsonReader::FillRouting(const std::map<std::string, Node>& dic) { // add Add Routing settings request ///SPRINT12
        router_.SetSettings(RouterMap(dic));
    }

    void JsonReader::FillSerialization(const std::map<std::string, Node>& dic) { ///SPRINT14
        serialization_.SetSettings(SerializationCatalogue(dic));
    }
    
    void JsonReader::ProcessStopStatRequest(const transport_db::TransportCatalogue::StopOutput& request, Builder& dict) { //overload for GetRoute() STOP        
        auto [X, buses] = request;        
        if (X[0] == '!') {            
            dict.Key("error_message"s).Value("not found"s);
            return;
        }            
        if (buses.size() == 0) {
            dict.Key("buses"s).StartArray().EndArray();
        }
        else { 
            dict.Key("buses"s).StartArray();
            for (auto& bus : buses) {
                std::string s_bus(bus);
                dict.Value(std::move(s_bus));
            }            
            dict.EndArray();
        }
    }

    void JsonReader::ProcessBusStatRequest(const transport_db::TransportCatalogue::RouteOutput& request, Builder& dict) { //overload for GetRoute() BUS
        auto [X, R, U, L, C] = request;
        if (X[0] != '!') {
            dict.Key("curvature"s).Value(C)
                .Key("route_length"s).Value(L)
                .Key("stop_count"s).Value(static_cast<int>(R))
                .Key("unique_stop_count"s).Value(static_cast<int>(U));
        }
        else {
            dict.Key("error_message"s).Value("not found"s);
        }        
    }   
}