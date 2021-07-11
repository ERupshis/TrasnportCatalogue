#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
//#include <transport_catalogue.pb.h> //SPRINT_14
#include "serialization.h"

#include <optional>
#include <iostream>

namespace transport_db {
    class RequestHandler {
    public:        
        explicit RequestHandler(const TransportCatalogue& db, map_renderer::MapRenderer& renderer, transport_router::TransportRouter& router, 
                                transport_base::TransportSerialization& serialization) ///SPRINT14
            :db_(db),renderer_(renderer), router_(router), serialization_(serialization) {
        }
        
        std::optional<const Bus*> GetBusStat(std::string_view bus_name);
        
        const std::set<std::string_view>* GetBusesByStop(const std::string_view& stop_name) const;
        
        void RenderMap() const;
        void SetCatalogueDataToRender() const;         

        void GenerateRouter() const; ///SPRINT12  

        void SerializeBase() const; ///SPRINT14
        void DeserializeBase() const; ///SPRINT14

    private:        
        const TransportCatalogue& db_;
        map_renderer::MapRenderer& renderer_; 
        transport_router::TransportRouter& router_; ///SPRINT12
        transport_base::TransportSerialization& serialization_; // SPRINT14

        void SetStopsForRender() const;
        void SetRoutesForRender() const;       
    };
}