
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

			//!updates the column_time structure; for the list is not important
			files[file_index] = constants;
			pair<int, int> ct_pair;
			ct_pair.first = 1;
			ct_pair.second = 0;

			column_time[file_index] = ct_pair;

		}
		else
		{
			throw Exception("The file could not be opened");
		}
	}

/*!
  function that read a table from a file, without the column time
*/
	void readFileTable(int file_index){


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

			//!save the list in the files structure
			files[file_index] = linearizedMatrix;

		}
		else
		{
			throw Exception("The file could not be opened");
		}

		//!update column_time with the right number of column; no time associated.
		pair<int, int> ct_pair;
		ct_pair.first = rowLenght;
		ct_pair.second = 0;

		column_time[file_index] = ct_pair;

	}

/*!read a table which first column is time
 * molto simile a readTable; conviene unirli e differenziare con un flag?
*/
	void readFileTimeTable(int file_index){

		ifstream file (name_file[file_index]);
		vector<double> linearizedMatrix;
		vector<double> time_tmp;
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
			//!to read more elements separated by whitespace or comma or semicolon
				stringstream ss(line); 
				string token;
				while (getline(ss, token, ' ') || getline(ss, token, ',')  || getline(ss, token, ';')) 
			//!to read the single elements of the line
				{   
					try{
						//!To save the time in a vector separated from other values
						if(rowControll == 0){
							time_tmp.push_back(stod(token));
						}
						else{
							linearizedMatrix.push_back(stod(token));
						}
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

			files[file_index] = linearizedMatrix;

			//!update column_time; the effective table has one less column, because there's not the time. 
			//! the time vector is inserted in time_v and the position saved in column time.
			pair<int, int> ct_pair;
			ct_pair.first = rowLenght - 1;
			time_v.push_back(time_tmp);
			ct_pair.second = time_v.size() - 1;

			column_time[file_index] = ct_pair;
			
		}
		else
		{
			throw Exception("The file could not be opened");
		}

	}


	double getConstantFromTimeTable(int file_index, double time, int index){

		//!checks if the file has already been written
		if(files[file_index].empty()){
			readFileTimeTable(file_index);
		}

		int row = column_time[file_index].first;
		int time_index = column_time[file_index].second;
		int value_index;

		//da capire se si vuole il valore precedente o il valore successivo al time che viene passato
		auto it_up = upper_bound(time_v[time_index].begin(), time_v[time_index].end(), time);
		int upper_time_index = std::distance(time_v[time_index].begin(), it_up);
		//upper_time_index--;

	  //!if time is lower of the minimun value
		if(upper_time_index == 0)
		{
			value_index = index;
			return files[file_index][value_index];
		}
		//!if time si bigger of the maximum value
		else if (upper_time_index == time_v[time_index].size())
		{
			value_index = (time_v[time_index].size()-1*row) + index;
			cout << value_index << " mezzo\n";
			return files[file_index][value_index];

		}
		//!middle cases
		else
		{
			value_index = (upper_time_index*row) + index;
			return files[file_index][value_index];
		}

	}


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

		//int row;
		//!checks if the file has already been written
		if(files[file_index].empty()){
			readFileTable(file_index);
		}

		int row = column_time[file_index].first; 

		//!get the right index in the linearized table
		int value_index = (index1*row) + index2 -1; // -1? non -1? dipende da che indice si contano le colonne

		if (value_index <= (int)files[file_index].size()){
			return files[file_index][value_index];
		}
		else {
			throw Exception("Index out of range");
		}


	}

}



