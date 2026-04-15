#include "engine_state.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

void save_state(const MatchingEngine& engine,
                std::uint64_t next_id,
                const std::string& filename)
{
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Failed to save state to " << filename << "\n";
        return;
    }

    auto metrics = engine.get_metrics();

    out << "# ENGINE STATE\n";
    out << "NEXT_ID " << next_id << "\n";
    out << "TOTAL_ORDERS " << metrics.total_orders << "\n";
    out << "TOTAL_TRADES " << metrics.total_trades << "\n";
    out << "TOTAL_VOLUME " << metrics.total_volume_traded << "\n";
    out << "CANCELLED " << metrics.cancelled_orders << "\n";
    out << "REJECTED " << metrics.rejected_orders << "\n\n";

    out << "# BIDS\n";
    for (const auto& [price, orders] : engine.book.bids) {
        for (const auto& o : orders) {
            out << price << " " << o.quantity << " " << o.id << "\n";
        }
    }

    out << "\n# ASKS\n";
    for (const auto& [price, orders] : engine.book.asks) {
        for (const auto& o : orders) {
            out << price << " " << o.quantity << " " << o.id << "\n";
        }
    }

    out << "\n# TRADES\n";
    for (const auto& t : engine.get_trade_history()) {
        out << t.taker_order_id << " "
            << t.maker_order_id << " "
            << t.price << " "
            << t.quantity << "\n";
    }

    std::cout << "State saved to " << filename << "\n";
}

bool load_state(MatchingEngine& engine,
                std::uint64_t& next_id,
                const std::string& filename)
{
    std::ifstream in(filename);
    if (!in) {
        std::cerr << "Failed to load state from " << filename << "\n";
        return false;
    }

    engine.reset();

    std::string line;
    std::string section;

    while (std::getline(in, line)) {
        if (line.empty()) continue;
        if (line[0] == '#') {
            section = line;
            continue;
        }

        std::stringstream ss(line);

        if (section == "# ENGINE STATE") {
            std::string key;
            ss >> key;

            if (key == "NEXT_ID") ss >> next_id;
            else if (key == "TOTAL_ORDERS") ss >> engine.metrics.total_orders;
            else if (key == "TOTAL_TRADES") ss >> engine.metrics.total_trades;
            else if (key == "TOTAL_VOLUME") ss >> engine.metrics.total_volume_traded;
            else if (key == "CANCELLED") ss >> engine.metrics.cancelled_orders;
            else if (key == "REJECTED") ss >> engine.metrics.rejected_orders;
        }

        else if (section == "# BIDS") {
            double price;
            uint64_t qty, id;
            ss >> price >> qty >> id;
            Order o { id, OrderSide::BUY, OrderType::LIMIT, price, qty };
            engine.book.add_order(o);
        }

        else if (section == "# ASKS") {
            double price;
            uint64_t qty, id;
            ss >> price >> qty >> id;
            Order o { id, OrderSide::SELL, OrderType::LIMIT, price, qty };
            engine.book.add_order(o);
        }

        else if (section == "# TRADES") {
            uint64_t taker, maker, qty;
            double price;
            ss >> taker >> maker >> price >> qty;
            engine.trade_history.push_back({ taker, maker, price, qty });
        }
    }

    std::cout << "State loaded from " << filename << "\n";
    return true;
}

void replay_script(MatchingEngine& engine,
                   std::uint64_t& next_id,
                   const std::string& filename)
{
    std::ifstream in(filename);
    if (!in) {
        std::cerr << "Replay file not found: " << filename << "\n";
        return;
    }

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        if (line[0] == '#') continue;

        std::cout << "> " << line << "\n";

        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (cmd == "BUY" || cmd == "SELL") {
            double price;
            uint64_t qty;
            ss >> price >> qty;
            OrderSide side = (cmd == "BUY") ? OrderSide::BUY : OrderSide::SELL;
            Order o { next_id++, side, OrderType::LIMIT, price, qty };
            engine.submit_order(o);
        }

        else if (cmd == "MARKET") {
            std::string side_str;
            uint64_t qty;
            ss >> side_str >> qty;
            OrderSide side = (side_str == "BUY") ? OrderSide::BUY : OrderSide::SELL;
            Order o { next_id++, side, OrderType::MARKET, 0.0, qty };
            engine.submit_order(o);
        }

        else if (cmd == "CANCEL") {
            uint64_t id;
            ss >> id;
            engine.cancel(id);
        }
    }
}
