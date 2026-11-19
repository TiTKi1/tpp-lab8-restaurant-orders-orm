#include <algorithm>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include <sqlite3.h>
#include <sqlite_orm/sqlite_orm.h>

#include "Database.h"
#include "IRestaurantTableRepository.h"
#include "KitchenQueueMonitor.h"
#include "OrderJournal.h"
#include "OrderSubject.h"
#include "OrderWorkflow.h"
#include "PriorityStrategy.h"
#include "RawSqliteDemo.h"
#include "SQLiteRestaurantTableRepository.h"
#include "TestData.h"

namespace
{

// 1. Базові операції ORM (insert / get_optional) на окремій БД.
void runOrmDemo()
{
    auto storage = createStorage("restaurant_orders_orm.db");
    storage.sync_schema();

    RestaurantTable table{
        0,
        10,
        4,
        "зала",
        "вільний",
        "2026-11-02"
    };

    int id = storage.insert(table);

    std::cout << "\n=== ORM DEMONSTRATION ===\n";
    std::cout << "Inserted RestaurantTable ID: " << id << '\n';

    auto found = storage.get_optional<RestaurantTable>(id);
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

// 2. Зв'язок 1:N між RestaurantTable і Order, перевірка зовнішнього ключа.
void runRelationshipDemo()
{
    using namespace sqlite_orm;

    auto storage = createStorage("restaurant_orders_orm.db");

    storage.on_open = [](sqlite3* db)
    {
        sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    };
    storage.open_forever();
    storage.sync_schema();

    std::cout << "\n=== RELATIONSHIP DEMONSTRATION (1:N) ===\n";

    RestaurantTable table{
        0,
        11,
        6,
        "тераса",
        "зайнятий",
        "2026-11-03"
    };
    int tableId = storage.insert(table);
    std::cout << "RestaurantTable ID: " << tableId << '\n';

    Order o1{0, tableId, "Салат Грецький",   "completed",   180.0, 2, 1, 15, "2026-11-03"};
    Order o2{0, tableId, "Чай трав'яний",    "completed",    60.0, 1, 1, 15, "2026-11-03"};

    int orderId1 = storage.insert(o1);
    int orderId2 = storage.insert(o2);
    std::cout << "Order records inserted: " << orderId1 << ", " << orderId2 << '\n';

    auto tableOrders = storage.get_all<Order>(
        where(c(&Order::tableId) == tableId)
    );

    std::cout << "\nOrders for table " << tableId << ":\n";
    for (const auto& order : tableOrders)
    {
        std::cout
            << "ID: " << order.id
            << ", Items: " << order.itemsDescription
            << ", Amount: " << order.totalAmount
            << '\n';
    }

    // Перевірка посилальної цілісності: некоректний tableId повинен бути відхилений.
    try
    {
        Order invalidOrder{0, -1, "Капучино", "new", 50.0, 1, 1, 0, "2026-11-04"};
        storage.insert(invalidOrder);
        std::cout << "Unexpected insertion success\n";
    }
    catch (const std::exception& ex)
    {
        std::cout << "\nForeign key constraint test:\n";
        std::cout << "Insertion rejected: " << ex.what() << '\n';
    }
}

// 3. CRUD через патерн Repository (IRestaurantTableRepository / SQLiteRestaurantTableRepository).
void runRepositoryDemo()
{
    SQLiteRestaurantTableRepository sqliteRepository("restaurant_orders_orm.db");
    IRestaurantTableRepository& repository = sqliteRepository;

    std::cout << "\n=== REPOSITORY DEMONSTRATION ===\n";

    // CREATE
    RestaurantTable table{
        0,
        12,
        2,
        "VIP",
        "вільний",
        "2026-11-05"
    };
    int id = repository.add(table);
    std::cout << "\nCREATE\nInserted RestaurantTable ID: " << id << '\n';

    // READ
    std::cout << "\nREAD\n";
    auto found = repository.findById(id);
    if (found)
    {
        std::cout << "ID: " << found->id << '\n';
        std::cout << "Table number: " << found->tableNumber << '\n';
        std::cout << "Status: " << found->status << '\n';
    }

    // UPDATE
    std::cout << "\nUPDATE\n";
    if (found)
    {
        found->status = "зайнятий";
        bool updated = repository.update(*found);
        std::cout << "Updated: " << std::boolalpha << updated << '\n';

        auto afterUpdate = repository.findById(id);
        if (afterUpdate)
        {
            std::cout << "New status: " << afterUpdate->status << '\n';
        }
    }

    // READ ALL
    std::cout << "\nREAD ALL\n";
    for (const auto& item : repository.findAll())
    {
        std::cout
            << item.id << " | "
            << item.tableNumber << " | "
            << item.status << '\n';
    }

    // DELETE
    std::cout << "\nDELETE\n";
    bool removed = repository.remove(id);
    std::cout << "Removed: " << std::boolalpha << removed << '\n';

    auto afterDelete = repository.findById(id);
    std::cout << "Record exists after deletion: "
               << std::boolalpha << afterDelete.has_value() << '\n';
}

// 4. Фільтрація, сортування та обмеження кількості результатів.
void runQueryDemo()
{
    using namespace sqlite_orm;

    auto storage = createStorage("restaurant_orders_queries.db");
    storage.sync_schema();

    std::cout << "\n=== ORM QUERY DEMONSTRATION ===\n";

    std::cout << "\n1. Vacant tables:\n";
    auto vacantTables = storage.get_all<RestaurantTable>(
        where(c(&RestaurantTable::status) == "вільний")
    );
    for (const auto& item : vacantTables)
    {
        std::cout << item.id << " | " << item.tableNumber << " | " << item.status << '\n';
    }

    std::cout << "\n2. Tables sorted by seats count:\n";
    auto sortedTables = storage.get_all<RestaurantTable>(
        order_by(&RestaurantTable::seatsCount).asc()
    );
    for (const auto& item : sortedTables)
    {
        std::cout << item.id << " | table #" << item.tableNumber
                   << " | seats: " << item.seatsCount << '\n';
    }

    std::cout << "\n3. Top 3 most expensive orders:\n";
    auto topOrders = storage.get_all<Order>(
        order_by(&Order::totalAmount).desc(),
        limit(3)
    );
    for (const auto& order : topOrders)
    {
        std::cout << order.id << " | " << order.itemsDescription << " | " << order.totalAmount << '\n';
    }

    std::cout << "\n4. Completed orders sorted by date:\n";
    auto completedOrders = storage.get_all<Order>(
        where(c(&Order::status) == "completed"),
        order_by(&Order::orderedDate).asc()
    );
    for (const auto& order : completedOrders)
    {
        std::cout << order.id << " | " << order.orderedDate << " | " << order.totalAmount << '\n';
    }
}

// 5. Агрегатні запити: COUNT, SUM, AVG, GROUP BY.
void runAggregateDemo()
{
    using namespace sqlite_orm;

    auto storage = createStorage("restaurant_orders_queries.db");
    storage.sync_schema();

    std::cout << "\n=== AGGREGATE QUERY DEMONSTRATION ===\n";

    auto orderCount = storage.count<Order>();
    std::cout << "\n1. Order count: " << orderCount << '\n';

    auto totalRevenue = storage.sum(&Order::totalAmount);
    std::cout << "\n2. Total revenue: " << (totalRevenue ? *totalRevenue : 0.0) << '\n';

    auto averageCheck = storage.avg(&Order::totalAmount);
    std::cout << "\n3. Average check: " << averageCheck << '\n';

    std::cout << "\n4. Total revenue by table:\n";
    auto revenueByTable = storage.select(
        columns(
            &Order::tableId,
            sum(&Order::totalAmount)
        ),
        group_by(&Order::tableId)
    );
    for (const auto& [tableId, totalAmount] : revenueByTable)
    {
        std::cout << "Table ID: " << tableId
                   << " | Total revenue: " << (totalAmount ? *totalAmount : 0.0) << '\n';
    }
}

// 6. Об'єднання таблиць RestaurantTable і Order (INNER JOIN).
void runJoinDemo()
{
    using namespace sqlite_orm;

    auto storage = createStorage("restaurant_orders_queries.db");
    storage.sync_schema();

    std::cout << "\n=== JOIN QUERY DEMONSTRATION ===\n";

    std::cout << "\n1. Order history per table:\n";
    auto records = storage.select(
        columns(
            &RestaurantTable::tableNumber,
            &Order::itemsDescription,
            &Order::status,
            &Order::totalAmount
        ),
        inner_join<Order>(
            on(c(&RestaurantTable::id) == &Order::tableId)
        ),
        order_by(&Order::id)
    );
    for (const auto& [tableNumber, itemsDescription, status, totalAmount] : records)
    {
        std::cout << "table #" << tableNumber << " | " << itemsDescription
                   << " | " << status << " | " << totalAmount << '\n';
    }

    std::cout << "\n2. Orders above 200:\n";
    auto largeOrders = storage.select(
        columns(
            &RestaurantTable::tableNumber,
            &Order::itemsDescription,
            &Order::totalAmount
        ),
        inner_join<Order>(
            on(c(&RestaurantTable::id) == &Order::tableId)
        ),
        where(c(&Order::totalAmount) > 200.0),
        order_by(&Order::totalAmount).desc()
    );
    for (const auto& [tableNumber, itemsDescription, totalAmount] : largeOrders)
    {
        std::cout << "table #" << tableNumber << " | " << itemsDescription
                   << " | " << totalAmount << '\n';
    }
}

// 7. Успішна транзакція: реєстрація нового столу разом із першим замовленням.
void runTransactionDemo()
{
    auto storage = createStorage("restaurant_orders_queries.db");

    storage.on_open = [](sqlite3* db)
    {
        if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK)
        {
            throw std::runtime_error("Cannot enable foreign keys");
        }
    };
    storage.sync_schema();

    std::cout << "\n=== TRANSACTION DEMONSTRATION (COMMIT) ===\n";

    auto tablesBefore = storage.count<RestaurantTable>();
    auto ordersBefore = storage.count<Order>();
    std::cout << "\nBefore transaction:\n";
    std::cout << "RestaurantTable: " << tablesBefore << '\n';
    std::cout << "Order: " << ordersBefore << '\n';

    bool committed = storage.transaction([&]()
    {
        RestaurantTable table{
            0,
            20,
            4,
            "зала",
            "зайнятий",
            "2026-11-10"
        };
        int tableId = storage.insert(table);

        Order order{
            0,
            tableId,
            "Крем-суп грибний",
            "new",
            0.0,
            0, 0, 0,
            "2026-11-10"
        };
        storage.insert(order);

        std::cout << "\nRecords inserted inside transaction\n";
        std::cout << "RestaurantTable ID: " << tableId << '\n';

        return true; // COMMIT
    });

    std::cout << "\nTransaction committed: " << std::boolalpha << committed << '\n';
    std::cout << "\nAfter transaction:\n";
    std::cout << "RestaurantTable: " << storage.count<RestaurantTable>() << '\n';
    std::cout << "Order: " << storage.count<Order>() << '\n';
}

// 8. Відкат транзакції (ROLLBACK) через порушення зовнішнього ключа.
void runRollbackDemo()
{
    using namespace sqlite_orm;

    auto storage = createStorage("restaurant_orders_queries.db");

    storage.on_open = [](sqlite3* db)
    {
        if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK)
        {
            throw std::runtime_error("Cannot enable foreign keys");
        }
    };
    storage.sync_schema();

