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
  return lob::Order{id, side, price, qty};
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

  if (failures != 0) {
    std::cerr << failures << " failure(s)\n";
    return EXIT_FAILURE;
  }
  std::cout << "book tests passed\n";
  return EXIT_SUCCESS;
}
