# Restaurant Orders ORM — Лабораторна робота 8 (варіант 10)

Консольний застосунок C++20 для предметної області «Облік замовлень ресторану».

- Основна сутність: **RestaurantTable** (стіл ресторану).
- Залежна сутність: **Order** (замовлення), зв'язок 1:N через `tableId`.

Один стіл може мати декілька замовлень, створених у різний час — модель не
обмежується лише поточним замовленням столу.

## Збірка

Потрібні: компілятор C++20 (MSVC), CMake, vcpkg (manifest mode).

```bash
cmake --preset default
cmake --build build
```

Переконайтесь, що встановлено змінну середовища `VCPKG_ROOT`, яка вказує на
корінь встановленого vcpkg (використовується в `CMakePresets.json`).

## Запуск

```bash
./build/RestaurantOrdersORM.exe
```

Програма послідовно демонструє:

1. Операції через "сирий" SQLite C API (`RawSqliteDemo`).
2. Базові CRUD-операції через `sqlite_orm`.
3. Зв'язок 1:N між `RestaurantTable` і `Order`, перевірку зовнішнього ключа.
4. Патерн Repository (`IRestaurantTableRepository` / `SQLiteRestaurantTableRepository`).
5. Фільтрацію, сортування та обмеження кількості результатів.
6. Агрегатні запити (COUNT, SUM, AVG, GROUP BY).
7. Об'єднання таблиць (INNER JOIN).
8. Успішну транзакцію (COMMIT), відкат (ROLLBACK) та порівняння з виконанням
   без явної транзакції.

Повторні запуски не накопичують тестові дані: демонстраційні БД для запитів
(`restaurant_orders_queries.db`, `restaurant_orders_no_transaction.db`)
перестворюються кожного разу.
