#pragma once

#include "json.h"

namespace json {
    class Builder {

        class KeyContext;
        class DictStartContext;
        class ArrayContext;

    public:
        Builder();

        KeyContext Key(std::string key);

        DictStartContext StartDict();

        ArrayContext StartArray();

        Builder& EndDict();

        Builder& EndArray();

        json::Node Build();

        Builder& Value(json::Node value);

    private:
        Node root_;
        std::vector<Node*> nodes_stack_;

        Node::Value& GetCurrentValue();

        class DictStartContext {
        public:
            DictStartContext(Builder& builder)
                :builder_(builder)
            {}

            KeyContext Key(std::string key) {
                return builder_.Key(std::move(key));
            }

            Builder& EndDict() {
                return builder_.EndDict();
            }

        private:
            Builder& builder_;
        };

        class ArrayContext {
        public:
            ArrayContext(Builder& builder)
                :builder_(builder)
            {}

            ArrayContext Value(json::Node value) {
                return ArrayContext(builder_.Value(std::move(value)));
            }

            DictStartContext StartDict();

            ArrayContext StartArray() {
                return builder_.StartArray();
            }

            Builder& EndArray() {
                return builder_.EndArray();
            }


        private:
            Builder& builder_;
        };

        class KeyContext {
        public:
            KeyContext(Builder& builder)
                :builder_(builder)
            {}

            DictStartContext Value(json::Node value) {
                return DictStartContext(builder_.Value(std::move(value)));
            }

            DictStartContext StartDict() {
                return builder_.StartDict();
            }

            ArrayContext StartArray() {
                return builder_.StartArray();
            }

        private:
            Builder& builder_;
        };
    };
    
} // namespace json
    

/*
    + Key: Value, StartDict, StartArray.
    + StartDict: Key, EndDict.
    + StartArray: Value, StartDict, StartArray, EndArray.
    
    StartArray => Value: Value, StartDict, StartArray, EndArray.
    Key => Value: Key, EndDict.
    
*/