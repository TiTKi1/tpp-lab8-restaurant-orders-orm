#include "OrderSubject.h"

#include <algorithm>

void OrderSubject::subscribe(const std::weak_ptr<IOrderObserver>& observer)
{
    observers_.push_back(observer);
}

void OrderSubject::notifyCreated(const Order& order)
{
    purgeExpired();

    for (const auto& weakObserver : observers_)
    {
        if (auto observer = weakObserver.lock())
        {
            observer->onOrderCreated(order);
        }
    }
}

void OrderSubject::notifyCompleted(const Order& order)
{
    purgeExpired();

    for (const auto& weakObserver : observers_)
    {
        if (auto observer = weakObserver.lock())
        {
            observer->onOrderCompleted(order);
        }
    }
}

std::size_t OrderSubject::activeSubscriberCount() const
{
    std::size_t count = 0;
    for (const auto& weakObserver : observers_)
    {
        if (!weakObserver.expired())
        {
            ++count;
        }
    }
    return count;
}

void OrderSubject::purgeExpired()
{
    observers_.erase(
        std::remove_if(
            observers_.begin(),
            observers_.end(),
            [](const std::weak_ptr<IOrderObserver>& observer)
            {
                return observer.expired();
            }
        ),
        observers_.end()
    );
}
