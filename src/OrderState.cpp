#include "OrderState.h"

#include <stdexcept>

std::unique_ptr<OrderState> OrderState::accept()
{
    rejectTransition("accept");
}

std::unique_ptr<OrderState> OrderState::complete()
{
    rejectTransition("complete");
}

std::unique_ptr<OrderState> OrderState::cancel()
{
    rejectTransition("cancel");
}

void OrderState::rejectTransition(const std::string& action) const
{
    throw std::logic_error(
        "Invalid transition '" + action + "' from state '" + name() + "'"
    );
}

std::unique_ptr<OrderState> CreatedState::accept()
{
    return std::make_unique<AcceptedState>();
}

std::unique_ptr<OrderState> CreatedState::cancel()
{
    return std::make_unique<CancelledState>();
}

std::unique_ptr<OrderState> AcceptedState::complete()
{
    return std::make_unique<CompletedState>();
}

std::unique_ptr<OrderState> AcceptedState::cancel()
{
    return std::make_unique<CancelledState>();
}

std::unique_ptr<OrderState> makeOrderState(const std::string& name)
{
    if (name == "created")
    {
        return std::make_unique<CreatedState>();
    }
    if (name == "accepted")
    {
        return std::make_unique<AcceptedState>();
    }
    if (name == "completed")
    {
        return std::make_unique<CompletedState>();
    }
    if (name == "cancelled")
    {
        return std::make_unique<CancelledState>();
    }

    throw std::invalid_argument("Unknown order state: " + name);
}

std::unique_ptr<OrderState> OrderStateFactory::create(const std::string& name)
{
    return makeOrderState(name);
}
