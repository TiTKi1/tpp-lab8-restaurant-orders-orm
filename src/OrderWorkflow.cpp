#include "OrderWorkflow.h"

#include <ctime>

namespace
{

std::string currentTimestamp()
{
    std::time_t now = std::time(nullptr);
    std::tm localTime{};
    localtime_s(&localTime, &now);

    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", &localTime);
    return buffer;
}

} // namespace

OrderWorkflow::OrderWorkflow(Order order, Storage& storage, OrderSubject& subject)
    : order_(std::move(order))
    , state_(OrderStateFactory::create(order_.status))
    , storage_(storage)
    , subject_(subject)
{
}

OrderWorkflow OrderWorkflow::createNew(Order order, Storage& storage, OrderSubject& subject)
{
    order.status = "created";
    order.id = storage.insert(order);

    OrderStateHistory initialRecord{0, order.id, "-", "created", currentTimestamp()};
    storage.insert(initialRecord);

    OrderWorkflow workflow(order, storage, subject);
    workflow.subject_.notifyCreated(workflow.order_);
    return workflow;
}

void OrderWorkflow::accept()
{
    applyTransition(state_->accept());
}

void OrderWorkflow::complete()
{
    applyTransition(state_->complete());
}

void OrderWorkflow::cancel()
{
    applyTransition(state_->cancel());
}

void OrderWorkflow::applyTransition(std::unique_ptr<OrderState> newState)
{
    std::string fromState = state_->name();
    std::string toState = newState->name();

    order_.status = toState;
    storage_.update(order_);

    OrderStateHistory record{0, order_.id, fromState, toState, currentTimestamp()};
    storage_.insert(record);

    state_ = std::move(newState);

    if (toState == "completed")
    {
        subject_.notifyCompleted(order_);
    }
}
