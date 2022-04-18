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

	class Node {
	public:
		Node() = default;
		Node(std::nullptr_t) { node_ = {}; }
		Node(Array array) :node_(std::move(array)) {}
		Node(Dict dictonary) :node_(std::move(dictonary)) {}
		Node(bool value) :node_(std::move(value)) {}
		Node(int value) :node_(std::move(value)) {}
		Node(double value) :node_(std::move(value)) {}
		Node(std::string str) :node_(std::move(str)) {}

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
		const Variable& GetNode() const { return node_; }

		inline bool operator==(const Node& right) const {
			return node_ == right.node_;
		}

		inline bool operator!=(const Node& right) const {
			return node_ != right.node_;
		}

	private:
		Variable node_;
	};

	class Document {
	public:
		explicit Document(Node root);

		const Node& GetRoot() const;

		inline bool operator==(const Document& right) const {
			return root_ == right.root_;
		}

		inline bool operator!=(const Document& right) const {
			return root_ != right.root_;
		}

	private:
		Node root_;
	};

	Document Load(std::istream& input);

	void Print(const Document& doc, std::ostream& output);

}  // namespace json