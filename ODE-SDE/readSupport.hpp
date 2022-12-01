namespace CRS {

  using namespace std;


/*!class that define and read from the table written on a file
*/
  class Table
  {
  private: 
    int column ;
    static int class_number;
    int file_index;
    vector<double> file;
    vector<double> time;
  public:
    //! Empty Constructor
    Table(){ file_index = class_number; };
    //!empty copy Constructor
    Table(const Table& t){ file_index = t.class_number; ++class_number; };
    ~Table() {};;
   //!function that extracts the constant from a table written in the file
    double getConstantFromTable(int row, int column);
  /*! get a constant from a table which first column is time; the row is the position
   * of the first value lower than the one passed as parameter
  */
    double getConstantFromTimeTable(double time, int column_index);
  //! get a value from a list of constants in a file 
    double getConstantFromList(int index);
    //!function that read a table from a file, without the column time
    inline void readFileTable(int file_index);
    //!read a table which first column is time
    inline void readFileTimeTable(int file_index);
    //!set the number of column of the table file
    inline void setColumn(int column);

  };
}