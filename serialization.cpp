#include "serialization.h"

#include <filesystem>
#include <fstream>
#include <variant>
#include "svg.h"
#include <iostream>

namespace transport_base {
    void TransportSerialization::SetSettings(const  transport_db::SerializationSettings& settings) {
        settings_ = settings;
    }
/// SERIALIZATION
    //Solution for full data serialization
    void TransportSerialization::SerializeBase() {
        transport_base::TransportCatalogue base;
        
        *base.mutable_catalogue() = std::move(SerializeCatalogueData());   
        *base.mutable_map_renderer() = std::move(SerializeMapRendererData());
        *base.mutable_transport_router() = std::move(SerializeTransportRouterData());
       
        std::filesystem::path path = settings_.file_name;
        std::ofstream out_file(path, std::ios::binary);        
        base.SerializeToOstream(&out_file);
    }  
    /// CATALOGUE
    transport_base::Catalogue TransportSerialization::SerializeCatalogueData() {
        transport_base::Catalogue tmp_catalogue;
        int s = 0; // stop index in proto
        for (auto& [stop_name, stop_data] : catalogue_.GetStopsForRender()) { // ADD STOPS IN PROTO
            ser_stops_ind[stop_name] = s;
            tmp_catalogue.add_stops(); // add element in array of proto
            *tmp_catalogue.mutable_stops(s) = std::move(SerializeStopData(stop_data)); // modify new element
            ++s;
        }
        int b = 0; // bus index in proto
        for (auto& [bus_name, bus_data] : catalogue_.GetRoutesForRender()) { // ADD BUSES IN PROTO
            ser_buses_ind[bus_name] = b;
            tmp_catalogue.add_buses(); // add element in array of proto
            *tmp_catalogue.mutable_buses(b) = std::move(SerializeBusData(bus_data, ser_stops_ind)); // modify new element
            ++b;
        }
        int d = 0; // dist counter
        for (auto& [stops, length] : catalogue_.GetDistForRouter()) { // ADD DISTANCES BTW STOPS IN PROTO
            tmp_catalogue.add_distances();          // add element in array of proto   
            *tmp_catalogue.mutable_distances(d) = std::move(SerializeDistanceData(stops, length, ser_stops_ind)); // modify new element
            ++d;
        }
        return tmp_catalogue;
    }

    transport_base::Stop TransportSerialization::SerializeStopData(const transport_db::Stop& stop_data) {    
        transport_base::Stop tmp_stop;
        std::string s_name{ stop_data.name }; //name as string        
        tmp_stop.set_name(s_name); // set name
        tmp_stop.mutable_coords()->set_geo_lat(stop_data.geo.lat); // set coord
        tmp_stop.mutable_coords()->set_geo_lng(stop_data.geo.lng); // set coord   
        return tmp_stop;
    }

    transport_base::Bus TransportSerialization::SerializeBusData(const transport_db::Bus& bus_data, const std::map<std::string_view, int>& stops_ind) {     
        transport_base::Bus tmp_bus;
        std::string s_name{ bus_data.name }; //name as string
        tmp_bus.set_name(s_name);
        for (std::string_view stop : bus_data.stops) { // accumulate vector of stop indexes in PROTO
            tmp_bus.add_stop_index(stops_ind.at(stop));
        }
        tmp_bus.set_round_trip(bus_data.is_round_trip); // set route type
        tmp_bus.set_end_stop_ind(stops_ind.at(bus_data.end_stop)); // set ind of end stop        
        return tmp_bus;
    }

    transport_base::Dist TransportSerialization::SerializeDistanceData(const std::pair<std::string_view, std::string_view> stops, int length, const std::map<std::string_view, int>& stops_ind) {
        transport_base::Dist tmp_dist;
        tmp_dist.set_from(stops_ind.at(stops.first)); // set ind of stop
        tmp_dist.set_to(stops_ind.at(stops.second)); // set ind of stop
        tmp_dist.set_distance(length); // set length    
        return tmp_dist;
    }
    /// MAP RENDERER
    transport_base::MapRenderer TransportSerialization::SerializeMapRendererData() {
        transport_base::MapRenderer tmp_map_renderer;

        *tmp_map_renderer.mutable_settings() = std::move(SerializeRenderSettingsData());

        return tmp_map_renderer;
    }

