#pragma once

#include "Order.h"

// Патерн Observer: інтерфейс підписника на події замовлення.
class IOrderObserver
{
public:
    virtual ~IOrderObserver() = default;

    virtual void onOrderCreated(const Order& order) = 0;
    virtual void onOrderCompleted(const Order& order) = 0;
};
