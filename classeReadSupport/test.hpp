#include <utility>
#include <algorithm>

using namespace std;

//i define potrò metterli nel namespace del file.cpp creato da buildode
#define constantList_txt 0
#define constantTable_txt 1
#define constantTimeTable_txt 2


extern vector<string> name_file;
extern vector<vector<double>> files; //numero dei file; dovrà essere impostato dinamicamente.
extern vector <pair<int, int>> column_time;
extern vector<vector<double>> time_v;


