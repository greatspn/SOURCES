
#ifndef __IOS_H__
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
#endif


namespace CRS {

  using namespace std;

  double getConstantFromList(int file_index, int index);
  double getConstantFromTable(int file_index, int index1, int index2);
  double getConstantFromTimeTable(int file_index, double time, int index);

  //using namespace SDE; //qui dentro sarà definito l'extern vector

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