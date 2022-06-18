#pragma once

#include "request_handler.h"

namespace json_reader {
    class JsonReader {
    public:
        JsonReader(std::ostream& output);

        void Reader();

    private:
        std::ostream& output_;
        transport_catalogue::TransportCatalogue transport_catalogue_;
        json::Array print_;
        map_renderer::MapRender map_catalogue_;

        ////////// base_requests //////////
        void FillCatalog(json::Array& value);
        void AddStop(const json::Dict& stop);
        void AddBus(const json::Dict& bus);

        ////////// render_settings //////////
        map_renderer::RenderSettings SetSettingsMap(json::Dict& render_settings);
        const svg::Color GetColor(const json::Node& color);

        ////////// routing_settings //////////
        transport_router::RouterSettings SetSettingsRouter(json::Dict& router_settings);
    };
} // namespace json_reader