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
		
		void ConvertCatalog(transport_base::TransportCatalogue& base);
		void ConvertStop(const transport_db::Stop& stop_data, transport_base::Stop& base_stop);
		void ConvertBus(const transport_db::Bus& bus_data, const std::map<std::string_view, int>& stops_ind, transport_base::Bus& base_bus);
		void ConvertDist(const std::pair<std::string_view, std::string_view> stops, int length, const std::map<std::string_view, int>& stops_ind, transport_base::Dist& base_dist);

		void ConvertTransportRouter(transport_base::TransportCatalogue& base);
		void ConvertRouter(transport_base::TransportCatalogue& base);
		void ConvertGraph(transport_base::TransportCatalogue& base);
		void ConvertColor(const svg::Color& catalogue_color, transport_base::Color& base_color);
		void SetSettingsForBase(transport_base::TransportCatalogue& base);			

		void ConvertBaseCatalogue(const transport_base::TransportCatalogue& base);
		svg::Color ConvertBaseColor(const transport_base::Color& base_color);
		transport_db::RenderSettings ConverBaseRenderSettings(const transport_base::RenderSettings& base_render_settings);
		transport_router::RouterSettings ConvertBaseRouterSettings(const transport_base::RouterSettings& base_router_settings);
		void ConvertBaseTransportRouter(const transport_base::TransportCatalogue& base);
		void ConvertBaseGraphEdges(const transport_base::TransportCatalogue& base);
		void ConvertBaseGraphIncidenceLists(const transport_base::TransportCatalogue& base);
		std::optional < graph::Router<double>::RouteInternalData> FillRouteInternalData(transport_base::RouteInternalDataVectorElem& base);
		void ConvertBaseRouter(const transport_base::TransportCatalogue& base);
	};
}