#include "json_builder.h"

namespace json {
// вспомогательная структура для visitor (получение значения из variant)
Node SettingVisitor::operator()(std::nullptr_t) const {
    return Node();
}
Node SettingVisitor::operator()(int&& value) const {
    return Node(value);
}
Node SettingVisitor::operator()(double&& value) const {
    return Node(value);
}
Node SettingVisitor::operator()(bool&& value) const {
    return Node(value);
}
Node SettingVisitor::operator()(std::string&& value) const {
    return Node(std::move(value));
}
Node SettingVisitor::operator()(Array&& value) const {
    return Node(std::move(value));
}
Node SettingVisitor::operator()(Dict&& value) const {
    return Node(std::move(value));
}

// класс, позваоляющий запускать цепочку вызовов для JSON
Builder& Builder::Value(Variable&& value) {
    Node node = std::visit(SettingVisitor{}, std::move(value));
    if (nodes_stack_.empty()) {
        nodes_stack_.emplace_back(std::make_unique<Node>(std::move(node)));
    }
    else if (nodes_stack_.back()->IsArray() && counter_array_ > 0) {
        Array& value = const_cast<Array&>(nodes_stack_.back()->AsArray());
        value.push_back(std::move(node));
    }
    else if (nodes_stack_.back()->IsString() && counter_dict_ > 0) {
        Node key_node = *nodes_stack_.back();
        nodes_stack_.pop_back();
        Dict& value = const_cast<Dict&>(nodes_stack_.back()->AsMap());
        value.insert({ key_node.AsString(), std::move(node) });
    }
    else {
        throw std::logic_error("Builder failed on Value");
    }
    return *this;
}
    
KeyItemContext Builder::Key(const std::string&& key) {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
        throw std::logic_error("Builder failed on Key");
    }
    
    nodes_stack_.emplace_back(std::make_unique<Node>(key));
    return KeyItemContext{*this};
}

DictItemContext Builder::StartDict() {
    if (!nodes_stack_.empty() && 
        !(nodes_stack_.back()->IsString() || nodes_stack_.back()->IsArray())) {
        throw std::logic_error("Builder failed on StartDict");
    }
    
    ++counter_dict_;
    
    nodes_stack_.emplace_back(std::make_unique<Node>(Node(Dict{})));
    return DictItemContext{*this};
}
    
Builder& Builder::EndDict() {
    if (counter_dict_ <= 0 && counter_dict_ < counter_array_) {
        throw std::logic_error("Builder failed on EndDict");
    }
    
    --counter_dict_;
    
    Dict value = nodes_stack_.back()->AsMap();
    nodes_stack_.pop_back();
    return Value(std::move(value));
}
 
ArrayItemContext Builder::StartArray() {
    if (!nodes_stack_.empty() && 
        !(nodes_stack_.back()->IsString() || nodes_stack_.back()->IsArray())) {
        throw std::logic_error("Builder failed on StartArray");
    }
    
    ++counter_array_;
    
    nodes_stack_.emplace_back(std::make_unique<Node>(Node(Array{})));
    return ArrayItemContext{*this};
}
    
Builder& Builder::EndArray() {
    if (counter_array_ <= 0 && counter_array_ < counter_dict_) {
        throw std::logic_error("Builder failed on EndArray");
    }
    
    --counter_array_;
    
    Array value = nodes_stack_.back()->AsArray();
    nodes_stack_.pop_back();
    return Value(std::move(value));
}

Node Builder::Build() {
    if (counter_dict_ < 0) {
        throw std::logic_error("Too much EndDict");
    }
    else if (counter_array_ < 0) {
        throw std::logic_error("Too much EndArray");
    }
    else if (counter_dict_ > 0) {
        throw std::logic_error("Not enough EndDict");
    }
    else if (counter_array_ > 0) {
        throw std::logic_error("Not enough EndArray");
    }
    else if (nodes_stack_.size() != 1) {
        throw std::logic_error("Builder failed");
    }
    
    root_ = *nodes_stack_.back();
    nodes_stack_.pop_back();
    
    return root_;
}

// вспоиогательный класс, позволяющий отследить ошибки неправильного вызова цепочки Key(...) для JSON на этапе компиляции
DictItemContext KeyItemContext::Value(Variable&& value) {
    Get().Value(std::move(value));
    return DictItemContext{Get()};
}
DictItemContext KeyItemContext::StartDict() {
    return Get().StartDict();
}
ArrayItemContext KeyItemContext::StartArray() {
    return Get().StartArray();
}
// вспоиогательный класс, позволяющий отследить ошибки неправильного вызова цепочки Dict() для JSON на этапе компиляции
KeyItemContext DictItemContext::Key(std::string&& value) {
    return Get().Key(std::move(value));
}
Builder& DictItemContext::EndDict() {
    return Get().EndDict();
}
// вспоиогательный класс, позволяющий отследить ошибки неправильного вызова цепочки Array() для JSON на этапе компиляции
ArrayItemContext ArrayItemContext::Value(Variable&& value)
{
    Get().Value(std::move(value));
    return ArrayItemContext{Get()};
}
DictItemContext ArrayItemContext::StartDict() {
    return Get().StartDict();
}
ArrayItemContext ArrayItemContext::StartArray() {
    return Get().StartArray();
}
Builder& ArrayItemContext::EndArray() {
    return Get().EndArray();
}
} // namespace json