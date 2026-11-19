#include "RawSqliteDemo.h"

#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

#include <sqlite3.h>

#include "RestaurantTable.h"

namespace
{

int insertRestaurantTableRaw(sqlite3* db, const RestaurantTable& table)
{
    const char* sql = R"(
        INSERT INTO RestaurantTable (tableNumber, seatsCount, location, status, openedDate)
        VALUES (?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error(sqlite3_errmsg(db));
    }

    sqlite3_bind_int(stmt, 1, table.tableNumber);
    sqlite3_bind_int(stmt, 2, table.seatsCount);
    sqlite3_bind_text(stmt, 3, table.location.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, table.status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, table.openedDate.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        std::string error = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw std::runtime_error(error);
    }

    int id = static_cast<int>(sqlite3_last_insert_rowid(db));
    sqlite3_finalize(stmt);
    return id;
}

std::optional<RestaurantTable> findRestaurantTableByIdRaw(sqlite3* db, int id)
{
    const char* sql = R"(
        SELECT id, tableNumber, seatsCount, location, status, openedDate
        FROM RestaurantTable
        WHERE id = ?;
    )";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error(sqlite3_errmsg(db));
    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        RestaurantTable table;
        table.id = sqlite3_column_int(stmt, 0);
        table.tableNumber = sqlite3_column_int(stmt, 1);
        table.seatsCount = sqlite3_column_int(stmt, 2);
        table.location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        table.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        table.openedDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        sqlite3_finalize(stmt);
        return table;
    }

    if (rc != SQLITE_DONE)
    {
        std::string error = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw std::runtime_error(error);
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

} // namespace

void runRawSqliteDemo()
{
    sqlite3* db = nullptr;

    int rc = sqlite3_open("restaurant_orders_raw.db", &db);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << '\n';
        sqlite3_close(db);
        return;
    }

    const char* createSql = R"(
        CREATE TABLE IF NOT EXISTS RestaurantTable (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            tableNumber INTEGER NOT NULL,
            seatsCount INTEGER NOT NULL,
            location TEXT NOT NULL,
            status TEXT NOT NULL,
            openedDate TEXT NOT NULL
        );
    )";

    char* errorMessage = nullptr;
    rc = sqlite3_exec(db, createSql, nullptr, nullptr, &errorMessage);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << (errorMessage ? errorMessage : "unknown") << '\n';
        sqlite3_free(errorMessage);
        sqlite3_close(db);
        return;
    }

    std::cout << "\n=== RAW SQLITE C API DEMONSTRATION ===\n";

    try
    {
        RestaurantTable table{
            0,
            1,
            4,
            "зала",
            "вільний",
            "2026-11-01"
        };

        int id = insertRestaurantTableRaw(db, table);
        std::cout << "Inserted RestaurantTable ID: " << id << '\n';

        auto found = findRestaurantTableByIdRaw(db, id);
        if (found)
        {
            std::cout << "Found: table #" << found->tableNumber
                       << " | " << found->location
                       << " | " << found->status << '\n';
        }
        else
        {
            std::cout << "RestaurantTable not found\n";
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Raw SQLite error: " << ex.what() << '\n';
    }

    sqlite3_close(db);
}