    transport_base::RenderSettings TransportSerialization::SerializeRenderSettingsData() {
        transport_base::RenderSettings tmp_render_settings;
        transport_db::RenderSettings cat_rend_set = renderer_.GetRenderSettings();
        tmp_render_settings.set_width(cat_rend_set.width);
        tmp_render_settings.set_height(cat_rend_set.height);
        tmp_render_settings.set_padding(cat_rend_set.padding);
        tmp_render_settings.set_line_width(cat_rend_set.line_width);
        tmp_render_settings.set_stop_radius(cat_rend_set.stop_radius);
        tmp_render_settings.set_bus_label_font_size(cat_rend_set.bus_label_font_size);
        tmp_render_settings.add_bus_label_offset(cat_rend_set.bus_label_offset[0]);
        tmp_render_settings.add_bus_label_offset(cat_rend_set.bus_label_offset[1]);
        tmp_render_settings.set_stop_label_font_size(cat_rend_set.stop_label_font_size);
        tmp_render_settings.add_stop_label_offset(cat_rend_set.stop_label_offset[0]);
        tmp_render_settings.add_stop_label_offset(cat_rend_set.stop_label_offset[1]);
        *tmp_render_settings.mutable_underlayer_color() = SerializeColorData(cat_rend_set.underlayer_color);
        tmp_render_settings.set_underlayer_width(cat_rend_set.underlayer_width);

        for (int i = 0; i < cat_rend_set.color_palette.size(); ++i) {
            tmp_render_settings.add_color_palette();
            *tmp_render_settings.mutable_color_palette(i) = SerializeColorData(cat_rend_set.color_palette[i]);           
        }

        return tmp_render_settings;
    }
    
    transport_base::Color TransportSerialization::SerializeColorData(const svg::Color& catalogue_color) {
        transport_base::Color tmp_color;
        if (std::holds_alternative<std::monostate>(catalogue_color)) {
            return {};
        }
        else if (std::holds_alternative<std::string>(catalogue_color)) {
            tmp_color.set_string_name(std::get<std::string>(catalogue_color));
        }
        else if (std::holds_alternative<svg::Rgb>(catalogue_color)) {
            svg::Rgb tmp_rgb_color = std::get<svg::Rgb>(catalogue_color);
            tmp_color.mutable_rgb()->set_r(tmp_rgb_color.red);
            tmp_color.mutable_rgb()->set_g(tmp_rgb_color.green);
            tmp_color.mutable_rgb()->set_b(tmp_rgb_color.blue);
        }
        else {
            svg::Rgba tmp_rgba_color = std::get<svg::Rgba>(catalogue_color);
            tmp_color.mutable_rgba()->set_r(tmp_rgba_color.red);
            tmp_color.mutable_rgba()->set_g(tmp_rgba_color.green);
            tmp_color.mutable_rgba()->set_b(tmp_rgba_color.blue);
            tmp_color.mutable_rgba()->set_opacity(tmp_rgba_color.opacity);
        }
        return tmp_color;
    }

    /// TRASNPORT ROUTER
    transport_base::TransportRouter TransportSerialization::SerializeTransportRouterData() {
        transport_base::TransportRouter tmp_transp_router;

        *tmp_transp_router.mutable_settings() = std::move(SerializeRouterSettingsData());
        *tmp_transp_router.mutable_transport_router() = std::move(SerializeTransportRouterClassData());
        *tmp_transp_router.mutable_router() = std::move(SerializeRouterData());
        *tmp_transp_router.mutable_graph() = std::move(SerializeGraphData());

        return tmp_transp_router;
    }

