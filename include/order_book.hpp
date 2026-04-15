#pragma once

#include <vector>
#include <map>
#include <functional>
#include <optional>
#include <cstdint>
#include "order.hpp"   // or matching_engine.hpp if Order is defined there

class OrderBook {
public:
    void add_order(const Order& order);
    bool cancel_order(std::uint64_t order_id);

    double best_bid() const;
    double best_ask() const;

    std::optional<Order> find_order(std::uint64_t order_id) const;

    struct DepthLevel {
        double price;
        std::uint64_t total_quantity;
        std::size_t order_count;
    };

    struct OrderBookDepth {
        std::vector<DepthLevel> bids;
        std::vector<DepthLevel> asks;
    };

    // Depth function
    OrderBookDepth get_depth(int levels) const;

    // Expose raw maps so engine_state.cpp and CLI can access them
    std::map<double, std::vector<Order>, std::greater<double>> bids;
    std::map<double, std::vector<Order>> asks;
};
