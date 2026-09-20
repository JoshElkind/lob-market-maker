#include "lob/sim.hpp"

#include <iostream>

int main() {
  const lob::SimReport report = lob::run_sim(lob::SimConfig{});
  std::cout << lob::format_report(report);
  return 0;
}
