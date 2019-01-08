double Ft1(double *Value,  map <string,int>& NumTrans,  map <string,int>& NumPlaces,const vector<string> & NameTrans, const struct InfTr* Trans, const int T) {
/* 
map <string,int>::iterator it= NumPlaces.begin();
while(it!=NumPlaces.end()){
	cout<<it->first<<" "<<it->second<<endl;
	++it;
}
cout<<"\n\n";
it= NumTrans.begin();
while(it!=NumTrans.end()){
         cout<<it->first<<" "<<it->second<<endl;
	 ++it;
}
it=NumPlaces.find("P1");
if (it!=NumPlaces.end())
	cout<<it->first<<" "<<it->second<<endl;
else
	cout<<"Not Found"<<endl;
*/
//return Value[1]*6.0;
cout<<"Firing:"<<NameTrans[T]<<endl;
for (unsigned int k=0; k<Trans[T].InPlaces.size(); k++)//for all variables in the components
		{
		cout<<"Place id: "<< Trans[T].InPlaces[k].Id<<" value:"<<Value[Trans[T].InPlaces[k].Id]<< " edge label:"<<Trans[T].InPlaces[k].Card<<endl;
		}
return Value[NumPlaces["P1"]]*5.0;
}

double Ft0(double *Value,  map <string,int>& NumTrans,  map <string,int>& NumPlaces,const vector<string>& NameTrans, const struct InfTr* Trans, const int T) {
return Value[NumPlaces["P0"]]*5.0;
}
