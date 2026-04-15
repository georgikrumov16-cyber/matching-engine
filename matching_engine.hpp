#pragma once
#include <vector>
#include <optional>
#include <string>
#include <cstdint>
#include "order_book.hpp"

// ---------------------------------------------------------
// ENUMS
// ---------------------------------------------------------
enum class OrderSide { BUY, SELL };
enum class OrderType { LIMIT, MARKET };

// ---------------------------------------------------------
// ORDER STRUCT
// ---------------------------------------------------------
struct Order {
    std::uint64_t id;
    OrderSide side;
    OrderType type;
    double price;
    std::uint64_t quantity;
};

// ---------------------------------------------------------
// TRADE STRUCT
// ---------------------------------------------------------
struct Trade {
    std::uint64_t taker_order_id;
    std::uint64_t maker_order_id;
    double price;
    std::uint64_t quantity;
};

// ---------------------------------------------------------
// METRICS STRUCT
// ---------------------------------------------------------
struct EngineMetrics {
    std::uint64_t total_orders = 0;
    std::uint64_t total_trades = 0;
    std::uint64_t total_volume_traded = 0;
    std::uint64_t cancelled_orders = 0;
    std::uint64_t rejected_orders = 0;
};

// Forward declare load_state so it can access private members
bool load_state(class MatchingEngine&, std::uint64_t&, const std::string&);

// ---------------------------------------------------------
// MATCHING ENGINE CLASS
// ---------------------------------------------------------
class MatchingEngine {
public:
    MatchingEngine() = default;

    // Core functionality
    std::vector<Trade> submit_order(const Order& order);

    bool cancel(std::uint64_t order_id) { 
        bool ok = book.cancel_order(order_id);
        if (ok) metrics.cancelled_orders++;
        return ok;
    }

    // Lookup
    std::optional<Order> get_order(std::uint64_t id) const {
        return book.find_order(id);
    }

    // Metrics
    EngineMetrics get_metrics() const;

    // Trade history
    const std::vector<Trade>& get_trade_history() const {
        return trade_history;
    }

    // Reset engine (used by LOAD)
    void reset();

    // Order book is public for CLI printing and engine_state
    OrderBook book;

private:
    std::vector<Trade> trade_history;
    EngineMetrics metrics;

    void log(const std::string& msg) const;

    // Allow load_state() to access private members
    friend bool load_state(MatchingEngine&, std::uint64_t&, const std::string&);
};
