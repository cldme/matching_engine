#pragma once
#include <string>
#include <vector>
#include <optional>
#include <absl/container/flat_hash_map.h>
#include "order.h"
#include "level.h"

/**
 * Operations supported by the order book
 * -> ADD
 *  --> AddOrder(id, side, price, volume)
 * -> MODIFY
 *  --> ModifyOrder(id, volume)
 * -> DELETE
 *  --> DeleteOrder(id)
 * 
 * Data structures used for the order book
 * -> one vector to keep track of price levels for bid side
 * -> one vector to keep track of price levels for ask side
 * -> one flat_hash_map to keep track of order ids to orders
 *    needed for order modify/delete
 * 
 * Notes
 * -> we use vectors to optimize for cache locality
 * -> bid side vector is sorted in ascending order
 * -> ask side vector is sorted in descending order
 * -> most order book operations will happen around the TOP of the book
 * -> we keep the TOP of the book for both sides at the end of the vectors
 *    this way we avoid extra shifting when removing TOP levels
 * 
 * Complexity
 * -> ADD
 *  --> log(N) if price level already exists
 *  --> log(N) + N if new price level is added
 * -> MODIFY
 *  --> log(N) to find the price level (binary search)
 * -> DELETE
 *  --> log(N) to find the price level (binary search)
 */

class OrderBook
{
public:
    OrderBook();
    bool AddOrder(const Id, const Side, const Price, const Volume);
    bool ModifyOrder(const Id orderId, const Volume);
    bool DeleteOrder(const Id orderId);
    const Order* FindOrder(const Id);
    void SetOnTradeCallback(OnTradeCallback);

    std::optional<double> GetBestBid() const;
    std::optional<double> GetBestAsk() const;

private:
    void MatchOrders();

    template<class T, class Compare>
    size_t AddOrder(T& levels, const Id id, const Side side, const Price price, const Volume volume, Compare compare)
    {
        auto it = std::lower_bound(levels.begin(), levels.end(), price, [compare](const auto& level, Price price)
        {
            return compare(level.first, price);
        });
        size_t levelIndex = 0;
        if (it != levels.end() && it->first == price)
        {
            auto& level = it->second;
            levelIndex = level.GetEnd();
            level.EmplaceBack(Order{ id, side, price, volume, levelIndex });
        }
        else
        {
            Level newLevel;
            newLevel.EmplaceBack(Order{ id, side, price, volume });
            levels.insert(it, { price, std::move(newLevel) });
        }
        return levelIndex;
    }

    template<class T, class Compare>
    bool ModifyOrder(T& levels, const Order& order, const Volume newVolume, Compare compare)
    {
        auto it = std::lower_bound(levels.begin(), levels.end(), order.mPrice, [compare](const auto& level, Price price)
        {
            return compare(level.first, price);
        });
        if (it != levels.end() && it->first == order.mPrice)
        {
            auto& level = it->second;
            level.ModifyOrder(order, newVolume);
        }
        else
        {
            std::cout << "Could not find existing order on price level while calling ModifyOrder for order=" << order << std::endl;
            return false;
        }
        return true;
    }

    template<class T, class Compare>
    bool DeleteOrder(T& levels, const Order& order, Compare compare)
    {
        auto it = std::lower_bound(levels.begin(), levels.end(), order.mPrice, [compare](const auto& level, Price price)
        {
            return compare(level.first, price);
        });
        if (it != levels.end() && it->first == order.mPrice)
        {
            auto& level = it->second;
            level.DeleteOrder(order);
        }
        else
        {
            std::cout << "Could not find existing order on price level while calling DeleteOrder for order=" << order << std::endl;
            return false;
        }
        return true;
    }

    OnTradeCallback mOnTradeCallback;
    absl::flat_hash_map<Id, Order> mOrders;
    std::vector<std::pair<Price, Level>> mBidLevels;
    std::vector<std::pair<Price, Level>> mAskLevels;
};
