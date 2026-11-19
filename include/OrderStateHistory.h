#pragma once

#include <string>

// Запис історії переходів станів замовлення (патерн State).
// Кожен успішний перехід зберігається в базі даних для подальшого аудиту.
struct OrderStateHistory
{
    int id = 0;
    int orderId = 0;       // зовнішній ключ на Order.id
    std::string fromState;
    std::string toState;
    std::string changedDate; // формат YYYY-MM-DD HH:MM
};
