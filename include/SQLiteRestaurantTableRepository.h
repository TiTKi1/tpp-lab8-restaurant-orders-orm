#pragma once

#include <string>

#include "IRestaurantTableRepository.h"
#include "Database.h"

// Реалізація Repository для RestaurantTable через sqlite_orm.
class SQLiteRestaurantTableRepository : public IRestaurantTableRepository
{
public:
    explicit SQLiteRestaurantTableRepository(const std::string& databaseFile);

    int add(const RestaurantTable& table) override;

    std::optional<RestaurantTable> findById(int id) override;
    std::vector<RestaurantTable> findAll() override;

    bool update(const RestaurantTable& table) override;
    bool remove(int id) override;

    // Доступ до сховища потрібен демонстраційному коду для роботи
    // із залежною сутністю Order та для транзакцій.
    Storage& storage();

private:
    Storage storage_;
};
