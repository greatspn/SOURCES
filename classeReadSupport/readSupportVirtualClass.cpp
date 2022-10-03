
#ifndef READ_SUP
	#define READ_SUP
	#include "/home/utente/tesiMagistrale/pnpro/readSupport.hpp"
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
		int columnLenght = 0;
		bool init = false;
		if(file.is_open())
		{
			string line;
		//!to read the single line                   
			while (getline(file, line))
			{
				//!to check that each rows is the same leght
				int columnControll = 0;
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
						columnLenght++;
					}
					columnControll++;
				}
				init = true;
				if(columnLenght != columnControll){
					throw Exception("The table must have rows of the same lenght.\n");
				}
				if(columnLenght < 1){
					throw Exception("The table must have one column at least.");
				}
			}
			file.close();
		}
		else
		{
			throw Exception("The file could not be opened");
		}


		Table tmp = Table(columnLenght, linearizedMatrix);
		class_files[file_index] = tmp;


	}

/*!read a table which first column is time
*/
	void readFileTimeTable(int file_index){

		ifstream file (name_file[file_index]);
		vector<double> linearizedMatrix;
		vector<double> time_tmp;
	//!memorize the lenght of the row for right value extraction
		int columnLenght = 0;
		bool init = false;
		if(file.is_open())
		{
			string line;
		//!to read the single line                   
			while (getline(file, line))
			{
			//!to  controll that each row is the same lenght
				int columnControll = 0;
			//!to read more elements separated by whitespace or comma or semicolon
				stringstream ss(line); 
				string token;
				while (getline(ss, token, ' ') || getline(ss, token, ',')  || getline(ss, token, ';')) 
			//!to read the single elements of the line
				{   
					try{
						//!To save the time in a vector separated from other values
						if(columnControll == 0){
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
						columnLenght++;
					}
					columnControll++;
				}
				init = true;
				if(columnLenght != columnControll){
					throw Exception("The table must have rows of the same lenght.\n");
				}
				if(columnLenght < 1){
					throw Exception("The table must have two columns at least.");
				}
			}
			file.close();

			TimeTable tmp = TimeTable(columnLenght-1, linearizedMatrix, time_tmp);
			class_files[file_index] = tmp;
			
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
		if(class_files[file_index].file.empty()){
			readFileTimeTable(file_index);
		}

		vector<double> tab_file = class_files[file_index].file;
		int column = class_files[file_index].column;
		vector<double> time_v = class_files[file_index].time;

		auto it_up = lower_bound(time_v.begin(), time_v.end(), time);
		int lower_time_index = std::distance(time_v.begin(), it_up);
		lower_time_index--;

	  //!if time is lower of the minimun value
		if(lower_time_index == 0)
		{
			return tab_file[index];
		}
		//!if time si bigger of the maximum value
		else if (lower_time_index == time_v.size())
		{
			int value_index = ((time_v.size()-1)*column) + index;
			return tab_file[value_index];
		}
		//!middle cases
		else
		{
			int value_index = (lower_time_index*column) + index;
			return tab_file[value_index];
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

		//!checks if the file has already been written

		if(class_files[file_index].file.empty()){
			readFileTable(file_index);
		}

		vector<double> tab_file = class_files[file_index].file; 
		int column = class_files[file_index].column;

		if(column == 1 && index2 != 0){
			throw Exception("Number of column out of range");
		}

		//!get the right index in the linearized table
		int value_index = (index1*column) + index2; // -1? non -1? dipende da che indice si contano le colonne

		if (value_index <= (int)tab_file.size()){
			return tab_file[value_index];
		}
		else {
			throw Exception("Index out of range");
		}
	}

	Table::Table(int column, vector<double> new_file){

		this -> column = column;
		this -> file = new_file;
		type = "classic";
	}


}



