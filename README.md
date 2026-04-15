# FlashForge: SSD Flash Translation Layer (FTL) Simulator

## Overview

**FlashForge** is a high-fidelity SSD firmware simulator that models the behavior of NAND flash memory and implements a Flash Translation Layer (FTL) to manage logical-to-physical address mapping, garbage collection, and wear leveling.

This project is designed to replicate the core challenges faced in real SSD firmware:

* No in-place writes (erase-before-write constraint)
* Limited block endurance
* Fragmentation and garbage collection overhead
* Performance tradeoffs under realistic workloads

---

## Key Features

### NAND Flash Simulation

* Block/page hierarchy
* Strict erase-before-write enforcement
* Page states: FREE, VALID, INVALID
* Block-level erase operations

### Flash Translation Layer (FTL)

* Page-level logical-to-physical (L2P) mapping
* O(1) address translation via in-memory table
* Reverse mapping for efficient garbage collection

### Garbage Collection (GC)

* Greedy GC policy (selects block with most invalid pages)
* Valid page migration
* Block reclamation and reuse
* GC-triggered under low free space conditions

### Wear Tracking

* Per-block erase counters
* Wear distribution statistics (mean, min/max, stddev)
* Foundation for wear leveling strategies

### Workload Simulation

* Sequential fill phase
* Hot/Cold workload:

  * 80% of accesses target 20% of logical space (hot data)
* Mixed operation distribution:

  * 70% writes
  * 25% reads
  * 5% trims

### Metrics & Observability

* Write Amplification (WA)
* GC frequency and overhead
* Flash vs host operations
* Wear distribution analysis

---

## System Configuration

| Parameter         | Value        |
| ----------------- | ------------ |
| Pages per block   | 64           |
| Total blocks      | 256          |
| Physical capacity | 16,384 pages |
| Logical pages     | 12,288 pages |
| Overprovisioning  | 25%          |

---

## Simulation Results

```
Host Writes: 82378
Host Reads:  24883
Flash Writes: 216623

GC Events: 3243 (134245 pages moved)
Erase Ops: 3243

Write Amplification (WA): 2.63

Wear Mean: 12.67 erases
Wear Min/Max: 3 / 26
Wear StdDev: 4.31
```

---

## Interpretation

### Write Amplification (WA ≈ 2.63)

* Indicates significant internal data movement due to garbage collection
* Expected under heavy random + hot data workloads
* Demonstrates realistic SSD behavior under pressure

---

### Garbage Collection Behavior

* 3,243 GC cycles triggered
* ~134k pages relocated
* Confirms:

  * Active space reclamation
  * High GC overhead under skewed workloads

---

### Wear Distribution

* Mean: ~12.7 erases
* Range: 3 → 26 erases
* StdDev: 4.3

Insights:

* Wear is not perfectly uniform (expected without advanced wear leveling)
* Hot data causes localized stress on certain blocks
* System still avoids catastrophic imbalance

---

## Architecture

```
Host Operations
      ↓
 FTL Engine
  ├── L2P Mapping Table
  ├── Free Block Manager
  ├── Garbage Collector
      ↓
 Flash Model
  ├── Blocks
  ├── Pages
  ├── Erase Counters
```

---

## Workload Design

The simulator uses a **two-phase workload**:

### Phase 1: Sequential Fill

* Fills all logical pages
* Establishes baseline mapping

### Phase 2: Hot/Cold Mixed Workload

* 80% of operations target 20% of address space
* Simulates real-world skewed access patterns
* Introduces fragmentation and GC pressure

---

## 🛠️ How to Run

### Build

```bash
g++ -std=c++17 src/*.cpp -o ftl_sim
```

### Run

```bash
./ftl_sim
```

---

## Design Highlights

### Strict Separation of Concerns

* `FlashModel` → hardware constraints only
* `FtlEngine` → firmware logic
* `MetricsCollector` → observability

### Realistic Constraints

* No in-place overwrite
* Block-level erase only
* Reverse mapping for GC

### Defensive Engineering

* Bounds checking
* Exception handling for invalid states
* Reproducible simulation (fixed RNG seed)

---

## Future Improvements

* Advanced GC policies (cost-benefit, temperature-aware)
* Dynamic wear leveling
* Latency modeling (p50 / p99)
* SSD caching layers (SLC write buffer)
* Visualization dashboard (heatmaps, timelines)
* Trace-driven workload replay

---

## Why This Project Matters

This simulator demonstrates:

* Systems-level thinking
* Understanding of storage hardware constraints
* Tradeoff analysis (performance vs endurance)
* Ability to design and measure complex systems

It mirrors real challenges in SSD firmware development, particularly:

* Garbage collection efficiency
* Write amplification reduction
* Wear balancing under skewed workloads
