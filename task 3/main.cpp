#include <iostream>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include "../task 2/main.cpp"


template <typename... Types>
class TypeMap {
    using StorageType = std::tuple<std::optional<Types>...>;
    StorageType storage;

public:
    template <typename T>
    void AddValue(T&& value) {
        static_assert(TypeListUtilities::Contains<T, TypeListUtilities::TypeList<Types...>>::value,
                      "Type not allowed in this TypeMap");
        std::get<std::optional<T>>(storage) = std::forward<T>(value);
    }

    template <typename T>
    decltype(auto) GetValue() const {
        static_assert(TypeListUtilities::Contains<T, TypeListUtilities::TypeList<Types...>>::value,
                      "Type not allowed in this TypeMap");
        const auto& opt = std::get<std::optional<T>>(storage);
        if (!opt) throw std::runtime_error("Value not found");
        return *opt;
    }

    template <typename T>
    bool Contains() const {
        return std::get<std::optional<T>>(storage).has_value();
    }

    template <typename T>
    void RemoveValue() {
        std::get<std::optional<T>>(storage).reset();
    }
};


struct DataA { std::string value; };
struct DataB { int value; };

int main() {
    TypeMap<int, DataA, double, DataB> myTypeMap;

    myTypeMap.AddValue(42);
    myTypeMap.AddValue(DataA{"Hello"});
    myTypeMap.AddValue(3.14);
    myTypeMap.AddValue(DataB{10});

    std::cout << "DataA: " << myTypeMap.GetValue<DataA>().value << std::endl;
    myTypeMap.RemoveValue<double>();
    std::cout << "Contains double: " << myTypeMap.Contains<double>() << std::endl;
}
