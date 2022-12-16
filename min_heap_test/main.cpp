#include "reverse_heap.h"
#include <iostream>
#include <ostream>



using namespace std;


int main (){

min_heap future_event_list = min_heap();
future_event_list.setHeapSize(10);


Event *event_distribution = new Event(1.6, 2);
future_event_list.pushHeap(event_distribution);

Event* pop = future_event_list.popHeap();
cout << pop -> getTime() << endl;

	
}