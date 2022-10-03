#include <utility>
#include <cstdio>
#include <ctime>
#include <algorithm>

using namespace std;
using namespace CRS;


//i define potrò metterli nel namespace del file.cpp creato da buildode
#define constantList_txt 0
#define constantTable_txt 1
#define constantTimeTable_txt 2


extern vector<string> name_file;
extern vector<Table> class_files;
extern vector<vector<double>> files;
extern vector<vector<double>> time_v;
extern vector<pair<int, int>> column_time;

