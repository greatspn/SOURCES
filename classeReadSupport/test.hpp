using namespace std;

//i define potrò metterli nel namespace del file.cpp creato da buildode
#define constantList_txt 0
#define constantTable_txt 1
#define constantTimetable_txt 2

//ma da quel che capisco l'extern nel namespace rischia di avere problemi di inizializzazione
//quindi va messo nell'header semplice. Da vedere se usare extern;
extern vector<string> name_file;
extern vector<vector<double>> files; //numero dei file; dovrà essere impostato dinamicamente.



