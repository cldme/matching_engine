#pragma once
#include "order.h"

class Level
{
public:
    Level();

    template <typename... Args>
    void EmplaceBack(Args&&... args)
    {
        mOrders.emplace_back(std::forward<Args>(args)...);
    }

    bool ModifyOrder(const Order&, const Volume);
    bool DeleteOrder(const Order&);
    void PopFront();
    Order& Front();
    bool Empty() const;
    size_t Size() const;
    size_t GetStart() const;
    size_t GetEnd() const;

private:
    std::vector<Order> mOrders;
    size_t mStart = 0;
};