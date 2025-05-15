#include <fstream>

#include "json.h"

using std::literals::string_literals::operator""s;

namespace json {

namespace {

using Number = std::variant<int, double>;

Node LoadNode(std::istream& input);

Node LoadNull(std::istream& input) {
    const std::string nameNull = "null"s;
    for (std::size_t i = 0; i < nameNull.size(); ++ i) {
        if (nameNull.at(i) == input.get()) {
            continue;
        } else {
            throw ParsingError("Null parsing error"s);
        }
    }
    return {};
}

std::string LoadString(std::istream& input) {
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            throw ParsingError("String parsing error"s);
        }
        const char ch = *it;
        if (ch == '"') {
            ++ it;
            break;
        } else if (ch == '\\') { 
            ++ it;
            if (it == end) { 
                throw ParsingError("String parsing error"s);
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
        } else if ((ch == '\n') || (ch == '\r')) { 
            throw ParsingError("Unexpected end of line"s);
        } else { 
            s.push_back(ch);
        }
        ++it;
    }

    return s;
}

Node LoadNumber(std::istream& input) {
    std::string parsed_num;

    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };
 
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
    if (input.peek() == '0') {
        read_char(); 
    } else {
        read_digits();
    }

    bool is_int = true; 
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }
 
    if (int ch = input.peek(); (ch == 'e') || (ch == 'E')) {
        read_char();
        if (ch = input.peek(); (ch == '+') || (ch == '-')) {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) { 
            try {
                return std::stoi(parsed_num);
            }
            catch (...) {}
        }
        return std::stod(parsed_num);
    }
    catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadBool(std::istream& input) {
    const std::string nameFalse = "false"s;
    const std::string nameTrue = "true"s;
    char c = input.get();
    bool value = (c == 't');
    std::string const* name = value ? &nameTrue : &nameFalse;
    for (std::size_t i = 1; i < name->size(); ++ i) {
        if (name->at(i) == input.get()) {
            continue;
        } else {
            throw ParsingError("Bool parsing error"s);
        }
    }
    return Node(value);
}

Node LoadArray(std::istream& input) {
    Array result;
    if (input.peek() == -1) {
        throw ParsingError("Array parsing error"s);
    }

    for (char c; (input >> c) && (c != ']');) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    return Node(std::move(result));
}

Node LoadDict(std::istream& input) {
    Dict result;
    if (input.peek() == -1) {
        throw ParsingError("Array parsing error"s);
    }

    for (char c; (input >> c) && (c != '}');) {
        if (c == ',') {
            input >> c;
        }

        std::string key = LoadString(input);
        input >> c;
        result.insert({ std::move(key), LoadNode(input) });
    }

    return Node(std::move(result));
}

Node LoadNode(std::istream& input) {
    char c;
    input >> c;

    if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if ((c == 't') || (c == 'f')) {
        input.putback(c);
        return LoadBool(input);
    } else if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else {
        input.putback(c);
        return LoadNumber(input);
    }
}

}  // namespace

Node::Node(std::nullptr_t)
    : value_(nullptr) {}

Node::Node(std::string value)
    : value_(std::move(value)) {}

Node::Node(int value)
    : value_(value) {}

Node::Node(double value)
    : value_(value) {}

Node::Node(bool value)
    : value_(value) {}

Node::Node(Array array)
    : value_(std::move(array)) {}

Node::Node(Dict map)
    : value_(std::move(map)) {}

bool Node::IsInt() const {
    return std::holds_alternative<int>(value_);
}

bool Node::IsDouble() const {
    return std::holds_alternative<double>(value_)
           || std::holds_alternative<int>(value_);
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(value_);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(value_);
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(value_);
}

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(value_);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(value_);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(value_);
}

int Node::AsInt() const {
    if (!IsInt()) {
        throw ParsingError("not int"s);
    }
    return std::get<int>(value_);
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw ParsingError("not bool"s);
    }
    return std::get<bool>(value_);
}

double Node::AsDouble() const {
    if (!IsDouble()) {
        throw ParsingError("not double"s);
    }
    if (IsInt()) {
        return static_cast<double>(std::get<int>(value_));
    }
    return std::get<double>(value_);
}

const std::string& Node::AsString() const {
    if (!IsString()) {
        throw ParsingError("not string"s);
    }
    return std::get<std::string>(value_);
}

const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw ParsingError("not array"s);
    }
    return std::get<Array>(value_);
}

const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw ParsingError("wrong map"s);
    }
    return std::get<Dict>(value_);
}

const Node::Value& Node::GetValue() const {
    return value_;
}

bool Node::operator==(const Node& rhs) const {
    return value_ == rhs.value_;
}

bool Node::operator!=(const Node& rhs) const {
    return !(value_ == rhs.value_);
}

Document::Document(Node root)
    : root_(std::move(root)) {}

const Node& Document::GetRoot() const {
    return root_;
}

bool Document::operator==(const Document& rhs) const {
    return root_ == rhs.root_;
}

bool Document::operator!=(const Document& rhs) const {
    return !(root_ == rhs.root_);
}

Document Load(std::istream& input) {
    return Document{ LoadNode(input) };
}

void PrintValue(std::nullptr_t, const PrintContext& ctx) {
    ctx.out << "null"s;
}

void PrintValue(std::string value, const PrintContext& ctx) {
    ctx.out << "\""s;
    for (const char& c : value) {
        if (c == '\n') {
            ctx.out << "\\n"s;
            continue;
        }
        if (c == '\r') {
            ctx.out << "\\r"s;
            continue;
        }
        if (c == '\"') {
            ctx.out << "\\"s;
        }
        if (c == '\t') {
            ctx.out << "\t"s;
            continue;
        }
        if (c == '\\') {
            ctx.out << "\\"s;
        }
        ctx.out << c;
    }
    ctx.out << "\""s;
}

void PrintValue(bool value, const PrintContext& ctx) {
    ctx.out << std::boolalpha << value;
}

void PrintValue(Array array, const PrintContext& ctx) {
    ctx.out << "[\n"s;
    auto inner_ctx = ctx.Indented();
    bool first = true;
    for (const auto& elem : array) {
        if (first) {
            first = false;
        } else {
            ctx.out << ",\n"s;
        }
        inner_ctx.PrintIndent();
        PrintNode(elem, inner_ctx);
    }
    ctx.out << "\n"s;
    ctx.PrintIndent();
    ctx.out << "]"s;
}

void PrintValue(Dict dict, const PrintContext& ctx) {
    ctx.out << "{\n"s;
    auto inner_ctx = ctx.Indented();
    bool first = true;
    for (auto& [key, node] : dict) {
        if (first) {
            first = false;
        } else {
            ctx.out << ",\n"s;
        }
        inner_ctx.PrintIndent();
        PrintValue(key, ctx);
        ctx.out << ": ";
        PrintNode(node, inner_ctx);
    }
    ctx.out << "\n"s;
    ctx.PrintIndent();
    ctx.out << "}"s;
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit(
        [&ctx](const auto& value) { PrintValue(value, ctx); },
        node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    PrintContext ctx{ output };
    PrintNode(doc.GetRoot(), ctx);
}

}  // namespace json