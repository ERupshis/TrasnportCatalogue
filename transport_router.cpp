#include "transport_router.h"

namespace transport_router { 
    //using namespace transport_db;    
    using namespace domain;
    ///// ROUTER////////////////////////////////////////////////////////// 
    void TransportRouter::SetSettings(const RouterSettings& settings) {
        settings_ = settings;
    }
    
    void TransportRouter::GenerateRouter() {
        AddStops();
        AddEdges();
        router_ = std::make_unique<graph::Router<double>>(graph::Router(graph_));
    }  

    std::optional<TransportRouter::RouteData> TransportRouter::GetRoute(std::string_view from, std::string_view to) {
        return router_->BuildRoute(vertexes_.at(from).in.id, vertexes_.at(to).in.id);
    }

    const std::vector<Edges>* TransportRouter::GetEdgesData() const {
        return &edges_;
    }

    ///// ROUTER////////PRIVATE//////////////////////////////////////////      
    void TransportRouter::AddStops() { // Add stops: 1 stop = 2 vertexes
        size_t vertex_count = 0;
        for (auto [name, stop_ptr] : catalogue_.GetStopsForRender()) {
            Vertex in = { name, vertex_type::IN, vertex_count++ };
            Vertex out = { name, vertex_type::OUT, vertex_count++ };
            vertexes_[name] = { in, out };
        }
        graph_.ResizeIncidenceLists(vertexes_.size()); // resize incidence list in accrodance to vertex qty. Otherwise exception 'out of range' in graph
        for (auto [name, data] : vertexes_) {
            graph_.AddEdge({ data.in.id, data.out.id, settings_.bus_wait_time * 1.0 }); 
            edges_.push_back({ edge_type::WAIT, name, settings_.bus_wait_time * 1.0, 0 }); // add edge in vector of edges (index in vector is equal to graph_.AddEdge return)
        }
    }

    double TransportRouter::CalculateWeight(int distance) {
        return distance / (settings_.bus_velocity_kmh * 1000.0 / 60.0); // calculate edge weight (equal to minutes)
    }

    void TransportRouter::AddEdges() {
        for (auto [name, route_ptr] : catalogue_.GetRoutesForRender()) { // handle all routes one-by-one
            for (auto it = route_ptr.stops.begin(); it != prev(route_ptr.stops.end()); ++it) { // handle all stops on routes one-by-one except last stop (last stop = first stop)
                int dist_to_next_stop = 0;
                std::string_view out_stop = *it;
                for (auto sub_it = it + 1; sub_it != route_ptr.stops.end(); ++sub_it) { // generate all possible edges from current stop to other
                    if (!route_ptr.is_round_trip && // handle of NOT round route due to it has 2 end stops -> passenger has to wait next bus on these stops
                        sub_it - route_ptr.stops.begin() == ceil(route_ptr.stops.size() / 2) + 1 && // edges variants are limited by end stops
                        *prev(sub_it) == route_ptr.end_stop &&
                        it - route_ptr.stops.begin() != ceil(route_ptr.stops.size() / 2)) {
                        break;
                    }
                    if (catalogue_.GetDistForRouter().count(std::pair(out_stop, *sub_it))) { // accumulate distance
                        dist_to_next_stop += catalogue_.GetDistForRouter().at(std::pair(out_stop, *sub_it));
                    }
                    else {
                        dist_to_next_stop += catalogue_.GetDistForRouter().at(std::pair(*sub_it, out_stop));
                    }
                    size_t span = sub_it - it; // calculate span_count for every edge
                    edges_.push_back({ edge_type::BUS, name, CalculateWeight(dist_to_next_stop), span });
                    graph_.AddEdge({ vertexes_.at(*it).out.id, vertexes_.at(*sub_it).in.id, edges_.back().time });
                    out_stop = *sub_it;
                }
            }
        }
    }    
} // namespace map_router