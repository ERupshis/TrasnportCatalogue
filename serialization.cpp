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
/////SERIALIZATION//////////////////////////////////
    //Solution for full data serialization
    void TransportSerialization::SerializeBase() {
        transport_base::TransportCatalogue base;
        
        ConvertCatalog(base);        
        SetSettingsForBase(base); 
        ConvertTransportRouter(base);
        ConvertRouter(base);
        ConvertGraph(base);    
        
        std::filesystem::path path = settings_.file_name;
        std::ofstream out_file(path, std::ios::binary);
        
        base.SerializeToOstream(&out_file);
    }

   /*  //Solution for trasnport_catalogue data + router settings serialization
    void TransportSerialization::SerializeBase() {
        transport_base::TransportCatalogue base;

        ConvertCatalog(base);
        SetSettingsForBase(base);
        
        std::filesystem::path path = settings_.file_name;
        std::ofstream out_file(path, std::ios::binary);        
        base.SerializeToOstream(&out_file);
    }
    */  

    void TransportSerialization::ConvertCatalog(transport_base::TransportCatalogue& base) {        
        int s = 0; // stop index in proto
        for (auto& [stop_name, stop_data] : catalogue_.GetStopsForRender()) { // ADD STOPS IN PROTO
            ser_stops_ind[stop_name] = s;
            base.add_stops(); // add element in array of proto
            ConvertStop(stop_data, *base.mutable_stops(s)); // modify new element
            ++s;
        }
        int b = 0; // bus index in proto
        for (auto& [bus_name, bus_data] : catalogue_.GetRoutesForRender()) { // ADD BUSES IN PROTO
            ser_buses_ind[bus_name] = b;
            base.add_buses(); // add element in array of proto
            ConvertBus(bus_data, ser_stops_ind, *base.mutable_buses(b)); // modify new element
            ++b;
        }
        int d = 0; // dist counter
        for (auto& [stops, length] : catalogue_.GetDistForRouter()) { // ADD DISTANCES BTW STOPS IN PROTO
            base.add_distances();          // add element in array of proto   
            ConvertDist(stops, length, ser_stops_ind, *base.mutable_distances(d)); // modify new element
            ++d;
        }
    }

    void TransportSerialization::ConvertStop(const transport_db::Stop& stop_data, transport_base::Stop& base_stop) {        
        std::string s_name{ stop_data.name }; //name as string        
        base_stop.set_name(s_name); // set name
        base_stop.set_geo_lat(stop_data.geo.lat); // set coord
        base_stop.set_geo_lng(stop_data.geo.lng); // set coord        
    }

    void TransportSerialization::ConvertBus(const transport_db::Bus& bus_data, const std::map<std::string_view, int>& stops_ind, transport_base::Bus& base_bus) {        
        std::string s_name{ bus_data.name }; //name as string
        base_bus.set_name(s_name);
        for (std::string_view stop : bus_data.stops) { // accumulate vector of stop indexes in PROTO
            base_bus.add_stop_index(stops_ind.at(stop));
        }
        base_bus.set_round_trip(bus_data.is_round_trip); // set route type
        base_bus.set_end_stop_ind(stops_ind.at(bus_data.end_stop)); // set ind of end stop        
    }

    void TransportSerialization::ConvertDist( const std::pair<std::string_view, std::string_view> stops, int length, const std::map<std::string_view, int>& stops_ind, transport_base::Dist& base_dist) {        
        base_dist.set_from(stops_ind.at(stops.first)); // set ind of stop
        base_dist.set_to(stops_ind.at(stops.second)); // set ind of stop
        base_dist.set_distance(length); // set length        
    }


    /////COLOR CONVERTION//////////////////////////////////
    void TransportSerialization::SetSettingsForBase(transport_base::TransportCatalogue& base) {
        transport_base::RenderSettings& base_rend_set = *base.mutable_render_settings();
        transport_db::RenderSettings cat_rend_set = renderer_.GetRenderSettings();

        base_rend_set.set_width(cat_rend_set.width);
        base_rend_set.set_height(cat_rend_set.height);
        base_rend_set.set_padding(cat_rend_set.padding);
        base_rend_set.set_line_width(cat_rend_set.line_width);
        base_rend_set.set_stop_radius(cat_rend_set.stop_radius);
        base_rend_set.set_bus_label_font_size(cat_rend_set.bus_label_font_size);
        base_rend_set.add_bus_label_offset(cat_rend_set.bus_label_offset[0]);
        base_rend_set.add_bus_label_offset(cat_rend_set.bus_label_offset[1]);
        base_rend_set.set_stop_label_font_size(cat_rend_set.stop_label_font_size);
        base_rend_set.add_stop_label_offset(cat_rend_set.stop_label_offset[0]);
        base_rend_set.add_stop_label_offset(cat_rend_set.stop_label_offset[1]);

        ConvertColor(cat_rend_set.underlayer_color, *base_rend_set.mutable_underlayer_color());
        base_rend_set.set_underlayer_width(cat_rend_set.underlayer_width);

        for (int i = 0; i < cat_rend_set.color_palette.size(); ++i) {
            base_rend_set.add_color_palette();
            ConvertColor(cat_rend_set.color_palette[i], *base_rend_set.mutable_color_palette(i));
        } 

        transport_base::RouterSettings& base_route_set = *base.mutable_transport_router()->mutable_settings();
        base_route_set.set_bus_velocity_kmh(router_.GetSettings().bus_velocity_kmh); // CCONVERT ROUTER SETTINGS
        base_route_set.set_bus_wait_time(router_.GetSettings().bus_wait_time); // CCONVERT ROUTER SETTINGS
    }

    void TransportSerialization::ConvertColor(const svg::Color& catalogue_color, transport_base::Color& base_color) {        
        if (std::holds_alternative<std::monostate>(catalogue_color)) {
            return;
        }
        else if (std::holds_alternative<std::string>(catalogue_color)) {            
            base_color.set_string_name(std::get<std::string>(catalogue_color));            
        }
        else if (std::holds_alternative<svg::Rgb>(catalogue_color)) {            
            svg::Rgb tmp_rgb_color = std::get<svg::Rgb>(catalogue_color);            
            base_color.mutable_rgb()->set_r(tmp_rgb_color.red);
            base_color.mutable_rgb()->set_g(tmp_rgb_color.green);
            base_color.mutable_rgb()->set_b(tmp_rgb_color.blue);            
        }
        else {             
            svg::Rgba tmp_rgba_color = std::get<svg::Rgba>(catalogue_color);
            base_color.mutable_rgba()->set_r(tmp_rgba_color.red);
            base_color.mutable_rgba()->set_g(tmp_rgba_color.green);
            base_color.mutable_rgba()->set_b(tmp_rgba_color.blue);
            base_color.mutable_rgba()->set_opacity(tmp_rgba_color.opacity);
        }        
    }
    /////TRANSPORT ROUTER//////////////////////////////////  
        /////TRANSPORT ROUTER CLASS//////////////////////////////////  
    void TransportSerialization::ConvertTransportRouter(transport_base::TransportCatalogue& base) {    
        transport_base::TransportRouter& transp_router = *base.mutable_transport_router();
        int i = 0; // counter
        for (const auto& [name, data] : *router_.GetVertexes()) { //CONVERTING VERTEXES (name and input ID only) - necessary data for BuildRoute function
            transp_router.add_vertexes(); //ADD NEW VERTEX            
            transp_router.mutable_vertexes(i)->set_stop_id(ser_stops_ind.at(name)); //FILL NEW VERTEX
            transp_router.mutable_vertexes(i)->set_id(data.in.id); //FILL NEW VERTEX
            ++i;
        }

        int j = 0;
        for (const auto& edge : *router_.GetEdgesData()) { //CONVERTING EDGES 
            transp_router.add_edges();  
            transp_router.mutable_edges(j)->set_type(static_cast<int>(edge.type));
            if (edge.type == transport_router::edge_type::WAIT) {
                transp_router.mutable_edges(j)->set_name_id(ser_stops_ind.at(edge.name));
            }
            else {
                transp_router.mutable_edges(j)->set_name_id(ser_buses_ind.at(edge.name));
            }            
            transp_router.mutable_edges(j)->set_span_count(edge.span_count);
            transp_router.mutable_edges(j)->set_time(edge.time);            
            ++j;
        }
    }
        /////ROUTER CLASS//////////////////////////////////  
    void TransportSerialization::ConvertRouter(transport_base::TransportCatalogue& base) {
        transport_base::Router& base_router = *base.mutable_router();
        int i = 0;
        for (const auto& data_vector : router_.GetRouterData()) {
            int j = 0;
            base_router.add_routes_internal_data();
            for (const auto& data : data_vector) {
                base_router.mutable_routes_internal_data(i)->add_route_internal_data_elem();
                if (data) { // CHECK IF std::optional<RouteInternalData exists in catalogue
                    transport_base::RouteInternalData& elem_data = *base_router.mutable_routes_internal_data(i)->mutable_route_internal_data_elem(j)->mutable_data(); // tmp element
                    elem_data.set_weight(data.value().weight); // weight is always defined
                    if (data.value().prev_edge) { // check if elem has prev_edge
                        elem_data.set_edgeid(data.value().prev_edge.value()); // assign prev_edge ID
                    }                     
                }
                ++j;                
            }
            ++i;
        }        
    }
        /////GRAPH CLASS//////////////////////////////////  
    void TransportSerialization::ConvertGraph(transport_base::TransportCatalogue& base) {
        transport_base::Graph& base_graph = *base.mutable_graph();
        for (int i = 0; i < router_.GetGraph().GetEdgeCount(); ++i) { // convert graph's vector edges_            
            graph::Edge tmp_cat_edge = router_.GetGraph().GetEdge(i); // catalogue Edge
            base_graph.add_edges(); // create new elem in base
            transport_base::Edge& tmp_base_edge = *base_graph.mutable_edges(i); // base Edge
            tmp_base_edge.set_from(tmp_cat_edge.from); // assign values
            tmp_base_edge.set_to(tmp_cat_edge.to);
            tmp_base_edge.set_weight(tmp_cat_edge.weight);  
        }

        for (size_t i = 0; i < router_.GetGraph().GetVertexCount(); ++i) {
            base_graph.add_incidence_lists();
            transport_base::IncidenceList& tmp_base_inc_list = *base_graph.mutable_incidence_lists(i);
            int j = 0; // counter
            for (const auto inc_edge : router_.GetGraph().GetIncidentEdges(i)) { // convert incedence_list of X edge
                tmp_base_inc_list.add_edges(inc_edge);                
                ++j;
            }
                        
        }        
    }

