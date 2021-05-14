//---------------------------------------------------------------------------------------
#ifndef UNION_FIND__H
#define UNION_FIND__H
//---------------------------------------------------------------------------------------

// A union find data structure with union-by-rank optimization
class union_find {
    int *parent = nullptr;
    int *rank = nullptr;
    const size_t sz;
public:
    union_find(size_t _sz) : sz(_sz) {
        parent = new int[sz];
        rank = new int[sz];
    }
    ~union_find() {
        delete[] parent;
        delete[] rank;
    }
    // All elements form a separate set
    void initialize() {
        for (size_t i=0; i<sz; i++) {
            parent[i] = i;
            rank[i] = 0;
        }
    }
    // Find the representative of element x
    int find(int x) {
        if (parent[x] != x)
            parent[x] = find(parent[x]);
        return parent[x];
    }
    // Merge the sets of x and y
    bool merge(int x, int y) {
        int rx = find(x), ry = find(y);
        if (rx == ry)
            return false; // already in the same set
        if (rank[rx] < rank[ry])
            parent[rx] = ry;
        else if (rank[rx] > rank[ry])
            parent[ry] = rx;
        else {
            parent[rx] = ry;
            rank[ry]++;
        }
        return true;
    }
};

//---------------------------------------------------------------------------------------
#endif // UNION_FIND__H