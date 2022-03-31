//---------------------------------------------------------------------------------------
#ifndef __REVERSE_HEAP_H__
#define __REVERSE_HEAP_H__
//---------------------------------------------------------------------------------------

// Heap data structure with reverse index
class reverse_heap {
    const size_t N;
    std::vector<double> W;  // weights
    // contains the (up to N) inserted elements
    std::vector<size_t> heap;
    // index of the N-th element in the heap
    std::vector<ssize_t> rev_heap; // reverse heap

    void percolate_up(int i) {
        while (i > 0) {
            size_t parent = ((i + 1) / 2) - 1;
            if (W[heap[parent]] > W[heap[i]])
                break;
            rev_heap[ heap[i] ] = parent;
            rev_heap[ heap[parent] ] = i;
            swap(heap[i], heap[parent]);
            i = parent;
            // verify();
        }
        // verify(true);
    }

    void percolate_down(int i) {
        while (true) {
            int left = ((i + 1) * 2) - 1;
            int right = left + 1;
            // cout << "("<<i<<"-"<<left<<"-"<<right<<")"<<flush;
            if (right < heap.size() && W[heap[right]] > W[heap[left]] && W[heap[right]] > W[heap[i]])
            {
                // Move right leaf up
                rev_heap[ heap[i] ] = right;
                rev_heap[ heap[right] ] = i;
                swap(heap[i], heap[right]);
                assert(W[heap[i]] >= W[heap[right]] && W[heap[i]] >= W[heap[left]]);
                // cout << "["<<i<<","<<right<<"] " << flush;
                i = right;
                // verify();
            }
            else if (left < heap.size() && W[heap[left]] > W[heap[i]]) {
                // Move left leaf up
                rev_heap[ heap[i] ] = left;
                rev_heap[ heap[left] ] = i;
                swap(heap[i], heap[left]);
                assert(W[heap[i]] >= W[heap[left]]);
                // cout << "["<<i<<","<<left<<"] " << flush;
                i = left;
                // verify();
            }
            else break;
        }
        // cout << endl;
        // verify(true);
    }

    void percolate_up_or_down(int i) {
        size_t parent = ((i + 1) / 2) - 1;
        if (i != 0 && W[heap[parent]] < W[heap[i]])
            percolate_up(i);
        else
            percolate_down(i);
    }

    // inline void verify(bool check_weights = false) {
    //     for (int i=0; i<heap.size(); i++) {
    //         assert(rev_heap[heap[i]] == i);
    //         if (check_weights) {
    //             size_t parent = ((i + 1) / 2) - 1;
    //             assert(i==0 || W[heap[i]] <= W[heap[parent]]);
    //         }
    //     }
    //     for (int i=0; i<rev_heap.size(); i++) {
    //         assert(rev_heap[i] == -1 || heap[ rev_heap[i] ] == i);
    //     }
    // }

public:
    reverse_heap(size_t initN) : N(initN) {
        heap.reserve(N);
        W.resize(N, -1.0);
        rev_heap.resize(N, -1);
    }

    void push_heap(int elem, double w) {
        // cout << "push_heap("<<elem<<", "<<w<<")"<<endl;
        size_t i = heap.size();
        assert(rev_heap[elem] == -1);
        rev_heap[elem] = i;
        heap.push_back(elem);
        W[elem] = w;
        // verify();
        // push up i
        percolate_up(i);
    }

    void remove_heap(int elem) {
        int i = rev_heap[elem];
        // cout << "remove_heap("<<elem<<"@"<<i<<"): ";
        assert(i >= 0 && i < heap.size());
        rev_heap[elem] = -1;
        W[elem] = -1.0;
        // move last element to position i
        if (i == heap.size() - 1) {
            heap.resize(heap.size() - 1);
            // verify();
        }
        else {
            heap[i] = heap.back();
            rev_heap[heap.back()] = i;
            heap.resize(heap.size() - 1);
            // verify();
            // push down the inserted element
            percolate_up_or_down(i);
        }
        // cout << endl;
    }

    int pop_heap() {
        int elem0 = heap[0];
        remove_heap(elem0);
        return elem0;
    }

    inline double get_weight(int elem) {
    	return W[elem];
    }

    inline double top_weight() {
        return W[heap[0]];
    }

    // change the weight of an element already in the heap
    void update_weight(int elem, double w) {
        int i = rev_heap[elem];
        assert(i >= 0 && i < heap.size());
        W[elem] = w;
        percolate_up_or_down(i);
    }
};

//---------------------------------------------------------------------------------------
#endif // __REVERSE_HEAP_H__