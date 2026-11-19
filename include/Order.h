#pragma once

#include <string>

// Залежна сутність: замовлення, що оформлюється за конкретним столом.
// Один стіл може мати декілька замовлень, створених у різний час
// (модель не обмежується лише поточним замовленням столу).
struct Order
{
    int id = 0;
    int tableId = 0;            // зовнішній ключ на RestaurantTable.id
    std::string itemsDescription; // перелік страв замовлення
    std::string status;         // new | in_progress | completed | cancelled
    double totalAmount = 0.0;   // числове поле для агрегатних обчислень
    std::string orderedDate;    // формат YYYY-MM-DD
};
