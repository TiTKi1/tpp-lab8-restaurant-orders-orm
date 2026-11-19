#pragma once

#include <string>

#include <sqlite_orm/sqlite_orm.h>

#include "RestaurantTable.h"
#include "Order.h"

// Створює ORM-сховище з відображенням RestaurantTable (1) -> Order (N).
inline auto createStorage(const std::string& filename)
{
    using namespace sqlite_orm;

    return make_storage(
        filename,

        // Таблиця столів ресторану (основна сутність)
        make_table(
            "RestaurantTable",
            make_column(
                "id",
                &RestaurantTable::id,
                primary_key().autoincrement()
            ),
            make_column(
                "tableNumber",
                &RestaurantTable::tableNumber,
                not_null()
            ),
            make_column(
                "seatsCount",
                &RestaurantTable::seatsCount,
                not_null()
            ),
            make_column(
                "location",
                &RestaurantTable::location,
                not_null()
            ),
            make_column(
                "status",
                &RestaurantTable::status,
                not_null()
            ),
            make_column(
                "openedDate",
                &RestaurantTable::openedDate,
                not_null()
            )
        ),

        // Таблиця замовлень (залежна сутність)
        make_table(
            "Order",
            make_column(
                "id",
                &Order::id,
                primary_key().autoincrement()
            ),
            make_column(
                "tableId",
                &Order::tableId
            ),
            make_column(
                "itemsDescription",
                &Order::itemsDescription,
                not_null()
            ),
            make_column(
                "status",
                &Order::status,
                not_null()
            ),
            make_column(
                "totalAmount",
                &Order::totalAmount
            ),
            make_column(
                "orderedDate",
                &Order::orderedDate,
                not_null()
            ),
            foreign_key(
                &Order::tableId
            ).references(
                &RestaurantTable::id
            )
        )
    );
}

using Storage = decltype(createStorage(""));