    transport_base::RouterSettings TransportSerialization::SerializeRouterSettingsData() {
        transport_base::RouterSettings tmp_router_settings;
        transport_db::RouterSettings cat_router_set = router_.GetSettings();
        tmp_router_settings.set_bus_velocity_kmh(cat_router_set.bus_velocity_kmh); // CCONVERT ROUTER SETTINGS
        tmp_router_settings.set_bus_wait_time(cat_router_set.bus_wait_time); // CCONVERT ROUTER SETTINGS
        return tmp_router_settings;
    }

    transport_base::TransportRouterData TransportSerialization::SerializeTransportRouterClassData() {
        transport_base::TransportRouterData tmp_transp_router_class_data;
        int i = 0; // counter
        for (const auto& [name, data] : *router_.GetVertexes()) { //CONVERTING VERTEXES (name and input ID only) - necessary data for BuildRoute function
            tmp_transp_router_class_data.add_vertexes(); //ADD NEW VERTEX            
            tmp_transp_router_class_data.mutable_vertexes(i)->set_stop_id(ser_stops_ind.at(name)); //FILL NEW VERTEX
            tmp_transp_router_class_data.mutable_vertexes(i)->set_id(data.in.id); //FILL NEW VERTEX
            ++i;
        }
        int j = 0;
        for (const auto& edge : *router_.GetEdgesData()) { //CONVERTING EDGES 
            tmp_transp_router_class_data.add_edges();
            tmp_transp_router_class_data.mutable_edges(j)->set_type(static_cast<int>(edge.type));
            if (edge.type == transport_router::edge_type::WAIT) { // WAIT is equal to STOP
                tmp_transp_router_class_data.mutable_edges(j)->set_name_id(ser_stops_ind.at(edge.name));
            }
            else {
                tmp_transp_router_class_data.mutable_edges(j)->set_name_id(ser_buses_ind.at(edge.name));
            }
            tmp_transp_router_class_data.mutable_edges(j)->set_span_count(edge.span_count);
            tmp_transp_router_class_data.mutable_edges(j)->set_time(edge.time);
            ++j;
        }
        return tmp_transp_router_class_data;
    }

    transport_base::Router TransportSerialization::SerializeRouterData() {
        transport_base::Router tmp_router;
        int i = 0;
        for (const auto& data_vector : router_.GetRouterData()) {
            int j = 0;
            tmp_router.add_routes_internal_data();
            for (const auto& data : data_vector) {                
                tmp_router.mutable_routes_internal_data(i)->add_route_internal_data_elem();
                if (data) { // CHECK IF std::optional<RouteInternalData exists in catalogue
                    transport_base::RouteInternalData& elem_data = *tmp_router.mutable_routes_internal_data(i)->mutable_route_internal_data_elem(j)->mutable_data(); // tmp element
                    elem_data.set_weight(data.value().weight); // weight is always defined
                    if (data.value().prev_edge) { // check if elem has prev_edge
                        elem_data.set_edgeid(data.value().prev_edge.value()); // assign prev_edge ID
                    }
                }
                ++j;
            }
            ++i;
        }
        return tmp_router;
    }
    
