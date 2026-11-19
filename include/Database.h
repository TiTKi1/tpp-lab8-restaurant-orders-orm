#pragma once

#include <string>

#include <sqlite_orm/sqlite_orm.h>

#include "RestaurantTable.h"
#include "Order.h"
#include "OrderStateHistory.h"
#include "OrderEvent.h"

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
                "itemsCount",
                &Order::itemsCount,
                default_value(0)
            ),
            make_column(
                "urgencyLevel",
                &Order::urgencyLevel,
                default_value(0)
            ),
            make_column(
                "waitTimeMinutes",
                &Order::waitTimeMinutes,
                default_value(0)
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
        ),

        // Історія переходів станів замовлення (патерн State).
        make_table(
            "OrderStateHistory",
            make_column(
                "id",
                &OrderStateHistory::id,
                primary_key().autoincrement()
            ),
            make_column(
                "orderId",
                &OrderStateHistory::orderId
            ),
            make_column(
                "fromState",
                &OrderStateHistory::fromState,
                not_null()
            ),
            make_column(
                "toState",
                &OrderStateHistory::toState,
                not_null()
            ),
            make_column(
                "changedDate",
                &OrderStateHistory::changedDate,
                not_null()
            ),
            foreign_key(
                &OrderStateHistory::orderId
            ).references(
                &Order::id
            )
        ),

        // Журнал подій замовлення (підписник патерну Observer).
        make_table(
            "OrderEvent",
            make_column(
                "id",
                &OrderEvent::id,
                primary_key().autoincrement()
            ),
            make_column(
                "orderId",
                &OrderEvent::orderId
            ),
            make_column(
                "eventType",
                &OrderEvent::eventType,
                not_null()
            ),
            make_column(
                "eventDate",
                &OrderEvent::eventDate,
                not_null()
            ),
            foreign_key(
                &OrderEvent::orderId
            ).references(
                &Order::id
            )
        )
    );
}

using Storage = decltype(createStorage(""));
