#pragma once 

#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <sstream>

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а 
// с другими подсистемами приложения. 

////////// stat_requests //////////
namespace request_handler {
    // играет роль Фасада, упрощающего взаимодействие JSON reader-а 
    class RequestHandler {
    public:
        RequestHandler(const transport_catalogue::TransportCatalogue& catalog, const map_renderer::MapRender& map_catalog, const transport_router::RouterSettings& settings_router);

        void RequestProcess(json::Array& value, svg::Document& map_svg, std::ostream& output);

    private:
        transport_catalogue::TransportCatalogue catalog_;
        map_renderer::MapRender map_catalog_;
        transport_router::TransportRouter transport_router_;

        void StopInfoPrint(const json::Dict& value, json::Builder& request);
        void BusInfoPrint(const json::Dict& value, json::Builder& request);
        void MapPrint(int id, svg::Document& map_svg, json::Builder& request);
        void RoutePrint(const json::Dict& value, json::Builder& request);
    };
} // namespace request_handler