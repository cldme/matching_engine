#include "order.h"

bool Order::operator==(const Order& other) const
{
    return mId == other.mId &&
           mSide == other.mSide &&
           mPrice == other.mPrice &&
           mVolume == other.mVolume &&
           mLevelIndex == other.mLevelIndex &&
           mIsActive == other.mIsActive;
}

bool Order::operator!=(const Order& other) const
{
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& os, const Side& side)
{
    switch (side)
    {
        case Side::Bid:
            os << "BID";
            break;
        case Side::Ask:
            os << "ASK";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Order& order)
{
    return os 
        << "(id=" << order.mId 
        << " side=" << order.mSide 
        << " volume=" << order.mVolume 
        << " price=" << order.mPrice 
        << " levelIndex=" << order.mLevelIndex
        << " isActive=" << order.mIsActive
        << ")";
}