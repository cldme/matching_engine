#include "order_book.h"
#include <queue>
#include <gtest/gtest.h>

class OrderBookTest : public ::testing::Test 
{
protected:
    void SetUp() override
    {
        mOrderBook.SetOnTradeCallback([&](const Order& bidOrder, const Order& askOrder, Volume volume){ OnTrade(bidOrder, askOrder, volume); });
    }

    void TearDown() override
    {}

    void OnTrade(const Order& bidOrder, const Order& askOrder, Volume volume)
    {
        EXPECT_EQ(bidOrder, mBidMatches.front());
        EXPECT_EQ(askOrder, mAskMatches.front());
        EXPECT_EQ(volume, mVolumeMatches.front());
        mBidMatches.pop();
        mAskMatches.pop();
        mVolumeMatches.pop();
    }

    OrderBook mOrderBook;
    std::queue<Order> mBidMatches;
    std::queue<Order> mAskMatches;
    std::queue<Volume> mVolumeMatches;
};

TEST_F(OrderBookTest, AddOrder)
{
    EXPECT_TRUE(mOrderBook.AddOrder(Side::Bid, 10.7 /*=price*/, 5 /*=volume*/));
    auto* order = mOrderBook.FindOrder(0 /*=id*/);
    EXPECT_TRUE(order != nullptr);
    EXPECT_EQ(order->mId, 0);
    EXPECT_EQ(order->mSide, Side::Bid);
    EXPECT_EQ(order->mPrice, 10.7);
    EXPECT_EQ(order->mVolume, 5);
    EXPECT_EQ(order->mIsActive, true);
}

TEST_F(OrderBookTest, ModifyOrder)
{
    EXPECT_TRUE(mOrderBook.AddOrder(Side::Ask, 11.4 /*=price*/, 7 /*=volume*/));
    EXPECT_TRUE(mOrderBook.ModifyOrder(0 /*=id*/, 11.4 /*=price*/, 14 /*=volume*/));
    auto* order = mOrderBook.FindOrder(1 /*=id*/);
    EXPECT_TRUE(order != nullptr);
    EXPECT_EQ(order->mId, 1);
    EXPECT_EQ(order->mSide, Side::Ask);
    EXPECT_EQ(order->mPrice, 11.4);
    EXPECT_EQ(order->mVolume, 14);
    EXPECT_EQ(order->mIsActive, true);
    auto* delOrder = mOrderBook.FindOrder(0 /*=id*/);
    EXPECT_TRUE(delOrder == nullptr);
}

TEST_F(OrderBookTest, DeleteOrder)
{
    EXPECT_TRUE(mOrderBook.AddOrder(Side::Bid, 15 /*=price*/, 50 /*=volume*/));
    auto* order = mOrderBook.FindOrder(0 /*=id*/);
    EXPECT_TRUE(order != nullptr);
    EXPECT_EQ(order->mId, 0);
    EXPECT_EQ(order->mSide, Side::Bid);
    EXPECT_EQ(order->mPrice, 15);
    EXPECT_EQ(order->mVolume, 50);
    EXPECT_EQ(order->mIsActive, true);
    EXPECT_TRUE(mOrderBook.DeleteOrder(0 /*=id*/));
    EXPECT_EQ(order->mIsActive, false);
}

TEST_F(OrderBookTest, ModifyAfterDelete)
{
    EXPECT_TRUE(mOrderBook.AddOrder(Side::Bid, 15 /*=price*/, 50 /*=volume*/));
    auto* order = mOrderBook.FindOrder(0 /*=id*/);
    EXPECT_TRUE(order != nullptr);
    EXPECT_EQ(order->mId, 0);
    EXPECT_EQ(order->mSide, Side::Bid);
    EXPECT_EQ(order->mPrice, 15);
    EXPECT_EQ(order->mVolume, 50);
    EXPECT_EQ(order->mIsActive, true);
    EXPECT_TRUE(mOrderBook.DeleteOrder(0 /*=id*/));
    EXPECT_EQ(order->mIsActive, false);
    EXPECT_FALSE(mOrderBook.ModifyOrder(0 /*=id*/, 10 /*=price*/, 100 /*=volume*/));
}

