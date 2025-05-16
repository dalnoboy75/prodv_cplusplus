#include <iostream>
#include <cassert>

// MixIn для операторов сравнения
template <typename Derived>
class less_than_comparable {
public:
    friend bool operator>(const Derived& lhs, const Derived& rhs) {
        return rhs < lhs;
    }

    friend bool operator<=(const Derived& lhs, const Derived& rhs) {
        return !(rhs < lhs);
    }

    friend bool operator>=(const Derived& lhs, const Derived& rhs) {
        return !(lhs < rhs);
    }

    friend bool operator==(const Derived& lhs, const Derived& rhs) {
        return !(lhs < rhs) && !(rhs < lhs);
    }

    friend bool operator!=(const Derived& lhs, const Derived& rhs) {
        return (lhs < rhs) || (rhs < lhs);
    }
};

// MixIn для подсчета экземпляров
template <typename T>
class counter {
protected:
    counter() { count_++; }
    counter(const counter&) { count_++; }
    counter(counter&&)  noexcept { count_++; }
    ~counter() { count_++; }

public:
    static size_t count() {
        return count_;
    }

private:
    inline static size_t count_ = 0;
};

// Класс Number с MixIn
class Number : public less_than_comparable<Number>,
               public counter<Number> {
public:
    explicit Number(int value) : m_value{value} {}
    Number(const Number&) = default;
    Number(Number&&) = default;
    ~Number() = default;

    int value() const { return m_value; }

    bool operator<(const Number& other) const {
        return m_value < other.m_value;
    }

private:
    int m_value;
};

int main() {
    Number one{1};
    Number two{2};
    Number three{3};
    Number four{4};

    std::cout << "Count: " << counter<Number>::count() << std::endl;

    {
        Number five{5};
        std::cout << "Count: " << counter<Number>::count() << std::endl;
    }

    std::cout << "Count: " << counter<Number>::count() << std::endl;

    assert(one >= one);
    assert(three <= four);
    assert(two == two);
    assert(three > two);
    assert(one < two);

    return 0;
}
