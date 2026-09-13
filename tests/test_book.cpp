#include "lob/book.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect(bool cond, const std::string& msg) {
  if (!cond) {
    std::cerr << "FAIL " << msg << "\n";
    ++failures;
  }
}

lob::Order limit(std::uint64_t id, lob::Side side, std::int64_t price, std::int64_t qty) {
  return lob::Order{id, side, price, qty, false};
}

}  // namespace

int main() {
  using lob::OrderBook;
  using lob::Side;

  {
    OrderBook book;
    book.add(limit(1, Side::Buy, 100, 2));
    book.add(limit(2, Side::Buy, 100, 2));
    book.add(limit(3, Side::Buy, 99, 5));
    const auto trades = book.add(limit(4, Side::Sell, 100, 3));
    expect(trades.size() == 2, "sell should hit two price levels or two orders");
    expect(trades[0].maker_id == 1 && trades[0].qty == 2, "first order at the touch has priority");
    expect(trades[1].maker_id == 2 && trades[1].qty == 1, "second fill is the next FIFO order");
    expect(book.bid_depth(100) == 1, "one lot remains behind the partial fill");
    expect(book.best_bid() == 100, "touch stays at 100");
  }

  {
    OrderBook book;
    book.add(limit(1, Side::Buy, 100, 1));
    book.add(limit(2, Side::Sell, 105, 1));
    const auto trades = book.add(limit(3, Side::Buy, 104, 1));
    expect(trades.empty(), "bid below the ask does not trade");
    expect(book.best_bid() == 104, "the better bid becomes the touch");
    expect(book.live_orders() == 3, "new bid is live");
  }

  {
    OrderBook book;
    book.add(limit(1, Side::Sell, 101, 4));
    book.add(limit(2, Side::Sell, 103, 4));
    lob::Order market{3, Side::Buy, 0, 6, true};
    const auto trades = book.add(market);
    expect(trades.size() == 2, "market buy sweeps two ask levels");
    expect(trades[0].price == 101 && trades[0].qty == 4, "first level is the best ask");
    expect(trades[1].price == 103 && trades[1].qty == 2, "remainder walks to the next level");
    expect(book.ask_depth(103) == 2, "unfilled size stays at the second level");
  }

  {
    OrderBook book;
    book.add(limit(1, Side::Buy, 50, 1));
    expect(book.cancel(1), "cancel removes a live order");
    expect(!book.best_bid().has_value(), "book is empty after cancel");
    expect(!book.cancel(1), "double cancel fails");
  }

  {
    OrderBook book;
    lob::Order market{1, Side::Buy, 0, 1, true};
    expect(book.add(market).empty(), "market order on an empty book does not trade");
    expect(book.live_orders() == 0, "market orders do not rest");
  }

  if (failures != 0) {
    std::cerr << failures << " failure(s)\n";
    return EXIT_FAILURE;
  }
  std::cout << "book tests passed\n";
  return EXIT_SUCCESS;
}