    std::cout << "\n=== ROLLBACK DEMONSTRATION ===\n";

    auto tablesBefore = storage.count<RestaurantTable>();
    auto ordersBefore = storage.count<Order>();
    std::cout << "\nBefore transaction:\n";
    std::cout << "RestaurantTable: " << tablesBefore << '\n';
    std::cout << "Order: " << ordersBefore << '\n';

    try
    {
        storage.transaction([&]()
        {
            RestaurantTable table{
                0,
                21,
                4,
                "зала",
                "зайнятий",
                "2026-11-11"
            };
            int tableId = storage.insert(table);
            std::cout << "\nRestaurantTable inserted, ID: " << tableId << '\n';

            // Навмисно некоректний зовнішній ключ.
            Order invalidOrder{0, -1, "Борщ", "new", 10.0, 1, 1, 0, "2026-11-11"};
            storage.insert(invalidOrder);

            return true;
        });
    }
    catch (const std::exception& ex)
    {
        std::cout << "\nTransaction failed: " << ex.what() << '\n';
        std::cout << "Changes rolled back\n";
    }

    std::cout << "\nAfter transaction:\n";
    std::cout << "RestaurantTable: " << storage.count<RestaurantTable>() << '\n';
    std::cout << "Order: " << storage.count<Order>() << '\n';

