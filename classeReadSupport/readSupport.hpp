#include <utility>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

namespace CRS {

  double getConstantFrom(string name_file, int method, int row_index, int column_index);


/*!class that define and read from the table written on a file
*/
  class Table
  {
  private: 
    int column ;
    string file_name;
    vector<double> file;
    vector<double> time;
  public:
    //! Empty Constructor
    Table(){};
    //!constructor with file name
    Table(string file_name);
    //!empty copy Constructor
    //Table(const Table& t){};
    //virtual ~Table();
   /*!
  function that extracts the constant from a table written in the file
  */  
    double getConstantFromTable(int row_index, int column_index);
  /*! get a constant from a table which first column is time; the row is the position
   * of the first value lower than the one passed as parameter
  */
    double getConstantFromTimeTable(double time, int column_index);
  /*!
   * get a value from a list of constants in a file 
  */
    double getConstantFromList(int row_index);
    /* set the number of column of the table file
    */
    //!read a number table from the file
    inline void readFileTable();
    //!read a time table from the file
    inline void readFileTimeTable();
    //!set the column number
    inline void setColumn(int column);
  };

    /*!Exception
    */
  struct Exception{
    std::string mess;
  public:
        //! Empty constructors
    Exception() {mess="";};
        //! constructor takes in input a message
    Exception(std::string mess) {this->mess=mess;};
        //! \name Get of methods use to access at the data structures
        //@{
        //!It returns message
    std::string what(void) {return mess;};
        //@}
  };

}