/////DESERIALIZATION//////////////////////////////////
    //Solution for full data serialization
    void TransportSerialization::DeserializeBase() {
        std::filesystem::path path = settings_.file_name;
        std::ifstream in_file(path, std::ios::binary);        

        transport_base::TransportCatalogue base;
        base.ParseFromIstream(&in_file);        

        ConvertBaseCatalogue(base);  
        router_.GenerateEmptyRouter(); // create link between graph - router classes         
        ConvertBaseTransportRouter(base); // copy transport_router data;
        ConvertBaseGraphEdges(base); // copy graph edges;
        ConvertBaseGraphIncidenceLists(base); // copy graph incidence_lists;
        ConvertBaseRouter(base);   // copy router routes_internal_data;
        
        renderer_.SetSettings(ConverBaseRenderSettings(base.render_settings())); // SET RENDER SETTINGS        
        router_.SetSettings(ConvertBaseRouterSettings(base.transport_router().settings())); // SET ROUTER SETTINGS  
        
    }    
    /* //Solution for trasnport_catalogue data + router settings serialization
    void TransportSerialization::DeserializeBase() {        
        std::filesystem::path path = settings_.file_name;
        std::ifstream in_file(path, std::ios::binary);

        transport_base::TransportCatalogue base;
        base.ParseFromIstream(&in_file);

        ConvertBaseCatalogue(base);
        renderer_.SetSettings(ConverBaseRenderSettings(base.render_settings())); // SET RENDER SETTINGS
        router_.SetSettings(ConvertBaseRouterSettings(base.transport_router().settings())); // SET ROUTER SETTINGS
        router_.GenerateRouter();
    }
    */
    void TransportSerialization::ConvertBaseCatalogue(const transport_base::TransportCatalogue& base) {        
        for (int i = 0; i < base.stops_size(); ++i) { // EXTRACT STOPS FROM PROTO            
            catalogue_.AddStop(base.stops(i).name(), base.stops(i).geo_lat(), base.stops(i).geo_lng());
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

    svg::Color TransportSerialization::ConvertBaseColor(const transport_base::Color& base_color) {
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

    transport_db::RenderSettings TransportSerialization::ConverBaseRenderSettings(const transport_base::RenderSettings& base_render_settings) {
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
        tmp_settings.underlayer_color = ConvertBaseColor(base_render_settings.underlayer_color());
        tmp_settings.underlayer_width = base_render_settings.underlayer_width();

        tmp_settings.color_palette.reserve(base_render_settings.color_palette_size());
        for (int i = 0; i < base_render_settings.color_palette_size(); ++i) {
            tmp_settings.color_palette.emplace_back(std::move(ConvertBaseColor(base_render_settings.color_palette(i))));
        }
        return tmp_settings;
    }

    transport_router::RouterSettings TransportSerialization::ConvertBaseRouterSettings(const transport_base::RouterSettings& base_router_settings ) {
        transport_router::RouterSettings tmp_settings;        
        tmp_settings.bus_velocity_kmh = base_router_settings.bus_velocity_kmh();
        tmp_settings.bus_wait_time = base_router_settings.bus_wait_time();
        return tmp_settings;
    }
    
    void TransportSerialization::ConvertBaseTransportRouter(const transport_base::TransportCatalogue& base) {        
        auto& vertexes = router_.ModifyVertexes(); // copy data from base. (only vertexes with INPUT ID) 
        for (int i = 0; i < base.transport_router().vertexes_size(); ++i) {
            vertexes[deser_stops_ind.at(base.transport_router().vertexes(i).stop_id())].in.id = base.transport_router().vertexes(i).id();
        }
        auto& edges = router_.ModifyEdgesData();// copy data from base.
        edges.reserve(base.transport_router().edges_size());
        for (int i = 0; i < base.transport_router().edges_size(); ++i) {
            transport_router::Edges tmp_edge;
            switch (base.transport_router().edges(i).type()) {
            case 0:
                tmp_edge.type = transport_router::edge_type::WAIT;
                tmp_edge.name = deser_stops_ind.at(base.transport_router().edges(i).name_id());    //catalogue_.SearchStop(base.router_data().transport_router().edges(i).name())->name;
                break;
            case 1:
                tmp_edge.type = transport_router::edge_type::BUS;
                tmp_edge.name = deser_buses_ind.at(base.transport_router().edges(i).name_id());
                break;
            }
            tmp_edge.time = base.transport_router().edges(i).time();
            tmp_edge.span_count = base.transport_router().edges(i).span_count();
            edges.push_back(std::move(tmp_edge));
        }
    }

    void TransportSerialization::ConvertBaseGraphEdges(const transport_base::TransportCatalogue& base) {
        auto& edges = router_.ModifyGraph().ModifyEdges();
        edges.reserve(base.graph().edges_size());
        for (int i = 0; i < base.graph().edges_size(); ++i) {
            graph::Edge<double> tmp_edge;
            tmp_edge.from = base.graph().edges(i).from();
            tmp_edge.to = base.graph().edges(i).to();
            tmp_edge.weight = base.graph().edges(i).weight();
            edges.emplace_back(std::move(tmp_edge));
        }
    }

    void TransportSerialization::ConvertBaseGraphIncidenceLists(const transport_base::TransportCatalogue& base) {
        auto& incidence_lists = router_.ModifyGraph().ModifyIncidenceLists();  
        incidence_lists.reserve(base.graph().incidence_lists_size());
        for (int i = 0; i < base.graph().incidence_lists_size(); ++i) {
            graph::IncidenceList tmp_list;
            tmp_list.reserve(base.graph().incidence_lists(i).edges_size());
            for (int j = 0; j < base.graph().incidence_lists(i).edges_size(); ++j) {
                tmp_list.emplace_back(base.graph().incidence_lists(i).edges(j));
            }
            incidence_lists.emplace_back(std::move(tmp_list));
        }
    }
 
    std::optional<graph::Router<double>::RouteInternalData> TransportSerialization::FillRouteInternalData(transport_base::RouteInternalDataVectorElem& base) {
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

    void TransportSerialization::ConvertBaseRouter(const transport_base::TransportCatalogue& base) {
        transport_base::Router base_router = base.router();        
        auto& routes_internal_data = router_.ModifyRouter().get()->ModifyRoutesInternalData();
        routes_internal_data.resize(base_router.routes_internal_data_size());
        for (int i = 0; i < base_router.routes_internal_data_size(); ++i) {
            routes_internal_data[i].reserve(base_router.routes_internal_data(i).route_internal_data_elem_size());
            for (int j = 0; j < base_router.routes_internal_data(i).route_internal_data_elem_size(); ++j) {
                transport_base::RouteInternalDataVectorElem base_elem = base_router.routes_internal_data(i).route_internal_data_elem(j);
                routes_internal_data[i].emplace_back(std::move(FillRouteInternalData(base_elem)));
            }            
        }
    }
}
    