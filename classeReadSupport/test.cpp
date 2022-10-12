#include "test.hpp"

vector<string> name_file = {"constantList.txt", "constantTable.txt", "constantTimeTable.txt"};
vector<Table> class_files(3, Table());

int main()
{

	/*std::clock_t start;
	double duration;

	start = std::clock();*/

	//for(int i = 0; i< 4; i++){
	double consttt = class_files[constantTable_txt].getConstantFromTable(3, 3);
	cout << consttt <<"\n";
	//}
	
	//double constt = class_files[constantTimeTable_txt].getConstantFromTimeTable(constantTimeTable_txt, 3, 4);
	//cout << constt <<"\n";





/*	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;

	cout<<"printf: "<< duration <<'\n';*/



}
