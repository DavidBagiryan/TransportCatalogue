#pragma once 

#include "json.h"
#include "map_renderer.h"

#include <sstream>

 // Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а 
 // с другими подсистемами приложения. 

////////// stat_requests //////////
namespace request_handler {
    // играет роль Фасада, упрощающего взаимодействие JSON reader-а 
    class RequestHandler {
    public:
        RequestHandler(const transport_catalogue::TransportCatalogue& catalog, const map_renderer::MapRender& map_catalog);

        json::Dict StopInfoPrint(const json::Dict& value);
        json::Dict BusInfoPrint(const json::Dict& value);
        json::Dict MapPrint(int id, svg::Document& map_svg);

    private:
        transport_catalogue::TransportCatalogue catalog_;
        map_renderer::MapRender map_catalog_;
    };
} // конец пространства имен request_handler