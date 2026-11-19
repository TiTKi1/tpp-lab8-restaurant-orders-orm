#include "SQLiteRestaurantTableRepository.h"

#include <stdexcept>

#include <sqlite3.h>

SQLiteRestaurantTableRepository::SQLiteRestaurantTableRepository(
    const std::string& databaseFile
)
    : storage_(createStorage(databaseFile))
{
    storage_.on_open = [](sqlite3* db)
    {
        if (sqlite3_exec(
                db,
                "PRAGMA foreign_keys = ON;",
                nullptr,
                nullptr,
                nullptr
            ) != SQLITE_OK)
        {
            throw std::runtime_error(
                "Cannot enable foreign key constraints"
            );
        }
    };

    storage_.open_forever();
    storage_.sync_schema();
}

int SQLiteRestaurantTableRepository::add(const RestaurantTable& table)
{
    return storage_.insert(table);
}

std::optional<RestaurantTable> SQLiteRestaurantTableRepository::findById(int id)
{
    return storage_.get_optional<RestaurantTable>(id);
}

std::vector<RestaurantTable> SQLiteRestaurantTableRepository::findAll()
{
    return storage_.get_all<RestaurantTable>();
}

bool SQLiteRestaurantTableRepository::update(const RestaurantTable& table)
{
    auto existing = storage_.get_optional<RestaurantTable>(table.id);

    if (!existing)
    {
        return false;
    }

    storage_.update(table);
    return true;
}

bool SQLiteRestaurantTableRepository::remove(int id)
{
    auto existing = storage_.get_optional<RestaurantTable>(id);

    if (!existing)
    {
        return false;
    }

    storage_.remove<RestaurantTable>(id);
    return true;
}

Storage& SQLiteRestaurantTableRepository::storage()
{
    return storage_;
}
