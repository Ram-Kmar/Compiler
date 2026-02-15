# Hy-Compiler

A high-performance, multi-backend compiler for the **Hy** language—a C-like domain-specific language (DSL) featuring unique primitives for compute graph construction. 

Hy-Compiler is designed to demonstrate the full lifecycle of a modern compiler, from manual lexical analysis to native machine code and LLVM IR generation.

## 🚀 Key Features

*   **Classic Pipeline:** Implements a robust Lexer, a recursive-descent Parser producing a rich AST, and a Semantic Analyzer for type checking and scope management.
*   **Dual Backends:** 
    *   **ARM64 Assembly:** Generates native assembly code for Apple Silicon/AArch64.
    *   **LLVM IR:** Interfaces with the LLVM ecosystem for industry-standard optimization and cross-platform support.
*   **Compute Graph Engine:** Features a unique `layer` syntax that automatically builds a directed acyclic graph (DAG), performing topological sorts and exporting to DOT format for visualization.
*   **AST Optimizer:** Includes a dedicated optimization pass currently supporting constant folding and expression simplification via the Visitor pattern.
*   **Automated Testing:** A comprehensive suite of `.hy` programs verified by a Python-based test runner.

## 🏗️ Project Architecture

The compiler is built with modularity in mind, using the **Visitor Pattern** to decouple the AST structure from the various analysis and generation passes:

1.  **Frontend:** `lexer.cpp` (Tokenization) → `parser.cpp` (AST Construction).
2.  **Middle-end:** `semantic_analysis.cpp` (Type & Scope Validation) → `optimizer.cpp` (Constant Folding).
3.  **Backend:** `generation.cpp` (ARM64) | `llvm_generation.cpp` (LLVM IR).
4.  **Graph Analysis:** `compute_graph.cpp` (Topological Sort & Visualization).

## ⚠️ Project Status: Work in Progress

This project is currently an **experimental prototype**. While the core compiler pipeline and language primitives are functional, it is considered "incomplete" as several advanced features are still under development. 

### Current Roadmap
- [ ] **Floating Point Support:** Currently optimized for integer and boolean logic.
- [ ] **Advanced Register Allocation:** Improving the current ARM64 stack-based allocation.
- [ ] **Extended Layer Semantics:** Enhancing the compute graph DSL for more complex neural-like structures.
- [ ] **Standard Library:** Basic I/O is implemented, but a broader runtime library is planned.

## 🛠️ Building & Running

### Prerequisites
- C++17 Compiler (GCC/Clang)
- CMake 3.10+
- (Optional) LLVM & Graphviz (for DOT visualization)

### Build
```bash
mkdir build && cd build
cmake ..
make
```

### Run Tests
```bash
python3 tests/test_runner.py
```

## 📜 Example Hy Code
```c
layer MyLayer {
    // Layer definition
}

int main() {
    int x = 10 + 5 * 2; // Constant folded to 20
    print(x);
    return 0;
}
```

## 📄 License
MIT
