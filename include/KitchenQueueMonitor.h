#pragma once

#include <vector>

#include "IOrderObserver.h"

// Конкретний підписник: монітор черги кухні. Тримає перелік ідентифікаторів
// замовлень, що зараз очікують на виконання на кухні.
class KitchenQueueMonitor : public IOrderObserver
{
public:
    void onOrderCreated(const Order& order) override;
    void onOrderCompleted(const Order& order) override;

    const std::vector<int>& pendingOrderIds() const { return pendingOrderIds_; }

private:
    std::vector<int> pendingOrderIds_;
};
