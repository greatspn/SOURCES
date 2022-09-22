#ifndef READ_SUP
	#define READ_SUP
	#include "readSupport.hpp"
#endif

#include "test.hpp"

using namespace CRS;

vector<string> name_file;
vector<vector<double>> files;

int main()
{

	name_file = {"constantList.txt", "constantTable.txt"};

	//se si può fare così si può fare tutto dal file creato con build.ode
	files.reserve(3);
	//dove salvo la lunghezza delle righe o il vettore tempo per trovare la riga giusta?
	//aggiungerla all'inizio non mi piace. Però così uso vettori un po' inutili.

	double constt = getConstantFromList(constantList_txt, 4);
	cout << constt << "\n";

	double consttt = getConstantFromTable(constantTable_txt, 2, 2);
	cout << consttt << "\n";
	cout << "ma quindi qui ci arrivi.....?\n";


	return 1;
}