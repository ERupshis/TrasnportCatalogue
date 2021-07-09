cmake_minimum_required(VERSION 3.10)
project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)



find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS svg.proto graph.proto transport_router.proto transport_catalogue.proto map_renderer.proto)


set(CATALOGUE_FILES domain.h domain.cpp geo.h geo.cpp transport_catalogue.h transport_catalogue.cpp svg.proto graph.proto transport_router.proto transport_catalogue.proto map_renderer.proto request_handler.h request_handler.cpp serialization.h serialization.cpp main.cpp)
set(ROUTER_FILES transport_router.h transport_router.cpp graph.h ranges.h router.h)
set(RENDER_FILES svg.h svg.cpp map_renderer.h map_renderer.cpp)
set(JSON_FILES json.h json.cpp json_reader.h json_reader.cpp json_builder.h json_builder.cpp)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${CATALOGUE_FILES} ${ROUTER_FILES} ${RENDER_FILES} ${JSON_FILES})

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)