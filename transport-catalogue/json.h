#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {
    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final : private std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict> {
    public:
        using variant::variant;

        int AsInt() const;
        double AsDouble() const;
        bool AsBool() const;
        const std::string& AsString() const;
        std::nullptr_t AsNull() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        const variant& GetValue() const;

    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

    bool operator==(const Node& lhs, const Node& rhs);

    bool operator!=(const Node& lhs, const Node& rhs);

    bool operator==(const Document& lhs, const Document& rhs);

    bool operator!=(const Document& lhs, const Document& rhs);

}  // namespace json