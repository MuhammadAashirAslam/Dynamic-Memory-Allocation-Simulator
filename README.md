# Memory Allocation Simulator

A C++ simulation of Operating System memory management strategies. This tool visualizes how different algorithms handle memory requests, internal splitting, and external fragmentation.

## 🚀 Features
- **Four core algorithms**: First-Fit, Next-Fit, Best-Fit, and Worst-Fit.
- **Dynamic Management**: 
  - **Splitting**: Carves out exact sizes from larger blocks.
  - **Coalescing**: Merges adjacent free blocks during deallocation.
- **Visual Feedback**: Color-coded tables showing memory state, PIDs, and fragmentation.
- **Stress Test**: Includes a randomized test suite to compare algorithm efficiency.

## 🛠️ How to Use
1. **Compile**:
   ```bash
   g++ simulation.cpp -o memsim
