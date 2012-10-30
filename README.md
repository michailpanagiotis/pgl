## What is PGL

This is a library of algorithms and data structures. 
Its main goal is to provide data structures optimized for large scale graphs.

The main building block is a dynamic graph structure, called Packed Memory Graph, that combines the static forward star graph structure with dynamic arrays (packed-memory arrays).
It achieves a good cache efficiency during both normal static graph operations, like algorithm queries (which usually operate on a static graph layout), and updates of the layout of the graph.

The library also contains several algorithms for shortest path calculations on networks with both single criterion and multi criteria edge costs. 

## Getting Started

To test the library, you can try the example that is provided. 
The library requires the Boost C++ libraries to be installed.
Make sure you fill in the correct paths in the provided Makefile.
