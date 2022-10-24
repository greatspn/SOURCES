#include <utility>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <iostream>
#include <sstream>
#include "readSupport.hpp"

#define constantList_txt 0
#define constantTable_txt 1
#define constantTimeTable_txt 2

using namespace std;
using namespace CRS;

vector<string> name_file = {"constantList.txt","constantTable.txt","constantTimeTable.txt"};
vector<Table> class_files(3, Table());;

int main()
{

	/*std::clock_t start;
	double duration;

	start = std::clock();*/

	//for(int i = 0; i< 4; i++){
	double aaa = class_files[constantList_txt].getConstantFromList(3);


	
	cout << aaa <<"\n";
	//}
	
	//double constt = class_files[constantTimeTable_txt].getConstantFromTimeTable(constantTimeTable_txt, 3, 4);
	//cout << constt <<"\n";*/








/*	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;

	cout<<"printf: "<< duration <<'\n';*/



}
