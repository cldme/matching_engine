#include "level.h"
#include <iostream>

Level::Level()
{
    mOrders.reserve(1000);
}

void Level::PopFront()
{
    mStart++;
}

bool Level::ModifyOrder(const Order& order, const Volume newVolume)
{
    auto& levelOrder = mOrders[order.mLevelIndex];
    if (levelOrder == order)
    {
        levelOrder.mVolume = newVolume;
    }
    else
    {
        std::cout << "Could not find match in ModifyOrder between order=" << order << " and levelOrder=" << levelOrder << std::endl;
        return false;
    }
    return true;
}

bool Level::DeleteOrder(const Order& order)
{
    auto& levelOrder = mOrders[order.mLevelIndex];
    if (levelOrder == order)
    {
        levelOrder.mIsActive = false;
    }
    else
    {
        std::cout << "Could not find match in DeleteOrder between order=" << order << " and levelOrder=" << levelOrder << std::endl;
        return false;
    }
    return true;
}

Order& Level::Front()
{
    return mOrders[mStart];
}

bool Level::Empty() const
{
    return mStart >= mOrders.size();
}

size_t Level::Size() const
{
    return mOrders.size() - mStart;
}

size_t Level::GetStart() const
{
    return mStart;
}

size_t Level::GetEnd() const
{
    return mOrders.size();
}