TEST_F(OrderBookTest, MatchAfterModifyWithEmptyLevel)
{
    Order bid1 = Order(1 /*=id*/, Side::Bid, 50 /*=price*/, 5 /*=volume*/);
    Order ask1 = Order(2 /*=id*/, Side::Ask, 50 /*=price*/, 5 /*=volume*/);
    EXPECT_TRUE(mOrderBook.AddOrder(Side::Bid, 100 /*=price*/, 10 /*=volume*/));
    EXPECT_TRUE(mOrderBook.ModifyOrder(0 /*=id*/, 50 /*=price*/, 5 /*=volume*/));

    // setup expected matches in correct order
    mBidMatches.emplace(bid1);
    mAskMatches.emplace(ask1);
    mVolumeMatches.emplace(5);
    EXPECT_TRUE(mOrderBook.AddOrder(Side::Ask, 50 /*=price*/, 5 /*=volume*/));
    EXPECT_TRUE(mBidMatches.empty());
    EXPECT_TRUE(mAskMatches.empty());
}

TEST_F(OrderBookTest, MatchOrders)
{
    Order bid1 = Order(0 /*=id*/, Side::Bid, 10.7 /*=price*/, 5 /*=volume*/);
    Order bid2 = Order(1 /*=id*/, Side::Bid, 11 /*=price*/, 5 /*=volume*/);
    Order ask1 = Order(2 /*=id*/, Side::Ask, 7 /*=price*/, 10 /*=volume*/);
    Order ask2 = Order(2 /*=id*/, Side::Ask, 7 /*=price*/, 5 /*=volume*/);

    // setup expected matches in correct order
    mBidMatches.emplace(bid2);
    mAskMatches.emplace(ask1);
    mVolumeMatches.emplace(5);
    mBidMatches.emplace(bid1);
    mAskMatches.emplace(ask2);
    mVolumeMatches.emplace(5);

    EXPECT_TRUE(mOrderBook.AddOrder(bid1.mSide, bid1.mPrice, bid1.mVolume));
    EXPECT_TRUE(mOrderBook.AddOrder(bid2.mSide, bid2.mPrice, bid2.mVolume));
    EXPECT_TRUE(mOrderBook.AddOrder(ask1.mSide, ask1.mPrice, ask1.mVolume));
    EXPECT_TRUE(mBidMatches.empty());
    EXPECT_TRUE(mAskMatches.empty());
}

