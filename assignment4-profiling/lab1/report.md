# Intro Profiling Lab Report

## 1. Optimizations Made

- Replaced the raw `new[]` arrays in `shortest_path_bfs` with `vector<int>` and `vector<unsigned char>`. That fixes the leak from the early return.
- Swapped the congestion-pressure loops to row-major order so the code accesses vector in a cache-friendly way.
- Built the binary with `-Ofast` so the compiler can make aggressive optimizations for the final benchmark build.

## 2. Methodology Walkthrough

I started with `time` and `perf stat` to see whether the program was mostly CPU-bound or whether memory behavior was worth looking at. The first obvious issue was the leak in `shortest_path_bfs`, and the second was the cache-unfriendly loop order in `compute_congestion_pressure`.

Before the changes, the normal run was:

```text
grid = 260 x 260
open_cells = 51260
requests = 1200
reachable = 1177
unreachable = 23
average_distance = 180.575
route_label_checksum = 3703473789245134517
heatmap_total_visits = 32914184
heatmap_active_cells = 51041
heatmap_max_visits = 957
heatmap_threshold_checksum = 17645577948039157950
congestion_passes = 4096
congestion_total_pressure = 3719781
congestion_max_pressure = 175
congestion_pressure_checksum = 5595025244828244209
time_sec = 1.58407
```

The before `perf stat` output had `390722434 cache-misses` and `90001887 branch-misses`, which made cache locality look like the better first target.

After the changes, the normal run was:

```text
grid = 260 x 260
open_cells = 51260
requests = 1200
reachable = 1177
unreachable = 23
average_distance = 180.575
route_label_checksum = 3703473789245134517
heatmap_total_visits = 32914184
heatmap_active_cells = 51041
heatmap_max_visits = 957
heatmap_threshold_checksum = 17645577948039157950
congestion_passes = 4096
congestion_total_pressure = 3719781
congestion_max_pressure = 175
congestion_pressure_checksum = 5595025244828244209
time_sec = 1.37878
```

The after `perf stat` output had `6443241 cache-misses` and `86634641 branch-misses`, and the runtime dropped from about `1.58407 s` to about `1.37878 s`.

The final `-Ofast` build that I would hand in produced:

```text
grid = 260 x 260
open_cells = 51260
requests = 1200
reachable = 1177
unreachable = 23
average_distance = 180.575
route_label_checksum = 3703473789245134517
heatmap_total_visits = 32914184
heatmap_active_cells = 51041
heatmap_max_visits = 957
heatmap_threshold_checksum = 17645577948039157950
congestion_passes = 4096
congestion_total_pressure = 3719781
congestion_max_pressure = 175
congestion_pressure_checksum = 5595025244828244209
time_sec = 1.23944
```

The final `perf stat` numbers for that same build were:

- `2989548034 cycles`
- `6973934542 instructions`
- `750214246 branches`
- `47998856 branch-misses`
- `334022193 cache-references`
- `6001780 cache-misses`
- `2769727777 L1-dcache-loads`
- `245340351 L1-dcache-load-misses`

I also checked the heavier tools after the fix:

- FlameGraph: the biggest stacks were still BFS and congestion, but the congestion phase was easier to justify after the loop-order change.
- Callgrind/KCachegrind: `shortest_path_bfs`, `compute_congestion_pressure`, and `next_pressure_value` were the main hotspots on the smaller workload.
- Valgrind: the leak summary now reports `0 bytes in 0 blocks` in use at exit on the reduced workload.

## 3. Correctness Evidence

`make test`:

```text
sanity check passed
```

Final normal run output:

```text
grid = 260 x 260
open_cells = 51260
requests = 1200
reachable = 1177
unreachable = 23
average_distance = 180.575
route_label_checksum = 3703473789245134517
heatmap_total_visits = 32914184
heatmap_active_cells = 51041
heatmap_max_visits = 957
heatmap_threshold_checksum = 17645577948039157950
congestion_passes = 4096
congestion_total_pressure = 3719781
congestion_max_pressure = 175
congestion_pressure_checksum = 5595025244828244209
time_sec = 1.23944
```

All the checksums matched before and after optimization

## 4. Conceptual Questions

Answer Q1.1 through Q6.1 from the README.

A1.1 
user + sys is the amount of time spent by the cpu which is for me 99% of "real" which is the total time it took for the command to finish
`taskset -c 0 ./grid_bfs  1.41s user 0.17s system 99% cpu 1.585 total`

A2.1 
perf counts these events when the program is running, and these values are derived from those measured events. 
- insn per cycle = instructions / cycles 
- % of all branches = branch misses / branches * 100 
- % of all cache refs = cache misses / cache refs * 100 

A2.2 
That is the amount of samples / total samples percentage that perf has used to measure in the time that the program was running. Since perf has to measure multiple events, it samples different events at different times. (multiplexing)

A2.3 
It is not the exact number since it is scaled from the sampled version of whatever perf could sample. But it is near-accurate to the exact number of cache misses. 

A3.1 
Frame pointer is the pointer that points to the base of the stack stored in rbp.
perf -g uses this chain of saved rbp's to rebuild the tack frame. 
Usually we compile with -fno-mit-frame-pointer so that the call stack data is clean and isn't optimized away. 

A3.2 
Self cost is time spent directly inside a function’s own instructions.
Inclusive cost is self cost + time spent in everything else that the function calls.

A4.1 
gprof instruments the program at compile time (similar to Tracy Profiler)
At runtime the profiling hook records function call graphs. When the program is compiled with `-pg`, extra calls are added near function entry. Those hooks count which functions call which other functions, so `gprof` can show call counts and caller/callee relationships. It also uses timer samples to estimate where time was spent.

A4.2
`gprof` gives a different view than `perf`. `perf` and FlameGraphs are better for real runtime hotspots, but `gprof` makes function call counts and caller/callee relationships easier to see. In this lab that helps compare tools, the only disadvantage being, -pg flag adding extra overhead to the binary which makes it less accurate for benchmarks. 

A5.1
Valgrind Memcheck runs the program under an instrumentation tool and checks memory operations while the program runs. It is slow, but it can find invalid reads/writes, uninitialized values, leaks, and bad frees without needing a special sanitizer build. AddressSanitizer is compiled into the program with flags like `-fsanitize=address`. It is way faster than Valgrind and is better for development but it requires rebuilding the program. I'd use ASan first for quick debugging, then Valgrind when I want slower but detailed memory checking or when I cannot rebuild with sanitizers.

A6.1
The tools mostly agreed on the main idea: BFS and the congestion-pressure code were the important parts of the program. The differences were not real contradictions. `perf` measured native execution, FlameGraphs visualized the same sampled stacks, and `gprof` used an instrumented build. Since the tools measure in different ways, their percentages and runtimes do not match exactly, but they pointed to the same general hotspots.
