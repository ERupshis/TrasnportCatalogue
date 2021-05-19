#pragma once

#include "domain.h"
#include "geo.h"
#include "svg.h"
#include "router.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <map>
#include <memory>


namespace transport_db {
    struct RouterSettings { ///SPRINT12
        int bus_wait_time = 6; // minutes    
        int bus_velocity = 40; // km/h       
    };    
}


namespace transport_router {
    using namespace std::literals;
    using namespace transport_db;
    using namespace domain;

    enum class edge_type {
        WAIT,
        BUS
    };

    struct Edges {
        edge_type type;
        std::string_view name;  /// route or stop
        double time;
        size_t span_count;
    };
    
    /////MapRenderer Class part//////////////////////////////////////////////////////////  
    class TransportRouter { ///SPRINT12
    public:
        TransportRouter() = default;

        void SetSettings(const RouterSettings& settings);

        void SetStops(const std::map<std::string_view, const Stop*> stops);
        void SetRoutes(const std::map<std::string_view, const Bus*> routes);
        void SetDistances(const std::unordered_map<std::pair<std::string_view, std::string_view>, int, SV_SV_Hasher>& distances);

        void GenerateRouter();

        using RouteData = graph::Router<double>::RouteInfo;       
        std::optional<RouteData> GetRoute(std::string_view from, std::string_view to);

        const std::vector<Edges>* GetEdgesData() const;
        
    private:
        RouterSettings settings_;   
        std::map<std::string_view, const Stop*> stops_;
        std::map<std::string_view, const Bus*> routes_;
        const std::unordered_map<std::pair<std::string_view, std::string_view>, int, SV_SV_Hasher>* distances_ = nullptr; // STOP to STOP dist storage
        std::unique_ptr<graph::Router<double>> router_ = nullptr;

        enum class vertex_type {
            IN,
            OUT,
            EMPTY
        };

        graph::DirectedWeightedGraph<double> graph_;
        struct Vertexes {            
            std::string_view name;
            vertex_type type = vertex_type::EMPTY;
            size_t id;
        };  
        
        std::map<std::string_view, std::pair<Vertexes, Vertexes>> vertexes_;
        std::vector<Edges> edges_;
        
        //////////////////////////////////////////////////////////////////////////////////////       
        void AddStops();

        double CalculateWeight(int distance);

        void AddEdges();
    };
} // namespace map_renderer