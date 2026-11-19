#include "OrderJournal.h"

#include <ctime>
#include <iostream>

namespace
{

std::string currentTimestamp()
{
    std::time_t now = std::time(nullptr);
    std::tm localTime{};
    localtime_s(&localTime, &now);

    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", &localTime);
    return buffer;
}

} // namespace

OrderJournal::OrderJournal(Storage& storage)
    : storage_(storage)
{
}

void OrderJournal::onOrderCreated(const Order& order)
{
    record(order, "created");
}

void OrderJournal::onOrderCompleted(const Order& order)
{
    record(order, "completed");
}

void OrderJournal::record(const Order& order, const std::string& eventType)
{
    OrderEvent event{0, order.id, eventType, currentTimestamp()};
    storage_.insert(event);

    std::cout << "[OrderJournal] Logged event '" << eventType
              << "' for order #" << order.id << '\n';
}