    transport_base::Graph TransportSerialization::SerializeGraphData() {
        transport_base::Graph tmp_graph;
        for (int i = 0; i < router_.GetGraph().GetEdgeCount(); ++i) { // convert graph's vector edges_            
            graph::Edge tmp_cat_edge = router_.GetGraph().GetEdge(i); // catalogue Edge
            tmp_graph.add_edges(); // create new elem in base
            transport_base::Edge& tmp_base_edge = *tmp_graph.mutable_edges(i); // base Edge
            tmp_base_edge.set_from(tmp_cat_edge.from); // assign values
            tmp_base_edge.set_to(tmp_cat_edge.to);
            tmp_base_edge.set_weight(tmp_cat_edge.weight);
        }
        for (size_t i = 0; i < router_.GetGraph().GetVertexCount(); ++i) {
            tmp_graph.add_incidence_lists();
            transport_base::IncidenceList& tmp_base_inc_list = *tmp_graph.mutable_incidence_lists(i);
            int j = 0; // counter
            for (const auto inc_edge : router_.GetGraph().GetIncidentEdges(i)) { // convert incedence_list of X edge
                tmp_base_inc_list.add_edges(inc_edge);
                ++j;
            }
        }
        return tmp_graph;
    }

/// DESERIALIZATION
    //Solution for full data serialization
    void TransportSerialization::DeserializeBase() {
        std::filesystem::path path = settings_.file_name;
        std::ifstream in_file(path, std::ios::binary);

        transport_base::TransportCatalogue base;
        base.ParseFromIstream(&in_file);        

        DeserializeCatalogueData(base.catalogue());        
        DeserializeMapRendererData(base.map_renderer());
        router_.GenerateEmptyRouter(); // create link between graph - router classes
        DeserializeTransportRouterData(base.transport_router());
        DeserializeRouterData(base.transport_router().router());
        DeserializeGraphData(base.transport_router().graph());
    }        
    /// CATALOGUE
    void TransportSerialization::DeserializeCatalogueData(const transport_base::Catalogue& base) {        
        for (int i = 0; i < base.stops_size(); ++i) { // EXTRACT STOPS FROM PROTO            
            catalogue_.AddStop(base.stops(i).name(), base.stops(i).coords().geo_lat(), base.stops(i).coords().geo_lng());
            deser_stops_ind[i] = catalogue_.SearchStop(base.stops(i).name())->name;
        }         
        for (int i = 0; i < base.buses_size(); ++i) { // EXTRACT BUSES FROM PROTO
            std::vector<std::string_view> stops;
            for (int j = 0; j < base.buses(i).stop_index_size(); ++j) { // PREPARE VECTOR OF ROUTE STOPS
                stops.push_back(deser_stops_ind.at(base.buses(i).stop_index(j)));
            }            
            catalogue_.AddRoute(base.buses(i).name(), stops, base.buses(i).round_trip(), deser_stops_ind.at(base.buses(i).end_stop_ind()));
            deser_buses_ind[i] = catalogue_.SearchRoute(base.buses(i).name())->name;
        }
        for (int i = 0; i < base.distances_size(); ++i) { // EXTRACT DISTANCES BTW STOPS FROM PROTO
            catalogue_.SetDistBtwStops(deser_stops_ind.at(base.distances(i).from()), deser_stops_ind.at(base.distances(i).to()), base.distances(i).distance());
        }
    }

    /// MAP RENDERER
    void TransportSerialization::DeserializeMapRendererData(const transport_base::MapRenderer& base_map_renderer) {
        renderer_.SetSettings(DeserializeRenderSettingsData(base_map_renderer.settings()));
    }

    transport_db::RenderSettings TransportSerialization::DeserializeRenderSettingsData(const transport_base::RenderSettings& base_render_settings) {
        transport_db::RenderSettings tmp_settings;
        tmp_settings.width = base_render_settings.width();
        tmp_settings.height = base_render_settings.height();
        tmp_settings.padding = base_render_settings.padding();
        tmp_settings.line_width = base_render_settings.line_width();
        tmp_settings.stop_radius = base_render_settings.stop_radius();
        tmp_settings.bus_label_font_size = base_render_settings.bus_label_font_size();
        tmp_settings.bus_label_offset[0] = base_render_settings.bus_label_offset(0);
        tmp_settings.bus_label_offset[1] = base_render_settings.bus_label_offset(1);
        tmp_settings.stop_label_font_size = base_render_settings.stop_label_font_size();
        tmp_settings.stop_label_offset[0] = base_render_settings.stop_label_offset(0);
        tmp_settings.stop_label_offset[1] = base_render_settings.stop_label_offset(1);
        tmp_settings.underlayer_color = DeserializeColorData(base_render_settings.underlayer_color());
        tmp_settings.underlayer_width = base_render_settings.underlayer_width();
        tmp_settings.color_palette.reserve(base_render_settings.color_palette_size());
        for (int i = 0; i < base_render_settings.color_palette_size(); ++i) {
            tmp_settings.color_palette.emplace_back(std::move(DeserializeColorData(base_render_settings.color_palette(i))));
        }
        return tmp_settings;
    }

