
#ifndef READ_SUP
	#define READ_SUP
	#include "readSupport.hpp"
#endif

#include "test.hpp"

//ovviamente gestire il path ma i define dei file dovranno essere qui
//#include "/home/utente/tesiMagistrale/pnpro/EsempiExpMTDep.hpp"

namespace CRS {

/*!
  function that read a constants list from a file
*/

	void readFileList(int file_index){

		vector<double> constants;
		ifstream f (name_file[file_index]);
		string line;
		if(f.is_open())
		{
			while (getline(f,line))
			{
				try{
					constants.push_back(stod(line));
				}
				catch(std::invalid_argument const& ex){
					cout << "There's an invalid argument" + name_file[file_index] + "\n";
				}
			}
			f.close();
			auto itPos = files.begin() + file_index;
			files.insert(itPos, constants);
		}
		else
		{
			throw Exception("The file could not be opened");
		}
	}

/*!
  function that read a table from a file, without the column time
*/
	int readFileTable(int file_index){


		ifstream file (name_file[file_index]);
		vector<double> linearizedMatrix;
	//!memorize the lenght of the row for right value extraction
		int rowLenght = 0;
		bool init = false;
		if(file.is_open())
		{
			string line;
		//!to read the single line                   
			while (getline(file, line))
			{
				//!to check that each rows is the same leght
				int rowControll = 0;
			//!to read more elements separated by whitespace
				stringstream ss(line); 
				string token;
				while (getline(ss, token, ' ') || getline(ss, token, ',')  || getline(ss, token, ';') ) 
			//!to read the single elements of the line
				{   
					try{
						linearizedMatrix.push_back(stod(token));
					}
					catch(std::invalid_argument const& ex){
						cout << "There's an invalid argument" + name_file[file_index] + " (the separator should be the blankspace)\n";
					}
					if(!init){
						rowLenght++;
					}
					rowControll++;
				}
				init = true;
				if(rowLenght != rowControll){
					throw Exception("The table must have rows of the same lenght.\n");
				}
			}
			file.close();

			//!insert at the beginning the the vector the lenght of the rows.
			auto itLM = files.begin() + file_index;
			files.insert(itLM, linearizedMatrix);


			//auto itLenght = additionalInformation.begin() + file_index;
			//vector<int> lenght = {rowLenght};
			//additionalInformation.insert(itLenght, lenght);

		}
		else
		{
			throw Exception("The file could not be opened");
		}

		return rowLenght;



	}

/*!read a table which first column is time
*/
/*	int readFileTimeTable(int file_index){

		ifstream file (name_file[file_index]);
		vector<double> linearizedMatrix;
		vector<double> time_v;
	//!memorize the lenght of the row for right value extraction
		int rowLenght = 0;
		bool init = false;
		if(file.is_open())
		{
			string line;
		//!to read the single line                   
			while (getline(file, line))
			{
			//!to  controll that each row is the same lenght
				int rowControll = 0;
			//!to read more elements separated by whitespace
				stringstream ss(line); 
				string token;
				while (getline(ss, token, ' ') || getline(ss, token, ',')  || getline(ss, token, ';') ) 
			//!to read the single elements of the line
				{   
					try{
						//!To save the time in a vector separated from other values
						if(rowControll == 0){
							time_v.push_back(stod(token));
						}
						else{
							linearizedMatrix.push_back(stod(token));
						}
					}
					catch(std::invalid_argument const& ex){
					//controllare se posso generalizzare il separatore con , ; e spazio
						cout << "There's an invalid argument" + name_file[file_index] + " (the separator should be the blankspace)\n";
					}
					if(!init){
						rowLenght++;
					}
					rowControll++;
				}
				init = true;
				if(rowLenght != rowControll){
					throw Exception("The table must have rows of the same lenght.\n");
				}
			}
			file.close();

			auto itPos = files.begin() + file_index;
			files.insert(itPos, linearizedMatrix);


			//auto itLenght = additionalInformation.begin() + file_index;
			//additionalInformation.insert(itLenght, time_v);
		}
		else
		{
			throw Exception("The file could not be opened");
		}

		return rowLenght;

	}


	double getConstantFromTimeTable(int file_index, double time, int index){

		int row;
		//!checks if the file has already been written
		if(files[file_index].empty()){
			readFileTimeTable(file_index);
		}


		//decltype(time_v)::iterator it_up = upper_bound(time_v.begin(), time_v.end(), time);
	//cout<<"Upper bound of " << time << " is: ";
	//cout << *it_up << "\n";
		int upper_time_index = std::distance(time_v.begin(), it_up);
	//cout << "indice " << upper_time_index << "\n";

		if(upper_time_index == 0)
		{
			inf_rate = Infection_rate[0][1];
		}
		else if (upper_time_index == time_v.size())
		{
			inf_rate = Infection_rate[time_v.size()-1][1];
		//cout << "time size " << time_v.size() << "\n";

		}
		else
		{
		//qui ho solo due colonne... immagino che nel caso dovrò salvarmi anche a cosa faccio riferimento e quindi quale colonna
			inf_rate = Infection_rate[upper_time_index-1][1];	
		}

		return 0.1;





	}*/


/*!
  function that extracts the constant from the list written in the file
*/
	double getConstantFromList(int file_index, int index) {


	//!checks if the file has already been written
		if(files[file_index].empty()){
			readFileList(file_index);
		}

		if (index <= (int)files[constantList_txt].size()){
			return files[file_index][index];
		}
		else {
			throw Exception("Index out of range");
		}
	}	

/*!
  function that extracts the constant from a table written in the file
*/	

	double getConstantFromTable(int file_index, int index1, int index2){

		int row;
		//!checks if the file has already been written
		if(files[file_index].empty()){
			row = readFileTable(file_index);
		}

		// row = additionalInformation[file_index][0];

		//!get the right index in the linearized table
		int value_index = (index1*row) + index2 - 1;

		if (value_index <= (int)files[file_index].size()){
			return files[file_index][value_index];
		}
		else {
			throw Exception("Index out of range");
		}


	}

}



