#pragma once

#include <memory>

#include "Database.h"
#include "Order.h"
#include "OrderState.h"
#include "OrderSubject.h"

// Інтегрує патерн State з існуючою архітектурою ORM/Repository:
// кожен успішний перехід оновлює поле Order.status у базі даних,
// додає запис в історію переходів (OrderStateHistory) та сповіщає
// підписників (патерн Observer) про створення/завершення замовлення.
class OrderWorkflow
{
public:
    OrderWorkflow(Order order, Storage& storage, OrderSubject& subject);

    // Створює нове замовлення в стані "created" у базі даних
    // та сповіщає підписників про подію створення.
    static OrderWorkflow createNew(Order order, Storage& storage, OrderSubject& subject);

    void accept();
    void complete();
    void cancel();

    const Order& order() const { return order_; }
    std::string currentStateName() const { return state_->name(); }

private:
    void applyTransition(std::unique_ptr<OrderState> newState);

    Order order_;
    std::unique_ptr<OrderState> state_;
    Storage& storage_;
    OrderSubject& subject_;
};