    svg::Color TransportSerialization::DeserializeColorData(const transport_base::Color& base_color) {
        svg::Color empty_color{};
        switch (base_color.color_case()) {
        case transport_base::Color::ColorCase::COLOR_NOT_SET:
            return  empty_color;
            break;
        case transport_base::Color::ColorCase::kStringName:
            return base_color.string_name();
            break;
        case transport_base::Color::ColorCase::kRgb:
            return svg::Rgb(base_color.rgb().r(), base_color.rgb().g(), base_color.rgb().b());
            break;
        case transport_base::Color::ColorCase::kRgba:
            return svg::Rgba(base_color.rgba().r(), base_color.rgba().g(), base_color.rgba().b(), base_color.rgba().opacity());
            break;
        }
        return  empty_color;
    }

    /// TRASNPORT ROUTER
    void TransportSerialization::DeserializeTransportRouterData(const transport_base::TransportRouter& base_transport_router) {
        router_.SetSettings(DeserializeTrasnportRouterSettingsData(base_transport_router.settings()));
        DeserializeTransportRouterClassData(base_transport_router.transport_router());
        DeserializeGraphData(base_transport_router.graph());
    }
        /// TRANSPORT ROUTER CLASS
    transport_router::RouterSettings TransportSerialization::DeserializeTrasnportRouterSettingsData(const transport_base::RouterSettings& base_router_settings ) {
        transport_router::RouterSettings tmp_settings;        
        tmp_settings.bus_velocity_kmh = base_router_settings.bus_velocity_kmh();
        tmp_settings.bus_wait_time = base_router_settings.bus_wait_time();
        return tmp_settings;
    }    
    
    void TransportSerialization::DeserializeTransportRouterClassData(const transport_base::TransportRouterData& base_transport_router_data) {                   
        router_.ModifyVertexes() = std::move(DeserializeTranspRouterVertexesData(base_transport_router_data));// get link to data in transport route
        router_.ModifyEdgesData() = DeserializeTranspRouterEdgesData(base_transport_router_data); // get link to data in transport router        
    }

    TransportSerialization::Vertexes TransportSerialization::DeserializeTranspRouterVertexesData(const transport_base::TransportRouterData& base_transport_router_data) {
        Vertexes tmp_vertexes;
        for (int i = 0; i < base_transport_router_data.vertexes_size(); ++i) {
            tmp_vertexes[deser_stops_ind.at(base_transport_router_data.vertexes(i).stop_id())].in.id = base_transport_router_data.vertexes(i).id();
        }
        return tmp_vertexes;
    }

