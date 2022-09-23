#ifndef READ_SUP
	#define READ_SUP
	#include "readSupport.hpp"
#endif

#include "test.hpp"

using namespace CRS;

vector<string> name_file;
vector<vector<double>> time_v;
vector<vector<double>> files;
vector<pair<int, int>> column_time;

int main()
{

	name_file = {"constantList.txt", "constantTable.txt", "constantTimeTable.txt"};

	column_time.reserve(3);
	files.reserve(3);


	double consttt = getConstantFromTable(constantTable_txt, 0, 0);
	cout << consttt << "\n";

	double constt = getConstantFromTable(constantList_txt, 0, 0);
	/*double constt = getConstantFromList(constantList_txt, 4);*/

	cout << constt << "\n";

	double constttt = getConstantFromTimeTable(constantTimeTable_txt, 4, 2);
	cout << constttt << "\n";



}