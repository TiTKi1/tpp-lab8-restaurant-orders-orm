#pragma once

#include <string>

// Запис журналу подій замовлення (підписник патерну Observer — журнал замовлень).
struct OrderEvent
{
    int id = 0;
    int orderId = 0;      // зовнішній ключ на Order.id
    std::string eventType; // created | completed
    std::string eventDate; // формат YYYY-MM-DD HH:MM
};
