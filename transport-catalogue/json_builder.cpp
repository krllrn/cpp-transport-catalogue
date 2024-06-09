#include "json_builder.h"

namespace json {
    using namespace std::string_literals;

    Builder::Builder()
        : root_(),
        nodes_stack_{ &root_ }
    {
    }

    Builder::KeyContext Builder::Key(std::string key)
    {
        auto& current_value = GetCurrentValue();

        if (!std::holds_alternative<json::Dict>(current_value)) {
            throw std::logic_error("[Key]: The key can only be added to the json::Dict"s);
        }
        nodes_stack_.emplace_back(&std::get<json::Dict>(current_value)[std::move(key)]);
        return KeyContext(*this);
    }

    Builder& Builder::Value(json::Node value)
    {
        auto& current_value = GetCurrentValue();

        if (std::holds_alternative<json::Array>(current_value)) {
            std::get<Array>(current_value).emplace_back(std::move(value));
        }
        else {
            if (!std::holds_alternative<std::nullptr_t>(current_value)) {
                throw std::logic_error("[Value]: Back value isn't null"s);
            }
            current_value = std::move(value.GetValue());
            nodes_stack_.pop_back();
        }
        return *this;
    }


    Builder::DictStartContext Builder::StartDict()
    {
        auto& current_value = GetCurrentValue();

        if (std::holds_alternative<json::Dict>(current_value)) {
            throw std::logic_error("[StartDict]: Dict can't be created as none value"s);
        }

        if (std::holds_alternative<json::Array>(current_value)) {
            Node& node
                = std::get<Array>(current_value).emplace_back(std::move(json::Dict()));
            nodes_stack_.push_back(&node);
        }
        else {
            current_value = json::Dict{};
        }

        return DictStartContext(*this);
    }

    Builder::ArrayContext Builder::StartArray()
    {
        auto& current_value = GetCurrentValue();

        if (std::holds_alternative<json::Dict>(current_value)) {
            throw std::logic_error("[StartArray]: Array can't be created as none value"s);
        }

        if (std::holds_alternative<json::Array>(current_value)) {
            Node& node
                = std::get<Array>(current_value).emplace_back(std::move(json::Array()));
            nodes_stack_.push_back(&node);
        }
        else {
            current_value = json::Array{};
        }

        return ArrayContext(*this);
    }

    Builder& Builder::EndDict()
    {
        if (!std::holds_alternative<json::Dict>(GetCurrentValue())) {
            throw std::logic_error("[EndDict]: It's not the end of json::Dict"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder& Builder::EndArray()
    {
        if (!std::holds_alternative<json::Array>(GetCurrentValue())) {
            throw std::logic_error("[EndArray]: It's not the end of json::Array"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    json::Node Builder::Build()
    {
        if (nodes_stack_.size() != 0) {
            throw std::logic_error("[Build]: Stack not empty"s);
        }
        return std::move(root_);
    }

    Node::Value& Builder::GetCurrentValue() {
        if (nodes_stack_.size() == 0) {
            throw std::logic_error("[GetCurrentValue]: Stack is empty"s);
        }
        return nodes_stack_.back()->GetValue();
    }

    Builder::DictStartContext Builder::ArrayContext::StartDict()
    {
        return DictStartContext(builder_.StartDict());
    }
    Builder::DictStartContext Builder::ArrayValueContext::StartDict()
    {
        return DictStartContext(builder_.StartDict());
    }
}


