#pragma once

#include "geo.h"
#include <string_view>
#include <vector>
#include <string>
#include <set>

namespace transport_db {
	namespace domain {
		struct Stop {
			std::string_view name = {}; // stop name
			geo::Coordinates geo = { {}, {} };
			std::set<std::string_view> buses; // set of buses for STOP
			bool operator==(const Stop& other) const {
				return name == other.name;
			}
		};

		struct Bus {
			std::string_view name;  // route name
			std::vector<std::string_view> stops; // route's STOPS
			bool is_round_trip = false;
			std::string_view end_stop;
			bool operator==(const Bus& other) const {
				return name == other.name;
			}
		};
	}
}