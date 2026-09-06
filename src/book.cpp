#include "lob/book.hpp"

#include <algorithm>

namespace lob {
namespace {

bool crosses(Side side, std::int64_t limit, std::int64_t level) {
  if (side == Side::Buy) {
    return limit >= level;
  }
  return limit <= level;
}

}  // namespace

template <typename Book>
std::vector<Trade> OrderBook::match(Book& opposite, Order& incoming) {
  std::vector<Trade> trades;
  while (incoming.qty > 0 && !opposite.empty()) {
    auto level_it = opposite.begin();
    const std::int64_t level_price = level_it->first;
    if (!crosses(incoming.side, incoming.price, level_price)) {
      break;
    }
    auto& queue = level_it->second;
    while (incoming.qty > 0 && !queue.empty()) {
      Order& resting = queue.front();
      const std::int64_t fill = std::min(incoming.qty, resting.qty);
      trades.push_back(Trade{
          incoming.id,
          resting.id,
          incoming.side,
          level_price,
          fill,
      });
      incoming.qty -= fill;
      resting.qty -= fill;
      if (resting.qty == 0) {
        queue.pop_front();
      }
    }
    if (queue.empty()) {
      opposite.erase(level_it);
    }
  }
  return trades;
}

template <typename Book>
void OrderBook::rest(Book& book, Order order) {
  auto& queue = book[order.price];
  queue.push_back(order);
}

std::vector<Trade> OrderBook::add(Order order) {
  std::vector<Trade> trades;
  if (order.side == Side::Buy) {
    trades = match(asks_, order);
    if (order.qty > 0) {
      rest(bids_, order);
    }
  } else {
    trades = match(bids_, order);
    if (order.qty > 0) {
      rest(asks_, order);
    }
  }
  return trades;
}

std::optional<std::int64_t> OrderBook::best_bid() const {
  if (bids_.empty()) {
    return std::nullopt;
  }
  return bids_.begin()->first;
}

std::optional<std::int64_t> OrderBook::best_ask() const {
  if (asks_.empty()) {
    return std::nullopt;
  }
  return asks_.begin()->first;
}

}  // namespace lob
