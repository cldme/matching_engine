#include "order_book.h"
#include <gtest/gtest.h>

class OrderBookTest : public ::testing::Test 
{
protected:
    OrderBook mOrderBook;

    void SetUp() override
    {}

    void TearDown() override
    {}
};

TEST_F(OrderBookTest, AddOrder)
{
    EXPECT_TRUE(mOrderBook.AddOrder(1 /*=id*/, Side::Buy, 10.7 /*=price*/, 5 /*=volume*/));
    auto* order = mOrderBook.FindOrder(1 /*=id*/);
    EXPECT_TRUE(order != nullptr);
    EXPECT_EQ(order->mId, 1);
    EXPECT_EQ(order->mSide, Side::Buy);
    EXPECT_EQ(order->mPrice, 10.7);
    EXPECT_EQ(order->mVolume, 5);
    EXPECT_EQ(order->mIsActive, true);
}

TEST_F(OrderBookTest, ModifyOrder)
{
    EXPECT_TRUE(mOrderBook.AddOrder(1 /*=id*/, Side::Sell, 11.4 /*=price*/, 7 /*=volume*/));
    EXPECT_TRUE(mOrderBook.ModifyOrder(1 /*=id*/, 14 /*=volume*/));
    auto* order = mOrderBook.FindOrder(1 /*=id*/);
    EXPECT_TRUE(order != nullptr);
    EXPECT_EQ(order->mId, 1);
    EXPECT_EQ(order->mSide, Side::Sell);
    EXPECT_EQ(order->mPrice, 11.4);
    EXPECT_EQ(order->mVolume, 14);
    EXPECT_EQ(order->mIsActive, true);
}

TEST_F(OrderBookTest, DeleteOrder)
{
    EXPECT_TRUE(mOrderBook.AddOrder(1 /*=id*/, Side::Buy, 15 /*=price*/, 50 /*=volume*/));
    auto* order = mOrderBook.FindOrder(1 /*=id*/);
    EXPECT_TRUE(order != nullptr);
    EXPECT_EQ(order->mId, 1);
    EXPECT_EQ(order->mSide, Side::Buy);
    EXPECT_EQ(order->mPrice, 15);
    EXPECT_EQ(order->mVolume, 50);
    EXPECT_EQ(order->mIsActive, true);
    EXPECT_TRUE(mOrderBook.DeleteOrder(1 /*=id*/));
    EXPECT_EQ(order->mIsActive, false);
}

TEST_F(OrderBookTest, MatchOrders)
{
    EXPECT_TRUE(mOrderBook.AddOrder(1 /*=id*/, Side::Buy, 10.7 /*=price*/, 5 /*=volume*/));
    EXPECT_TRUE(mOrderBook.AddOrder(2 /*=id*/, Side::Buy, 11 /*=price*/, 5 /*=volume*/));
    EXPECT_TRUE(mOrderBook.AddOrder(3 /*=id*/, Side::Sell, 7 /*=price*/, 10 /*=volume*/));
}
