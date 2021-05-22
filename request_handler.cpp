#include "request_handler.h"

namespace transport_db {
    std::optional<const Bus*> RequestHandler::GetBusStat(std::string_view bus_name) {
        const Bus* bus = db_.SearchRoute(bus_name);
        if (bus != nullptr) {
            return bus;
        }
        else {
            return std::nullopt;
        }
    }

    const std::set<std::string_view>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
        const Stop* stop = db_.SearchStop(stop_name);
        if (!stop->buses.empty()) {
            return &(stop->buses);
        }
        else {
            return nullptr;
        }
    }
    ///// RENDER//////////////////////////////////////////////////////////////////
    void RequestHandler::RenderMap() const {
        SetStopsForRender();
        SetRoutesForRender();
        renderer_.Render(std::cout);
    }

    void RequestHandler::SetCatalogueDataToRender() const {
        SetStopsForRender();
        SetRoutesForRender();
    }

    void RequestHandler::SetStopsForRender() const {
        std::map<std::string_view, const Stop*> stops;
        for (const auto& stop : db_.GetStopsForRender()) {
            stops[stop.first] = &stop.second;
        }
        renderer_.SetStops(stops);
    }

    void RequestHandler::SetRoutesForRender() const {
        std::map<std::string_view, const Bus*> routes;
        for (const auto& route : db_.GetRoutesForRender()) {
            routes[route.first] = &route.second;
        }
        renderer_.SetRoutes(routes);
    }
    ///// ROUTER//////////////////////////////////////////////////////////////////
    void RequestHandler::GenerateRouter() const { ///SPRINT12
        router_.GenerateRouter();        
    }    
}