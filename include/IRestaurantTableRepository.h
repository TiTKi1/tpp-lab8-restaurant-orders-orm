#pragma once

#include <optional>
#include <vector>

#include "RestaurantTable.h"

// Інтерфейс доступу до даних основної сутності RestaurantTable (патерн Repository).
class IRestaurantTableRepository
{
public:
    virtual ~IRestaurantTableRepository() = default;

    // Create
    virtual int add(const RestaurantTable& table) = 0;

    // Read
    virtual std::optional<RestaurantTable> findById(int id) = 0;
    virtual std::vector<RestaurantTable> findAll() = 0;

    // Update
    virtual bool update(const RestaurantTable& table) = 0;

    // Delete
    virtual bool remove(int id) = 0;
};
