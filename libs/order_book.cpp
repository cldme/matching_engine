#include "order_book.h"
#include <sstream>
#include <iostream>

OrderBook::OrderBook()
{
    mOrders.reserve(1000);
    mBidLevels.reserve(1000);
    mAskLevels.reserve(1000);
    mOnTradeCallback = nullptr;
}

std::optional<Id> OrderBook::AddOrder(const Side side, const Price price, const Volume volume)
{
    Id newId = mId++;
    auto& sideLevels = side == Side::Bid ? mBidLevels : mAskLevels;
    auto compare = side == Side::Bid ? BidComparator : AskComparator;
    
    auto [orderIt, inserted] = mOrders.emplace(newId, Order{ newId, side, price, volume });
    if (!inserted)
    {
        std::cout << "[WARN] Failed to emplace in mOrders while calling AddOrder with id=" << newId << std::endl;
        return std::nullopt;
    }
    orderIt->second.mLevelIndex = AddOrder(sideLevels, newId, side, price, volume, compare);
    std::cout << "[UPDATE] Added order=" << orderIt->second << std::endl;

    MatchOrders();
    return newId;
}

bool OrderBook::ModifyOrder(const Id orderId, const Price newPrice, const Volume newVolume)
{
    auto it = mOrders.find(orderId);
    if (it == mOrders.end() || !it->second.mIsActive)
    {
        std::cout << "[WARN] Could not find existing order while calling ModifyOrder with id=" << orderId << std::endl;
        return false;
    }

    Side side = it->second.mSide;
    DeleteOrder(orderId);
    AddOrder(side, newPrice, newVolume);
    return true;
}

bool OrderBook::DeleteOrder(const Id orderId)
{
    auto it = mOrders.find(orderId);
    if (it == mOrders.end() || !it->second.mIsActive)
    {
        std::cout << "[WARN] Could not find existing order while calling DeleteOrder with id=" << orderId << std::endl;
        return false;
    }

    auto& order = it->second;
    auto& sideLevels = order.mSide == Side::Bid ? mBidLevels : mAskLevels;
    auto compare = order.mSide == Side::Bid ? BidComparator : AskComparator;
    bool deleted = DeleteOrder(sideLevels, order, compare);
    order.mIsActive = false;
    mOrders.erase(it);

    std::cout << "[" << (deleted ? "UPDATE]" : "ERROR]") << " Deleted order=" << order << std::endl;
    return true;
}

const Order* OrderBook::FindOrder(const Id orderId)
{
    auto it = mOrders.find(orderId);
    if (it == mOrders.end())
    {
        std::cout << "[WARN] Could not find existing order while calling GetOrder with id=" << orderId << std::endl;
        return nullptr;
    }
    return &it->second;
}

void OrderBook::SetOnTradeCallback(OnTradeCallback callback)
{
    mOnTradeCallback = std::move(callback);
}

std::optional<Price> OrderBook::GetBestBid() const
{
    if (mBidLevels.empty())
    {
        return std::nullopt;
    }
    return mBidLevels.back().first;
}

std::optional<Price> OrderBook::GetBestAsk() const
{
    if (mAskLevels.empty())
    {
        return std::nullopt;
    }
    return mAskLevels.back().first;
}

void OrderBook::MatchOrders()
{
    while (true)
    {
        auto bestBid = GetBestBid();
        auto bestAsk = GetBestAsk();

        if (!bestBid || !bestAsk || *bestBid < *bestAsk)
        {
            break;
        }

        auto& bidLevel = mBidLevels.back().second;
        auto& askLevel = mAskLevels.back().second;

        // cleanup inactive bids
        while (!bidLevel.Empty() && !bidLevel.Front().mIsActive)
        {
            bidLevel.PopFront();
        }

        // cleanup inactive asks
        while (!askLevel.Empty() && !askLevel.Front().mIsActive)
        {
            askLevel.PopFront();
        }

        if (bidLevel.Empty() || askLevel.Empty())
        {
            bidLevel.Empty() ? mBidLevels.pop_back() : mAskLevels.pop_back();
            continue;
        }

        auto& bidOrder = bidLevel.Front();
        auto& askOrder = askLevel.Front();

        uint64_t matchVolume = std::min(bidOrder.mVolume, askOrder.mVolume);
        std::cout << "[TRADE] volume=" << matchVolume << " bid order=" << bidOrder << " ask order=" << askOrder << std::endl;
        if (mOnTradeCallback)
        {
            mOnTradeCallback(bidOrder, askOrder, matchVolume);
        }

        bidOrder.mVolume -= matchVolume;
        askOrder.mVolume -= matchVolume;

        if (bidOrder.mVolume == 0)
        {
            mOrders.erase(bidOrder.mId);
            bidLevel.PopFront();
        }

        if (askOrder.mVolume == 0)
        {
            mOrders.erase(askOrder.mId);
            askLevel.PopFront();
        }

        if (bidLevel.Empty())
        {
            mBidLevels.pop_back();
        }

        if (askLevel.Empty())
        {
            mAskLevels.pop_back();
        }
    }
    return;
}
