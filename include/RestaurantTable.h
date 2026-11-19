#pragma once

#include <string>

// Основна сутність: стіл ресторану.
struct RestaurantTable
{
    int id = 0;
    int tableNumber = 0;
    int seatsCount = 0;
    std::string location;    // зала | тераса | VIP
    std::string status;      // вільний | зайнятий | заброньований
    std::string openedDate;  // формат YYYY-MM-DD, дата введення столу в роботу
};
