#include "json.h"

#include <cmath>

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            return Node(move(result));
        }

        Node LoadString(istream& input) {
            std::string json_line;
            for (char c; input.get(c) && c != '\"';) {
                if (c == '\\') {
                    input.get(c);
                    if (!input.eof()) {
                        if (c == 'n') {
                            json_line += '\n';
                            continue;
                        }
                        else if (c == 'r') {
                            json_line += '\r';
                            continue;
                        }
                        else if (c == 't') {
                            json_line += '\t';
                            continue;
                        }
                        else if (c == '\"' || c == '\\') {
                            json_line += c;
                            continue;
                        }
                    }
                }
                else {
                    json_line += c;
                }
            }
            if (input.eof())  throw ParsingError("Failed to load String"s);
            return Node(move(json_line));
        }

        Node LoadDict(istream& input) {
            Dict result;

            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }

            return Node(move(result));
        }

        using Number = std::variant<int, double>;

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

        Node LoadConst(istream& input) {
            string result;
            while (isalpha(input.peek())) {
                result.push_back(static_cast<char>(input.get()));
            }
            if (result == "true"s) {
                return Node(true);
            }
            else if (result == "false"s) {
                return Node(false);
            }
            else if (result == "null"s) {
                return Node();
            }
            else {
                throw ParsingError("Failed to load Bool"s);
            }
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                input >> c;
                if (input.eof()) throw ParsingError("Failed to load Array"s);
                input.unget();
                return LoadArray(input);
            }
            else if (c == '{') {
                input >> c;
                if (input.eof()) throw ParsingError("Failed to load Dict"s);
                input.unget();
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 't' || c == 'f' || c == 'n') {
                input.unget();
                return LoadConst(input);
            }
            else {
                input.unget();
                return LoadNumber(input);
            }
        }

    }  // namespace
    ///////////////////////////////////////

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(GetNode());
    }
    bool Node::IsInt() const {
        return std::holds_alternative<int>(GetNode());
    }
    bool Node::IsDouble() const {
        return IsPureDouble() || IsInt();
    }
    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(GetNode());
    }
    bool Node::IsString() const {
        return std::holds_alternative<std::string>(GetNode());
    }
    bool Node::IsBool() const {
        return std::holds_alternative<bool>(GetNode());
    }
    bool Node::IsArray() const {
        return std::holds_alternative<Array>(GetNode());
    }
    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(GetNode());
    }
    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw std::invalid_argument("No matching variable type"s);
        }
        return std::get<Array>(GetNode());
    }
    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::invalid_argument("No matching variable type"s);
        }
        return std::get<bool>(GetNode());
    }
    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw std::logic_error("No matching variable type"s);
        }
        return IsPureDouble() ? std::get<double>(GetNode()) : static_cast<double>(AsInt());
    }
    const int& Node::AsInt() const {
        if (!IsInt()) {
            throw std::invalid_argument("No matching variable type");
        }
        return std::get<int>(GetNode());
    }
    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw std::invalid_argument("No matching variable type");
        }
        return std::get<Dict>(GetNode());
    }
    const std::string& Node::AsString() const {
        if (!IsString()) {
            throw std::invalid_argument("No matching variable type");
        }
        return std::get<string>(GetNode());
    }

    //////////////////////////////////////
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
        auto root = doc.GetRoot();
        if (root.IsNull()) {
            output << "null"s;
            return;
        }
        if (root.IsInt()) {
            output << root.AsInt();
            return;
        }
        if (root.IsPureDouble()) {
            output << root.AsDouble();
            return;
        }
        if (root.IsString()) {
            std::string text = root.AsString();
            std::string json_line;
            for (size_t i = 0; i != text.size(); ++i) {
                char c = text[i];
                if (c == '\n') {
                    json_line += "\\n";
                    continue;
                }
                else if (c == '\r') {
                    json_line += "\\r";
                    continue;
                }
                else if (c == '\"') {
                    json_line += "\\\"";
                    continue;
                }
                else if (c == '\t') {
                    json_line += "\\t";
                    continue;
                }
                else if (c == '\\') {
                    json_line += "\\\\";
                    continue;
                }

                else {
                    json_line += c;
                }
            }
            output << "\""s << json_line << "\""s;
            return;
        }
        if (root.IsBool()) {
            root.AsBool() ? output << "true"s : output << "false"s;
            return;
        }
        if (root.IsArray()) {
            Array vector = root.AsArray();
            bool f = 0;
            output << '[';
            for (size_t j = 0; j < vector.size(); ++j) {
                if (f == 1) output << ", "s;
                Print(Document(vector[j]), output);
                f = 1;
            }
            output << ']';
            return;
        }
        if (root.IsMap()) {
            string word;
            output << '{' << endl;
            auto map = root.AsMap();
            bool f = 0;
            for (auto& [key, value] : map) {
                if (f == 1) output << ",\n"s;
                output << "\""s << key << "\""s << ": "s;
                Print(Document(value), output);
                f = 1;
            }
            output << '}' << endl;
            return;
        }
    }

    bool Node::operator==(const Node& right) const {
        return GetNode() == right.GetNode();
    }

    bool Node::operator!=(const Node& right) const {
        return GetNode() != right.GetNode();
    }

    bool Document::operator==(const Document& right) const {
        return root_ == right.root_;
    }

    bool Document::operator!=(const Document& right) const {
        return root_ != right.root_;
    }

}  // namespace json
