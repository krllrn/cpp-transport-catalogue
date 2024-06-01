#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& cap) {
        switch (cap)
        {
        case StrokeLineCap::BUTT: return (out << "butt");
        case StrokeLineCap::ROUND: return (out << "round");
        case StrokeLineCap::SQUARE: return (out << "square");
        default:
            break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& join) {
        switch (join)
        {
        case StrokeLineJoin::ARCS: return (out << "arcs");
        case StrokeLineJoin::BEVEL: return (out << "bevel");
        case StrokeLineJoin::MITER: return (out << "miter");
        case StrokeLineJoin::MITER_CLIP: return (out << "miter-clip");
        case StrokeLineJoin::ROUND: return (out << "round");
        default:
            break;
        }
        return out;
    }

    struct ColorPrinter {
        ColorPrinter(std::ostream& out)
            :out_(out)
        {}

        void operator()(std::monostate) const {
            out_ << "none"sv;
        }
        void operator()(std::string str) const {
            out_ << str;
        }
        void operator()(Rgb rgb) const {
            out_ << "rgb("sv << +rgb.red << ","sv << +rgb.green << ","sv << +rgb.blue << ")"sv;
        }
        void operator()(Rgba rgba) const {
            out_ << "rgba("sv << +rgba.red << ","sv << +rgba.green << ","sv << +rgba.blue << ","sv << rgba.opacity << ")"sv;
        }

        std::ostream& out_;
    };

    std::ostream& operator<<(std::ostream& out, const Color& color) {
        std::visit(ColorPrinter{ out }, color);
        return out;
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
        out << "  <circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------
    Polyline& Polyline::AddPoint(Point point) {
        points_.emplace_back(std::move(point));
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "  <polyline points=\""sv;
        bool is_first = true;
        for (const auto& point : points_) {
            if (is_first == false) {
                out << " "sv;
            }
            out << point.x << ","sv << point.y;
            is_first = false;
        }
        out << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Text ------------------
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "  <text";
        RenderAttrs(context.out);
        out << " x = \""sv << pos_.x << "\" y=\""sv << pos_.y
            << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" font-size=\""sv << size_;
        if (!font_family_.empty()) {
            out << "\" font-family=\""sv << font_family_;
        }
        if (!font_weight_.empty()) {
            out << "\" font-weight=\""sv << font_weight_;
        }
        out << "\""sv;
        
        out << ">"sv;
        for (const auto& c : data_) {
            switch (c)
            {
            case '\"':
                out << "&quot;"sv;
                break;
            case '\'':
                out << "&apos;"sv;
                break;
            case '<':
                out << "&lt;"sv;
                break;
            case '>':
                out << "&gt;"sv;
                break;
            case '&':
                out << "&amp;"sv;
                break;
            default:
                out << c;
                break;
            }
        }
        out << "</text>"sv;
    }

    // ---------- Document ------------------
    Document::Document() {}

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

        for (const auto& obj : objects_) {
            obj->Render(out);
        }

        out << "</svg>"sv;
    }

}  // namespace svg