    auto rolledBackTables = storage.get_all<RestaurantTable>(
        where(c(&RestaurantTable::tableNumber) == 21)
    );
    std::cout << "\nTable #21 exists: "
               << std::boolalpha << !rolledBackTables.empty() << '\n';
}

// 9. Ті самі дві пов'язані операції без явної транзакції — демонстрація часткового збереження.
void runWithoutTransactionDemo()
{
    const std::string databaseFile = "restaurant_orders_no_transaction.db";
    std::filesystem::remove(databaseFile);

    auto storage = createStorage(databaseFile);

    storage.on_open = [](sqlite3* db)
    {
        if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK)
        {
            throw std::runtime_error("Cannot enable foreign keys");
        }
    };
    storage.sync_schema();

    std::cout << "\n=== WITHOUT TRANSACTION ===\n";

    try
    {
        RestaurantTable table{
            0,
            22,
            4,
            "зала",
            "зайнятий",
            "2026-11-11"
        };
        int tableId = storage.insert(table);
        std::cout << "\nRestaurantTable inserted, ID: " << tableId << '\n';

        Order invalidOrder{0, -1, "Борщ", "new", 10.0, 1, 1, 0, "2026-11-11"};
        storage.insert(invalidOrder);
    }
    catch (const std::exception& ex)
    {
        std::cout << "\nOperation failed: " << ex.what() << '\n';
    }

    std::cout << "\nAfter operations:\n";
    std::cout << "RestaurantTable: " << storage.count<RestaurantTable>() << '\n';
    std::cout << "Order: " << storage.count<Order>() << '\n';
}

