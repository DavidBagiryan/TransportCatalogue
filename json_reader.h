#pragma once

#include "json.h"
#include "map_renderer.h"

#include <sstream>

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
        void ProcessingRequest(json::Array& value, svg::Document& map_svg);
        void StopInformationPrinting(const json::Dict& value);
        void BusInformationPrinting(const json::Dict& value);

        ////////// render_settings //////////
        map_renderer::RenderSettings SetSettingsMap(json::Dict& render_settings);
        const svg::Color GetColor(const json::Node& color);

        void PrintResult();
        void MapPrinting(const json::Dict& value, svg::Document& map_svg);
    };
} // конец пространства имен json_reader