#pragma once

#include "json.h"
#include "json_builder.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "serialization.h"

#include <sstream>
#include <variant>
#include <iomanip>
#include <iostream>


namespace json_reader {
    using namespace std::literals;
    using namespace transport_db;
    using namespace json;
    using namespace map_renderer;
    using namespace transport_router;
    namespace detail {
/////Request structures Part//////////////////////////////////////////////////////////////
        enum class RequestType {
            BASE = 0, // add
            STAT, // get
            RENDER, // generste map
            EMPTY // non initialized              
        };
        enum class request_type { BUS, STOP, MAP, ROUTE, EMPTY }; ///SPRINT12

        struct Request { // Parent
            RequestType e_type = RequestType::EMPTY;
            request_type type = request_type::EMPTY;
            virtual ~Request() = default;
        };

        struct RequestStatRoute { ///SPRINT12
            std::string from;
            std::string to;
        };

        struct RequestStat : public Request { // Child for get data
            std::string name; // name is needed for Bus of Stop name to search
            int id = 0;
            RequestStatRoute route; ///SPRINT12
        };
        
        struct RequestStop : public Request { // Child for get Stop data
            std::string name;
            double latitude = 0;
            double longitude = 0;
            std::map<std::string, int> road_distances;
        };
        struct RequestBus : public Request { // Child for get Stop data
            std::string name;
            bool is_roundtrip = false;
            std::vector<std::string> stops;
        };
        /////Add data to catalogue Area///////////////////////////////////////////////////////////
        std::map<std::string, int> DistBtwStops(const Dict& dic); // Handle stops distances for BaseStop()
        RequestStop BaseStop(const Dict& dic);
        std::vector<std::string> StopsForBus(const Array& arr); // Handle array of stops for BaseBus()
        RequestBus BaseBus(const Dict& dic);

        /////Get data from catalogue Area///////////////////////////////////////////////////////////
        RequestStat Stat(const Dict& dic);

        svg::Color ColorFromNode(const Node& node);
        transport_db::RenderSettings RenderMap(const Dict& dic);
    }
    using namespace detail;
/////JsonReader class part/////////////////////////////////////////////////////////////////
    class JsonReader {
    public:
        JsonReader(transport_db::TransportCatalogue& catalogue, MapRenderer& renderer, transport_router::TransportRouter& router, transport_base::TransportSerialization& serialization) ///SPRINT12
            :catalogue_(catalogue), renderer_(renderer), router_(router), serialization_(serialization) {
        }

        void ReadInput(std::istream& input);

        void FillCatalogue();

        void PrintRequests(std::ostream& out, RequestHandler& request_handler);

    private:
        transport_db::TransportCatalogue& catalogue_;
        Document document_;
        std::vector<std::unique_ptr<Request>> requests_;
        MapRenderer& renderer_;
        transport_router::TransportRouter& router_; ///SPRINT 12
        transport_base::TransportSerialization& serialization_; // SPRINT 14

        void FillDoc(std::istream& strm);
        void FillBase(const std::vector <Node>& vec);
        void FillStat(const std::vector<Node>& vec);
        void FillRender(const std::map<std::string, json::Node>& dic);
        void FillRouting(const std::map<std::string, Node>& dic); ///SPRINT12
        void FillSerialization(const std::map<std::string, Node>& dic); // SPRINT 14
        
        void ProcessStopStatRequest(const transport_db::TransportCatalogue::StopOutput& request, Builder& dict);        
        void ProcessBusStatRequest(const transport_db::TransportCatalogue::RouteOutput& request, Builder& dict);
    };
}

