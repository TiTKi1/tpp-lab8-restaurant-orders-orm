#pragma once

#include <memory>
#include <string>

#include "Order.h"

// Патерн Strategy: обчислення пріоритету замовлення на основі кількості
// позицій, терміновості та часу очікування. Усі значення нормуються
// у діапазон [0; 1], 1 відповідає найвищому пріоритету.
class IPriorityStrategy
{
public:
    virtual ~IPriorityStrategy() = default;

    virtual double calculate(const Order& order) const = 0;
    virtual std::string name() const = 0;
};

// Максимум нормованих параметрів: пріоритет визначається тим параметром,
// що найбільше наблизився до свого максимуму.
class MaxNormalizedParamStrategy : public IPriorityStrategy
{
public:
    double calculate(const Order& order) const override;
    std::string name() const override { return "max-normalized-param"; }
};

// Захист суми: зважена сума нормованих параметрів, обмежена зверху
// значенням 1.0, щоб жодна комбінація факторів не перевищувала межу шкали.
class ProtectedSumStrategy : public IPriorityStrategy
{
public:
    double calculate(const Order& order) const override;
    std::string name() const override { return "protected-sum"; }
};

// Нормована залишку суми: пріоритет зростає в міру наближення часу
// очікування до граничного значення SLA (залишок часового бюджету).
class NormalizedRemainderStrategy : public IPriorityStrategy
{
public:
    double calculate(const Order& order) const override;
    std::string name() const override { return "normalized-remainder"; }
};

// Контекст патерну Strategy: дозволяє обирати та змінювати алгоритм
// обчислення пріоритету під час виконання програми.
class PriorityCalculator
{
public:
    explicit PriorityCalculator(std::unique_ptr<IPriorityStrategy> strategy);

    void setStrategy(std::unique_ptr<IPriorityStrategy> strategy);
    double calculate(const Order& order) const;
    std::string strategyName() const;

private:
    std::unique_ptr<IPriorityStrategy> strategy_;
};
