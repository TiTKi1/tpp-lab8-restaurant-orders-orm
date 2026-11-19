#include "TestData.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "Database.h"

void prepareTestData()
{
    const std::string databaseFile = "restaurant_orders_queries.db";

    // Видаляємо лише окрему демонстраційну БД перед кожним запуском,
    // щоб забезпечити повторюваність результатів запитів.
    std::filesystem::remove(databaseFile);

    auto storage = createStorage(databaseFile);

    storage.on_open = [](sqlite3* db)
    {
        if (sqlite3_exec(
                db,
                "PRAGMA foreign_keys = ON;",
                nullptr,
                nullptr,
                nullptr
            ) != SQLITE_OK)
        {
            throw std::runtime_error("Cannot enable foreign keys");
        }
    };

    storage.open_forever();
    storage.sync_schema();

    std::vector<RestaurantTable> tables = {
        {0, 1, 2,  "зала",     "вільний",       "2026-09-01"},
        {0, 2, 4,  "зала",     "зайнятий",      "2026-09-01"},
        {0, 3, 6,  "тераса",   "вільний",       "2026-09-05"},
        {0, 4, 2,  "VIP",      "заброньований", "2026-09-10"},
        {0, 5, 8,  "зала",     "зайнятий",      "2026-09-12"}
    };

    std::vector<int> tableIds;
    for (const auto& table : tables)
    {
        tableIds.push_back(storage.insert(table));
    }

    std::vector<Order> orders = {
        {0, tableIds[0], "Капучино, круасан",               "completed",   125.0, 2, 1, 10, "2026-09-01"},
        {0, tableIds[0], "Салат Цезар",                      "completed",   210.5, 1, 2, 15, "2026-09-02"},
        {0, tableIds[0], "Паста Карбонара",                  "completed",   285.0, 3, 2, 20, "2026-09-03"},

        {0, tableIds[1], "Борщ, хліб",                       "completed",   160.0, 2, 1, 12, "2026-09-01"},
        {0, tableIds[1], "Стейк, картопля фрі",               "in_progress", 520.0, 2, 4, 35, "2026-09-11"},
        {0, tableIds[1], "Лимонад",                           "new",          65.0, 1, 1,  5, "2026-09-11"},

        {0, tableIds[2], "Піца Маргарита",                   "completed",   310.0, 1, 3, 25, "2026-09-05"},
        {0, tableIds[2], "Морозиво",                          "completed",   95.0, 1, 1,  8, "2026-09-06"},
        {0, tableIds[2], "Чай зелений",                       "completed",   45.0, 1, 1,  6, "2026-09-06"},

        {0, tableIds[3], "Суші-сет",                          "cancelled",   680.0, 5, 3, 40, "2026-09-10"},
        {0, tableIds[3], "Вино біле, келих",                  "completed",   150.0, 1, 2, 10, "2026-09-10"},
        {0, tableIds[3], "Фруктова тарілка",                  "completed",   220.0, 2, 2, 14, "2026-09-11"},

        {0, tableIds[4], "Бургер, кола",                      "new",         245.0, 3, 3, 18, "2026-09-12"},
        {0, tableIds[4], "Картопляні дольки",                 "new",          85.0, 1, 2,  9, "2026-09-12"},
        {0, tableIds[4], "Еспресо",                            "in_progress",  55.0, 1, 5, 50, "2026-09-13"}
    };

    for (const auto& order : orders)
    {
        storage.insert(order);
    }

    std::cout << "Test database prepared\n";
    std::cout << "RestaurantTable records: " << storage.count<RestaurantTable>() << '\n';
    std::cout << "Order records: " << storage.count<Order>() << '\n';
}
