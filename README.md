# TrasnportCatalogue
Application that allows to create public transport database with routes and find shortest way from stop A to stop B. Also, transport map can be generated in SVG format.

Interaction with database via requests and answers are made in JSON format. Map generation is performed in SVG format. 
Database storage is performed via data serialization in Google protobuf.

# Data which is used in transport catalogue:
  - Bus(route) data:
    - name 
    - type 
    - stops on route
  - Stop data: 
    - name 
    - location coordinates 
    - list of distances to other stops (road length)

# Requests to database:
  - Add new data in db (base requests):
    - new bus route
    ```json
    "base_requests": [
      {
        "type": "Bus",
        "name": "24",
        "stops": [
          "Улица Докучаева",
          "Параллельная улица",
          "Электросети",
          "Санаторий Родина"
         ],
         "is_roundtrip": false
       },
    ```
    - new stop
    ```json
      {
        "type": "Stop",
        "name": "Улица Лизы Чайкиной",
        "latitude": 43.590317,
        "longitude": 39.746833,
        "road_distances": {
        "Электросети": 4300,
        "Улица Докучаева": 2000
        }
      }
    ]
    ```
  - Get data from db (stat request):
    - route info
    ```json
    "stat_requests": [
      {
        "id": 1,
        "type": "Bus",
        "name": "14"
      },
     ```
    - stop info
    ```json
      {
        "id": 2,
        "type": "Stop",
        "name": "Электросети"
      },
    ```
    - find route from stop A to stop B
    ```json
      {
        "id": 3,
        "type": "Route",
        "from": "Морской вокзал",
        "to": "Параллельная улица"
       },
     ```
    - generate map in SVG  
    ```json
      {
        "id": 4,
        "type": "Map"
      }
    ]
    ```
    
    # Database responce
    - Examples of db answers:
      - bus(route) info request
      ```json
      [
        {
          "curvature": 1.60481,
          "request_id": 1,
          "route_length": 11230,
          "stop_count": 8,
          "unique_stop_count": 7
        },
      ```
      - stop info request:
      ```json
        {
          "buses": [
            "14",
            "24"
          ],
          "request_id": 2
        },
      ```
      - generate route request (shortest way)
      ```json
        {
          "items": [
            {
              "stop_name": "Морской вокзал",
              "time": 2,
              "type": "Wait"
            },
            {
              "bus": "114",
              "span_count": 1,
              "time": 1.7,
              "type": "Bus"
            },
            ...
            {
              "bus": "24",
              "span_count": 1,
              "time": 2.2,
              "type": "Bus"
            }
          ],
          "request_id": 3,
          "total_time": 15.96
        },
      ```
      - map generation request
      ```json
        {
          "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n  <polyline points=\"125.25,382.708 74.2702,281.925 125.25,382.708\" fill=\"none\" stroke=\"green\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <polyline points=\"592.058,238.297 311.644,93.2643 74.2702,281.925 267.446,450 317.457,442.562 365.599,429.138 367.969,320.138 592.058,238.297\" fill=\"none\" stroke=\"rgb(255,160,0)\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <polyline points=\"367.969,320.138 350.791,243.072 311.644,93.2643 50,50 311.644,93.2643 350.791,243.072 367.969,320.138\" fill=\"none\" stroke=\"red\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"125.25\" y=\"382.708\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"125.25\" y=\"382.708\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"74.2702\" y=\"281.925\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"74.2702\" y=\"281.925\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"592.058\" y=\"238.297\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">14</text>\n  <text fill=\"rgb(255,160,0)\" x=\"592.058\" y=\"238.297\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">14</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"367.969\" y=\"320.138\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">24</text>\n  <text fill=\"red\" x=\"367.969\" y=\"320.138\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">24</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"50\" y=\"50\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">24</text>\n  <text fill=\"red\" x=\"50\" y=\"50\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">24</text>\n  <circle cx=\"267.446\" cy=\"450\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"317.457\" cy=\"442.562\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"125.25\" cy=\"382.708\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"350.791\" cy=\"243.072\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"365.599\" cy=\"429.138\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"74.2702\" cy=\"281.925\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"50\" cy=\"50\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"367.969\" cy=\"320.138\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"592.058\" cy=\"238.297\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"311.644\" cy=\"93.2643\" r=\"5\" fill=\"white\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"267.446\" y=\"450\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Гостиница Сочи</text>\n  <text fill=\"black\" x=\"267.446\" y=\"450\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Гостиница Сочи</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"317.457\" y=\"442.562\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Кубанская улица</text>\n  <text fill=\"black\" x=\"317.457\" y=\"442.562\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Кубанская улица</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"125.25\" y=\"382.708\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"black\" x=\"125.25\" y=\"382.708\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"350.791\" y=\"243.072\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Параллельная улица</text>\n  <text fill=\"black\" x=\"350.791\" y=\"243.072\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Параллельная улица</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"365.599\" y=\"429.138\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">По требованию</text>\n  <text fill=\"black\" x=\"365.599\" y=\"429.138\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">По требованию</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"74.2702\" y=\"281.925\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Ривьерский мост</text>\n  <text fill=\"black\" x=\"74.2702\" y=\"281.925\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Ривьерский мост</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"50\" y=\"50\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Санаторий Родина</text>\n  <text fill=\"black\" x=\"50\" y=\"50\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Санаторий Родина</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"367.969\" y=\"320.138\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Улица Докучаева</text>\n  <text fill=\"black\" x=\"367.969\" y=\"320.138\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Улица Докучаева</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"592.058\" y=\"238.297\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Улица Лизы Чайкиной</text>\n  <text fill=\"black\" x=\"592.058\" y=\"238.297\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Улица Лизы Чайкиной</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"311.644\" y=\"93.2643\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Электросети</text>\n  <text fill=\"black\" x=\"311.644\" y=\"93.2643\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Электросети</text>\n</svg>",
          "request_id": 4
        }      
      ]
      ```
      
Example of request and answer to db you can find in example folder
request to db are performed via cin & cout streams.
