#pragma once

#include <optional>
#include <string>
#include <vector>

#include "json.h"

namespace json {

class DictKeyContext;
class DictItemContext;
class ArrayItemContext;

class Builder {
public:
    Builder();
    Node Build();
    void Reset();

    DictKeyContext Key(std::string key);
    Builder& Value(Node::Value value);

    DictItemContext StartDict();
    ArrayItemContext StartArray();

    Builder& EndDict();
    Builder& EndArray();

private:
    Node root_;
    std::vector<Node*> nodes_stack_;
    bool built_;
    std::optional<std::string> key_;

    template <typename ContainerType>
    auto StartContainer()
        -> std::conditional_t<std::is_same_v<ContainerType, Dict>, DictItemContext, ArrayItemContext>;
};

class DictItemContext {
public:
    DictItemContext(Builder& builder);

    DictKeyContext Key(std::string key);
    Builder& EndDict();

private:
    Builder& builder_;
};

class ArrayItemContext {
public:
    ArrayItemContext(Builder& builder);

    ArrayItemContext Value(Node::Value value);
    DictItemContext StartDict();
    Builder& EndArray();
    ArrayItemContext StartArray();

private:
    Builder& builder_;
};

class DictKeyContext {
public:
    DictKeyContext(Builder& builder);
    
    DictItemContext Value(Node::Value value);
    ArrayItemContext StartArray();
    DictItemContext StartDict();
    
private:
    Builder& builder_;
};

}  // namespace json