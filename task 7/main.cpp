#include <iostream>
#include <vector>
#include <unordered_set>
#include <memory>
#include <algorithm>

class SetImpl {
public:
    virtual ~SetImpl() = default;
    virtual void add(int value) = 0;
    virtual void remove(int value) = 0;
    virtual bool contains(int value) const = 0;
    virtual size_t size() const = 0;
    virtual std::unique_ptr<SetImpl> clone() const = 0;
    virtual std::vector<int> get_elements() const = 0;
    virtual void print() const = 0;

    virtual std::unique_ptr<SetImpl> union_with(const SetImpl& other) const = 0;
    virtual std::unique_ptr<SetImpl> intersection_with(const SetImpl& other) const = 0;
};

class VectorSetImpl : public SetImpl {
    std::vector<int> data;

public:
    void add(int value) override {
        if (std::find(data.begin(), data.end(), value) == data.end()) {
            data.push_back(value);
        }
    }

    void remove(int value) override {
        data.erase(std::remove(data.begin(), data.end(), value), data.end());
    }

    bool contains(int value) const override {
        return std::find(data.begin(), data.end(), value) != data.end();
    }

    size_t size() const override { return data.size(); }

    std::unique_ptr<SetImpl> clone() const override {
        auto copy = std::make_unique<VectorSetImpl>();
        copy->data = data;
        return copy;
    }

    std::vector<int> get_elements() const override { return data; }

    void print() const override {
        std::cout << "VectorSet[ ";
        for (int v : data) std::cout << v << " ";
        std::cout << "]\n";
    }

    std::unique_ptr<SetImpl> union_with(const SetImpl& other) const override {
        auto result = std::make_unique<VectorSetImpl>();
        result->data = this->data;
        for (int val : other.get_elements()) {
            if (!contains(val)) {
                result->data.push_back(val);
            }
        }
        return result;
    }

    std::unique_ptr<SetImpl> intersection_with(const SetImpl& other) const override {
        auto result = std::make_unique<VectorSetImpl>();
        for (int val : data) {
            if (other.contains(val)) {
                result->data.push_back(val);
            }
        }
        return result;
    }
};

class HashSetImpl : public SetImpl {
    std::unordered_set<int> data;

public:
    void add(int value) override { data.insert(value); }

    void remove(int value) override { data.erase(value); }

    bool contains(int value) const override { return data.count(value); }

    size_t size() const override { return data.size(); }

    std::unique_ptr<SetImpl> clone() const override {
        auto copy = std::make_unique<HashSetImpl>();
        copy->data = data;
        return copy;
    }

    std::vector<int> get_elements() const override {
        return std::vector<int>(data.begin(), data.end());
    }

    void print() const override {
        std::cout << "HashSet{ ";
        for (int v : data) std::cout << v << " ";
        std::cout << "}\n";
    }

    std::unique_ptr<SetImpl> union_with(const SetImpl& other) const override {
        auto result = std::make_unique<HashSetImpl>();
        result->data = this->data;
        for (int val : other.get_elements()) {
            result->data.insert(val);
        }
        return result;
    }

    std::unique_ptr<SetImpl> intersection_with(const SetImpl& other) const override {
        auto result = std::make_unique<HashSetImpl>();
        for (int val : data) {
            if (other.contains(val)) {
                result->data.insert(val);
            }
        }
        return result;
    }
};

class Set {
    std::unique_ptr<SetImpl> impl;
    static constexpr size_t SWITCH_THRESHOLD = 20;

    void switch_to_hash() {
        auto elements = impl->get_elements();
        impl = std::make_unique<HashSetImpl>();
        for (int v : elements) impl->add(v);
    }

    void switch_to_vector() {
        auto elements = impl->get_elements();
        impl = std::make_unique<VectorSetImpl>();
        for (int v : elements) impl->add(v);
    }

public:
    Set() : impl(std::make_unique<VectorSetImpl>()) {}

    void add(int value) {
        if (impl->size() >= SWITCH_THRESHOLD &&
            dynamic_cast<VectorSetImpl*>(impl.get())) {
            switch_to_hash();
        }

        impl->add(value);
    }

    void remove(int value) {
        impl->remove(value);

        if (impl->size() <= SWITCH_THRESHOLD &&
            dynamic_cast<HashSetImpl*>(impl.get())) {
            switch_to_vector();
        }
    }

    bool contains(int value) const { return impl->contains(value); }
    size_t size() const { return impl->size(); }
    void print() const { impl->print(); }

    Set union_with(const Set& other) const {
        Set result;
        result.impl = impl->union_with(*other.impl);
        return result;
    }

    Set intersection_with(const Set& other) const {
        Set result;
        result.impl = impl->intersection_with(*other.impl);
        return result;
    }
};
int main() {
    Set s1;
    std::cout << "Adding elements to s1:\n";
    for (int i = 0; i < 25; ++i) {
        s1.add(i);
        s1.print();
    }

    Set s2;
    s2.add(10);
    s2.add(20);
    s2.add(25);
    s2.add(30);

    std::cout << "\nUnion of s1 and s2:\n";
    s1.union_with(s2).print();

    std::cout << "\nIntersection of s1 and s2:\n";
    s1.intersection_with(s2).print();

    std::cout << "\nRemoving elements from s1:\n";
    for (int i = 0; i < 20; ++i) {
        s1.remove(i);
        s1.print();
    }

    return 0;
}
