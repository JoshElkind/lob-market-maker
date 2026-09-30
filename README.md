# Limit order book and market-making simulator

A price-time priority order book and a small simulation of quoting against uninformed and informed flow.

This is a study model, not a backtest of a real market. There is one latent fair price. The market maker quotes before the next jump. Noise traders send a limit order at the unskewed touch, so a quote skewed away from them does not fill. After the fair price jumps by more than the half spread, an informed trader sends a limit at the new fair and trades only if the stale quote is through that price. That loss is adverse selection.

## Book

- Bids are ordered best price first. Asks are ordered cheapest first.
- Orders at the same price trade first-in, first-out.
- Limit orders that do not fully fill rest on the book. Market orders never rest.
- Trades print at the resting order's price.

## Two quoters

Both quote `half_spread` ticks around a reference price and stop bidding or offering at `max_inventory`.

- `fixed_spread` uses the latent fair price as the reference.
- `inventory_skew` uses `fair - inventory * skew`. A long inventory lowers both quotes, so the maker is more willing to sell and less willing to buy.

The same seed drives both runs.

## Build

```bash
cmake -S . -B build
cmake --build build
./build/lob_tests
./build/lob_sim
```
