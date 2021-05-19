#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <optional>
#include <iostream>

namespace transport_db {
    class RequestHandler {
    public:        
        explicit RequestHandler(const TransportCatalogue& db, map_renderer::MapRenderer& renderer, transport_router::TransportRouter& router) ///SPRINT12
            :db_(db),renderer_(renderer), router_(router) {
        }
        
        std::optional<const Bus*> GetBusStat(std::string_view bus_name);
        
        const std::set<std::string_view>* GetBusesByStop(const std::string_view& stop_name) const;
        
        void RenderMap() const;
        void SetCatalogueDataToRender() const;         

        void GenerateRouter() const; ///SPRINT12
        void SetCatalogueDataToRouter() const; ///SPRINT12           


    private:        
        const TransportCatalogue& db_;
        map_renderer::MapRenderer& renderer_; 
        transport_router::TransportRouter& router_; ///SPRINT12

        void SetStopsForRender() const;
        void SetRoutesForRender() const;

        void SetStopsForRouter() const; ///SPRINT12
        void SetRoutesForRouter() const; ///SPRINT12
        void SetDistForRouter() const; ///SPRINT12
    };
}