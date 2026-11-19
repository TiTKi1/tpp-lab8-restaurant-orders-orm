#include "KitchenQueueMonitor.h"

#include <algorithm>
#include <iostream>

void KitchenQueueMonitor::onOrderCreated(const Order& order)
{
    pendingOrderIds_.push_back(order.id);
    std::cout << "[KitchenQueueMonitor] Order #" << order.id
              << " added to kitchen queue. Queue length: "
              << pendingOrderIds_.size() << '\n';
}

void KitchenQueueMonitor::onOrderCompleted(const Order& order)
{
    pendingOrderIds_.erase(
        std::remove(pendingOrderIds_.begin(), pendingOrderIds_.end(), order.id),
        pendingOrderIds_.end()
    );
    std::cout << "[KitchenQueueMonitor] Order #" << order.id
              << " removed from kitchen queue. Queue length: "
              << pendingOrderIds_.size() << '\n';
}
