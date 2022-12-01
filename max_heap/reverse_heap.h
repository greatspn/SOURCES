//---------------------------------------------------------------------------------------
#ifndef __REVERSE_HEAP_H__
#define __REVERSE_HEAP_H__
//---------------------------------------------------------------------------------------

class event {
    double time;
    int index = -1;
public:
    event(double time){
        this -> time = time;
    }

    inline double getTime(){
        return this -> time;
    }

    inline int getIndex(){
        return this -> index;
    }

    inline void setIndex(int index){
        this -> index = index;
    }

    inline void setTime(double time){
        this -> time = time;
    }

};


// Heap data structure with reverse index
class reverse_heap {

    size_t N;
    std::vector<event*> heap;

    void percolateUp(event* elem) {
        int i = elem -> getIndex();
        while (i > 0) {
            size_t parent = ((i + 1) / 2) - 1;
            if (heap[parent]-> getTime() > heap[i]-> getTime())
                break;
            heap[i]->setIndex(parent);
            heap[parent]->setIndex(i);
            swap(heap[i], heap[parent]);
            i = parent;
        }
    }


    void percolateDown(event* elem) {
        int i = elem -> getIndex();
        while (true) {
            int left = ((i + 1) * 2) - 1;
            int right = left + 1;
            // cout << "("<<i<<"-"<<left<<"-"<<right<<")"<<flush;
            if (right < heap.size() && heap[right]-> getTime() > heap[left]-> getTime() && heap[right]-> getTime() > heap[i]-> getTime())
            {
                // Move right leaf up
                heap[i]->setIndex(right);
                heap[right]-> setIndex(i);
                swap(heap[i], heap[right]);
                assert(heap[i]-> getTime() >= heap[right]-> getTime() && heap[i] -> getTime() >= heap[left] -> getTime());
                // cout << "["<<i<<","<<right<<"] " << flush;
                i = right;
            }
            else if (left < heap.size() && heap[left]-> getTime() > heap[i] -> getTime()) {
                // Move left leaf up
                heap[i]-> setIndex(left);
                heap[left]-> setIndex(i);
                swap(heap[i], heap[left]);
                assert(heap[i]-> getTime() >= heap[left]-> getTime());
                // cout << "["<<i<<","<<left<<"] " << flush;
                i = left;
            }
            else break;
        }
    }

    void percolateUpOrDown(event* elem) {
        int i = elem -> getIndex();
        size_t parent = ((i + 1) / 2) - 1;
        if (i != 0 && heap[parent] -> getTime() < heap[i] -> getTime())
            percolateUp(elem);
        else
            percolateDown(elem);
    }

public:
    reverse_heap() {};

    reverse_heap(size_t initN) {
        heap.reserve(N);
    }

    void setHeapSize(size_t sizeN){
        this -> N = sizeN;
        heap.reserve(sizeN);
    }




    void pushHeap(event* elem) {
        size_t i = heap.size();
        assert(elem-> getIndex() == -1);
        elem -> setIndex(i);
        heap.push_back(elem);
        // push up elem
        percolateUp(elem);
    }

    void removeHeap(event* elem) {
        int i = elem -> getIndex();
        assert(i >= 0 && i < heap.size());
        // move last element to position i
        if (i == heap.size() - 1) {
            heap.resize(heap.size() - 1);
        }
        else {
            heap[i] = heap.back();
            heap.back() -> setIndex(i);
            heap.resize(heap.size() - 1);
            percolateUpOrDown(elem);
        }
        // cout << endl;
    }

    event popHeap() {
        event* elem0 = heap[0];
        removeHeap(elem0);
        return *elem0;
    }

    inline double getWeightOfElem(int index) {
        event* elem = heap[index];
        return elem -> getTime();
    }

    inline double topWeight() {
        return heap[0] -> getTime();
    }

    // change the weight of an element already in the heap
    void updateWeight(event* elem, double w) {
        int i = elem -> getIndex();
        assert(i >= 0 && i < heap.size());
        //W[elem] = w;
        elem -> setTime(w);
        percolateUpOrDown(elem);
    }
};





//---------------------------------------------------------------------------------------
#endif // __REVERSE_HEAP_H__