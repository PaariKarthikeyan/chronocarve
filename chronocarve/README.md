# ChronoCarve: Content-Aware Image Reframing Engine

ChronoCarve is a high-performance, end-to-end C++20 and Python engine that reframes landscape images (16:9, 4:3, wide photos) into vertical stories / portraits (9:16), square feeds (1:1), or portrait posts (4:5) using content-aware seam carving. It benchmarks four distinct algorithmic paradigms on the same image input and provides an embedded REST API alongside a dark-themed glassmorphic web dashboard with live seam overlays and energy heatmaps.

---

## 1. Algorithmic Paradigms for Image Seam Carving

| # | Algorithm | Complexity | Description |
|---|---|---|---|
| **1** | **Greedy Baseline** | $\mathcal{O}(H)$ | Greedily chooses local minimum energy neighbor row-by-row. Extremely fast baseline, but prone to seam collisions and spatial distortions. |
| **2** | **Backward-Energy DP** | $\mathcal{O}(W \cdot H)$ | Classic Avidan & Shamir (2007) Bellman recurrence using Sobel $L_1$ gradient magnitude energy. Globally optimal for gradient energy. |
| **3** | **Forward-Energy DP** | $\mathcal{O}(W \cdot H)$ | Rubinstein, Shamir & Avidan (2008) formulation measuring energy *inserted* into the retargeted image ($C_U, C_L, C_R$). Prevents artificial edge artifacts. |
| **4** | **2D Graph Cut Seam Carving** | Polynomial (Max-Flow) | Boykov–Kolmogorov Min-Cut on a 2D lattice. Finds globally optimal monotonic cut boundaries using max-flow network duality. |

---

## 2. Mathematical Formulations

### A. Avidan & Shamir Backward Energy
$$e_1(x, y) = \left|\frac{\partial I}{\partial x}\right| + \left|\frac{\partial I}{\partial y}\right|$$
Recurrence:
$$M(i, j) = e(i, j) + \min(M(i-1, j-1), M(i-1, j), M(i-1, j+1))$$

### B. Rubinstein Forward Energy
When column $j$ is removed at row $i$, new neighbors are created:
$$C_U(i, j) = |I(i, j+1) - I(i, j-1)|$$
$$C_L(i, j) = C_U(i, j) + |I(i-1, j) - I(i, j-1)|$$
$$C_R(i, j) = C_U(i, j) + |I(i-1, j) - I(i, j+1)|$$

Recurrence:
$$M(i, j) = \min \begin{cases} M(i-1, j-1) + C_L(i, j) \\ M(i-1, j) + C_U(i, j) \\ M(i-1, j+1) + C_R(i, j) \end{cases}$$

### C. 2D Graph Cut Seam Carving
Directed graph $\mathcal{G} = (\mathcal{V}, \mathcal{E})$ with pixels $(i, j)$ as nodes:
- Leftmost column nodes connect to Source ($S$) with capacity $\infty$.
- Rightmost column nodes connect to Sink ($T$) with capacity $\infty$.
- Horizontal forward edges $(i, j) \to (i, j+1)$ carry capacity $e(i, j)$.
- Diagonal edges $(i, j) \to (i \pm 1, j-1)$ have capacity $\infty$ to enforce 8-connected seam monotonicity ($|s(i) - s(i-1)| \le 1$).

---

## 3. Directory Layout

```
chronocarve/
├── CMakeLists.txt                 # Modern C++20 build definition
├── README.md                      # Architecture & manual
├── run_server.py                  # High-performance Python REST Server & Carver
├── third_party/
│   ├── httplib.h                  # Embedded cpp-httplib header
│   ├── json.hpp                   # Embedded nlohmann/json header
│   └── maxflow/                   # Boykov-Kolmogorov Max-Flow solver
├── include/
│   ├── common.hpp                 # Enums, configs, benchmarks, metrics
│   ├── energy.hpp                 # Gradient & forward energy maps + heatmaps
│   ├── image_io.hpp               # Image I/O & synthetic landscape generator
│   ├── carver_base.hpp            # CarverBase interface & seam removal
│   ├── greedy_carver.hpp          # Algorithm 1
│   ├── backward_dp_carver.hpp     # Algorithm 2
│   ├── forward_dp_carver.hpp      # Algorithm 3
│   ├── graph_cut_carver.hpp       # Algorithm 4
│   └── benchmark.hpp              # Throughput, latency & memory tracking
├── src/
│   ├── energy.cpp
│   ├── image_io.cpp
│   ├── greedy_carver.cpp
│   ├── backward_dp_carver.cpp
│   ├── forward_dp_carver.cpp
│   ├── graph_cut_carver.cpp
│   ├── benchmark.cpp
│   ├── server.cpp                 # Embedded HTTP server & REST API
│   └── main.cpp                   # Application CLI entrypoint
└── web/
    ├── index.html                 # Glassmorphic dark UI dashboard
    ├── css/
    │   └── styles.css             # Glassmorphism styling & animations
    └── js/
        ├── app.js                 # Dual comparison, seam overlay & heatmap logic
        └── charts.js              # Chart.js latency and throughput visualizers
```

---

## 4. Setup & Running

Start the server:
```bash
python chronocarve/run_server.py
```
Open **`http://localhost:8080/`** in your browser.
Select an aspect ratio preset (**9:16 Vertical Story**, **1:1 Square**, or **4:5 Portrait**), choose your algorithmic paradigm, and click **Reframe Image**!
