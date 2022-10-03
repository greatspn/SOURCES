
/*ifndef __IOS_H__
  #define __IOS_H__
  #include <iostream>
#endif

#ifndef __FSTREAM__
  #define __FSTREAM__
  #include <fstream>
#endif

#ifndef __STR__
    #define __STR__
    #include <string>
    #include <sstream>
#endif


#ifndef __VEC__
  #define __VEC__
#include <vector>
#endif*/


namespace CRS {

  using namespace std;

  /*!
  function that extracts the constant from a table written in the file
*/  
  double getConstantFromTable(int file_index, int index1, int index2);
  /*! get a constant from a table which first column is time; the row is the position
   * of the first value lower than the one passed as parameter
  */
  double getConstantFromTimeTable(int file_index, double time, int index);
  /*!
   * get a value from a list of constants in a file 
  */
  double getConstantFromList(int file_index, int index);


  class Table
  {
  public: 
    int column;
    string type;
    vector<double> file;
    vector<double> time;
    //! Empty Constructor
    Table(void){};
    //! Constructor that takes column as parameter
    Table(int column, vector<double> file);
    //!returns the type of the table
    string getTypeTable();
  };

  class TimeTable:virtual public Table
  {
  public:
    TimeTable(int column, vector<double> file, vector<double> time):Table(column, file){type = "time"; this -> time = time;};
  };


    //!Exception
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