    TransportSerialization::Edges TransportSerialization::DeserializeTranspRouterEdgesData(const transport_base::TransportRouterData& base_transport_router_data) {
        Edges tmp_edges;
        tmp_edges.reserve(base_transport_router_data.edges_size());
        for (int i = 0; i < base_transport_router_data.edges_size(); ++i) {
            transport_router::Edges tmp_edge;
            switch (base_transport_router_data.edges(i).type()) {
            case 0:
                tmp_edge.type = transport_router::edge_type::WAIT;
                tmp_edge.name = deser_stops_ind.at(base_transport_router_data.edges(i).name_id());
                break;
            case 1:
                tmp_edge.type = transport_router::edge_type::BUS;
                tmp_edge.name = deser_buses_ind.at(base_transport_router_data.edges(i).name_id());
                break;
            }
            tmp_edge.time = base_transport_router_data.edges(i).time();
            tmp_edge.span_count = base_transport_router_data.edges(i).span_count();
            tmp_edges.emplace_back(std::move(tmp_edge));
        }
        return tmp_edges;
    }    
        /// ROUTER
    void TransportSerialization::DeserializeRouterData(const transport_base::Router& base_router) {
        std::vector<std::vector<std::optional<graph::Router<double>::RouteInternalData>>>& routes_internal_data = router_.ModifyRouter().get()->ModifyRoutesInternalData();
        routes_internal_data.resize(base_router.routes_internal_data_size());
        for (int i = 0; i < base_router.routes_internal_data_size(); ++i) {
            routes_internal_data[i].reserve(base_router.routes_internal_data(i).route_internal_data_elem_size());
            for (int j = 0; j < base_router.routes_internal_data(i).route_internal_data_elem_size(); ++j) {
                transport_base::RouteInternalDataVectorElem base_elem = base_router.routes_internal_data(i).route_internal_data_elem(j);
                routes_internal_data[i].emplace_back(std::move(DeserializeRouteInternalData(base_elem)));
            }
        }
    }

    std::optional<graph::Router<double>::RouteInternalData> TransportSerialization::DeserializeRouteInternalData(transport_base::RouteInternalDataVectorElem& base) {
        graph::Router<double>::RouteInternalData res{};
        switch (base.elem_case()) {
        case transport_base::RouteInternalDataVectorElem::ElemCase::ELEM_NOT_SET:            
            return std::nullopt;
            break;
        case transport_base::RouteInternalDataVectorElem::ElemCase::kData:
            res.weight = base.data().weight();
            switch (base.data().prev_edge_case()) {
            case transport_base::RouteInternalData::PrevEdgeCase::kEdgeid:
                res.prev_edge = std::make_optional(base.data().edgeid());
                break;
            case transport_base::RouteInternalData::PrevEdgeCase::PREV_EDGE_NOT_SET:
                res.prev_edge = std::nullopt;
                break;
            }        
        }
        return res;
    }
        /// GRAPH
        void TransportSerialization::DeserializeGraphData(const transport_base::Graph & base_graph_data) {
        router_.ModifyGraph().ModifyEdges() = std::move(DeserializeGraphEdgesData(base_graph_data));
        router_.ModifyGraph().ModifyIncidenceLists() = std::move(DeserializeGraphIncidenceListsData(base_graph_data));
    }

    std::vector<graph::Edge<double>> TransportSerialization::DeserializeGraphEdgesData(const transport_base::Graph& base_graph_data) {
        std::vector<graph::Edge<double>> tmp_edges;;
        tmp_edges.reserve(base_graph_data.edges_size());
        for (int i = 0; i < base_graph_data.edges_size(); ++i) {
            graph::Edge<double> tmp_edge;
            tmp_edge.from = base_graph_data.edges(i).from();
            tmp_edge.to = base_graph_data.edges(i).to();
            tmp_edge.weight = base_graph_data.edges(i).weight();
            tmp_edges.emplace_back(std::move(tmp_edge));
        }
        return tmp_edges;
    }

    std::vector<graph::IncidenceList> TransportSerialization::DeserializeGraphIncidenceListsData(const transport_base::Graph& base_graph_data) {
        std::vector<graph::IncidenceList> tmp_inc_lists;
        tmp_inc_lists.reserve(base_graph_data.incidence_lists_size());
        for (int i = 0; i < base_graph_data.incidence_lists_size(); ++i) {
            graph::IncidenceList tmp_list;
            tmp_list.reserve(base_graph_data.incidence_lists(i).edges_size());
            for (int j = 0; j < base_graph_data.incidence_lists(i).edges_size(); ++j) {
                tmp_list.emplace_back(base_graph_data.incidence_lists(i).edges(j));
            }
            tmp_inc_lists.emplace_back(std::move(tmp_list));
        }
        return tmp_inc_lists;
    }
}
    