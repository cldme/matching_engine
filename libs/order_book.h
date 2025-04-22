#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <absl/container/flat_hash_map.h>

enum class Side { Buy, Sell };

using Id = uint64_t;
using Price = double;
using Volume = uint64_t;
using LevelIndex = size_t;
using Active = bool;

struct Order
{
    Id mId;
    Side mSide;
    Price mPrice;
    Volume mVolume;
    Active mIsActive;
    LevelIndex mLevelIndex;

    Order(Id id = 0, Side side = Side::Buy, Price price = 0.0, Volume volume = 0, Active isActive = true, LevelIndex levelIndex = 0) : 
        mId(id),
        mSide(side),
        mPrice(price),
        mVolume(volume),
        mIsActive(isActive),
        mLevelIndex(levelIndex)
    {}
};

struct Level
{
    std::vector<Order> mOrders;
    size_t mStart = 0;

    template <typename... Args>
    void emplace_back(Args&&... args)
    {
        mOrders.emplace_back(std::forward<Args>(args)...);
    }

    void pop_front()
    {
        mStart++;
        if (mStart > 1024 && mStart > mOrders.size() / 2)
        {
            mOrders.erase(mOrders.begin(), mOrders.begin() + mStart);
            mStart = 0;
        }
    }

    Order& front()
    {
        return mOrders[mStart];
    }

    bool empty() const
    {
        return mStart >= mOrders.size();
    }

    size_t size() const
    {
        return mOrders.size() - mStart;
    }
};

class OrderBook
{
public:
    OrderBook();
    bool AddOrder(const Id, const Side, const Price, const Volume);
    bool ModifyOrder(const Id orderId, const Volume);
    bool DeleteOrder(const Id orderId);
    const Order* FindOrder(const Id);

    std::optional<double> GetBestBid() const;
    std::optional<double> GetBestAsk() const;

private:
    void MatchOrders();

    absl::flat_hash_map<Id, Order> mOrders;
    absl::flat_hash_map<Price, Level> mBidLevels;
    absl::flat_hash_map<Price, Level> mAskLevels;
};

std::ostream& operator<<(std::ostream& os, const Side& side);
std::ostream& operator<<(std::ostream& os, const Order& order);
