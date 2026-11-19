#pragma once

#include "Database.h"
#include "IOrderObserver.h"

// Конкретний підписник: журнал замовлень. Зберігає кожну подію у таблиці
// OrderEvent через те саме ORM-сховище, що й основні сутності.
class OrderJournal : public IOrderObserver
{
public:
    explicit OrderJournal(Storage& storage);

    void onOrderCreated(const Order& order) override;
    void onOrderCompleted(const Order& order) override;

private:
    void record(const Order& order, const std::string& eventType);

    Storage& storage_;
};
