#include "PriorityStrategy.h"

#include <algorithm>

namespace
{
constexpr double MAX_ITEMS = 10.0;
constexpr double MAX_URGENCY = 5.0;
constexpr double MAX_WAIT_MINUTES = 60.0;
constexpr double SLA_MINUTES = 30.0;

double clamp01(double value)
{
    return std::clamp(value, 0.0, 1.0);
}

double itemsNorm(const Order& order)
{
    return clamp01(order.itemsCount / MAX_ITEMS);
}

double urgencyNorm(const Order& order)
{
    return clamp01(order.urgencyLevel / MAX_URGENCY);
}

double waitNorm(const Order& order)
{
    return clamp01(order.waitTimeMinutes / MAX_WAIT_MINUTES);
}
} // namespace

double MaxNormalizedParamStrategy::calculate(const Order& order) const
{
    return std::max({itemsNorm(order), urgencyNorm(order), waitNorm(order)});
}

double ProtectedSumStrategy::calculate(const Order& order) const
{
    double sum = 0.3 * itemsNorm(order) + 0.3 * urgencyNorm(order) + 0.4 * waitNorm(order);
    return clamp01(sum);
}

double NormalizedRemainderStrategy::calculate(const Order& order) const
{
    double remaining = std::max(0.0, SLA_MINUTES - order.waitTimeMinutes);
    return clamp01(1.0 - remaining / SLA_MINUTES);
}

PriorityCalculator::PriorityCalculator(std::unique_ptr<IPriorityStrategy> strategy)
    : strategy_(std::move(strategy))
{
}

void PriorityCalculator::setStrategy(std::unique_ptr<IPriorityStrategy> strategy)
{
    strategy_ = std::move(strategy);
}

double PriorityCalculator::calculate(const Order& order) const
{
    return strategy_->calculate(order);
}

std::string PriorityCalculator::strategyName() const
{
    return strategy_->name();
}
