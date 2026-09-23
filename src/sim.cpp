#include "lob/sim.hpp"

#include "lob/book.hpp"

#include <cmath>
#include <random>
#include <sstream>
#include <vector>

namespace lob {
namespace {

struct Maker {
  std::string name;
  std::int64_t cash = 0;
  std::int64_t inventory = 0;
  std::int64_t fills = 0;
  std::int64_t max_abs_inventory = 0;
  double abs_inv_sum = 0;
  double mtm_sum = 0;
  std::uint64_t bid_id = 0;
  std::uint64_t ask_id = 0;
};

void on_trade(Maker& maker, const Trade& trade) {
  const bool we_bought = trade.maker_id == maker.bid_id && trade.taker_side == Side::Sell;
  const bool we_sold = trade.maker_id == maker.ask_id && trade.taker_side == Side::Buy;
  if (!we_bought && !we_sold) {
    return;
  }
  const std::int64_t signed_qty = we_bought ? trade.qty : -trade.qty;
  maker.cash -= signed_qty * trade.price;
  maker.inventory += signed_qty;
  maker.fills += 1;
  const std::int64_t abs_inv = maker.inventory < 0 ? -maker.inventory : maker.inventory;
  if (abs_inv > maker.max_abs_inventory) {
    maker.max_abs_inventory = abs_inv;
  }
}

void pull_quotes(OrderBook& book, Maker& maker) {
  if (maker.bid_id != 0) {
    book.cancel(maker.bid_id);
    maker.bid_id = 0;
  }
  if (maker.ask_id != 0) {
    book.cancel(maker.ask_id);
    maker.ask_id = 0;
  }
}

void quote(OrderBook& book, Maker& maker, std::int64_t fair, const SimConfig& cfg, std::uint64_t& next_id) {
  pull_quotes(book, maker);
  const std::int64_t bid_px = fair - cfg.half_spread;
  const std::int64_t ask_px = fair + cfg.half_spread;
  if (bid_px <= 0) {
    return;
  }
  maker.bid_id = next_id++;
  book.add(Order{maker.bid_id, Side::Buy, bid_px, cfg.quote_size, false});
  maker.ask_id = next_id++;
  book.add(Order{maker.ask_id, Side::Sell, ask_px, cfg.quote_size, false});
}

StrategyReport summarize(const Maker& maker, int steps) {
  StrategyReport out;
  out.name = maker.name;
  out.mean_abs_inventory = maker.abs_inv_sum / static_cast<double>(steps);
  out.mean_mtm = maker.mtm_sum / static_cast<double>(steps);
  out.fills = maker.fills;
  out.max_abs_inventory = maker.max_abs_inventory;
  return out;
}

StrategyReport run_one(const SimConfig& cfg) {
  std::mt19937_64 rng(cfg.seed);
  std::uniform_real_distribution<double> coin(0.0, 1.0);
  std::uniform_int_distribution<int> noise_side(0, 1);

  OrderBook book;
  Maker maker;
  maker.name = "fixed_spread";
  std::uint64_t next_id = 1;
  const std::int64_t fair = cfg.fair0;

  auto mark = [&]() {
    const double mtm = static_cast<double>(maker.cash + maker.inventory * fair);
    maker.mtm_sum += mtm;
    const std::int64_t abs_inv = maker.inventory < 0 ? -maker.inventory : maker.inventory;
    maker.abs_inv_sum += static_cast<double>(abs_inv);
    return mtm;
  };

  double terminal = 0;
  for (int step = 0; step < cfg.steps; ++step) {
    quote(book, maker, fair, cfg, next_id);

    // Noise takers pay up to the touch.
    if (coin(rng) < cfg.noise_prob) {
      Order taker;
      taker.id = next_id++;
      taker.qty = 1;
      if (noise_side(rng) == 0) {
        taker.side = Side::Buy;
        taker.price = fair + cfg.half_spread;
      } else {
        taker.side = Side::Sell;
        taker.price = fair - cfg.half_spread;
      }
      for (const Trade& trade : book.add(taker)) {
        on_trade(maker, trade);
      }
    }

    terminal = mark();
  }

  StrategyReport report = summarize(maker, cfg.steps);
  report.terminal_mtm = terminal;
  return report;
}

}  // namespace

SimReport run_sim(const SimConfig& config) {
  SimReport report;
  report.fixed_spread = run_one(config);
  return report;
}

std::string format_report(const SimReport& report) {
  std::ostringstream out;
  out.setf(std::ios::fixed);
  out.precision(2);
  const StrategyReport* rows[] = {&report.fixed_spread};
  for (const StrategyReport* row : rows) {
    out << row->name
        << " terminal_mtm=" << row->terminal_mtm
        << " mean_mtm=" << row->mean_mtm
        << " mean_abs_inventory=" << row->mean_abs_inventory
        << " max_abs_inventory=" << row->max_abs_inventory
        << " fills=" << row->fills
        << "\n";
  }
  return out.str();
}

}  // namespace lob
