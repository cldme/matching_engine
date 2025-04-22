#include "order_book.h"
#include <sstream>
#include <iostream>

OrderBook::OrderBook()
{
    mOrders.reserve(1000);
    mOnTradeCallback = nullptr;
}

bool OrderBook::AddOrder(const Id id, const Side side, const Price price, const Volume volume)
{
    auto it = mOrders.find(id);
    if (it != mOrders.end())
    {
        std::cout << "Found existing order while calling AddOrder with id=" << id << std::endl;
        return false;
    }

    auto& sideLevels = side == Side::Bid ? mBidLevels : mAskLevels;
    auto compare = side == Side::Bid ? BidComparator : AskComparator;
    
    auto [orderIt, inserted] = mOrders.emplace(id, Order{ id, side, price, volume });
    if (!inserted)
    {
        std::cout << "Failed to emplace in mOrders while calling AddOrder with id=" << id << std::endl;
        return false;
    }
    orderIt->second.mLevelIndex = AddOrder(sideLevels, id, side, price, volume, compare);
    std::cout << "[SUCCESS] Added order=" << orderIt->second << std::endl;

    MatchOrders();
    return true;
}

bool OrderBook::ModifyOrder(const Id orderId, const Volume newVolume)
{
    auto it = mOrders.find(orderId);
    if (it == mOrders.end())
    {
        std::cout << "Could not find existing order while calling ModifyOrder with id=" << orderId << std::endl;
        return false;
    }

    auto& order = it->second;
    auto& sideLevels = order.mSide == Side::Bid ? mBidLevels : mAskLevels;
    auto compare = order.mSide == Side::Bid ? BidComparator : AskComparator;
    bool modified = ModifyOrder(sideLevels, order, newVolume, compare);
    order.mVolume = newVolume;

    std::cout << "[" << (modified ? "SUCCESS]" : "FAILURE]") << " Modified order=" << order << std::endl;
    return true;
}

bool OrderBook::DeleteOrder(const Id orderId)
{
    auto it = mOrders.find(orderId);
    if (it == mOrders.end())
    {
        std::cout << "Could not find existing order while calling DeleteOrder with id=" << orderId << std::endl;
        return false;
    }

    auto& order = it->second;
    auto& sideLevels = order.mSide == Side::Bid ? mBidLevels : mAskLevels;
    auto compare = order.mSide == Side::Bid ? BidComparator : AskComparator;
    bool deleted = DeleteOrder(sideLevels, order, compare);
    order.mIsActive = false;

    std::cout << "[" << (deleted ? "SUCCESS]" : "FAILURE]") << " Deleted order=" << order << std::endl;
    return true;
}

const Order* OrderBook::FindOrder(const Id orderId)
{
    auto it = mOrders.find(orderId);
    if (it == mOrders.end())
    {
        std::cout << "Could not find existing order while calling GetOrder with id=" << orderId << std::endl;
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
        std::cout << "Could not find any bid levels in GetBestBid" << std::endl;
        return std::nullopt;
    }
    return mBidLevels.back().first;
}

std::optional<Price> OrderBook::GetBestAsk() const
{
    if (mAskLevels.empty())
    {
        std::cout << "Could not find any ask levels in GetBestAsk" << std::endl;
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
            std::cout << "Could not find any new order matches" << std::endl;
            break;
        }

        auto& bidLevel = mBidLevels.back().second;
        auto& askLevel = mAskLevels.back().second;

        while (!bidLevel.Empty() && !bidLevel.Front().mIsActive)
        {
            std::cout << "Cleanup inactive bid order=" << bidLevel.Front() << std::endl;
            bidLevel.PopFront();
        }

        while (!askLevel.Empty() && !askLevel.Front().mIsActive)
        {
            std::cout << "Cleanup inactive ask order=" << bidLevel.Front() << std::endl;
            askLevel.PopFront();
        }

        if (bidLevel.Empty() || askLevel.Empty())
        {
            std::cout << (bidLevel.Empty() ? "Bid" : "Ask") << " side level empty, no matching can be done" << std::endl;
            break;
        }

        auto& bidOrder = bidLevel.Front();
        auto& askOrder = askLevel.Front();

        uint64_t matchVolume = std::min(bidOrder.mVolume, askOrder.mVolume);
        std::cout << "TRADE volume=" << matchVolume << " bid order=" << bidOrder << " ask order=" << askOrder << std::endl;
        if (mOnTradeCallback)
        {
            mOnTradeCallback(bidOrder, askOrder, matchVolume);
        }

        bidOrder.mVolume -= matchVolume;
        askOrder.mVolume -= matchVolume;

        if (bidOrder.mVolume == 0)
        {
            std::cout << "Bid order fully traded!" << std::endl;
            mOrders.erase(bidOrder.mId);
            bidLevel.PopFront();
        }

        if (askOrder.mVolume == 0)
        {
            std::cout << "Ask order fully traded!" << std::endl;
            mOrders.erase(askOrder.mId);
            askLevel.PopFront();
        }

        if (bidLevel.Empty())
        {
            std::cout << "Bid price level " << *bestBid << " fully traded!" << std::endl;
            mBidLevels.pop_back();
        }

        if (askLevel.Empty())
        {
            std::cout << "Ask price level " << *bestAsk << " fully traded!" << std::endl;
            mAskLevels.pop_back();
        }
    }
    return;
}
