# Answers

## how I profiled

Built with the default `-O2 -g` and used `perf` since that's what I used in lab1 too.

```
make all
perf record -g ./main
perf report
```

at `-O2` almost everything gets inlined into `main`, so `perf report` just showed `main` at ~96% and I couldn't tell which function was actually hot. So I rebuilt with `-fno-inline` to get per-function numbers:

```
g++ -std=c++20 -O2 -g -fno-inline main.cpp -o main_ni
perf record -g ./main_ni
perf report --stdio
```

That gave me real self-time per function. I also ran `perf stat` to look at branch misses. Outputs below are pasted from those runs (no GUI on this machine so no kcachegrind screenshots).

## baseline (history_cols = 128)

`perf report --stdio` on the `-fno-inline` baseline build. The second column is self time (time in the function itself, not callees), which is what I went by:

```
    60.71%    30.64%  base_ni  [.] chase_dependency(...) [clone .constprop.0] [clone .isra.0]
    15.80%    12.92%  base_ni  [.] branchy_score(Packet const&, ...) [clone .isra.0]
    13.11%    11.26%  base_ni  [.] refresh_history(...) [clone .constprop.0]
    12.12%    12.12%  base_ni  [.] std::vector<int>::operator[](unsigned long) const
    11.70%     9.21%  base_ni  [.] process_packets(...) [clone .isra.0]
```

Summarized self time:

| function | self % |
|---|---|
| chase_dependency | ~31% |
| branchy_score | ~13% |
| refresh_history | ~11% |
| process_packets | ~9% |
| cold_column_probe | ~5% |

`perf stat -r 5` on the baseline:

```
       48.10 msec task-clock                        ( +-  4.90% )
  25,015,073      branches                          ( +-  0.14% )
   1,185,048      branch-misses                     ( +-  0.26% )   # 4.7% of branches
 0.048871285 seconds time elapsed                   ( +-  4.93% )
```

So 4.7% branch miss rate, ~25M branches, whole program ~49ms.

The two things that jumped out were `chase_dependency` (way too hot) and `branchy_score` (those if/else look like they'd mispredict).

## what I changed

### 1. chase_dependency 

This walks a random "next" pointer chain 7 times per packet and adds up values.
The array is 2^18 ints (1MB) and the jumps are random, so it's basically just eating cache misses. It gets called 220000 * 6 = 1.32M times.

The key thing is `next` and `value` are built once and never change, and STEPS is constant (the comment on `#define STEPS 7` is hinting at this). So the result only depends on the starting index. I just cache the result per start index, so after the first time it's one lookup instead of 7 random hops. Reused across all 6 epochs.

### 2. branchy_score (branchless)

6 data dependent if/else on the bits of `x`. Those mispredict a lot. I rewrote each one as a branchless select with a small `pick()` helper that builds an all-ones / all-zeros mask from the bit. Same math, no branches.

### 3. cold_column_probe (this is the 2048 one, see below)

## result at 128

`perf report --stdio` after the changes, no single function dominates anymore:

```
    27.19%    14.11%  opt_ni   [.] branchy_score(Packet const&, ...) [clone .isra.0]
    26.22%    14.61%  opt_ni   [.] chase_dependency(...) [clone .constprop.0]
    21.65%    12.60%  opt_ni   [.] pick(int, int, int)
    19.63%    17.62%  opt_ni   [.] refresh_history(...) [clone .constprop.0]
```

`perf stat -r 5` after:

```
       23.79 msec task-clock                        ( +-  1.20% )
  17,542,016      branches                          ( +-  0.19% )
     566,719      branch-misses                     ( +-  0.43% )   # 3.2% of branches
 0.024545152 seconds time elapsed                   ( +-  1.20% )
```

| | before | after |
|---|---|---|
| time | ~49ms | ~24ms |
| branches | 25.0M | 17.5M |
| branch-miss rate | 4.7% | 3.2% |

About 2x faster. Same output `6040578838`. The profile is way flatter, with `branchy_score`, `chase_dependency`, `pick` and `refresh_history` all sitting around 12-18% self instead of one function at 30%+.

## history_cols = 2048

Changed only `history_cols` to 2048. Output is `6745589558` like the readme says.

This got slow fast, ~416ms vs 49ms. `perf report --stdio` on the (still otherwise-optimized) 2048 build makes the new hotspot obvious:

```
    69.26%    49.30%  base2048_ni  [.] cold_column_probe(...) [clone .constprop.0] [clone .isra.0]
    21.19%    21.19%  base2048_ni  [.] std::vector<int>::operator[](unsigned long) const
    14.29%    11.94%  base2048_ni  [.] refresh_history(...) [clone .constprop.0]
     8.25%     3.42%  base2048_ni  [.] chase_dependency(...) [clone .constprop.0]
```

| function | self % |
|---|---|
| cold_column_probe | ~49% |
| refresh_history | ~12% |
| chase_dependency | ~3% |

`cold_column_probe` walks the history array column by column. At 128 cols that was fine but at 2048 the array is 4096 x 2048 = 33MB, and going down a column means every step jumps 2048 ints (8KB) in memory, so it misses cache basically every single read.

The fix is just loop interchange: the function sums `& 31` of every cell, and every column gets visited exactly once, so the order doesn't matter at all (the `start_col`/seed stuff is irrelevant to the answer). I swapped the loops so the inner one walks contiguous memory.

### result at 2048

`perf stat -r 5`, before vs after the loop interchange:

```
# before (column-major walk)
      415.51 msec task-clock                        ( +-  1.09% )
 0.416394281 seconds time elapsed                   ( +-  1.09% )

# after (row-major / contiguous walk)
       82.11 msec task-clock                        ( +-  1.61% )
 0.082902316 seconds time elapsed                   ( +-  1.63% )
```

| | before | after |
|---|---|---|
| time | ~416ms | ~83ms |

About 5x faster, still `6745589558`.

## Conclusion

After fixing cold_column_probe at 2048, `refresh_history` becomes the top hotspot. `perf report --stdio` on the fully optimized 2048 build:

```
    39.66%    35.90%  opt2048_ni  [.] refresh_history(...) [clone .constprop.0]
    25.86%    22.13%  opt2048_ni  [.] cold_column_probe(...) [clone .constprop.0]
```

It's two passes: scattering packet writes across the 33MB array, then a per-row prefix sum. The prefix sum is a serial chain wihch makes it hard to speed up, and the readme says some hotspots aren't worth fixing. So I left it.

`make test` passes (128 -> 6040578838). For 2048 I just changed history_cols and ran it, got 6745589558. Didn't touch any of the build_ functions or their call sites.
