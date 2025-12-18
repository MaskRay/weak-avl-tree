#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <random>
#include <vector>
using namespace std;

struct Node {
  Node *ch[2]{};
  uintptr_t par_and_flg{};
  int i{}, sum{}, size{};

  Node *parent() const { return reinterpret_cast<Node*>(par_and_flg & ~3UL); }
  void set_parent(Node *p) { par_and_flg = (par_and_flg & 3) | reinterpret_cast<uintptr_t>(p); }
  uintptr_t flags() const { return par_and_flg & 3; }
  bool rd2(int d) const { return par_and_flg & (1 << d); }
  void flip(int d) { par_and_flg ^= (1 << d); }
  void clr_flags() { par_and_flg &= ~3UL; }

  void mconcat() {
    sum = i;
    size = 1;
    if (ch[0]) sum += ch[0]->sum, size += ch[0]->size;
    if (ch[1]) sum += ch[1]->sum, size += ch[1]->size;
  }

  bool operator<(const Node &o) const { return i < o.i; }
};

struct WAVL {
  Node *root{};

  ~WAVL() {
    auto destroy = [](auto &self, Node *n) -> void {
      if (!n) return;
      self(self, n->ch[0]);
      self(self, n->ch[1]);
      delete n;
    };
    destroy(destroy, root);
  }

  Node *rotate(Node *x, int d) {
    auto pivot = x->ch[d];
    if ((x->ch[d] = pivot->ch[d^1])) x->ch[d]->set_parent(x);
    pivot->set_parent(x->parent());
    if (!x->parent()) root = pivot;
    else x->parent()->ch[x != x->parent()->ch[0]] = pivot;
    pivot->ch[d^1] = x;
    x->set_parent(pivot);
    x->mconcat();
    return pivot;
  }

  void insert(Node *x) {
    assert(x->ch[0] == nullptr && x->ch[1] == nullptr);
    if (!root) {
      root = x;
      x->mconcat();
      return;
    }
    Node *p = root;
    int d = 0;
    for (;;) {
      d = *p < *x;
      if (!p->ch[d])
        break;
      p = p->ch[d];
    }
    p->ch[d] = x;
    x->par_and_flg = reinterpret_cast<uintptr_t>(p);
    auto *x2 = x;
    if (p->rd2(d)) {
      p->flip(d);
    } else {
      assert(p->rd2(d^1) == 0);
      p->flip(d^1);
      int d1 = d;
      for (x = p, p = x->parent(); p; x = p, p = x->parent()) {
        d = (p->ch[1] == x);
        if (p->rd2(d)) {
          p->flip(d);
          break;
        }
        p->flip(d^1);
        if (!p->rd2(d ^ 1)) {
          if ((d^1) == d1) {
            assert(!x->rd2(d1) && (x->ch[d1] == x2 || x->ch[d1]->flags() == 1 || x->ch[d1]->flags() == 2));
            x->flip(d);
            auto y = rotate(x, d^1); // y is previous x
            if (y->rd2(d))
              x->flip(d^1);
            else if (y->rd2(d^1))
              p->flip(d);
            x = y;
          }
          x = rotate(p, d);
          x->clr_flags();
          break;
        }
        d1 = d;
      }
    }
    for (; x2; x2 = x2->parent()) x2->mconcat();
  }

