How to define generic fluid function in ODE
===========================================

#From GUI
You have to use General transition and type FN:<name_function>

The default input parameters of the function are:
(double *Value, map <string,int>& NumTrans, map <string,int>& NumPlaces, const vector <string>& NameTrans, const struct InfTr* Trans,  const int Tran, const double& Time) 

The default output parameter is
double 


#From BASH
The function code (in C/C++ language) must be passed at PN2ODE.sh with option -C

Es.
 PN2ODE.sh ~/TMP/Prova -M  -C ~/tmp.cpp


Where tmp.cpp is:

double rateT0(double *Value,  map <string,int>& NumTrans,  map <string,int>& NumPlaces,const vector<string>& NameTrans, const struct InfTr* Trans, const int T,const double& Time) {
return Value[NumPlaces["P0"]]*1.0;
}




How to define generic discrete  function in ODE
===============================================


#From GUI
You have to use General transition and type:
	1. FN:D:<double_value> for exponential discrete transition with rate <double_value>
	2. FN:D:<name_function> for exponential discrete transition with generic rate

