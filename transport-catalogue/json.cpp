#include "json.h"

#include <optional>

using namespace std;

namespace json {

    namespace {

        std::string LoadWord(std::istream& input) {
            std::string str;
            while (std::isalpha(input.peek())) {
                str.push_back(input.get());
            }
            return str;
        }

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
                };

            // Считывает одну или более цифр в parsed_num из input
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
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
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
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        std::string LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
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
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        Node LoadNode(istream& input);

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
            Dict result;

            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input);
                if (result.find(key) != result.end()) {
                    throw ParsingError("Dublicate key in dict: "s + key);
                }
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }

            if (!input) {
                throw ParsingError("Dict parsing error");
            }

            return Node(move(result));
        }

        Node LoadBool(istream& input) {
            std::string str = LoadWord(input);
            if (str == "true"s) {
                return Node{ true };
            }
            else if (str == "false"s) {
                return Node{ false };
            }
            else {
                throw ParsingError("Parsing bool error."s);
            }
        }

        Node LoadNull(istream& input) {
            std::string str = LoadWord(input);
            if (str == "null"s) {
                return Node{ nullptr };
            }
            else {
                throw ParsingError("Parsing nullptr error."s);
            }
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            switch (c)
            {
            case '[':
                return LoadArray(input);
            case '{':
                return LoadDict(input);
            case '"':
                return LoadString(input);
            case 't':
                input.putback(c);
                return LoadBool(input);
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

    //------------------------- Node -----------------------------------------------------------
    int Node::AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("Wrong type: not int"s);
        }
        return std::get<int>(*this);
    }

    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw std::logic_error("Wrong type: not double"s);
        }
        return IsPureDouble() ? std::get<double>(*this) : AsInt();
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("Wrong type: not bool"s);
        }
        return std::get<bool>(*this);
    }

    const string& Node::AsString() const {
        if (!IsString()) {
            throw std::logic_error("Wrong type: not string"s);
        }
        return std::get<std::string>(*this);
    }

    std::nullptr_t Node::AsNull() const {
        if (!IsNull()) {
            throw std::logic_error("Wrong type: not null"s);
        }
        return std::get<std::nullptr_t>(*this);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw std::logic_error("Wrong type: not array"s);
        }
        return std::get<Array>(*this);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw std::logic_error("Wrong type: not map"s);
        }
        return std::get<Dict>(*this);
    }

    bool Node::IsInt() const {
        return holds_alternative<int>(*this);
    }

    bool Node::IsPureDouble() const {
        return holds_alternative<double>(*this);
    }

    bool Node::IsDouble() const {
        return IsInt() || IsPureDouble();
    }

    bool Node::IsBool() const {
        return holds_alternative<bool>(*this);
    }

    bool Node::IsString() const {
        return holds_alternative<std::string>(*this);
    }

    bool Node::IsNull() const {
        return holds_alternative<std::nullptr_t>(*this);
    }

    bool Node::IsArray() const {
        return holds_alternative<Array>(*this);
    }

    bool Node::IsMap() const {
        return holds_alternative<Dict>(*this);
    }

    const Node::variant& Node::GetValue() const { 
        return *this;
    }

    Node::variant& Node::GetValue() {
        return *this;
    }

    // -------------------------------------- Print ------------------------------------------------

    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        // Возвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    void PrintNode(const Node& node, std::ostream& out);

    struct ValuePrinter {
        ValuePrinter(const PrintContext& ctx)
            : ctx_(ctx)
        {}

        void operator()(std::nullptr_t) const {
            ctx_.PrintIndent();
            ctx_.out << "null"sv;
        }
        void operator()(int i) const {
            ctx_.PrintIndent();
            ctx_.out << i;
        }
        void operator()(double d) const {
            ctx_.PrintIndent();
            ctx_.out << d;
        }
        void operator()(bool b) const {
            ctx_.PrintIndent();
            ctx_.out << std::boolalpha << b;
        }

        void operator()(std::string s) const {
            ctx_.PrintIndent();
            ctx_.out.put('\"');
            for (const auto c : s) {
                switch (c)
                {
                case '\n':
                    ctx_.out << "\\n"sv;
                    break;
                case '\t':
                    ctx_.out << "\\t"sv;
                    break;
                case '\r':
                    ctx_.out << "\\r"sv;
                    break;
                case '\"':
                    [[fallthrough]]; // атрибут показывает, что оператор break внутри блока case отсутствует намеренно
                case '\\':
                    ctx_.out.put('\\');
                    [[fallthrough]]; // атрибут показывает, что оператор break внутри блока case отсутствует намеренно
                default:
                    ctx_.out.put(c);
                    break;
                }
            }
            ctx_.out.put('\"');
        }

        void operator()(Array a) const {
            ctx_.out.put('[');
            bool is_first = true;
            for (const auto& n : a) {
                if (is_first == false) {
                    ctx_.out << ", "sv;
                }
                PrintNode(n, ctx_.out);
                is_first = false;
            }
            ctx_.out.put(']');
        }

        void operator()(Dict d) const {
            ctx_.out.put('{');
            bool is_first = true;
            for (const auto& [key, value] : d) {
                if (is_first == false) {
                    ctx_.out << ", "sv;
                }
                ctx_.PrintIndent();
                ctx_.out << "\""sv << key << "\":"sv;
                PrintNode(value, ctx_.out);
                is_first = false;
            }
            ctx_.out.put('}');
        }

        PrintContext ctx_;
    };

    void PrintNode(const Node& node, std::ostream& out) {
        std::visit(ValuePrinter{ PrintContext{ out } }, node.GetValue());
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

    void Print(const Document& doc, std::ostream& output) {
        const auto& node = doc.GetRoot();
        PrintNode(node, output);
    }

    bool operator==(const Node& lhs, const Node& rhs) {
        return lhs.GetValue() == rhs.GetValue();
    }

    bool operator!=(const Node& lhs, const Node& rhs) {
        return lhs.GetValue() != rhs.GetValue();
    }

    bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    bool operator!=(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() != rhs.GetRoot();
    }

}  // namespace json