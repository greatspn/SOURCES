#include <cstdio>
#include <iostream>
#include <vector>
#include <cassert>
#include <random>

using namespace std;

#include "reverse_heap.h"

std::mt19937_64 generator;//(seed);


int main() {
	
	//reverse_heap reverse_h = reverse_heap(3);
	reverse_heap reverse_h = reverse_heap();

	reverse_h.setHeapSize(5);

	event prov = event(1.0);
	event prov2 = event(3.0);
	event prov3 = event(8.0);
	event prov4 = event(9.695);
	event prov5 = event(0.05);
	/*reverse_h.pushHeap(4,4);
	reverse_h.pushHeap(2, 2);
	reverse_h.pushHeap(3, 3);
	reverse_h.pushHeap(1, 1);*/
	reverse_h.pushHeap(&prov);
	reverse_h.pushHeap(&prov2);
	reverse_h.pushHeap(&prov3);
	reverse_h.pushHeap(&prov4);
	reverse_h.pushHeap(&prov5);

	/*for(int i = 0; i<3; i++){
		event pop = reverse_h.pop_heap();
		cout << pop.getTime() << " VERRÀ?\n";
	}*/

	//cout << prov3.getIndex() << " indice di " << prov3.getTime() << endl;
	//reverse_h.remove_heap(&prov2);

	cout << "Peso del primo elemento " << reverse_h.getWeightOfElem(0) << endl;

	for(int i = 0; i<5; i++){
		event pop = reverse_h.popHeap();
		cout << pop.getTime() << " post rimozione\n";
	}

	//double random = reverse_h.provaGen(0, 1.0, generator);
	//cout << random << endl;
	//std::uniform_real_distribution<double> unf_dis(0, 1);
    //double value = unf_dis(generator);
    //cout << random << endl;

    


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