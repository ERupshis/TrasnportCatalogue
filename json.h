#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {
    using namespace std::literals;
    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using Number = std::variant<int, double>;
    using NodeValue = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
        friend bool operator== (const Node& lhs, const Node& rhs);
        friend bool operator!= (const Node& lhs, const Node& rhs);
    public:
        /* //Cannot inherit std::variant constructors =((((
        using NodeValue = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
        using NodeValue::NodeValue;
        using NodeValue::operator=;
        */
        /////Constructors Area/////////////////////////////////////////////////
        Node() = default;
        Node(std::nullptr_t value);
        Node(Array value);
        Node(Dict value);
        Node(bool value);
        Node(int value);
        Node(double value);
        Node(std::string value);
        /////Type Comprasion Area//////////////////////////////////////////////
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;
        bool IsBool() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsString() const;
        /////Get Value Area////////////////////////////////////////////////////
        const Array& AsArray() const;
        const Dict& AsMap() const;
        bool AsBool() const;
        int AsInt() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const NodeValue& GetValue() const;
    private:
        NodeValue value_ = nullptr;
    };
    /////Document Part/////////////////////////////////////////////////////////
    class Document {
        friend bool operator== (const Document& lhs, const Document& rhs);
        friend bool operator!= (const Document& lhs, const Document& rhs);
    public:
        explicit Document() = default;
        explicit Document(Node root);

        const Node& GetRoot() const;
    private:
        Node root_;
    };

    Document Load(std::istream& input);
    /////Std::variant Solution Solver//////////////////////////////////////////
    struct NodeValueSolution {
        std::ostream& out;
        void operator() (std::nullptr_t);
        void operator() (const Array& value);
        void operator() (const Dict& value);
        void operator() (bool value);
        void operator() (int value);
        void operator() (double value);
        void operator() (const std::string& value);
    };

    void Print(const Document& doc, std::ostream& output);
}  // namespace json
