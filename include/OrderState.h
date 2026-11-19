#pragma once

#include <memory>
#include <string>

class OrderWorkflow;

// Патерн State: абстрактний інтерфейс стану замовлення.
// Кожен стан визначає, які переходи з нього допустимі; недопустимий
// перехід є помилкою використання (обробляється через виняток).
class OrderState
{
public:
    virtual ~OrderState() = default;

    virtual std::unique_ptr<OrderState> accept();
    virtual std::unique_ptr<OrderState> complete();
    virtual std::unique_ptr<OrderState> cancel();

    virtual std::string name() const = 0;

protected:
    [[noreturn]] void rejectTransition(const std::string& action) const;
};

class CreatedState : public OrderState
{
public:
    std::unique_ptr<OrderState> accept() override;
    std::unique_ptr<OrderState> cancel() override;
    std::string name() const override { return "created"; }
};

class AcceptedState : public OrderState
{
public:
    std::unique_ptr<OrderState> complete() override;
    std::unique_ptr<OrderState> cancel() override;
    std::string name() const override { return "accepted"; }
};

class CompletedState : public OrderState
{
public:
    std::string name() const override { return "completed"; }
};

class CancelledState : public OrderState
{
public:
    std::string name() const override { return "cancelled"; }
};

std::unique_ptr<OrderState> makeOrderState(const std::string& name);

// Додатковий (четвертий) патерн, обраний самостійно: Factory Method
// (породжуючий патерн). Інкапсулює створення конкретних об'єктів стану
// за рядковим ім'ям (зокрема, збереженим у базі даних), завдяки чому
// клієнтський код (OrderWorkflow) не залежить від конкретних класів
// CreatedState / AcceptedState / CompletedState / CancelledState.
class OrderStateFactory
{
public:
    static std::unique_ptr<OrderState> create(const std::string& name);
};
