#include "json.h"

#include <iomanip>

using namespace std;

namespace json {
    /////Node Class Part/////////////////////////////////////////////////////////
    bool operator== (const Node& lhs, const Node& rhs) {
        return lhs.value_ == rhs.value_;
    }
    bool operator!= (const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }
    /////Conctructor Area////////////////////////////////////////////////////////
    Node::Node(std::nullptr_t value)
        :value_(value) {
    }
    Node::Node(Array value)
        :value_(std::move(value)) {
    }
    Node::Node(Dict value)
        :value_(std::move(value)) {
    }
    Node::Node(bool value)
        :value_(value) {
    }
    Node::Node(int value)
        :value_(value) {
    }
    Node::Node(double value)
        :value_(value) {
    }
    Node::Node(std::string value)
        :value_(std::move(value)) {
    }
    /////Type Comprasion Area////////////////////////////////////////////////////
    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(value_);
    }
    bool Node::IsArray() const {
        return std::holds_alternative<Array>(value_);
    }
    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(value_);
    }
    bool Node::IsBool() const {
        return std::holds_alternative<bool>(value_);
    }
    bool Node::IsInt() const {
        return std::holds_alternative<int>(value_);
    }
    bool Node::IsDouble() const {
        return std::holds_alternative<double>(value_) || std::holds_alternative<int>(value_);
    }
    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(value_);
    }
    bool Node::IsString() const {
        return std::holds_alternative<std::string>(value_);
    }
    /////Get Value Area/////////////////////////////////////////////////////////////////////
    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw std::logic_error("");
        }
        return std::get<Array>(value_);
    }
    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw std::logic_error("");
        }
        return std::get<Dict>(value_);
    }
    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("");
        }
        return std::get<bool>(value_);
    }
    int Node::AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("");
        }
        return std::get<int>(value_);
    }
    double Node::AsDouble() const {
        auto value_index = value_.index();
        if (value_index != 4 && value_index != 5) {
            throw std::logic_error("");
        }
        if (value_.index() == 4) {
            return std::get<int>(value_);
        }
        return std::get<double>(value_);
    }
    const std::string& Node::AsString() const {
        if (!IsString()) {
            throw std::logic_error("");
        }
        return std::get<std::string>(value_);
    }

    const NodeValue& Node::GetValue() const {
        return value_;
    }
    /////END of Node Class Part///////////////////////////////////////////////
    /////LoadNode Namespace///////////////////////////////////////////////////
    namespace {
        Node LoadNode(std::istream& input);

        Node LoadString(std::istream& input);

        std::string LoadLiteral(std::istream& input) {
            std::string s;
            while (std::isalpha(input.peek())) {
                s.push_back(static_cast<char>(input.get()));
            }
            return s;
        }

        Node LoadNumber(std::istream& input) {
            std::string parsed_num;

            // Read queue symbol in parsed_num from input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Read one or more number in parsed_num from input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Integer part of number parsing
            if (input.peek() == '0') {
                read_char();
                // After 0 numbers are not allowed in JSON
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Fractional part of a number parsing
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // EXP part of a number parsing
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Trying to convert string to integer
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // In case of fail
                        // code below is trying to convert string into double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadArray(istream& input) {
            Array result;

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (!input) {
                throw ParsingError("Array parsing error");
            }
            return Node(move(result));
        }

        Node LoadDict(istream& input) {
            Dict dict;
            for (char c; input >> c && c != '}';) {
                if (c == '"') {
                    std::string key = LoadString(input).AsString();
                    if (input >> c && c == ':') {
                        if (dict.find(key) != dict.end()) {
                            throw ParsingError("Duplicate key '"s + key + "' have been found");
                        }
                        dict.emplace(std::move(key), LoadNode(input));
                    }
                    else {
                        throw ParsingError(": is expected but '"s + c + "' has been found"s);
                    }
                }
                else if (c != ',') {
                    throw ParsingError(R"(',' is expected but ')"s + c + "' has been found"s);
                }
            }
            if (!input) {
                throw ParsingError("Dictionary parsing error"s);
            }
            return Node(std::move(dict));
        }

        Node LoadString(std::istream& input) {
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    ++it;
                    if (it == end) {
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(std::move(s));
        }

        Node LoadBool(std::istream& input) {
            const auto s = LoadLiteral(input);
            if (s == "true"sv) {
                return Node{ true };
            }
            else if (s == "false"sv) {
                return Node{ false };
            }
            else {
                throw ParsingError("Failed to parse '"s + s + "' as bool"s);
            }
        }

        Node LoadNull(std::istream& input) {
            if (auto literal = LoadLiteral(input); literal == "null"sv) {
                return Node{ nullptr };
            }
            else {
                throw ParsingError("Failed to parse '"s + literal + "' as null"s);
            }
        }

        Node LoadNode(istream& input) {
            char c;
            if (!(input >> c)) {
                throw ParsingError("Unexpected EOF"s);
            }

            switch (c) {
            case '[':
                return LoadArray(input);
            case '{':
                return LoadDict(input);
            case '"':
                return LoadString(input);
            case 't':                
                // go to next case
                [[fallthrough]];
            case 'f':
                input.putback(c);
                return LoadBool(input);
            case 'n':
                input.putback(c);
                return LoadNull(input);
            default:
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace
    /////Document Class Part/////////////////////////////////////////////////////////
    bool operator== (const Document& lhs, const Document& rhs) {
        return lhs.root_ == rhs.root_;
    }
    bool operator!= (const Document& lhs, const Document& rhs) {
        return lhs.root_ != rhs.root_;
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }
    const Node& Document::GetRoot() const {
        return root_;
    }
    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }
    /////Std::variant Solution Solver//////////////////////////////////////////
    void NodeValueSolution::operator() (std::nullptr_t) {
        out << "null"s;
    }
    void NodeValueSolution::operator() (const Array& value) {
        out << "["s;
        for (auto iter = value.begin(); iter < value.end(); ++iter) {
            std::visit(NodeValueSolution{ out }, iter->GetValue());
            if (value.end() - iter > 1) {
                out << ", "s;
            }
        }
        out << "]"s;
    }
    void NodeValueSolution::operator() (const Dict& value) {
        out << "{"s;
        for (auto iter = value.begin(); iter != value.end(); ++iter) {
            out << '"' << iter->first << '"' << ':';
            std::visit(NodeValueSolution{ out }, iter->second.GetValue());
            if (iter != prev(value.end())) {
                out << ", "s;
            }
        }
        out << "}";
    }
    void NodeValueSolution::operator() (bool value) {
        out << std::boolalpha << value;
    }
    void NodeValueSolution::operator() (int value) {
        out << value;
    }
    void NodeValueSolution::operator() (double value) {
        out << value;
    }
    void NodeValueSolution::operator() (const std::string& value) {
        out << "\""s;
        for (const auto& ch : value) { // Escape-symbols converter
            switch (ch) {
            case '"':
                out << "\\\""s;
                break;
            case '\\':
                out << "\\\\"s;
                break;
            case '\n':
                out << "\\n"s;
                break;
            case '\r':
                out << "\\r"s;
                break;
            case '\t':
                out << "\\t"s;
                break;
            default:
                out << ch;
            }
        }
        out << "\""s;
    }

    void Print(const Document& doc, std::ostream& output) {
        std::visit(NodeValueSolution{ output }, doc.GetRoot().GetValue());        
    }    

}  // namespace json