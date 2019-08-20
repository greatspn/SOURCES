#include <iostream>
#include <iomanip>
#include <vector>
#include <cassert>
#include <cstdlib>
using namespace std;
#include "../../../NSRC/DSPN-Tool/sparsevector.h"

#define DLS_TEST
#include "dlcrs_matrix.h"

template class dlcrs_mat::doubly_linked_crs_matrix<int>;

//---------------------------------------------------------------------------------------

int main(int argc, char** argv) {
	srand(1);
	const size_t R=10, C=6;
	int mat[R][C];
	for (size_t i=0; i<R; i++)
		for (size_t j=0; j<C; j++)
			mat[i][j] = 0;

	typedef dlcrs_mat::doubly_linked_crs_matrix<int> matrix_t;
	matrix_t dlmat(R, C);
	dlmat.print();
	cout << "\n" << endl;
	dlmat.verify_integrity();

	for (size_t n=0; n<1000; n++) {
		switch (rand() % 3) {
			case 0: {
				matrix_t::row_vector_type row(R);
				for (size_t c=0, num=(2+rand())%C; c<num; c++) {
					row.add_element(rand()%C, 1+rand()%3);
				}
				dlmat.replace_row(rand()%R, row);
				break;
			}

			case 1: {
				std::vector<size_t> perm(C);
				for (size_t j=0; j<C; j++)
					perm[j] = j;
				for (size_t j=0; j<C; j++)
					std::swap(perm[rand()%C], perm[rand()%C]);
				dlmat.sort_columns(perm);
				break;
			}

			case 2: {
				size_t num = rand()%(C/2);
				size_t j = rand()%(C - num), s1, s2;
				// cout << "slide j="<<j<<" num="<<num<<endl;
				dlmat.slide_backward(j, num, s1, s2);
				break;
			}
		}
		dlmat.print();
		cout << "\n" << endl;
		dlmat.verify_integrity();
	}
}


//---------------------------------------------------------------------------------------






