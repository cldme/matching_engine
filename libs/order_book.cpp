#include "order_book.h"
#include <sstream>
#include <iostream>

OrderBook::OrderBook()
{
    mOrders.reserve(1000);
}

bool OrderBook::AddOrder(const Id id, const Side side, const Price price, const Volume volume)
{
    auto it = mOrders.find(id);
    if (it != mOrders.end())
    {
        std::cout << "Found existing order while calling AddOrder with id=" << id << std::endl;
        return false;
    }

    auto& sideLevels = side == Side::Buy ? mBidLevels : mAskLevels;
    auto& priceLevel = sideLevels[price];
    
    auto [orderIt, inserted] = mOrders.emplace(id, Order{ id, side, price, volume, true, priceLevel.mOrders.size() });
    if (!inserted)
    {
        std::cout << "Failed to emplace in mOrders while calling AddOrder with id=" << id << std::endl;
        return false;
    }
    priceLevel.mOrders.emplace_back(orderIt->second);
    std::cout << "Added order=" << orderIt->second << std::endl;

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
    order.mVolume = newVolume;
    auto& sideLevels = order.mSide == Side::Buy ? mBidLevels : mAskLevels;
    auto& priceLevel = sideLevels[order.mPrice];
    priceLevel.mOrders[order.mLevelIndex].mVolume = newVolume;

    std::cout << "Modified order=" << it->second << std::endl;
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
    order.mIsActive = false;
    auto& sideLevels = order.mSide == Side::Buy ? mBidLevels : mAskLevels;
    auto& priceLevel = sideLevels[order.mPrice];
    priceLevel.mOrders[order.mLevelIndex].mIsActive = false;

    std::cout << "Deleted order=" << order << std::endl;
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

std::optional<double> OrderBook::GetBestBid() const
{
    if (mBidLevels.empty())
    {
        std::cout << "Could not find any bid levels in GetBestBid()" << std::endl;
        return std::nullopt;
    }
    return std::max_element(mBidLevels.begin(), mBidLevels.end(), [](const auto& a, const auto& b){ return a.first < b.first; })->first;
}

std::optional<double> OrderBook::GetBestAsk() const
{
    if (mAskLevels.empty())
    {
        std::cout << "Could not find any ask levels in GetBestAsk()" << std::endl;
        return std::nullopt;
    }
    return std::min_element(mAskLevels.begin(), mAskLevels.end(), [](const auto& a, const auto& b){ return a.first < b.first; })->first;
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

        auto& bidLevel = mBidLevels[*bestBid];
        auto& askLevel = mAskLevels[*bestAsk];

        while (!bidLevel.empty() && !bidLevel.front().mIsActive)
        {
            std::cout << "Cleanup inactive bid order=" << bidLevel.front() << std::endl;
            bidLevel.pop_front();
        }

        while (!askLevel.empty() && !askLevel.front().mIsActive)
        {
            std::cout << "Cleanup inactive ask order=" << bidLevel.front() << std::endl;
            askLevel.pop_front();
        }

        if (bidLevel.empty() || askLevel.empty())
        {
            std::cout << "Side level empty, no matching can be done" << std::endl;
            break;
        }

        auto& bidOrder = bidLevel.front();
        auto& askOrder = askLevel.front();

        uint64_t matchVolume = std::min(bidOrder.mVolume, askOrder.mVolume);
        std::cout << "TRADE volume=" << matchVolume << " bid order=" << bidOrder << " ask order=" << askOrder << std::endl;

        bidOrder.mVolume -= matchVolume;
        askOrder.mVolume -= matchVolume;

        if (bidOrder.mVolume == 0)
        {
            std::cout << "Bid order fully traded!" << std::endl;
            mOrders.erase(bidOrder.mId);
            bidLevel.pop_front();
        }

        if (askOrder.mVolume == 0)
        {
            std::cout << "Ask order fully traded!" << std::endl;
            mOrders.erase(askOrder.mId);
            askLevel.pop_front();
        }

        if (bidLevel.empty())
        {
            std::cout << "Bid price level fully traded!" << std::endl;
            mBidLevels.erase(*bestBid);
        }

        if (askLevel.empty())
        {
            std::cout << "Ask price level fully traded!" << std::endl;
            mAskLevels.erase(*bestAsk);
        }
    }
    return;
}

std::ostream& operator<<(std::ostream& os, const Side& side)
{
    switch (side)
    {
        case Side::Buy:
            os << "BUY";
            break;
        case Side::Sell:
            os << "SELL";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Order& order)
{
    return os << "(id=" << order.mId << " side=" << order.mSide << " price=" << order.mPrice << " volume=" << order.mVolume << ")";
}
