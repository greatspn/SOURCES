#ifndef READ_SUP
	#define READ_SUP
	#include "readSupport.hpp"
#endif

#include "test.hpp"

vector<string> name_file = {"constantList.txt", "constantTable.txt", "constantTimeTable.txt"};
vector<Table> class_files;
vector<vector<double>> files;
vector<vector<double>> time_v;
vector<pair<int, int>> column_time;

int main()
{

	class_files.reserve(3);
	files.reserve(3);
	column_time.reserve(3);

	/*std::clock_t start;
	double duration;

	start = std::clock();*/


	for(int i=0; i<1000; i++){



		for(int p = 0; p< 30; p++){
			for(int j=0; j<7; j++){
				double consttt = getConstantFromList(constantList_txt, j);
			}
		}

		for(int p = 0; p< 60; p++){
			for(int k=0; k<3; k++){
			double rate = /*5 + */getConstantFromTable(constantTable_txt, k, 3);
			}
		}

		for(int x=0; x<64; x++){
			double constt = getConstantFromTimeTable(constantTimeTable_txt, 3, 3);
		}

	}

/*	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;

	cout<<"printf: "<< duration <<'\n';*/



}