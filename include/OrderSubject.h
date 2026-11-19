#pragma once

#include <memory>
#include <vector>

#include "IOrderObserver.h"

// Патерн Observer: суб'єкт, що розсилає сповіщення про події замовлення.
// Підписники зберігаються як std::weak_ptr — якщо підписник знищений,
// суб'єкт самостійно прибирає "висячі" посилання під час наступної розсилки,
// що унеможливлює звернення до вже неіснуючого об'єкта.
class OrderSubject
{
public:
    void subscribe(const std::weak_ptr<IOrderObserver>& observer);

    void notifyCreated(const Order& order);
    void notifyCompleted(const Order& order);

    std::size_t activeSubscriberCount() const;

private:
    void purgeExpired();

    std::vector<std::weak_ptr<IOrderObserver>> observers_;
};
