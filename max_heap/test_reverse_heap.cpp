#include <cstdio>
#include <iostream>
#include <vector>
#include <cassert>

using namespace std;

#include "reverse_heap.h"


int main() {
	
	reverse_heap reverse_h = reverse_heap(5);

	event prov = event(2.0);
	event prov2 = event(3.0);
	event prov3 = event(1.0);
	/*reverse_h.push_heap(4,4);
	reverse_h.push_heap(2, 2);
	reverse_h.push_heap(3, 3);
	reverse_h.push_heap(1, 1);*/
	reverse_h.push_heap(&prov);
	reverse_h.push_heap(&prov2);
	reverse_h.push_heap(&prov3);

	/*for(int i = 0; i<3; i++){
		event pop = reverse_h.pop_heap();
		cout << pop.getTime() << " VERRÀ?\n";
	}*/

	cout << prov3.getIndex() << " indice di " << prov3.getTime() << endl;
	reverse_h.remove_heap(&prov2);

	for(int i = 0; i<2; i++){
		event pop = reverse_h.pop_heap();
		cout << pop.getTime() << " post rimozione\n";
	}


	/*for(int i = 0; i<4; i++){
		double max = reverse_h.pop_heap();
		cout << max << "\n";
	}

	reverse_h.remove_heap(3);

	for(int i = 0; i<4; i++){
		double max = reverse_h.pop_heap();
		cout << max << "\n";
	}*/



}