#include "json_builder.h"

#include <exception>
#include <optional>
#include <variant>
#include <utility>

using std::literals::string_literals::operator""s;

namespace json {

class BuilderError : public std::logic_error {
public:
    explicit BuilderError(const std::string& what_arg)
        : std::logic_error("JSON Builder error: "s + what_arg) {}
};

Builder::Builder()
    : root_(nullptr), nodes_stack_{&root_}, built_(false), key_(std::nullopt) {}

Node Builder::Build() {
    if (built_) {
        throw BuilderError("JSON can be built only once");
    }

    if (!nodes_stack_.empty()) {
        throw BuilderError("Attempt to build JSON which isn't finalized");
    }

    built_ = true;
    return std::move(root_);
}

void Builder::Reset() {
    root_ = Node(nullptr);
    nodes_stack_.clear();
    nodes_stack_.push_back(&root_);
    built_ = false;
    key_.reset();
}

DictKeyContext Builder::Key(std::string key) {
    if (key_.has_value()) {
        throw BuilderError("Incorrect structure. Key() after Key() without Value()");
    }

    Node::Value &top = nodes_stack_.back()->GetValue();
    
    if (!std::holds_alternative<Dict>(top)) {
        throw BuilderError("Key() outside a dict");
    }

    key_ = std::move(key);

    return *this;
}

Builder& Builder::Value(Node::Value value) {
    Node* top = nodes_stack_.back();
    if (top->IsMap()) {
        if (!key_.has_value()) {
            throw BuilderError("Value() in Dictionary without Key()");
        }
        Dict &dict = std::get<Dict>(top->GetValue());
        auto [it, inserted] = dict.emplace(std::move(key_.value()), Node(nullptr));

        if (!inserted) {
            throw BuilderError("Key already exists in the dictionary");
        }
        key_.reset();
        (&it->second)->GetValue() = std::move(value);
    } else if (top->IsArray()) {
        Array& arr = std::get<Array>(top->GetValue());
        arr.emplace_back(Node(std::move(value)));
    } else if (root_.IsNull()) {
        root_.GetValue() = std::move(value);
    } else {
        throw BuilderError("Value() called for unknown container");
    }
    return *this;
}

template <typename ContainerType>
auto Builder::StartContainer()
    -> std::conditional_t<std::is_same_v<ContainerType, Dict>, DictItemContext, ArrayItemContext>
{
    Node* top = nodes_stack_.back();

    if (top->IsMap()) {
        if (!key_.has_value()) {
            throw BuilderError("Value() in Dictionary without Key()");
        }

        Dict& dict = std::get<Dict>(top->GetValue());
        auto [it, inserted] = dict.emplace(std::move(key_.value()), ContainerType());

        if (!inserted) {
            throw BuilderError("Key already exists in the dictionary");
        }

        key_.reset();
        nodes_stack_.emplace_back(&it->second);
    } 
    else if (top->IsArray()) {
        Array& arr = std::get<Array>(top->GetValue());
        arr.emplace_back(ContainerType());
        nodes_stack_.emplace_back(&arr.back());
    } 
    else if (top->IsNull()) {
        top->GetValue() = ContainerType();
    } 
    else {
        throw BuilderError("Incorrect node (StartContainer)");
    }

    if constexpr (std::is_same_v<ContainerType, Dict>) {
        return DictItemContext(*this);
    } else {
        return ArrayItemContext(*this);
    }
}

DictItemContext Builder::StartDict() {
    return StartContainer<Dict>();
}

ArrayItemContext Builder::StartArray() {
    return StartContainer<Array>();
}

Builder& Builder::EndDict() {
    Node::Value &top = nodes_stack_.back()->GetValue();
    if (!std::holds_alternative<Dict>(top)) {
        throw BuilderError("EndDict() outside a dict");
    }

    nodes_stack_.pop_back();
    return *this;
}

Builder& Builder::EndArray() {
    Node::Value &top = nodes_stack_.back()->GetValue();
    if (!std::holds_alternative<Array>(top)) {
        throw BuilderError("EndArray() outside an array");
    }
    nodes_stack_.pop_back();
    return *this;
}

DictKeyContext::DictKeyContext(Builder& builder)
    : builder_(builder) {}

DictItemContext DictKeyContext::Value(Node::Value value) {
    builder_.Value(std::move(value));
    return DictItemContext(builder_);
}

ArrayItemContext DictKeyContext::StartArray() {
    builder_.StartArray();
    return ArrayItemContext(builder_);
}

DictItemContext DictKeyContext::StartDict() {
    builder_.StartDict();
    return DictItemContext(builder_);
}

DictItemContext::DictItemContext(Builder& builder)
    : builder_(builder) {}

DictKeyContext DictItemContext::Key(std::string key) {
    return builder_.Key(std::move(key));
}

Builder& DictItemContext::EndDict() {
    return builder_.EndDict();
}

ArrayItemContext::ArrayItemContext(Builder& builder)
    : builder_(builder) {}

ArrayItemContext ArrayItemContext::Value(Node::Value value) {
    builder_.Value(std::move(value));
    return *this;
}

DictItemContext ArrayItemContext::StartDict() {
    builder_.StartDict();
    return DictItemContext(builder_);
}

ArrayItemContext ArrayItemContext::StartArray() {
    builder_.StartArray();
    return *this;
}

Builder& ArrayItemContext::EndArray() {
    return builder_.EndArray();
}

}  // namespace json