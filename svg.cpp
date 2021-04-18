#include "svg.h"

#include <iomanip>
#include <algorithm>
#include <cmath>

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    //  ----------------RenderContext---------------------
    RenderContext RenderContext::Indented() const {
        return { out, indent_step, indent + indent_step };
    }

    void RenderContext::RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    //  ----------------PathProps---------------------
    std::string to_str(int i) {
        return std::to_string(i);
    }
    std::string to_str(double i) {
        return std::to_string(i);
    }

    void SolutionColor::operator()(std::monostate) {
        out << "none"s;
    }
    void SolutionColor::operator()(svg::Rgb rgb) {
        out << "rgb("s + to_str(rgb.red) + ","s + to_str(rgb.green) + ","s + to_str(rgb.blue) + ")"s;
    }
    void SolutionColor::operator()(std::string color) {
        out << color;
    }
    void SolutionColor::operator()(svg::Rgba rgba) {
        out << "rgba("s + to_str(rgba.red) + ","s + to_str(rgba.green) + ","s + to_str(rgba.blue) + ","s;
        out << rgba.opacity;
        out << ")"s;
    }


    std::ostream& operator<<(std::ostream& out, const StrokeLineCap line_cap) {
        using namespace std::literals;
        switch (line_cap) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin line_join) {
        using namespace std::literals;
        switch (line_join) {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
        }
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
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }


    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.emplace_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        for (size_t i = 0; i < points_.size(); ++i) {
            out << points_[i].x << ","sv << points_[i].y;
            if (i != points_.size() - 1) {
                out << " "sv;
            }
        }
        out << "\" "sv;
        RenderAttrs(out);
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
        char ch1(34);
        char ch2(60);
        char ch3(62);
        char ch4(39);
        char ch5(38);

        std::vector<std::pair<char, std::string>> symbols{ {ch5,"&amp;"s},{ch1,"&quot;"s},{ch2,"&lt;"s},{ch3,"&gt;"s},{ch4,"&apos;"} };
        std::string result = data_;
        const size_t pos_end = data_.npos;
        size_t space;
        for (auto [ch, str] : symbols) {
            while (true) {
                if (result.find("&amp;"s) != pos_end && ch == ch5) {
                    break;
                }
                space = result.find(ch);
                if (space != pos_end) {
                    result.erase(space, 1u);
                    result.insert(space, str);
                }
                else {
                    break;
                }
            }
        }

        auto& out = context.out;        
        out << "<text"sv;
        RenderAttrs(out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << size_ << "\" "sv;
        if (!font_family_.empty()) {
            out << "font-family=\""sv << font_family_ << "\" "sv;
        }
        if (!font_weight_.empty()) {
            out << "font-weight=\""sv << font_weight_ << "\""sv;
        }

        out << ">"sv << result << "</text>"sv;
    }

    // ---------- Document ------------------
    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for (size_t i = 0; i < objects_.size(); ++i) {
            objects_[i]->Render({ out, 2, 2 });
        }
        out << "</svg>"sv;
    }
}  // namespace svg