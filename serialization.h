#pragma once

#include <string>
#include <transport_catalogue.pb.h> 
 
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

using namespace std::literals;

namespace transport_db {
	struct SerializationSettings {
		std::string file_name = ""s;
	};
}

namespace transport_base {
	class TransportSerialization {
	public:
		TransportSerialization(transport_db::TransportCatalogue& catalogue, map_renderer::MapRenderer& renderer, transport_router::TransportRouter& router)
			:catalogue_(catalogue), renderer_(renderer), router_(router) {
		}		

		void SetSettings(const transport_db::SerializationSettings& settings);

		void SerializeBase();
		void DeserializeBase();

	private:
		transport_db::SerializationSettings settings_;
		transport_db::TransportCatalogue& catalogue_;
		map_renderer::MapRenderer& renderer_;
		transport_router::TransportRouter& router_;

		std::map<std::string_view, int> ser_stops_ind;// indexes for serialization
		std::map<std::string_view, int> ser_buses_ind; // indexes for serialization

		std::map<int, std::string_view> deser_stops_ind;// indexes for deserialization
		std::map<int, std::string_view> deser_buses_ind; // indexes for deserialization

		/// Serialization
			/// Catalogue
		transport_base::Catalogue SerializeCatalogueData();
		transport_base::Stop SerializeStopData(const transport_db::Stop& stop_data);
		transport_base::Bus SerializeBusData(const transport_db::Bus& bus_data, const std::map<std::string_view, int>& stops_ind);
		transport_base::Dist SerializeDistanceData(const std::pair<std::string_view, std::string_view> stops, int length, const std::map<std::string_view, int>& stops_ind);
			/// MapRenderer
		transport_base::MapRenderer SerializeMapRendererData();
		transport_base::RenderSettings SerializeRenderSettingsData();
		transport_base::Color SerializeColorData(const svg::Color& catalogue_color);
			/// TransportRouter
		transport_base::TransportRouter SerializeTransportRouterData();
		transport_base::RouterSettings SerializeRouterSettingsData();
		transport_base::TransportRouterData SerializeTransportRouterClassData();
		transport_base::Router SerializeRouterData();
		transport_base::Graph SerializeGraphData();

		/// Deserialization
			/// Catalogue
		void DeserializeCatalogueData(const transport_base::Catalogue& base);
			/// MapRenderer
		void DeserializeMapRendererData(const transport_base::MapRenderer& base_map_renderer);
		transport_db::RenderSettings DeserializeRenderSettingsData(const transport_base::RenderSettings& base_render_settings);
		svg::Color DeserializeColorData(const transport_base::Color& base_color);
			/// TransportRouter				
		void DeserializeTransportRouterData(const transport_base::TransportRouter& base_transport_router);
		transport_router::RouterSettings DeserializeTrasnportRouterSettingsData(const transport_base::RouterSettings& base_router_settings);
				/// Transport router class
		void DeserializeTransportRouterClassData(const transport_base::TransportRouterData& base);
		using Vertexes = std::map<std::string_view, transport_router::StopAsVertexes>;		
		Vertexes DeserializeTranspRouterVertexesData(const transport_base::TransportRouterData& base_transport_router_data);
		using Edges = std::vector<transport_router::Edges>;
		Edges DeserializeTranspRouterEdgesData(const transport_base::TransportRouterData& base_transport_router_data);
				/// Graph
		void DeserializeGraphData(const transport_base::Graph& base_graph_data);
		std::vector<graph::Edge<double>> DeserializeGraphEdgesData(const transport_base::Graph& base_graph_data);
		std::vector<graph::IncidenceList> DeserializeGraphIncidenceListsData(const transport_base::Graph& base_graph_data);
				/// Router
		void DeserializeRouterData(const transport_base::Router& base_router);
		std::optional<graph::Router<double>::RouteInternalData> DeserializeRouteInternalData(transport_base::RouteInternalDataVectorElem& base);
		
	};
}