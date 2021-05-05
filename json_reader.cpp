#include "json_reader.h"

namespace json_reader {
    using namespace std::literals;
    using namespace transport_db;
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

        RenderSettings Map(const json::Dict& dic) {
            using namespace detail;
            RenderSettings res;
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
                std::vector<std::string> stops = std::move(b->stops);
                std::string end_stop = stops.back();
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

    void JsonReader::PrintRequests(std::ostream& out) {
        //Builder request{};
        //request.StartArray();
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
                    transport_db::RequestHandler request_handler(catalogue_, renderer_);
                    request_handler.SetRoutesForRender();
                    request_handler.SetStopsForRender();
                    std::stringstream strm;
                    renderer_.Render(strm);
                    request.Key("map"s).Value(strm.str());
                    request.EndDict();
                    Print(Document{ request.Build() }, out);
                }
                first = false;                
            }
        }
        out << std::endl << "]"s << std::endl;
        //request.EndArray();
         // print after 
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
                ) {
                    requests_.emplace_back(std::make_unique<RequestStat>(detail::Stat(elem.AsMap())));
                }
            }
        }
    }

    void JsonReader::FillRender(const std::map<std::string, Node>& dic) { // add Add Render settings request
        renderer_.SetSettings(Map(dic));
    }
    
    void JsonReader::ProcessStopStatRequest(const TransportCatalogue::StopOutput& request, Builder& dict) { //overload for GetRoute() STOP        
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

    void JsonReader::ProcessBusStatRequest(const TransportCatalogue::RouteOutput& request, Builder& dict) { //overload for GetRoute() BUS
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