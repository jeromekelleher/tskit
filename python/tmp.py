
import msprime

class LinkedTree(object):
    """
    Straightforward implementation of the quintuply linked tree for developing
    and testing the sample lists feature.

    NOTE: The interface is pretty awkward; it's not intended for anything other
    than testing.
    """
    def __init__(self, tree_sequence):
        self.tree_sequence = tree_sequence
        num_nodes = tree_sequence.num_nodes
        # Quintuply linked tree.
        self.parent = [-1 for _ in range(num_nodes)]
        self.left_sib = [-1 for _ in range(num_nodes)]
        self.right_sib = [-1 for _ in range(num_nodes)]
        self.left_child = [-1 for _ in range(num_nodes)]
        self.right_child = [-1 for _ in range(num_nodes)]
        self.num_samples = [0 for _ in range(num_nodes)]
        for u in tree_sequence.samples():
            self.num_samples[u] = 1

    def __str__(self):
        fmt = "{:<5}{:>8}{:>8}{:>8}{:>8}{:>8}{:>8}\n"
        s = fmt.format(
            "node", "parent", "lsib", "rsib", "lchild", "rchild", "nsamp")
        for u in range(self.tree_sequence.num_nodes):
            s += fmt.format(
                u, self.parent[u],
                self.left_sib[u], self.right_sib[u],
                self.left_child[u], self.right_child[u],
                self.num_samples[u])
        # Strip off trailing newline
        return s[:-1]

    def remove_edge(self, edge):
        p = edge.parent
        c = edge.child
        lsib = self.left_sib[c]
        rsib = self.right_sib[c]
        if lsib == -1:
            self.left_child[p] = rsib
        else:
            self.right_sib[lsib] = rsib
        if rsib == -1:
            self.right_child[p] = lsib
        else:
            self.left_sib[rsib] = lsib
        self.parent[c] = -1
        self.left_sib[c] = -1
        self.right_sib[c] = -1

        u = edge.parent
        while u != -1:
            self.num_samples[u] -= self.num_samples[c]
            u = self.parent[u]

        if self.num_samples[c] > 1 and self.left_child:
            print("IN: Update root for ", c)

    def insert_edge(self, edge):
        p = edge.parent
        c = edge.child
        assert self.parent[c] == -1, "contradictory edges"
        self.parent[c] = p
        u = self.right_child[p]
        if u == -1:
            self.left_child[p] = c
            self.left_sib[c] = -1
            self.right_sib[c] = -1
        else:
            self.right_sib[u] = c
            self.left_sib[c] = u
            self.right_sib[c] = -1
        self.right_child[p] = c

        u = edge.parent
        while u != -1:
            self.num_samples[u] += self.num_samples[c]
            u = self.parent[u]
        if self.num_samples[p] > 1 and self.parent[p] == -1:
            print("IN: Update root for ", p)


    def trees(self):
        """
        Iterate over the the trees in this tree sequence, yielding the (left, right)
        interval tuples. The tree state is maintained internally.
        """
        ts = self.tree_sequence
        sequence_length = ts.sequence_length
        edges = list(ts.edges())
        M = len(edges)
        time = [ts.node(edge.parent).time for edge in edges]
        in_order = sorted(range(M), key=lambda j: (
            edges[j].left, time[j], edges[j].parent, edges[j].child))
        out_order = sorted(range(M), key=lambda j: (
            edges[j].right, -time[j], -edges[j].parent, -edges[j].child))
        j = 0
        k = 0
        left = 0
        left_root = -1

        while j < M or left < sequence_length:
            while k < M and edges[out_order[k]].right == left:
                edge = edges[out_order[k]]
                self.remove_edge(edge)
                k += 1
            while j < M and edges[in_order[j]].left == left:
                edge = edges[in_order[j]]
                self.insert_edge(edge)
                j += 1
            right = sequence_length
            if j < M:
                right = min(right, edges[in_order[j]].left)
            if k < M:
                right = min(right, edges[out_order[k]].right)
            yield left, right, left_root
            left = right




import tests.tsutil as tsutil

ts = msprime.simulate(5, recombination_rate=2, random_seed=1)

ts = tsutil.decapitate(ts, ts.num_edges // 2)

tree = ts.first()
lt = LinkedTree(ts)
for left, right, left_root in lt.trees():
    print(left, right, left_root)
    print(tree.draw_text())
    print(lt)
    print("-" * 10)
    tree.next()

# tree = ts.first()

# print(tree.draw_text())

# x = tree.as_dict_of_dicts()
# print(x)

