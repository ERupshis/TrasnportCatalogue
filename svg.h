#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg {
    using namespace std::literals;

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };
    
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const;
        void RenderIndent() const;

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };
    
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };
    
    inline const std::string NoneColor{ "none"s };

    struct Rgb {
        Rgb() = default;
        Rgb(uint8_t r, uint8_t g, uint8_t b)
            :red(r), green(g), blue(b)
        {
        }
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        Rgba() = default;
        Rgba(uint8_t r, uint8_t g, uint8_t b, double a)
            :red(r), green(g), blue(b), opacity(a)
        {
        }
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };
    
    using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
    std::string to_str(int i);
    std::string to_str(double i);

    struct SolutionColor {
        std::ostream& out;
        void operator()(std::monostate);
        void operator()(svg::Rgb rgb);
        void operator()(std::string color);
        void operator()(svg::Rgba rgba);
    };

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap line_cap);
    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin line_join);
    std::ostream& operator<<(std::ostream& out, const Color& color);

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = color;
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = color;
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            line_cap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = line_join;
            return AsOwner();
        }
         
    protected:
        ~PathProps() = default;       
        

        template <typename T>
        void FillData(std::ostream& out, std::string_view att_name, const std::optional<T>& att_value) const {
            if (att_value) {
                out << ' ' << att_name << "=\""sv << *att_value << "\""sv;
            }
        }

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;
            FillData(out, "fill"sv, fill_color_);
            FillData(out, "stroke"sv, stroke_color_);
            FillData(out, "stroke-width"sv, width_);
            FillData(out, "stroke-linecap"sv, line_cap_);
            FillData(out, "stroke-linejoin"sv, line_join_);            
        }        

    private:
        Owner& AsOwner() {            
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };
    
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_ = { 0.0, 0.0 };
        double radius_ = 1.0;
    };
    
    class Polyline final : public Object, public PathProps<Polyline> {
    public:        
        Polyline& AddPoint(Point point);
        
    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_;
    };
    
    class Text final : public Object, public PathProps<Text> {
    public:        
        Text& SetPosition(Point pos);
        Text& SetOffset(Point offset);
        Text& SetFontSize(uint32_t size);
        Text& SetFontFamily(std::string font_family);
        Text& SetFontWeight(std::string font_weight);
        Text& SetData(std::string data);
    private:
        void RenderObject(const RenderContext& context) const override;

        Point pos_ = { 0.0, 0.0 };
        Point offset_ = { 0.0, 0.0 };
        uint32_t size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_ = "";
    };

    class ObjectContainer {
    public:        
        template <typename T>
        void Add(T obj) {
            objects_.emplace_back(std::make_unique<T>(std::move(obj)));
        }

        virtual ~ObjectContainer() = default;
    protected:        
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
        std::vector<std::unique_ptr<Object>> objects_;
    };

    class Document : public ObjectContainer {
    public:
        Document() = default;

        void AddPtr(std::unique_ptr<Object>&& obj) override {
            objects_.emplace_back(std::move(obj));
        }        

        void Render(std::ostream& out) const;
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& obj) const = 0;

        virtual ~Drawable() = default;
    };
}  // namespace svg

/////Template functions/methods//////////////////////////////////////////////////////////
/////PathProps///////////////////////////////////////////////////////////////////////////
template <typename T>
void FillData(std::ostream& out, std::string_view att_name, const std::optional<T>& att_value)  {
    using namespace std::literals;
    if (att_value) {
        out << ' ' << att_name << "=\""sv << *att_value << "\""sv;
    }
}

   