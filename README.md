# Weak AVL Tree

C++ implementation of Weak AVL trees with two different rank storage strategies.

## Overview

Weak AVL trees are replacements for AVL trees and red-black trees.
A single insertion or deletion operation requires at most two rotations (forming a double rotation when two are needed).
In contrast, AVL deletion requires O(log n) rotations, and red-black deletion requires up to three.

Without deletions, a weak AVL tree is exactly an AVL tree.
With deletions, its height remains at most that of an AVL tree with the same number of insertions but no deletions.

This project provides two implementations that differ in how they store rank information using pointer bit-packing for minimal memory overhead.

## Implementations

### `abs_rank.cc` - Absolute Rank Parity

Stores a single bit representing the parity (even/odd) of the node's absolute rank. The rank difference to a child is computed by comparing parities:
- Same parity → rank difference of 2
- Different parity → rank difference of 1

```cpp
bool rp() const { return par_and_flg & 1; }  // rank parity
void flip() { par_and_flg ^= 1; }            // toggle rank
```

### `child_rank_diff.cc` - Child Rank Differences

Stores two bits representing the rank differences to each child directly:
- Bit 0: rank difference to left child (1 = diff is 2, 0 = diff is 1)
- Bit 1: rank difference to right child

```cpp
bool rd2(int d) const { return par_and_flg & (1 << d); }  // is diff to child d equal to 2?
void flip(int d) { par_and_flg ^= (1 << d); }             // toggle diff to child d
```

## Features

Both implementations support:

| Operation | Time Complexity | Description |
|-----------|-----------------|-------------|
| `insert(Node*)` | O(log n) | Insert a node into the tree |
| `remove(Node*)` | O(log n) | Remove a node from the tree |
| `find(key)` | O(log n) | Find a node by key |
| `min()` | O(log n) | Find the minimum element |
| `rank(key)` | O(log n) | Count elements less than key |
| `select(k)` | O(log n) | Find k-th smallest element (0-indexed) |
| `prev(key)` | O(log n) | Find largest element < key |
| `next(key)` | O(log n) | Find smallest element > key |

### Augmented Data

Each node maintains:
- `size`: subtree size (enables order statistics)
- `sum`: sum of values in subtree

## Write-up

<https://maskray.me/blog/2025-12-14-weak-avl-tree>
