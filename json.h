#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

	class Node;
	using Dict = std::map<std::string, Node>;
	using Array = std::vector<Node>;
	using Variable = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

	// Эта ошибка должна выбрасываться при ошибках парсинга JSON
	class ParsingError : public std::runtime_error {
	public:
		using runtime_error::runtime_error;
	};

	class Node final : private Variable {
	public:
		using variant::variant;

		bool IsNull() const;
		bool IsInt() const;
		bool IsDouble() const;
		bool IsPureDouble() const;
		bool IsString() const;
		bool IsBool() const;
		bool IsArray() const;
		bool IsMap() const;

		const Array& AsArray() const;
		bool AsBool() const;
		double AsDouble() const;
		const int& AsInt() const;
		const Dict& AsMap() const;
		const std::string& AsString() const;
		const Variable& GetNode() const { return *this; }

		bool operator==(const Node& right) const;
		bool operator!=(const Node& right) const;
	};

	class Document {
	public:
		explicit Document(Node root);

		const Node& GetRoot() const;

		bool operator==(const Document& right) const;
		bool operator!=(const Document& right) const;

	private:
		Node root_;
	};

	Document Load(std::istream& input);

	void Print(const Document& doc, std::ostream& output);

}  // namespace json