// 10. Патерн State: життєвий цикл замовлення created -> accepted -> completed,
// окремий термінальний стан cancelled, перевірка недопустимих переходів,
// історія переходів зберігається у таблиці OrderStateHistory.
void runStatePatternDemo()
{
    using namespace sqlite_orm;

    auto storage = createStorage("restaurant_orders_patterns.db");
    storage.on_open = [](sqlite3* db)
    {
        sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    };
    storage.open_forever();
    storage.sync_schema();

    std::cout << "\n=== STATE PATTERN DEMONSTRATION ===\n";

    RestaurantTable table{0, 30, 4, "зала", "зайнятий", "2026-11-15"};
    int tableId = storage.insert(table);

    OrderSubject subject;

    Order order{0, tableId, "Борщ, вареники", "created", 0.0, 2, 3, 0, "2026-11-15"};
    auto workflow = OrderWorkflow::createNew(order, storage, subject);
    std::cout << "Order #" << workflow.order().id
               << " created, state: " << workflow.currentStateName() << '\n';

    workflow.accept();
    std::cout << "After accept(), state: " << workflow.currentStateName() << '\n';

    workflow.complete();
    std::cout << "After complete(), state: " << workflow.currentStateName() << '\n';

    try
    {
        workflow.accept();
        std::cout << "Unexpected: transition succeeded\n";
    }
    catch (const std::exception& ex)
    {
        std::cout << "Rejected invalid transition: " << ex.what() << '\n';
    }

    auto history = storage.get_all<OrderStateHistory>(
        where(c(&OrderStateHistory::orderId) == workflow.order().id),
        order_by(&OrderStateHistory::id)
    );

    std::cout << "\nState transition history for order #" << workflow.order().id << ":\n";
    for (const auto& record : history)
    {
        std::cout << "  " << record.fromState << " -> " << record.toState
                   << " (" << record.changedDate << ")\n";
    }
}

