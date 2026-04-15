#include "order_book.hpp"
#include <algorithm>

void OrderBook::add_order(const Order& order) {
    if (order.side == OrderSide::BUY) {
        bids[order.price].push_back(order);
    } else {
        asks[order.price].push_back(order);
    }
}

std::optional<Order> OrderBook::find_order(std::uint64_t order_id) const {
    // Search bids
    for (const auto& [price, orders] : bids) {
        for (const auto& o : orders) {
            if (o.id == order_id)
                return o;
        }
    }

    // Search asks
    for (const auto& [price, orders] : asks) {
        for (const auto& o : orders) {
            if (o.id == order_id)
                return o;
        }
    }

    return std::nullopt;
}

bool OrderBook::cancel_order(std::uint64_t order_id) {
    bool removed = false;

    // Remove from bids
    for (auto it = bids.begin(); it != bids.end(); ) {
        auto& vec = it->second;

        auto vit = std::remove_if(vec.begin(), vec.end(),
                                  [order_id](const Order& o) { return o.id == order_id; });

        if (vit != vec.end()) {
            vec.erase(vit, vec.end());
            removed = true;
        }

        if (vec.empty()) {
            it = bids.erase(it);
        } else {
            ++it;
        }
    }

    // Remove from asks
    for (auto it = asks.begin(); it != asks.end(); ) {
        auto& vec = it->second;

        auto vit = std::remove_if(vec.begin(), vec.end(),
                                  [order_id](const Order& o) { return o.id == order_id; });

        if (vit != vec.end()) {
            vec.erase(vit, vec.end());
            removed = true;
        }

        if (vec.empty()) {
            it = asks.erase(it);
        } else {
            ++it;
        }
    }

    return removed;
}

OrderBookDepth OrderBook::get_depth(int levels) const {
    OrderBookDepth depth;

    // BIDS: highest price first
    int count = 0;
    for (auto it = bids.begin(); it != bids.end() && count < levels; ++it) {
        int total_qty = 0;
        for (const auto& order : it->second) {
            total_qty += order.quantity;
        }

        depth.bids.push_back(DepthLevel{
            it->first,
            total_qty,
            static_cast<int>(it->second.size())
        });

        count++;
    }

    // ASKS: lowest price first
    count = 0;
    for (auto it = asks.begin(); it != asks.end() && count < levels; ++it) {
        int total_qty = 0;
        for (const auto& order : it->second) {
            total_qty += order.quantity;
        }

        depth.asks.push_back(DepthLevel{
            it->first,
            total_qty,
            static_cast<int>(it->second.size())
        });

        count++;
    }

    return depth;
}

double OrderBook::best_bid() const {
    if (bids.empty()) {
        return 0.0;
    }
    return bids.begin()->first;
}

double OrderBook::best_ask() const {
    if (asks.empty()) {
        return 0.0;
    }
    return asks.begin()->first;
}
