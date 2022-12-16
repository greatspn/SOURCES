#include <cassert>

using namespace std;

    //!Event
      /*!
   *     Class made to model Event in future Event list
   */

class Event {
    double time;
    //!Index in the min_heap
    int index_heap = -1;
    //!index of the transition
    int index_trans = -1;

public:
    //!Constructor
    Event(double time, int trans){
        this -> time = time;
        this -> index_trans = trans;
    }

    //!Copy Constructor
    Event(const Event& e){ 
        time = e.time; 
        index_trans = e.index_trans;
    };

    //!Deconstruct
    ~Event() {};

    inline double getTime(){
        return this -> time;
    }

    inline int getIndexHeap(){
        return this -> index_heap;
    }

    inline void setIndexHeap(int index_h){
        this -> index_heap = index_h;
    }

    inline void setTime(double time){
        this -> time = time;
    }

    inline int getIndexTran(){
        return this -> index_trans;
    }

};


//!Max Heap
    /*!
   *  Heap data structure with reverse index
   */

class min_heap {

    size_t N;
    std::vector<Event*> heap;

    void percolateUp(Event* elem) {
        int i = elem -> getIndexHeap();
        while (i > 0) {
            size_t parent = ((i + 1) / 2) - 1;
            if (heap[parent]-> getTime() < heap[i]-> getTime()){
                break;
            }
            heap[i]->setIndexHeap(parent);
            heap[parent]->setIndexHeap(i);
            swap(heap[i], heap[parent]);
            i = parent;
        }
    }

    void percolateDown(Event* elem) {
        int i = elem -> getIndexHeap();
        while (true) {
            int left = ((i + 1) * 2) - 1;
            int right = left + 1;
            if (right < (int)heap.size() && heap[right]-> getTime() < heap[left]-> getTime() && heap[right]-> getTime() < heap[i]-> getTime())
            {
                cout << "entro nel primo caso" << endl;
                // Move right leaf up
                heap[i]->setIndexHeap(right);
                heap[right]-> setIndexHeap(i);
                swap(heap[i], heap[right]);
                assert(heap[i]-> getTime() <= heap[right]-> getTime() && heap[i] -> getTime() <= heap[left] -> getTime());
                i = right;
            }
            else if (left < (int)heap.size() && heap[left]-> getTime() < heap[i] -> getTime()) {
                // Move left leaf up
                heap[i]-> setIndexHeap(left);
                heap[left]-> setIndexHeap(i);
                swap(heap[i], heap[left]);
                assert(heap[i]-> getTime() <= heap[left]-> getTime());
                i = left;
            }
            else break;
        }
    }

    void percolateUpOrDown(Event* elem) {
        int i = elem -> getIndexHeap();
        size_t parent = ((i + 1) / 2) - 1;
        if (i != 0 && heap[parent] -> getTime() > heap[i] -> getTime())
            percolateUp(elem);
        else
            percolateDown(elem);
    }

public:

    //std::vector<Event*> heap;


        //!Empty constructor
    min_heap(){};

        //!Constructor with size
    min_heap(size_t initN) {
        heap.reserve(N);
    }

    void setHeapSize(size_t sizeN){
        this -> N = sizeN;
        heap.reserve(sizeN);
    }

    void pushHeap(Event* elem) {
        size_t i = heap.size();
        assert(elem-> getIndexHeap() == -1);
        elem -> setIndexHeap(i);
        heap.push_back(elem);
        // push up elem
        percolateUp(elem);
    }

    void removeHeap(Event* elem) {
        int i = elem -> getIndexHeap();
        assert(i >= 0 && i < (int)heap.size());
        // move last element to position i
        if (i == (int)heap.size() - 1) {
            heap.resize(heap.size() - 1);
        }
        else {
            heap[i] = heap.back();
            heap.back() -> setIndexHeap(i);
            heap.resize(heap.size() - 1);
            percolateUpOrDown(elem);
        }
    }

    Event* popHeap() {
        Event* elem0 = heap[0];
        removeHeap(elem0);
        return elem0;
    }


    inline double topWeight() {
        return heap[0] -> getTime();
    }

    inline size_t getLenght(){
        return this -> heap.size();
    }

    // change the weight of an element already in the heap
    void updateWeight(Event* elem, double w) {
        int i = elem -> getIndexHeap();
        assert(i >= 0 && i < (int)heap.size());
        //W[elem] = w;
        elem -> setTime(w);
        percolateUpOrDown(elem);
    }
};




