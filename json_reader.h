#pragma once

#include "request_handler.h"

namespace json_reader {
    class JsonReader {
    public:
        JsonReader(std::ostream& output)
            : output_(output)
        {
        }
        
        void Reader();
        
    private:
        std::ostream& output_;

        transport_catalogue::TransportCatalogue catalog_;
        json::Array print_;
        map_renderer::MapRender map_catalog_;

        ////////// base_requests //////////
        void FillCatalog(json::Array& value);
        void AddStop(const json::Dict& stop);
        void AddBus(const json::Dict& bus);

        ////////// render_settings //////////
        map_renderer::RenderSettings SetSettingsMap(json::Dict& render_settings);
        const svg::Color GetColor(const json::Node& color);
    };
} // namespace json_reader