
#ifndef READ_SUP
	#define READ_SUP
	#include "readSupport.hpp"
#endif

#include "test.hpp"

//ovviamente gestire il path ma i define dei file dovranno essere qui
//#include "/home/utente/tesiMagistrale/pnpro/EsempiExpMTDep.hpp"

namespace CRS {


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
				if(rowLenght < 1){
					throw Exception("The table must have one column at least.");
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
				if(rowLenght < 1){
					throw Exception("The table must have two columns at least.");
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

	/*! get a constant from a table which first column is time; the row is the position
	 * of the first value lower than the one passed as parameter
	*/
	double getConstantFromTimeTable(int file_index, double time, int index){

		//!checks if the file has already been written
		if(files[file_index].empty()){
			readFileTimeTable(file_index);
		}

		int column = column_time[file_index].first;
		int time_index = column_time[file_index].second;
		int value_index;

		auto it_up = lower_bound(time_v[time_index].begin(), time_v[time_index].end(), time);
		int lower_time_index = std::distance(time_v[time_index].begin(), it_up);

	  //!if time is lower of the minimun value
		if(lower_time_index == 0)
		{
			value_index = index;
			return files[file_index][value_index];
		}
		//!if time si bigger of the maximum value
		else if (lower_time_index == time_v[time_index].size())
		{
			value_index = (time_v[time_index].size()-1*column) + index;
			return files[file_index][value_index];

		}
		//!middle cases
		else
		{
			value_index = (lower_time_index) + index;
			return files[file_index][value_index];
		}

	}


/*!
  function that extracts the constant from the list written in the file
*/
	double getConstantFromList(int file_index, int index) {

		return getConstantFromTable(file_index, index, 0);
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

		int column = column_time[file_index].first; 

		if(column == 1 && index2 != 0){
			throw Exception("Number of column out of range");
		}


		//!get the right index in the linearized table
		int value_index = (index1*column) + index2; // -1? non -1? dipende da che indice si contano le colonne

		if (value_index <= (int)files[file_index].size()){
			return files[file_index][value_index];
		}
		else {
			throw Exception("Index out of range");
		}


	}

}



