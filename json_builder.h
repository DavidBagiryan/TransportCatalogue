#pragma once

#include "json.h"

#include <memory>

namespace json {
// вспомогательная структура для visitor (получение значения из variant)
struct SettingVisitor {
    Node operator()(std::nullptr_t) const;
    Node operator()(int&& value) const;
    Node operator()(double&& value) const;
    Node operator()(bool&& value) const;
    Node operator()(std::string&& value) const;
    Node operator()(Array&& value) const;
    Node operator()(Dict&& value) const;
};

// вспомогательные классы для поиска ошибок на этапе компиляции
class DictItemContext;
class KeyItemContext;
class ArrayItemContext;
// класс, позваоляющий запускать цепочку вызовов для JSON
class Builder {
public:
    Builder() = default;
    
    Builder& Value(Variable&& value);
    KeyItemContext Key(const std::string&& key);
    
    DictItemContext StartDict();
    Builder& EndDict();
    
    ArrayItemContext StartArray();
    Builder& EndArray();
    
    Node Build();
    
private:
    Node root_;
    std::vector<std::unique_ptr<Node>> nodes_stack_;
    int counter_dict_ = 0;
    int counter_array_ = 0;
};

// шаблонный класс, позволяющий отследить ошибки неправильного вызова цепочки для JSON на этапе компиляции
class ItemContext {
public:
    ItemContext(Builder& builder)
        : builder_(builder) {
    }

protected:
    Builder& Get() {
        return builder_;
    }

private:
    Builder& builder_;
};

// вспоиогательный класс, позволяющий отследить ошибки неправильного вызова цепочки Key(...) для JSON на этапе компиляции
class KeyItemContext : public ItemContext {
public:
    KeyItemContext(Builder& builder)
        : ItemContext(builder) {
    }

    DictItemContext Value(Variable&& value);

    DictItemContext StartDict();

    ArrayItemContext StartArray();
};

// вспоиогательный класс, позволяющий отследить ошибки неправильного вызова цепочки Dict() для JSON на этапе компиляции
class DictItemContext : public ItemContext {
public:
    DictItemContext(Builder& builder)
        : ItemContext(builder) {
    }

    KeyItemContext Key(std::string&& value);

    Builder& EndDict();
};

// вспоиогательный класс, позволяющий отследить ошибки неправильного вызова цепочки Array() для JSON на этапе компиляции
class ArrayItemContext : public ItemContext {
public:
    ArrayItemContext(Builder& builder)
        : ItemContext(builder) {
    }

    ArrayItemContext Value(Variable&& value);

    DictItemContext StartDict();

    ArrayItemContext StartArray();

    Builder& EndArray();
};

} // namespace json