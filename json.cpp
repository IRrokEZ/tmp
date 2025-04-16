#include "json.h"

using std::literals::string_literals::operator""s;

namespace json {

namespace {

Node LoadNode(std::istream& input);

Node LoadNull(std::istream& input) {
    const std::string nameNull = "null"s;
    for (std::size_t i = 0; i < nameNull.size(); ++ i) {
        if (nameNull[i] != input.get()) {
            throw ParsingError("Null parsing error"s);
        }
    }

    char next_char;
    if (input >> next_char) {
        if ((!std::isspace(next_char))
            && (next_char != ',')
            && (next_char != '}')
            && (next_char != ']')) {
            throw ParsingError("Unexpected character after null: "s + std::string(1, next_char));
        }
        input.putback(next_char);
    }

    return Node(nullptr);
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
        }
        else if (ch == '\\') {
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
        }
        else if (ch == '\n' || ch == '\r') {
            throw ParsingError("Unexpected end of line"s);
        }
        else {
            s.push_back(ch);
        }
        ++ it;
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
    }
    else {
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
                return Node(std::stoi(parsed_num));
            }
            catch (...) {
            }
        }
        return Node(std::stod(parsed_num));
    }
    catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadBool(std::istream& input) {
    const std::string nameFalse = "false"s;
    const std::string nameTrue = "true"s;
    char c = input.get();
    
    if ((c != 't') && (c != 'f')) {
        throw ParsingError("Bool parsing error"s);
    }
    
    bool value = (c == 't');
    std::string const* name = value ? &nameTrue : &nameFalse;
    
    for (std::size_t i = 1; i < name->size(); ++ i) {
        if (input.peek() == -1) {
            throw ParsingError("Bool parsing error"s);
        }
        char next_char = input.get();
        if (name->at(i) != next_char) {
            throw ParsingError("Bool parsing error"s);
        }
    }
    
    if (isalpha(input.peek())) {
        throw ParsingError("Bool parsing error"s);
    }
    
    return Node(value);
}

Node LoadArray(std::istream& input) {
    Array result;
    if (input.peek() == -1) throw ParsingError("Array parsing error"s);

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
        throw ParsingError("Dict parsing error"s);
    }

    for (char c; (input >> c) && (c != '}');) {
        if (c == ',') {
            input >> c;
        }

        std::string key = LoadString(input);
        input >> c;
        result.insert({std::move(key), LoadNode(input)});
    }

    return Node(std::move(result));
}

Node LoadNode(std::istream& input) {
    char c;
    input >> c;

    if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    }
    else if (c == '"') {
        return Node(LoadString(input));
    }
    else if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    }
    else if (c == '[') {
        return LoadArray(input);
    }
    else if (c == '{') {
        return LoadDict(input);
    }
    else if (std::isdigit(c) || (c == '-')) { 
        input.putback(c);
        return LoadNumber(input);
    }
    else {
        throw ParsingError("Unexpected character: "s + std::string(1, c));
    }
}

}  // namespace

Node::Node(std::nullptr_t) : value_(nullptr) {}
Node::Node(std::string value) : value_(move(value)) {}
Node::Node(int value) : value_(value) {}
Node::Node(double value) : value_(value) {}
Node::Node(bool value) : value_(value) {}
Node::Node(Array array) : value_(move(array)) {}
Node::Node(Dict map) : value_(move(map)) {}

bool Node::IsInt() const { return std::holds_alternative<int>(value_); }
bool Node::IsDouble() const { return std::holds_alternative<double>(value_) || std::holds_alternative<int>(value_); }
bool Node::IsPureDouble() const { return std::holds_alternative<double>(value_); }
bool Node::IsBool() const { return std::holds_alternative<bool>(value_); }
bool Node::IsString() const { return std::holds_alternative<std::string>(value_); }
bool Node::IsNull() const { return std::holds_alternative<std::nullptr_t>(value_); }
bool Node::IsArray() const { return std::holds_alternative<Array>(value_); }
bool Node::IsMap() const { return std::holds_alternative<Dict>(value_); }

int Node::AsInt() const {
    if (!IsInt()) {
        throw std::logic_error("Not an int"s);
    }
    return std::get<int>(value_);
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw std::logic_error("Not a bool"s);
    }
    return std::get<bool>(value_);
}

double Node::AsDouble() const {
    if (!IsDouble()) {
        throw std::logic_error("Not a double"s);
    }
    if (IsInt()) {
        return static_cast<double>(std::get<int>(value_));
    }
    return std::get<double>(value_);
}

const std::string& Node::AsString() const {
    if (!IsString()) {
        throw std::logic_error("Not a string"s);
    }
    return std::get<std::string>(value_);
}

const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw std::logic_error("Not an array"s);
    }
    return std::get<Array>(value_);
}

const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw std::logic_error("Not a map"s);
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
    return !(*this == rhs);
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
    return !(*this == rhs);
}

Document Load(std::istream& input) {
    return Document{LoadNode(input)};
}

void ValuePrinter::operator()(std::nullptr_t) {
    ctx.out << "null"s;
}

void ValuePrinter::operator()(const std::string& value) {
    ctx.out << '"';
    for (const char c : value) {
        switch (c) {
        case '\n':
            ctx.out << "\\n"s;
            break;
        case '\r':
            ctx.out << "\\r"s;
            break;
        case '\t':
            ctx.out << "\\t"s;
            break;
        case '"':
            ctx.out << "\\\""s;
            break;
        case '\\':
            ctx.out << "\\\\"s;
            break;
        default:
            ctx.out << c;
        }
    }
    ctx.out << '"';
}

void ValuePrinter::operator()(int value) {
    ctx.out << value;
}

void ValuePrinter::operator()(double value) {
    ctx.out << value;
}

void ValuePrinter::operator()(bool value) {
    ctx.out << std::boolalpha << value;
}

void ValuePrinter::operator()(const Array& array) {
    ctx.out << '[';
    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const auto& item : array) {
        if (!first) {
            ctx.out << ',';
        }
        first = false;
        ctx.out << '\n';
        inner_ctx.PrintIndent();
        visit(ValuePrinter{inner_ctx}, item.GetValue());
    }
    if (!array.empty()) {
        ctx.out << '\n';
        ctx.PrintIndent();
    }
    ctx.out << ']';
}

void ValuePrinter::operator()(const Dict& dict) {
    ctx.out << '{';
    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const auto& [key, value] : dict) {
        if (!first) {
            ctx.out << ',';
        }
        first = false;
        ctx.out << '\n';
        inner_ctx.PrintIndent();
        ctx.out << '"' << key << "\": ";
        visit(ValuePrinter{inner_ctx}, value.GetValue());
    }
    if (!dict.empty()) {
        ctx.out << '\n';
        ctx.PrintIndent();
    }
    ctx.out << '}';
}

void Print(const Document& doc, std::ostream& out) {
    visit(ValuePrinter{{out}}, doc.GetRoot().GetValue());
}

}  // namespace json