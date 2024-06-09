#pragma once

#include "json.h"

namespace json {
    class Builder {

        class KeyContext;
        class DictStartContext;
        class ArrayContext;
        class KeyValueContext;
        class ArrayValueContext;

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

            ArrayValueContext Value(json::Node value) {
                return ArrayValueContext(builder_.Value(std::move(value)));
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

        class ArrayValueContext {
        public:
            ArrayValueContext(Builder& builder)
                :builder_(builder)
            {}

            ArrayContext Value(json::Node value) {
                return builder_.Value(std::move(value));
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

            KeyValueContext Value(json::Node value) {
                return KeyValueContext(builder_.Value(std::move(value)));
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

        class KeyValueContext {
        public:
            KeyValueContext(Builder& builder)
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
    };
    
} // namespace json
    

/*
* должен компилироваться в следующих ситуациях:
    + Key: Value, StartDict, StartArray.
    + StartDict: Key, EndDict.
    + StartArray: Value, StartDict, StartArray, EndArray.
    
    StartArray => Value: Value, StartDict, StartArray, EndArray.
    Key => Value: Key, EndDict.
    
*/