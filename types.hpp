#pragma once
#include <cstdint>

enum class OrderSide { BUY, SELL };
enum class OrderType { LIMIT, MARKET };

struct Order {
    std::uint64_t id;
    OrderSide side;
    OrderType type;
    double price;
    std::uint64_t quantity;
    std::uint64_t timestamp;
};
