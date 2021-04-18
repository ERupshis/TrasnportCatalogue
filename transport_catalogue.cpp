#include "transport_catalogue.h"

#include <iomanip>

namespace transport_db {
	void TransportCatalogue::AddRoute(std::string& name, const std::vector<std::string>& data, bool is_round, const std::string& end_stop) {
		std::string& ref_to_name = buses_names_.emplace_back(std::move(name)); // add string in deque
		std::string_view sv_name{ ref_to_name }; // get reference to value in deque
		std::vector<std::string_view> v_stops;
		for (const std::string& stop : data) {
			std::string_view sv_stop{ stop };
			v_stops.emplace_back(stops_[sv_stop].name);
			stops_[sv_stop].buses.insert(sv_name); // add this bus in containers of all stops on route
		}
		routes_[sv_name] = { sv_name, v_stops, is_round, stops_.at(end_stop).name };
	}

	const Bus* TransportCatalogue::SearchRoute(std::string_view name) const {
		if (!routes_.count(name)) {
			return &empty_bus; //is it requred?
		}
		return &routes_.at(name); // return pointer to BUS data
	}

	TransportCatalogue::RouteOutput TransportCatalogue::GetRoute(std::string_view name) {
		RouteOutput result;
		result.name = name.substr(0, name.size());
		if (!routes_.count(name)) { // if route doesn't exist - add "!" for processing it in ostream << overload 
			result.name.insert(0, "!");
			return result;
		}
		else {
			const Bus tmp = *SearchRoute(name);
			result.real_stops_count = tmp.stops.size();
			std::set<std::string_view> unique_stops{ tmp.stops.begin(), tmp.stops.end() }; // calculate unique stops via set container
			result.unique_stops_count = unique_stops.size();
			double road_distance = 0;
			double geo_distance = 0;
			for (auto it = tmp.stops.begin(); it < prev(tmp.stops.end()); ++it) { // cycle from begin() to (end()-1)
				road_distance += GetDistBtwStops((*it), *next(it));
				geo_distance += ComputeDistance(SearchStop(*it)->geo, SearchStop(*(it + 1))->geo);
			}
			result.route_length = road_distance;
			result.curvature = road_distance / geo_distance;
			return result;
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////	
	void  TransportCatalogue::AddStop(std::string name, double lat, double lng) {
		std::string& ref_to_name = stops_names_.emplace_back(std::move(name)); // add string in deque
		std::string_view sv_name{ ref_to_name }; // get reference to value in deque	
		geo::Coordinates geo = { lat, lng };
		stops_[sv_name] = { sv_name, geo, {} }; // create unique STOP without distances to other stops (1st stage of adding STOPs)
	}

	const  Stop* TransportCatalogue::SearchStop(std::string_view name) const {
		if (!stops_.count(name)) {
			return &empty_stop; //is it requred?
		}
		return &stops_.at(name); // return pointer to STOP data
	}

	TransportCatalogue::StopOutput  TransportCatalogue::GetStop(std::string_view name) {
		StopOutput result;
		result.name = name.substr(0, name.size());
		if (!stops_.count(name)) { // if STOP doesn't exist - add "!" for processing it in ostream << overload 
			result.name.insert(0, "!");
			return result;
		}
		else { // if STOP exist - send set of unique buses for STOP in ostream << overload
			const Stop tmp = *SearchStop(name);
			result.buses = stops_[name].buses;
			return result;
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	void  TransportCatalogue::SetDistBtwStops(std::string_view name, std::string_view name_to, const int dist) {
		const Stop stop = *SearchStop(name); // get data by string_view	 through pointers
		const Stop stop_to = *SearchStop(name_to);
		dist_btw_stops_[std::pair(stop.name, stop_to.name)] = dist; // call distance calculation
	}

	int  TransportCatalogue::GetDistBtwStops(std::string_view name, std::string_view name_to) {
		if (dist_btw_stops_.count(std::pair(name, name_to))) {
			return dist_btw_stops_.at(std::pair(name, name_to));
		}
		else /*if (dist_btw_stops_.count(std::pair(name_to, name)))*/ { // Check reverse combination of STOP's names
			return dist_btw_stops_.at(std::pair(name_to, name));
		}		
	}
}	
	