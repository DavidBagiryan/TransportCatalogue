#include "svg.h"

namespace svg {

    using namespace std::literals;

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap) {
        switch (line_cap) {
        case StrokeLineCap::BUTT: return (out << "butt");
        case StrokeLineCap::ROUND: return (out << "round");
        case StrokeLineCap::SQUARE: return (out << "square");
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join) {
        switch (line_join) {
        case StrokeLineJoin::ARCS: return (out << "arcs");
        case StrokeLineJoin::BEVEL: return (out << "bevel");
        case StrokeLineJoin::MITER: return (out << "miter");
        case StrokeLineJoin::MITER_CLIP: return (out << "miter-clip");
        case StrokeLineJoin::ROUND: return (out << "round");
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const Color& color) {
        std::visit(OstreamPrinter{ out }, color);
        return out;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        // выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" "sv << "dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << size_ << "\" "sv;
        if (font_family_ != "") out << "font-family=\""sv << font_family_ << "\""sv;
        if (font_family_ != "" && font_weight_ != "") out << " "sv;
        if (font_weight_ != "") out << "font-weight=\""sv << font_weight_ << "\""sv;
        // выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << ">"sv;
        out << Metamorphosis() << "</text>"sv;
    }

    std::string Text::Metamorphosis() const {
        std::string changed_data = ""s;
        for (const char c : data_) {
            switch (c) {
            case '\"':
                changed_data += "&quot;"s;
                break;
            case '\'':
                changed_data += "&apos;"s;
                break;
            case '<':
                changed_data += "&lt;"s;
                break;
            case '>':
                changed_data += "&gt;"s;
                break;
            case '&':
                changed_data += "&amp;"s;
                break;
            default:
                changed_data += c;
            }
        }
        return changed_data;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        if (points_.empty()) {
            out << "\"";
        }
        else {
            for (size_t i = 0; i < points_.size(); ++i) {
                out << points_[i].x << "," << points_[i].y;
                if (i + 1 == points_.size()) {
                    out << "\"";
                    break;
                }
                out << " ";
            }
        }
        // выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << " />"sv;
    }

    // ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext ctx(out, 2, 2);
        for (const auto& obj : objects_) {
            obj->Render(ctx);
        }
        out << "</svg> "sv;
    }

}  // namespace svg
