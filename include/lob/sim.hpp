#pragma once

#include <cstdint>
#include <string>

namespace lob {

struct SimConfig {
  std::uint64_t seed = 7;
  int steps = 10000;
  std::int64_t fair0 = 10000;
  std::int64_t half_spread = 2;
  // Fair value jumps by this many ticks. Larger than half_spread, so a stale quote
  // can be through the new fair and an informed taker will trade it.
  std::int64_t jump_size = 4;
  std::int64_t quote_size = 1;
  // Probability a step has a noise taker, then an independent chance the fair value jumps
  // and an informed taker hits the stale quote.
  double noise_prob = 0.5;
  double jump_prob = 0.2;
};

struct StrategyReport {
  std::string name;
  double mean_abs_inventory = 0;
  double mean_mtm = 0;
  double terminal_mtm = 0;
  std::int64_t fills = 0;
  std::int64_t max_abs_inventory = 0;
};

struct SimReport {
  StrategyReport fixed_spread;
};

SimReport run_sim(const SimConfig& config);
std::string format_report(const SimReport& report);

}  // namespace lob
