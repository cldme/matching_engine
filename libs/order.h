#pragma once
#include <cstdint>
#include <vector>
#include <iostream>
#include <functional>

enum class Side { Bid, Ask };

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
    LevelIndex mLevelIndex;
    Active mIsActive;

    Order(Id id = 0, Side side = Side::Bid, Price price = 0.0, Volume volume = 0, LevelIndex levelIndex = 0, Active isActive = true) : 
        mId(id),
        mSide(side),
        mPrice(price),
        mVolume(volume),
        mLevelIndex(levelIndex),
        mIsActive(isActive)
    {}

    bool operator==(const Order& other) const;
    bool operator!=(const Order& other) const;
};

using OnTradeCallback = std::function<void(const Order&, const Order&, Volume)>;

std::ostream& operator<<(std::ostream& os, const Side& side);
std::ostream& operator<<(std::ostream& os, const Order& order);

constexpr auto BidComparator = [](Price a, Price b) { return a < b; };
constexpr auto AskComparator = [](Price a, Price b) { return a > b; };
