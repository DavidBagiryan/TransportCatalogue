#pragma once

#include "request_handler.h"

namespace json_reader {
    class JsonReader {
    public:
        JsonReader(std::ostringstream& output)
            : output_(output)
        {
        }

        void Reader();

        std::ostringstream& Result();

    private:
        std::ostringstream& output_;

        transport_catalogue::TransportCatalogue catalog_;
        json::Array print_;
        map_renderer::MapRender map_catalog_;

        ////////// base_requests //////////
        void FillCatalog(json::Array& value);
        void AddStop(const json::Dict& stop);
        void AddBus(const json::Dict& bus);

        ////////// stat_requests //////////
        void ProcessRequest(json::Array& value, svg::Document& map_svg);

        ////////// render_settings //////////
        map_renderer::RenderSettings SetSettingsMap(json::Dict& render_settings);
        const svg::Color GetColor(const json::Node& color);

        void ResultPrint();
    };
} // конец пространства имен json_reader