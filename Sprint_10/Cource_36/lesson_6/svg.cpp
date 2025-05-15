#include "svg.h"

namespace svg {

std::string EscapeSymbols(const std::string& input) {
    std::string result;
    for (char ch : input) {
        switch (ch) {
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '&': result += "&amp;"; break;
            case '"': result += "&quot;"; break;
            case '\'': result += "&apos;"; break;
            default: result += ch; break;
        }
    }
    return result;
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
    out << "<circle cx=\"" << center_.x << "\" cy=\"" << center_.y << "\" ";
    out << "r=\"" << radius_ << "\"/>";
}

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\"";
    for (size_t i = 0; i < points_.size(); ++i) {
        if (i > 0) {
            out << " ";
        }
        out << points_[i].x << "," << points_[i].y;
    }
    out << "\"/>";
}

Text& Text::SetPosition(Point pos) {
    position_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(const std::string& font_family) {
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(const std::string& font_weight) {
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(const std::string& data) {
    data_ = EscapeSymbols(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text";
    out << " x=\"" << position_.x << "\" y=\"" << position_.y << "\"";
    out << " dx=\"" << offset_.x << "\" dy=\"" << offset_.y << "\"";
    out << " font-size=\"" << font_size_ << "\"";
    if (!font_family_.empty()) {
        out << " font-family=\"" << font_family_ << "\"";
    }
    if (!font_weight_.empty()) {
        out << " font-weight=\"" << font_weight_ << "\"";
    }
    out << ">" << data_ << "</text>";
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << std::endl;

    RenderContext ctx(out, 2);
    for (const auto& obj : objects_) {
        obj->Render(ctx);
    }

    out << "</svg>" << std::endl;
}

}  // namespace svg