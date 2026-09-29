# High Performance Hybrid AI E-Commerce Recommendation Engine

A low-latency, hybrid recommendation engine engineered in modern C++ from scratch. The system combines **User-User Collaborative Filtering**, **Item-Item Co-occurrence Matrixing** and **AI Dense Vector Embeddings** to deliver real time personalized product recommendations with sub-15ms inference latency.

---

## Key Features

* **High-Throughput Ingestion Pipeline:** Ingests and parses **1,000,000+ interaction events** across **90,000 unique users** in under **5.5 seconds**.
* **User-User Collaborative Filtering:** Uses implicit behavioral weighting and inverted index lookups for fast candidate generation.
* **Pre-computed Item-Item Similarity Matrix:** Builds co-occurrence matrices using cosine similarity for $O(1)$ real-time retrieval.
* **AI Vector Embeddings & Semantic Search:** Generates dense product embeddings to solve the **Cold-Start Problem** via nearest-neighbor vector similarity search.
* **Weighted Rank Fusion Engine:** Dynamically fuses behavioral signals ($\alpha = 0.6$) with semantic AI embeddings.
* **Offline Evaluation Framework:** Includes built-in Precision@K and Recall@K benchmark tools.

---

## System Benchmarks

| Metric / Pipeline Step | Value / Performance |
| :--- | :--- |
| **Dataset Ingestion** | 1,000,000 actions loaded in ~5.3s |
| **Matrix Pre-computation** | 4,901 unique items processed in ~19.6s |
| **Item-CF Inference Latency** | **~5.8 ms** per query |
| **AI Vector Search Latency** | **~3.6 ms** per query |
| **Hybrid Rank Fusion Latency** | **~12.5 ms** per query |
| **Recall at 5 (Evaluation)** | **15.00 %** (Leave-one-out validation) |

---

## Building and Running

### Prerequisites

* `g++` (GCC 7+ supporting C++17) or MSVC compiler.

### Build & Execution

```powershell
# Navigate to source directory
cd src

# Compile
g++ main.cpp Engine.cpp -I..\include -o main.exe

# Run
.\main.exe