TEST_F(OrderBookTest, MatchOrdersAfterMultipleModifies)
{
    // ASK 10@96
    Order ask1 = Order(0 /*=id*/, Side::Ask, 96 /*=price*/, 10 /*=volume*/);
    
    // 3 x BID 5@90
    Order bid1 = Order(1 /*=id*/, Side::Bid, 90 /*=price*/, 5 /*=volume*/);
    Order bid2 = Order(2 /*=id*/, Side::Bid, 90 /*=price*/, 5 /*=volume*/);
    Order bid3 = Order(3 /*=id*/, Side::Bid, 90 /*=price*/, 5 /*=volume*/);

    // BID 1@93 + BID 1@92 + BID 2@91
    Order bid4 = Order(4 /*=id*/, Side::Bid, 93 /*=price*/, 1 /*=volume*/);
    Order bid5 = Order(5 /*=id*/, Side::Bid, 92 /*=price*/, 1 /*=volume*/);
    Order bid6 = Order(6 /*=id*/, Side::Bid, 91 /*=price*/, 2 /*=volume*/);

    // ASK 3@91 + ASK 10@96
    Order ask2 = Order(7 /*=id*/, Side::Ask, 91 /*=price*/, 3 /*=volume*/);
    Order ask3 = Order(8 /*=id*/, Side::Ask, 96 /*=price*/, 10 /*=volume*/, 1 /*=levelIndex*/);

    // BID 10@91 + 2 x BID 10@96
    Order bid7 = Order(9 /*=id*/, Side::Bid, 91 /*=price*/, 10 /*=volume*/, 1 /*=levelIndex*/);
    Order bid8 = Order(10 /*=id*/, Side::Bid, 96 /*=price*/, 10 /*=volume*/);
    Order bid9 = Order(11 /*=id*/, Side::Bid, 96 /*=price*/, 10 /*=volume*/);

    // ASK 10@91
    Order ask4 = Order(12 /*=id*/, Side::Ask, 91 /*=price*/, 10 /*=volume*/);

    // setup expected matches in correct order
    mBidMatches.emplace(bid4);
    mAskMatches.emplace(ask2); // partial match
    mVolumeMatches.emplace(1);

    mBidMatches.emplace(bid5);
    mAskMatches.emplace(ask2.mId, ask2.mSide, ask2.mPrice, 2 /*=volume*/); // partial match
    mVolumeMatches.emplace(1);

    mBidMatches.emplace(bid6); // partial match
    mAskMatches.emplace(ask2.mId, ask2.mSide, ask2.mPrice, 1 /*=volume*/); // full match
    mVolumeMatches.emplace(1);

    mBidMatches.emplace(bid8);
    mAskMatches.emplace(ask1); // full match
    mVolumeMatches.emplace(10);

    mBidMatches.emplace(bid9);
    mAskMatches.emplace(ask3); // full match
    mVolumeMatches.emplace(10);

    mBidMatches.emplace(bid6.mId,bid6.mSide, bid6.mPrice, 1 /*=volume*/); // full match
    mAskMatches.emplace(ask4); // partial match
    mVolumeMatches.emplace(1);

    mBidMatches.emplace(bid7); // partial match
    mAskMatches.emplace(ask4.mId, ask4.mSide, ask4.mPrice, 9 /*=volume*/); // full match
    mVolumeMatches.emplace(9);

    EXPECT_TRUE(mOrderBook.AddOrder(ask1.mSide, ask1.mPrice, ask1.mVolume));
    EXPECT_TRUE(mOrderBook.AddOrder(bid1.mSide, bid1.mPrice, bid1.mVolume));
    EXPECT_TRUE(mOrderBook.AddOrder(bid2.mSide, bid2.mPrice, bid2.mVolume));

    EXPECT_TRUE(mOrderBook.AddOrder(bid3.mSide, bid3.mPrice, bid3.mVolume));
    EXPECT_TRUE(mOrderBook.AddOrder(bid4.mSide, bid4.mPrice, bid4.mVolume));
    EXPECT_TRUE(mOrderBook.AddOrder(bid5.mSide, bid5.mPrice, bid5.mVolume));
    EXPECT_TRUE(mOrderBook.AddOrder(bid6.mSide, bid6.mPrice, bid6.mVolume));

    EXPECT_TRUE(mOrderBook.AddOrder(ask2.mSide, ask2.mPrice, ask2.mVolume));
    EXPECT_TRUE(mOrderBook.AddOrder(ask3.mSide, ask3.mPrice, ask3.mVolume));

    EXPECT_TRUE(mOrderBook.AddOrder(bid7.mSide, bid7.mPrice, bid7.mVolume));
    EXPECT_TRUE(mOrderBook.AddOrder(bid8.mSide, bid8.mPrice, bid8.mVolume));
    EXPECT_TRUE(mOrderBook.AddOrder(bid9.mSide, bid9.mPrice, bid9.mVolume));

    EXPECT_TRUE(mOrderBook.AddOrder(ask4.mSide, ask4.mPrice, ask4.mVolume));
    EXPECT_TRUE(mBidMatches.empty());
    EXPECT_TRUE(mAskMatches.empty());
}
