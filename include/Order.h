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
    std::string status;         // created | accepted | completed | cancelled (патерн State)
    double totalAmount = 0.0;   // числове поле для агрегатних обчислень

    // Параметри для обчислення пріоритету замовлення (патерн Strategy).
    int itemsCount = 0;         // кількість позицій у замовленні
    int urgencyLevel = 0;       // терміновість, 1 (низька) .. 5 (висока)
    int waitTimeMinutes = 0;    // час очікування з моменту створення, хв

    std::string orderedDate;    // формат YYYY-MM-DD
};
