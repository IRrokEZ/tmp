#include "svg.h"

namespace svg {

std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
    switch (line_cap) {
        case StrokeLineCap::BUTT:
            out << "butt"s;
            break;
        case StrokeLineCap::ROUND:
            out << "round"s;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"s;
            break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
    switch (line_join) {
        case StrokeLineJoin::ARCS:
            out << "arcs"s;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"s;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"s;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"s;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"s;
            break;
    }
    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    RenderObject(context);

    context.out << std::endl;
}

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
    out << "<circle cx=\""s << center_.x << "\" cy=\""s << center_.y << "\" "s;
    out << "r=\""s << radius_ << "\""s;
    RenderAttrs(context.out);
    out << "/>"s;
}

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(std::move(point));
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""s;
    bool is_first = true;
    for (auto& point : points_) {
        if (is_first) {
            out << point.x << "," << point.y;
            is_first = false;
        } else {
            out << " "s << point.x << "," << point.y;
        }
    }
    out << "\"";
    RenderAttrs(context.out);
    out << "/>"s;
}

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

Text& Text::SetFontFamily(const std::string &font_family) {
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(const std::string &font_weight) {
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(const std::string &data) {
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text";
    RenderAttrs(context.out);
    out << " x=\""s << pos_.x << "\" y=\""s << pos_.y << "\" "s;
    out << "dx=\""s << offset_.x << "\" dy=\""s << offset_.y << "\" "s;
    out << "font-size=\""s << size_ << "\""s;
    out << ((!font_family_.empty()) ? (" font-family=\""s + font_family_ + "\" "s) : ""s);
    out <<  ((!font_weight_.empty()) ? ("font-weight=\""s + font_weight_ + "\""s) : ""s);;
    out << ">"s << data_ << "</text>"s;
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    RenderContext ctx(std::cout, 2, 2);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"s << std::endl;
    out << "<sg xmlns=\"http://www.w3.org/2000/sg\" version=\"1.1\">"s << std::endl;
    for (const auto& obj : objects_) {
        obj->Render(ctx);
    }
    out << "</sg>"s;
}

}  // namespace sg