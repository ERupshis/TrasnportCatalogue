#pragma once

#include <cmath>

namespace transport_db {
    namespace geo {
        struct Coordinates {
            double lat;
            double lng;
        };

        double ComputeDistance(Coordinates from, Coordinates to);
    }
}