#include "class.hpp"


namespace CRS {

	using namespace SDE;
	int Table::class_number = 0;


/*!
  function that read a table from a file, without the column time
*/
	inline void Table::readFileTable(int file_index){


		ifstream file_written (name_file[file_index]);
	//!memorize the lenght of the row for right value extraction
		int columnLenght = 0;
		bool init = false;
		if(file_written.is_open())
		{
			string line;
		//!to read the single line                   
			while (getline(file_written, line))
			{
				//!to check that each rows is the same leght
				int columnControll = 0;
			//!to read more elements separated by whitespace
				stringstream ss(line); 
				string token;
				while (getline(ss, token, ',')) 
			//!to read the single elements of the line
				{   
					try{
						file.push_back(stod(token));
					}
					catch(std::invalid_argument const& ex){
						cout << "There's an invalid argument" + name_file[file_index] + " (the separator should be comma)\n";
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
			file_written.close();

			setColumn(columnLenght);

		}
		else
		{
			throw Exception("The file could not be opened");
		}
	}

/*!read a table which first column is time
*/
	void Table::readFileTimeTable(int file_index){


		ifstream file_written (name_file[file_index]);
	//!memorize the lenght of the row for right value extraction
		int columnLenght = 0;
		bool init = false;
		if(file_written.is_open())
		{
			string line;
		//!to read the single line                   
			while (getline(file_written, line))
			{
			//!to  controll that each row is the same lenght
				int columnControll = 0;
			//!to read more elements separated by whitespace or comma or semicolon
				stringstream ss(line); 
				string token;
				while (getline(ss, token, ',')) 
			//!to read the single elements of the line
				{   
					try{
						//!To save the time in a vector separated from other values
						if(columnControll == 0){
							time.push_back(stod(token));
						}
						else{
							file.push_back(stod(token));
						}
					}
					catch(std::invalid_argument const& ex){
						cout << "There's an invalid argument" + name_file[file_index] + " (the separator should be the comma)\n";
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
			file_written.close();
			
		}
		else
		{
			throw Exception("The file could not be opened");
		}

	}

	/*! get a constant from a table which first column is time; the row is the position
	 * of the first value lower than the one passed as parameter
	*/
	double Table::getConstantFromTimeTable(double time_value, int column_index){

		//!checks if the file has already been written
		if(file.empty()){
			readFileTimeTable(file_index);
		}


		auto it_up = lower_bound(time.begin(), time.end(), time_value);
		int lower_time_index = std::distance(time.begin(), it_up);
		lower_time_index--;

		if(column_index > column){
			throw Exception("Index out of range.\n");
		}


	  //!if time is lower of the minimun value
		if(lower_time_index == 0)
		{	
			return file[column_index];
		}
		//!if time si bigger of the maximum value
		else if (lower_time_index == (int)time.size())
		{
			int value_index = ((time.size()-1)*column) + column_index;
			return file[value_index];
		}
		//!middle cases
		else
		{
			int value_index = (lower_time_index*column) + column_index;
			return file[value_index];
		}
	}


/*!
  function that extracts the constant from a list written in the file
*/
	double Table::getConstantFromList(int column_index) {

			return getConstantFromTable(column_index, 0);
	}	

/*!
  function that extracts the constant from a table written in the file
*/	

	double Table::getConstantFromTable(int row_index, int column_index){


		//!check if the file has already been written
		if(file.empty()){
			readFileTable(file_index);
		}


		if(column_index > column){
			throw Exception("Index out of range");
		}

		//!get the right index in the linearized table
		int value_index = (row_index*column) + column_index;

		if (value_index < (int)file.size()){
			return file[value_index];
		}
		else {
			throw Exception("Index out of range");
		}
	}

	inline void Table::setColumn(int column){
		this -> column = column;
	}

	Table::~Table() {};


}