  void remove(Node *n) {
    Node *y = n;
    if (n->ch[0] && n->ch[1])
      for (y = n->ch[1]; y->ch[0]; y = y->ch[0]);
    Node *p = y->parent(), *x = y->ch[0] ? y->ch[0] : y->ch[1];

    if (p) p->ch[p->ch[1] == y] = x;
    else root = x;
    if (x) x->set_parent(p);

    if (y != n) {
      y->ch[0] = n->ch[0]; y->ch[1] = n->ch[1];
      y->par_and_flg = n->par_and_flg;
      if (n->parent()) n->parent()->ch[n->parent()->ch[1] == n] = y;
      else root = y;
      y->ch[0]->set_parent(y);
      if (y->ch[1]) y->ch[1]->set_parent(y);
      if (p == n) p = y;
    }

    Node *x2 = p;
    if (p) {
      if (p->ch[0] == x && p->ch[1] == x) {
        p->clr_flags();
        x = p;
        p = x->parent();
      }
      while (p) {
        int d2 = (p->ch[1] == x);
        if (!p->rd2(d2)) {
          p->flip(d2);
          break;
        }
        if (p->rd2(d2 ^ 1)) {
          p->flip(d2 ^ 1);
          x = p;
          p = x->parent();
          continue;
        }
        auto sib = p->ch[d2^1];
        if (sib->flags() == 3) {
          sib->clr_flags();
          x = p;
          p = x->parent();
          continue;
        }
        sib->flip(d2^1);
        if (sib->rd2(d2))
          p->flip(d2);
        else if (!sib->rd2(d2^1)) {
          p->flip(d2);
          x = rotate(sib, d2);
          if (x->rd2(d2^1)) sib->flip(d2);
          if (x->rd2(d2)) p->flip(d2^1);
          x->par_and_flg |= 3;
        }
        rotate(p, d2^1);
        break;
      }
    }
    for (; x2; x2 = x2->parent()) x2->mconcat();
  }

  Node *find(int key) const {
    auto tmp = root;
    while (tmp) {
      if (key < tmp->i) tmp = tmp->ch[0];
      else if (key > tmp->i) tmp = tmp->ch[1];
      else return tmp;
    }
    return nullptr;
  }

  Node *min() const {
    Node *p = nullptr;
    for (auto n = root; n; n = n->ch[0]) p = n;
    return p;
  }

  int rank(int key) const {
    int r = 0;
    for (auto n = root; n; ) {
      if (key <= n->i) n = n->ch[0];
      else {
        r += 1 + (n->ch[0] ? n->ch[0]->size : 0);
        n = n->ch[1];
      }
    }
    return r;
  }

  int select(int k) const {
    auto x = root;
    while (x) {
      int lsz = x->ch[0] ? x->ch[0]->size : 0;
      if (k < lsz) x = x->ch[0];
      else if (k == lsz) return x->i;
      else k -= lsz + 1, x = x->ch[1];
    }
    return -1;
  }

  int prev(int key) const {
    int res = -1;
    for (auto x = root; x; )
      if (key <= x->i) x = x->ch[0];
      else { res = x->i; x = x->ch[1]; }
    return res;
  }

  int next(int key) const {
    int res = -1;
    for (auto x = root; x; )
      if (key >= x->i) x = x->ch[1];
      else { res = x->i; x = x->ch[0]; }
    return res;
  }

  static Node *next(Node *x) {
    if (x->ch[1]) {
      x = x->ch[1];
      while (x->ch[0]) x = x->ch[0];
    } else {
      while (x->parent() && x == x->parent()->ch[1]) x = x->parent();
      x = x->parent();
    }
    return x;
  }
};

void print_tree(Node *n, int d = 0) {
  if (!n) return;
  print_tree(n->ch[0], d + 1);
  printf("%*s%d (%d,%d)\n", 2*d, "", n->i, n->rd2(0) ? 2 : 1, n->rd2(1) ? 2 : 1);
  print_tree(n->ch[1], d + 1);
}

int compute_rank(Node *n, bool debug = false) {
  if (!n) return -1;
  int lr = compute_rank(n->ch[0], debug), rr = compute_rank(n->ch[1], debug);
  if (lr < -1 || rr < -1) return -2;
  int rank_l = lr + (n->rd2(0) ? 2 : 1);
  int rank_r = rr + (n->rd2(1) ? 2 : 1);
  if (rank_l != rank_r) {
    if (debug) printf("node %d: rank mismatch left=%d right=%d\n", n->i, rank_l, rank_r);
    return -2;
  }
  if (!n->ch[0] && !n->ch[1] && n->flags() != 0) {
    if (debug) printf("node %d: leaf must be 1,1 but flags=%lu\n", n->i, n->flags());
    return -2;
  }
  int expected_sum = n->i + (n->ch[0] ? n->ch[0]->sum : 0) + (n->ch[1] ? n->ch[1]->sum : 0);
  if (n->sum != expected_sum) {
    if (debug) printf("node %d: sum mismatch got=%d expected=%d\n", n->i, n->sum, expected_sum);
    return -2;
  }
  int expected_size = 1 + (n->ch[0] ? n->ch[0]->size : 0) + (n->ch[1] ? n->ch[1]->size : 0);
  if (n->size != expected_size) {
    if (debug) printf("node %d: size mismatch got=%d expected=%d\n", n->i, n->size, expected_size);
    return -2;
  }
  return rank_l;
}

