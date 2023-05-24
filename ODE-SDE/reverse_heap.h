#include <cassert>

using namespace std;


    //!Event
      /*!
   *     Class made to model Event in future Event list
   */

class Event {
    double time;
    //!Index in the min_heap
    int index_heap;
    //!index of the transition
    int index_trans;
    //!pointer to the next element
    Event* next;
    //!Pointer to the previous element
    Event* previous;

public:
    //!Constructor
    Event(double time, int trans){
        this -> time = time;
        this -> index_trans = trans;
        this -> next = NULL;
        this -> previous = NULL;
        this -> index_heap = -1;
    }

    //!Copy Constructor
    Event(const Event& e){ 
        time = e.time; 
        index_trans = e.index_trans;
    };

    //!Deconstruct
    ~Event() {};

    //!get the time in which the event fires
    inline double getTime(){
        return this -> time;
    }

    //! get the index of the event inside the future event list
    inline int getIndexHeap(){
        return this -> index_heap;
    }

    //!get the index of the transition associated to the event
    inline int getIndexTran(){
        return this -> index_trans;
    }

    //!get the next event in the linked list of the transition
    inline Event* getNext(){
        return this -> next;
    }

    inline void setIndexHeap(int index_h){
        this -> index_heap = index_h;
    }

    inline void setTime(double time){
        this -> time = time;
    }



    //!get the preovious event in the linked list of the transition
    inline Event* getPrevious(){
        return this -> previous;
    }

    //!set the next event in the linked list
    inline void setNext(Event* event){
        this -> next = event;
    }

    //!set the previous event in the linked list
    inline void setPrevious(Event* event){
        this -> previous = event;
    }

};


//!Min Heap
    /*!
   *  Heap data structure with reverse index
   */

class min_heap {

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


    //!Empty constructor
    min_heap(){};

    //!Constructor with size
    min_heap(size_t initN) {
        heap.reserve(initN);
    }

    void setHeapSize(size_t sizeN){
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

        delete elem;
        elem = NULL;

    }

    Event* popHeap() {
        Event* elem0 = heap[0];
        return elem0;
    }


    inline double topWeight() {
        return heap[0] -> getTime();
    }

    inline size_t getLenght(){
        return this -> heap.size();
    }

    // change the weight of an element already in the heap
    inline void updateWeight(Event* elem, double w) {
        int i = elem -> getIndexHeap();
        assert(i >= 0 && i < (int)heap.size());
        elem -> setTime(w);
        percolateUpOrDown(elem);
    }


    //delete and clear all heap deallocating objects
    inline void deleteHeap(){
        for(int i=0;i<(int)heap.size();i++){
            Event *delete_event = heap[i];
            delete delete_event;
            heap[i] = NULL;
        }
        heap.clear();
    }
    
};




