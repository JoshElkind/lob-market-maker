# Limit order book and market-making simulator

A price-time priority order book.

## Book

- Bids are ordered best price first. Asks are ordered cheapest first.
- Orders at the same price trade first-in, first-out.
- Limit orders that do not fully fill rest on the book. Market orders never rest.
- Trades print at the resting order's price.

## Build

```bash
cmake -S . -B build
cmake --build build
./build/lob_tests
```