bool verify_tree(const WAVL &tree, bool verbose = false) {
  int rank = compute_rank(tree.root);
  if (rank < -1) {
    printf("INVALID TREE\n");
    compute_rank(tree.root, true);
    return false;
  }
  if (verbose) printf("Tree verified, rank = %d\n", rank);
  return true;
}

int main() {
  srand(42);
  WAVL tree;
  int i = 0;
  std::vector<int> a(20);
  std::iota(a.begin(), a.end(), 1);
  std::shuffle(a.begin(), a.end(), std::default_random_engine(42));
  for (int val : a) {
    auto n = new Node;
    n->i = val;
    tree.insert(n);
    if (i++ < 6) {
      printf("-- %d After insertion of %d\n", i, val);
      print_tree(tree.root);
    }
  }
  printf("\nSum\tof values = %d\n", tree.root->sum);
  verify_tree(tree, true);

  for (int val : {5, 10, 15}) {
    if (auto found = tree.find(val)) {
      tree.remove(found);
      delete found;
    }
  }
  printf("After removing 5, 10, 15:\n");
  printf("\nSum\tof values = %d\n", tree.root->sum);
  verify_tree(tree, true);

  std::vector<Node*> ref;
  for (auto n = tree.min(); n; n = WAVL::next(n)) ref.push_back(n);

  for (int i = 0; i < 100000; i++) {
    if (ref.size() < 5 || (ref.size() < 1000 && rand() % 2 == 0)) {
      auto n = new Node;
      n->i = rand() % 100000;
      tree.insert(n);
      ref.push_back(n);
    } else {
      int idx = rand() % ref.size();
      tree.remove(ref[idx]);
      delete ref[idx];
      ref[idx] = ref.back();
      ref.pop_back();
    }
    if (i%100 == 0 && !verify_tree(tree)) {
      printf("FAILED at iteration %d\n", i);
      return 1;
    }
  }

  while (!ref.empty()) {
    tree.remove(ref.back());
    delete ref.back();
    ref.pop_back();
    if (tree.root && !verify_tree(tree)) {
      printf("FAILED during final cleanup\n");
      return 1;
    }
  }
  printf("Stress test passed\n");

  // Test rank, select, prev, next
  printf("\nTesting rank/select/prev/next...\n");
  std::vector<int> vals = {10, 20, 30, 40, 50};
  for (int v : vals) {
    auto n = new Node;
    n->i = v;
    tree.insert(n);
  }

  // rank tests (number of elements < key)
  assert(tree.rank(5) == 0);
  assert(tree.rank(10) == 0);
  assert(tree.rank(15) == 1);
  assert(tree.rank(20) == 1);
  assert(tree.rank(25) == 2);
  assert(tree.rank(50) == 4);
  assert(tree.rank(55) == 5);

  // select tests (0-indexed)
  assert(tree.select(0) == 10);
  assert(tree.select(1) == 20);
  assert(tree.select(2) == 30);
  assert(tree.select(3) == 40);
  assert(tree.select(4) == 50);
  assert(tree.select(5) == -1);

  // prev tests (largest < key)
  assert(tree.prev(10) == -1);
  assert(tree.prev(11) == 10);
  assert(tree.prev(20) == 10);
  assert(tree.prev(21) == 20);
  assert(tree.prev(50) == 40);
  assert(tree.prev(55) == 50);

  // next tests (smallest > key)
  assert(tree.next(5) == 10);
  assert(tree.next(10) == 20);
  assert(tree.next(15) == 20);
  assert(tree.next(40) == 50);
  assert(tree.next(50) == -1);
  assert(tree.next(55) == -1);

  printf("rank/select/prev/next tests passed\n");
}
