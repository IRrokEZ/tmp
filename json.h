#pragma once

#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using Number = std::variant<int, double>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, std::string, int, double, bool, Array, Dict>;

    Node (std::nullptr_t)
        : value_(nullptr) {}

    explicit Node (const char* cstr)
        : value_(std::string(cstr)) {}

    template<typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Node> &&
                                                    std::is_constructible_v<Value, T>>>
    explicit Node(T&& value)
        : value_(std::forward<T>(value)) {}

    bool IsInt () const;
    bool IsDouble () const;
    bool IsPureDouble () const;
    bool IsBool () const;
    bool IsString () const;
    bool IsNull () const;
    bool IsArray () const;
    bool IsMap () const;

    int AsInt () const;
    bool AsBool () const;
    double AsDouble () const;
    const std::string& AsString () const;
    const Array& AsArray () const;
    const Dict& AsMap () const;

    const Value& GetValue () const;
    Value& GetValue ();

    bool operator== (const Node& rhs) const;
    bool operator!= (const Node& rhs) const;

private:
    Value value_;
};

class Document {
public:
    explicit Document (Node root);

    const Node& GetRoot () const;

    bool operator== (const Document& rhs) const;
    bool operator!= (const Document& rhs) const;

private:
    Node root_;
};

Document Load (std::istream& input);

struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent () const {
        for (int i = 0; i < indent; ++ i) {
            out.put(' ');
        }
    }
    
    PrintContext Indented () const {
        return { out, indent_step, indent_step + indent };
    }
};

void PrintValue (std::nullptr_t, const PrintContext& ctx);
void PrintValue (std::string value, const PrintContext& ctx);
void PrintValue (bool value, const PrintContext& ctx);
void PrintValue (Array array, const PrintContext& ctx);
void PrintValue (Dict dict, const PrintContext& ctx);

template <typename Value>
void PrintValue (const Value& value, const PrintContext& ctx) {
    ctx.out << value;
}

void PrintNode (const Node& node, const PrintContext& ctx);
void Print (const Document& doc, std::ostream& output);

}  // namespace json