### 1. Comparison table

| # | Program | Collectives used | Full array on every process? | Manual sum loop on root? | Where is the final result? |
|---|---|---|---|---|---|
| 1 | `sum_bcast.c` | `MPI_Bcast` + Send/Recv | **Yes**, all hold 1,000,000 | Yes (Recv loop) | Root only |
| 2 | `sum_scatter.c` | `MPI_Scatter` + Send/Recv | No, only root; others hold a chunk | Yes (Recv loop) | Root only |
| 3 | `sum_gather.c` | `MPI_Scatter`, `MPI_Gather` | No | Yes (loop over `all_sums`) | Root only |
| 4 | `sum_reduce.c` | `MPI_Scatter`, `MPI_Reduce` | No | **No** | Root only |
| 5 | `sum_allreduce.c` | `MPI_Scatter`, `MPI_Allreduce` | No | No | **All** processes (same value) |
| 6 | `sum_scan.c` | `MPI_Scatter`, `MPI_Scan` | No | No | **Different on each rank** (prefix); the last rank holds the total |

### 2. Run with 2, 4 and 8 processes


```bash
for np in 2 4 8; do
  for p in Exercise01/sum_bcast Exercise02/sum_scatter Exercise03/sum_gather \
           Exercise04/sum_reduce Exercise05/sum_allreduce Exercise06/sum_scan; do
    echo -n "np=$np $p : "
    mpirun --oversubscribe -np $np ./$p | grep "Time"
  done
done
```

**My measured times** (seconds, median of 5 runs, on a **2-core** machine, so 4 and 8 processes share 2 cores).

| Program | np = 2 | np = 4 | np = 8 |
|---|---|---|---|
| sum_bcast | 0.0029 | 0.0102 | 0.0165 |
| sum_scatter | 0.0029 | 0.0034 | 0.0054 |
| sum_gather | 0.0032 | 0.0047 | 0.0045 |
| sum_reduce | 0.0036 | 0.0042 | 0.0045 |
| sum_allreduce | 0.0031 | 0.0033 | 0.0035 |
| sum_scan | 0.0056 | 0.0061 | 0.0078 |

**Which is fastest and why** :

- **Bcast (Ex1) is the slowest, and it gets worse as you add processes** (0.0029 s at 2 processes, 0.0165 s at 8). Root sends the **whole 4 MB array to every process**, so total data = P × 4 MB, even though each process only uses 1/P of it.
- **All the Scatter versions (Ex2–Ex6) are faster.** Each process gets only its chunk, so the total data sent is always 4 MB, whatever P is.
- **Among Ex2–Ex6 the times are very close.** After the Scatter, each process sends only **one** `long long` (8 bytes). That's tiny next to moving the array, so the choice of collection method barely shows at this size.
- **Reduce and Allreduce are the best design.** They combine values in a tree (log P steps) instead of root receiving P−1 messages one by one. The difference grows with many processes or many machines.
- **Scan is a little slower.** Each rank's answer depends on all ranks before it, so there's more coordination.
- On a 2-core machine, 4 and 8 processes **share** cores, so you won't see real speed-up beyond 2. Adding numbers is also very cheap, so communication cost dominates. That's why times don't drop as P grows.

### 3. Thinking question — when choose `MPI_Scan` over `MPI_Allreduce`?

Choose **Scan** when each process needs the total of the processes **before it**, not the grand total.

**Concrete example:** each process scans its share of bank transactions and finds the suspicious ones (say rank 0 finds 3, rank 1 finds 5, rank 2 finds 2). All results must go into **one output array/file, in order**. Rank *r* must know **where to start writing**, which is the number found by ranks 0…r−1:

```
counts:        R0 = 3    R1 = 5    R2 = 2
MPI_Scan:      R0 = 3    R1 = 8    R2 = 10
offset = scan - count:  R0 = 0    R1 = 3    R2 = 8     <- each one's start position
```

Allreduce would give everyone **10**, which tells nobody where to write. Other uses: giving unique global IDs and running balances across chunks.

---
