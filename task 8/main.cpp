#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <unordered_map>

class Expression {
public:
    virtual ~Expression() = default;
    virtual int evaluate(const std::map<std::string, int>& context) const = 0;
    virtual void print(std::ostream& os) const = 0;
};

class Constant : public Expression {
    int value;
    explicit Constant(int val) : value(val) {}

    friend class ExpressionFactory;

public:
    int evaluate(const std::map<std::string, int>&) const override {
        return value;
    }

    void print(std::ostream& os) const override {
        os << value;
    }
};

class Variable : public Expression {
    std::string name;
    explicit Variable(std::string varName) : name(std::move(varName)) {}

    friend class ExpressionFactory;

public:
    int evaluate(const std::map<std::string, int>& context) const override {
        return context.at(name);
    }

    void print(std::ostream& os) const override {
        os << name;
    }
};


template <typename Op>
class BinaryOperation : public Expression {
    std::shared_ptr<Expression> left;
    std::shared_ptr<Expression> right;
    char opSymbol;

public:
    BinaryOperation(std::shared_ptr<Expression> l,
                    std::shared_ptr<Expression> r,
                    char sym)
            : left(std::move(l)), right(std::move(r)), opSymbol(sym) {}

    int evaluate(const std::map<std::string, int>& context) const override {
        return Op::apply(left->evaluate(context), right->evaluate(context));
    }

    void print(std::ostream& os) const override {
        os << "(";
        left->print(os);
        os << " " << opSymbol << " ";
        right->print(os);
        os << ")";
    }
};

struct Add {
    static int apply(int a, int b) { return a + b; }
};
struct Multiply {
    static int apply(int a, int b) { return a * b; }
};

using Addition = BinaryOperation<Add>;
using Multiplication = BinaryOperation<Multiply>;

class ExpressionFactory {
    std::unordered_map<int, std::shared_ptr<Constant>> constants;
    std::unordered_map<std::string, std::shared_ptr<Variable>> variables;

    ExpressionFactory() {
        for(int i = -5; i <= 256; ++i) {
            constants[i] = std::shared_ptr<Constant>(new Constant(i));
        }
    }

public:
    ExpressionFactory(const ExpressionFactory&) = delete;
    void operator=(const ExpressionFactory&) = delete;

    static ExpressionFactory& instance() {
        static ExpressionFactory factory;
        return factory;
    }

    std::shared_ptr<Constant> createConstant(int value) {
        if(value >= -5 && value <= 256) {
            return constants[value];
        }

        auto it = constants.find(value);
        if(it == constants.end()) {
            constants[value] = std::shared_ptr<Constant>(new Constant(value));
        }
        return constants[value];
    }

    std::shared_ptr<Variable> createVariable(const std::string& name) {
        auto& var = variables[name];
        if(!var) {
            var = std::shared_ptr<Variable>(new Variable(name));
        }
        return var;
    }

    void removeConstant(int value) {
        if(value < -5 || value > 256) {
            constants.erase(value);
        }
    }

    void removeVariable(const std::string& name) {
        variables.erase(name);
    }
};

template <typename T1, typename T2>
std::shared_ptr<Addition> add(T1&& l, T2&& r) {
    return std::make_shared<Addition>(
            std::forward<T1>(l),
            std::forward<T2>(r),
            '+'
    );
}

template <typename T1, typename T2>
std::shared_ptr<Multiplication> mul(T1&& l, T2&& r) {
    return std::make_shared<Multiplication>(
            std::forward<T1>(l),
            std::forward<T2>(r),
            '*'
    );
}

int main() {
    auto& factory = ExpressionFactory::instance();

    auto c = factory.createConstant(2);
    auto x = factory.createVariable("x");
    auto expr = add(c, x);

    std::cout << "Expression: ";
    expr->print(std::cout);
    std::cout << "\n";

    std::map<std::string, int> context{{"x", 3}};
    std::cout << "Result: " << expr->evaluate(context) << "\n";

    auto expr2 = mul(expr, factory.createConstant(4));
    std::cout << "Expression 2: ";
    expr2->print(std::cout);
    std::cout << "\nResult: " << expr2->evaluate(context) << "\n";

    factory.removeVariable("x");
}