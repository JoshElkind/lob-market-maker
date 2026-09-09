#pragma once

#include <cstdint>
#include <list>
#include <map>
#include <optional>
#include <utility>
#include <vector>

namespace lob {

enum class Side { Buy, Sell };

struct Order {
  std::uint64_t id = 0;
  Side side = Side::Buy;
  // Limit price in ticks.
  std::int64_t price = 0;
  std::int64_t qty = 0;
};

struct Trade {
  std::uint64_t taker_id = 0;
  std::uint64_t maker_id = 0;
  Side taker_side = Side::Buy;
  std::int64_t price = 0;
  std::int64_t qty = 0;
};

class OrderBook {
 public:
  // Resting remainder is kept on the book. Returns fills against resting liquidity.
  std::vector<Trade> add(Order order);

  std::optional<std::int64_t> best_bid() const;
  std::optional<std::int64_t> best_ask() const;

  std::int64_t bid_depth(std::int64_t price) const;
  std::int64_t ask_depth(std::int64_t price) const;

 private:
  using Queue = std::list<Order>;
  using BidBook = std::map<std::int64_t, Queue, std::greater<std::int64_t>>;
  using AskBook = std::map<std::int64_t, Queue, std::less<std::int64_t>>;

  template <typename Book>
  std::vector<Trade> match(Book& opposite, Order& incoming);

  template <typename Book>
  void rest(Book& book, Order order);

  BidBook bids_;
  AskBook asks_;
};

}  // namespace lob
