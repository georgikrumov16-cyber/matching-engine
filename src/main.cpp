#include <iostream>
#include <sstream>
#include <string>
#include <set>
#include <iomanip>

#include "matching_engine.hpp"
#include "engine_state.hpp"   // <-- NEW include

// Pretty depth-of-book output
void print_book(const OrderBook& book) {
    std::cout << "\n==================== ORDER BOOK ====================\n";
    std::cout << "   PRICE        BID_QTY        ASK_QTY\n";
    std::cout << "----------------------------------------------------\n";

    std::set<double> prices;
    for (const auto& [p, _] : book.bids) prices.insert(p);
    for (const auto& [p, _] : book.asks) prices.insert(p);

    for (double price : prices) {
        std::uint64_t bid_qty = 0;
        std::uint64_t ask_qty = 0;

        if (book.bids.count(price)) {
            for (const auto& o : book.bids.at(price))
                bid_qty += o.quantity;
        }

        if (book.asks.count(price)) {
            for (const auto& o : book.asks.at(price))
                ask_qty += o.quantity;
        }

        std::cout << std::fixed << std::setprecision(2);
        std::cout << std::setw(8) << price << "    ";

        if (bid_qty > 0) std::cout << std::setw(8) << bid_qty << "        ";
        else std::cout << "    -          ";

        if (ask_qty > 0) std::cout << ask_qty;
        else std::cout << "-";

        std::cout << "\n";
    }

    std::cout << "====================================================\n";
}

// Help menu
void print_help() {
    std::cout << "\n==================== COMMANDS ====================\n";
    std::cout << "  BUY <price> <qty>          - Submit BUY limit order\n";
    std::cout << "  SELL <price> <qty>         - Submit SELL limit order\n";
    std::cout << "  MARKET BUY <qty>           - Submit BUY market order\n";
    std::cout << "  MARKET SELL <qty>          - Submit SELL market order\n";
    std::cout << "  CANCEL <id>                - Cancel order by ID\n";
    std::cout << "  ORDER <id>                 - Show order details\n";
    std::cout << "  BOOK                       - Print order book\n";
    std::cout << "  TRADES                     - Print trade history\n";
    std::cout << "  METRICS                    - Show engine metrics\n";
    std::cout << "  SAVE <file>                - Save engine state\n";
    std::cout << "  LOAD <file>                - Load engine state\n";
    std::cout << "  REPLAY <file>              - Replay script file\n";
    std::cout << "  HELP                       - Show this help menu\n";
    std::cout << "  EXIT                       - Quit program\n";
    std::cout << "=================================================\n\n";
}

int main() {
    MatchingEngine engine;
    std::string line;
    std::uint64_t next_id = 1;

    std::cout << "Simple Exchange CLI\n";
    print_help();

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, line);

        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;

        // EXIT
        if (cmd == "EXIT") {
            break;
        }

        // HELP
        else if (cmd == "HELP") {
            print_help();
        }

        // BUY / SELL LIMIT ORDERS
        else if (cmd == "BUY" || cmd == "SELL") {
            double price;
            std::uint64_t qty;
            ss >> price >> qty;

            OrderSide side = (cmd == "BUY") ? OrderSide::BUY : OrderSide::SELL;

            Order o { next_id++, side, OrderType::LIMIT, price, qty };
            auto trades = engine.submit_order(o);

            for (auto& t : trades) {
                std::cout << "TRADE: taker=" << t.taker_order_id
                          << " maker=" << t.maker_order_id
                          << " price=" << t.price
                          << " qty=" << t.quantity << "\n";
            }
        }

        // MARKET ORDERS
        else if (cmd == "MARKET") {
            std::string side_str;
            std::uint64_t qty;
            ss >> side_str >> qty;

            OrderSide side = (side_str == "BUY") ? OrderSide::BUY : OrderSide::SELL;

            Order o { next_id++, side, OrderType::MARKET, 0.0, qty };
            auto trades = engine.submit_order(o);

            for (auto& t : trades) {
                std::cout << "MARKET TRADE: taker=" << t.taker_order_id
                          << " maker=" << t.maker_order_id
                          << " price=" << t.price
                          << " qty=" << t.quantity << "\n";
            }
        }

        // CANCEL ORDER
        else if (cmd == "CANCEL") {
            std::uint64_t id;
            ss >> id;
            bool ok = engine.cancel(id);
            std::cout << (ok ? "Cancelled order " : "Order not found: ") << id << "\n";
        }

        // ORDER LOOKUP
        else if (cmd == "ORDER") {
            std::uint64_t id;
            ss >> id;

            auto result = engine.get_order(id);

            if (!result.has_value()) {
                std::cout << "Order " << id << " not found.\n";
            } else {
                const Order& o = result.value();
                std::cout << "\n--- ORDER INFO ---\n";
                std::cout << "ID: " << o.id << "\n";
                std::cout << "Side: " << (o.side == OrderSide::BUY ? "BUY" : "SELL") << "\n";
                std::cout << "Type: " << (o.type == OrderType::LIMIT ? "LIMIT" : "MARKET") << "\n";
                std::cout << "Price: " << o.price << "\n";
                std::cout << "Quantity: " << o.quantity << "\n";
                std::cout << "-------------------\n";
            }
        }

        // PRINT ORDER BOOK
        else if (cmd == "BOOK") {
            print_book(engine.book);
        }

        // TRADE HISTORY
        else if (cmd == "TRADES") {
            const auto& hist = engine.get_trade_history();

            std::cout << "\n--- TRADE HISTORY ---\n";
            for (const auto& t : hist) {
                std::cout << "taker=" << t.taker_order_id
                          << " maker=" << t.maker_order_id
                          << " price=" << t.price
                          << " qty=" << t.quantity << "\n";
            }
            std::cout << "----------------------\n";
        }

        // METRICS
        else if (cmd == "METRICS") {
            auto m = engine.get_metrics();

            std::cout << "\n--- ENGINE METRICS ---\n";
            std::cout << "Total Orders:       " << m.total_orders << "\n";
            std::cout << "Total Trades:       " << m.total_trades << "\n";
            std::cout << "Volume Traded:      " << m.total_volume_traded << "\n";
            std::cout << "Cancelled Orders:   " << m.cancelled_orders << "\n";
            std::cout << "Rejected Orders:    " << m.rejected_orders << "\n";
            std::cout << "-----------------------\n";
        }

        // SAVE STATE
        else if (cmd == "SAVE") {
            std::string filename;
            ss >> filename;
            save_state(engine, next_id, filename);
        }

        // LOAD STATE
        else if (cmd == "LOAD") {
            std::string filename;
            ss >> filename;
            load_state(engine, next_id, filename);
        }

        // REPLAY SCRIPT
        else if (cmd == "REPLAY") {
            std::string filename;
            ss >> filename;
            replay_script(engine, next_id, filename);
        }

        // UNKNOWN COMMAND
        else {
            std::cout << "Unknown command. Type HELP for a list of commands.\n";
        }
    }

    return 0;
}