// 11. Патерн Observer: сповіщення підписників (монітор черги кухні,
// журнал замовлень) про створення та завершення замовлення.
// Демонструється коректне відписування через std::weak_ptr/RAII.
void runObserverPatternDemo()
{
    auto storage = createStorage("restaurant_orders_patterns.db");
    storage.sync_schema();

    std::cout << "\n=== OBSERVER PATTERN DEMONSTRATION ===\n";

    OrderSubject subject;

    auto kitchenMonitor = std::make_shared<KitchenQueueMonitor>();
    auto journal = std::make_shared<OrderJournal>(storage);

    subject.subscribe(kitchenMonitor);
    subject.subscribe(journal);
    std::cout << "Active subscribers: " << subject.activeSubscriberCount() << '\n';

    RestaurantTable table{0, 31, 2, "тераса", "зайнятий", "2026-11-16"};
    int tableId = storage.insert(table);

    Order order{0, tableId, "Лимонад, брускета", "created", 0.0, 2, 2, 0, "2026-11-16"};
    auto workflow = OrderWorkflow::createNew(order, storage, subject);

    workflow.accept();
    workflow.complete();

    // Знищуємо підписника (журнал) до того, як станеться наступна подія:
    // суб'єкт має коректно пропустити його під час розсилки.
    journal.reset();
    std::cout << "\nJournal subscriber released. Active subscribers: "
               << subject.activeSubscriberCount() << '\n';

    Order secondOrder{0, tableId, "Чай, тірамісу", "created", 0.0, 2, 1, 0, "2026-11-16"};
    auto secondWorkflow = OrderWorkflow::createNew(secondOrder, storage, subject);
    secondWorkflow.accept();
    secondWorkflow.complete();
}

// 12. Патерн Strategy: обчислення пріоритету замовлень трьома алгоритмами
// та можливість зміни алгоритму під час виконання програми.
void runStrategyPatternDemo()
{
    std::cout << "\n=== STRATEGY PATTERN DEMONSTRATION ===\n";

    std::vector<Order> pendingOrders = {
        {1, 1, "Стейк, гарнір",      "accepted", 0.0, 3, 4, 40, "2026-11-16"},
        {2, 2, "Капучино",           "accepted", 0.0, 1, 1,  5, "2026-11-16"},
        {3, 3, "Суші-сет, рол",      "accepted", 0.0, 6, 3, 25, "2026-11-16"},
        {4, 4, "Салат, суп",         "accepted", 0.0, 2, 5, 55, "2026-11-16"},
    };

    PriorityCalculator calculator(std::make_unique<MaxNormalizedParamStrategy>());

    std::vector<std::unique_ptr<IPriorityStrategy>> strategies;
    strategies.push_back(std::make_unique<MaxNormalizedParamStrategy>());
    strategies.push_back(std::make_unique<ProtectedSumStrategy>());
    strategies.push_back(std::make_unique<NormalizedRemainderStrategy>());

    for (auto& strategy : strategies)
    {
        calculator.setStrategy(std::move(strategy));
        std::cout << "\nStrategy: " << calculator.strategyName() << '\n';

        std::vector<std::pair<int, double>> ranked;
        for (const auto& order : pendingOrders)
        {
            ranked.emplace_back(order.id, calculator.calculate(order));
        }

        std::sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b)
        {
            return a.second > b.second;
        });

        for (const auto& [orderId, priority] : ranked)
        {
            std::cout << "  Order #" << orderId << " | priority: " << priority << '\n';
        }
    }
}

} // namespace

int main(int argc, char* argv[])
{
    std::string mode = (argc > 1) ? argv[1] : "all";

    try
    {
        if (mode == "state")
        {
            runStatePatternDemo();
            return 0;
        }
        if (mode == "observer")
        {
            runObserverPatternDemo();
            return 0;
        }
        if (mode == "strategy")
        {
            runStrategyPatternDemo();
            return 0;
        }

        runRawSqliteDemo();

        runOrmDemo();
        runRelationshipDemo();
        runRepositoryDemo();

        prepareTestData();
        runQueryDemo();
        runAggregateDemo();
        runJoinDemo();

        runTransactionDemo();
        runRollbackDemo();
        runWithoutTransactionDemo();

        runStatePatternDemo();
        runObserverPatternDemo();
        runStrategyPatternDemo();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Application error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
