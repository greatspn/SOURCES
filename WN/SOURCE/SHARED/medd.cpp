#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "medd.h"

extern bool GORDER;
extern map <int,int> OrdPlaces;
extern ifstream fin;
extern ofstream fout;
extern int Max_Token_Bound;
int *a;

static int passo=0;

//-----------------------------------------------------------------------------



/*int FindInFile(stringstream &s1,ifstream &fin)
{
std::string mark; 
int id;
fout.flush();
fin.seekg(0,ios_base::beg);
while (!fin.eof())
	{
	fin >> mark >> id;
	if (mark==s1.str())
		return id;
	}
return -1;
}
*/

bool RSRG::create_RS(const int& npl) 
{
  d = MEDDLY_createDomain();
  int* bounds = (int *) malloc((npl+MAX_ID+1) * sizeof(int*));
  bounds[0]=4;
  for (int i=1;i<MAX_ID+1;i++)
  	{
    	bounds[i]=Max_Token_Bound;
  	}
  for (int i=MAX_ID+1;i<npl+MAX_ID;i++)
	{
	bounds[i]=Max_Token_Bound;
	}
  d->createVariablesBottomUp(bounds, npl+MAX_ID+1);
// Create an MDD forest in this domain (to store states)

//RS creation
  rs = d->createForest(false, forest::BOOLEAN,forest::MULTI_TERMINAL);
  rs->setReductionRule(forest::QUASI_REDUCED); 
  rs->setNodeStorage(forest::FULL_OR_SPARSE_STORAGE);
	//rs->setNodeStorage(forest::SPARSE_STORAGE);
  rs->setNodeDeletion(forest::PESSIMISTIC_DELETION);
  //Initialize the marking
  static dd_edge new1_marking(rs);
  marking=&new1_marking;
  //Initialize the unexplored marking
  static dd_edge un1_marking(rs);
  un_marking=&un1_marking;
  static dd_edge new2_marking(rs);
  new_marking=&new2_marking;
/*
-->Cancellato  provvisoriamente
  //RG creation
  rg = d->createForest(true, forest::REAL,forest::MULTI_TERMINAL);
  // JB: MXDs and MTMXDs do not support FULLY_ or QUASI_REDUCED
  // This will return an error.
  //rg->setReductionRule(forest::FULLY_REDUCED);
  rg->setNodeStorage(forest::FULL_OR_SPARSE_STORAGE);  
  // JB: disabling PESSIMISTIC_DELETION
  rg->setNodeDeletion(forest::PESSIMISTIC_DELETION);
  //Initialize the marking
  static dd_edge new_marking1(rg);
  mTm=&new_marking1;
  free(bounds);
*/

//init expert
expertStates = static_cast<expert_forest*>(rs);
//init expert

//init 
  v = (int **) malloc(1 * sizeof(int*));
  v[0] = (int *) malloc((npl+2+MAX_ID) * sizeof(int));
  k = (int *) malloc((npl+2+MAX_ID) * sizeof(int));
  a = (int *) malloc((npl) * sizeof(int));
//init

//free memory
  free(bounds);
//free memory

  return true;
}

bool RSRG::init_RS(const int& npl,const Net_Mark_p& net_mark,const int&num,int (*pf)(const int&, const Net_Mark_p&))
{
  v[0][0]=0;
  int count=MAX_ID;
  int MASK = 1<<(count-1);
  int conv=num;
  v[0][1]=0;//tangible
  int j=2;
  while(count--)
      	{
	v[0][j]=((conv & MASK ) ? 1 : 0);
#if DEBUG6
	cerr<<((conv & MASK ) ? 1 : 0);
#endif
      	conv <<= 1;
	j++;
      	} 
  for(int pl = 0 ; pl < npl; pl++)
  {/* foreach place */
    if (GORDER)
    {
      if (IS_NEUTRAL(pl))
      {//neutral place
        v[0][pl+j]=net_mark[pl].total;
      }//neutral place
      else
      {//colored
        if (IS_FULL(pl))
        {
          v[0][pl+j]=pf(pl,net_mark);     
        }
        else
        {//empty
          v[0][pl+j]=0;
        }//empty
      }//colored place
    }
    else
    {//colored not implemented
      v[0][pl+j]=net_mark[OrdPlaces[pl]].total;
    }//colored not implemented
  }/* foreach place */
  rs->createEdge(v, 1, *un_marking);


#if DEBUG3
for (int i=0;i<npl+MAX_ID+2;i++)
		{
		cout<<"["<<i<<"]"<<v[0][i]<<" ";
		}
cout<<endl<<endl;

#endif


#if DEBUG
  cout<<"\nInitial Marking\n";
  (*un_marking).show(stdout,true);
#endif
  return true;
}


int** RSRG::en_marking(const int& npl,const Net_Mark_p& net_mark,int (*pf)(const int&, const Net_Mark_p&))
{
 int** z = (int **) malloc(1 * sizeof(int*));
 z[0] = (int *) malloc((npl+2+MAX_ID) * sizeof(int));  
 int j=MAX_ID+2;
 for(int pl = 0 ; pl < npl; pl++)
  {/* foreach place */
    if (GORDER)
      if (IS_NEUTRAL(pl))
      {//neutral place
        z[0][pl+j]=net_mark[pl].total;
      }
      else
      {//colored
        if (IS_FULL(pl))
        {
          z[0][pl+j]=(*pf)(pl,net_mark);  
        }
        else
        {//empty
          z[0][pl+j]=0;
        }//empty
      }//colored place

    else
    {//colored not implemented
      z[0][pl+j]=net_mark[OrdPlaces[pl]].total;
    }//colored not implemented
  }/* foreach place */
 return z;
 }

bool RSRG::add_marking(const int& npl,int**& z,const int& num,const int& type,int (*pf)(const int&, const Net_Mark_p&))
{
 z[0][0]=0;
 int count=MAX_ID;
 int MASK = 1<<(count-1);
 int conv=num;
 z[0][1]=type;
 int j=2;
 while(count--)
	{
	z[0][j]=((conv & MASK ) ? 1 : 0);
#if DEBUG6
	cerr<<((conv & MASK ) ? 1 : 0);
#endif
      	conv <<= 1;
	j++;
      	} 

 //dd_edge new_marking(rs);
 rs->createEdge(z, 1, *new_marking);
 if ((type==3)||(type==1))
 	(*marking)+=(*new_marking);
 else
 	(*un_marking)+=(*new_marking);


#if DEBUG3
 for (int i=0;i<npl+MAX_ID+2;i++)
	{
	cout<<"["<<i<<"]"<<z[0][i]<<" ";
	}
 cout<<endl;
#endif

//free memory
 free(z[0]);
 free(z);
 z=NULL;
 (*new_marking).clear();
//free memory

 return true;
}


bool RSRG::add_marking(const int& npl,const Net_Mark_p& net_mark,const int& num,const int& type,int (*pf)(const int&, const Net_Mark_p&))
{
  v[0][0]=0;
  int count=MAX_ID;
  int MASK = 1<<(count-1);
  int conv=num;
  v[0][1]=type;
  int j=2;
  while(count--)
      	{
	v[0][j]=((conv & MASK ) ? 1 : 0);
#if DEBUG6
	cerr<<((conv & MASK ) ? 1 : 0);
#endif
      	conv <<= 1;
	j++;
      	} 
  for(int pl = 0 ; pl < npl; pl++)
  	{/* foreach place */
    	if (GORDER)
      		if (IS_NEUTRAL(pl))
      			{//neutral place
        		v[0][pl+j]=net_mark[pl].total;
      			}
      		else
      			{//colored
        		if (IS_FULL(pl))
        			{
          			v[0][pl+j]=(*pf)(pl,net_mark);  
        			}
        		else
        			{//empty
          			v[0][pl+j]=0;
        			}//empty
      			}//colored place

        else
    		{//colored not implemented
      		v[0][pl+j]=net_mark[OrdPlaces[pl]].total;
    		}//colored not implemented
  	}/* foreach place */
  //dd_edge new_marking(rs);
  rs->createEdge(v, 1, *new_marking);
  if ((type==3)||(type==1))
 	(*marking)+=(*new_marking);
  else
  	(*un_marking)+=(*new_marking);


#if DEBUG3
 for (int i=0;i<npl+MAX_ID+2;i++)
		{
		cout<<"["<<i<<"]"<<v[0][i]<<" ";
		}
 cout<<endl;
 }
#endif


//free memory
// free(v[0]);
// free(v);
//free memory


#if DEBUG
 cout<<"\nNew Marking\n";
 new_marking.show(stdout,true);
#endif



 (*new_marking).clear();


#if DEBUG
 cout<<"\nAfter union\n";
 (*marking).show(stdout,true);
#endif
 return true;
}




int RSRG::test_marking(const int& npl,const Net_Mark_p& net_mark,const Tree_p& reached_marking,int (*pf)(const int&, const Net_Mark_p&))
{
 // JB: static vector is ugly but I did not want to modify the class
 // definition.
 // static vector<int> k();

 int tro=NEW_MARKING;
 // JB: expand when needed
 // int* k = (int *) malloc((npl+1) * sizeof(int));

 k[0]=0;

 for (int pl=0;pl<npl;pl++)
	{
	if (GORDER)
		{
      		if (IS_NEUTRAL(pl))
      			{//neutral place
        		k[pl]=net_mark[pl].total;
      			}
      		else
      			{//colored
        		if (IS_FULL(pl))
        			{
          			k[pl]=(*pf)(pl,net_mark);     
        			}
        		else
        			{//empty
          			k[pl]=0;
       				}//empty
      			}//colored place

      		//s1<<"*"<<net_mark[pl].total;
    		}
    	else
    		{//colored not implemented
      		k[pl]=net_mark[OrdPlaces[pl]].total;
      		//s1<<"*"<<net_mark[OrdPlaces[pl]].total;
    		}//colored not implemented
  	}
  	//bool tmp=0;
  // JB: make array from vector for evaluate()
  	//rs->evaluate(*marking,kArray,tmp);
  	unsigned int st=0;
  	int type=0;
  	bool test=false;
	//search in the RS
  	testI(marking->getNode(),k,npl+MAX_ID+1,st,expertStates,test,type);
  	// JB: k is static, so do not free it
  	//free memory
 	 //free(k);
	if ((!test)&& (un_marking->getNode()!=0))
		{
		testI(un_marking->getNode(),k,npl+MAX_ID+1,st,expertStates,test,type);
		if (test)
			{
			Tree_p tmp_top=(Tree_p) st;
			st=tmp_top->marking->cont_tang;
			}
		}
 	 //free memory
  	if (test)
  		{
 		switch (type)
			{
			case 0:
				tro = TANGIBLE_OLD;
				reached_marking->marking->cont_tang=st;
			break;
			case 1:
				tro = VANISHING_OLD;
				reached_marking->marking->path=st;
			break;
			case 2:
				tro = VANISHING_LOOP;
			break;
			case 3:
				tro = DEAD_OLD;
				cont_tang=reached_marking->marking->cont_tang=st;
			break;
			default:
				ExceptionIO obj("Error MARKING TYPE\n");
      				throw (obj);
			break;
			}
    
  		}
  return tro;
}



bool RSRG::update_RG(const int& npl, vector<int>& source, vector< vector < int > >& dest,vector < float >& rate)
{


#if DEBUG2
  static int pass=0;
#endif  
  static bool Init=true;
  // JB: more static arrays
  static unsigned int size=0u;
  static int** from = 0;
  static int** to = 0;
  float *  v = &rate[0];  
  
  // JB: grow array as necessary
  if (size < dest.size()) {
    size = dest.size();
    from = (int **) realloc(from, size * sizeof(int*));
    to = (int **) realloc(to, size * sizeof(int*));
  }
#if DEBUG2
  cout<<"\n***********************|"<< pass  <<"|******************************\n";
  pass++;
#endif  
  if (dest.size()==0)
  {//dead  marking
    return false;
  }//dead marking
  // JB: using dest.size() instead of size; since size could be > dest.size()
  // for (int i=0;i<size;i++)
  for (int i=0;i<dest.size();i++)
  {
    //from[i] = (int *) malloc((npl+1) * sizeof(int));
    //to[i] = (int *) malloc((npl+1) * sizeof(int));
    from[i]=&source[0];
    to[i]=&dest[i][0];
    // for(int pl = 0 ; pl < npl; pl++)
    //   {/* foreach place */
    //              from[i][pl+1]=source[pl];
    //              to[i][pl+1]=dest[i][pl];
    //#if DEBUG2
    //              cout<<"S:" << source[pl] << "D:" << dest[i][pl]<< endl;
    //#endif
    //              }/* foreach place */
#if DEBUG2
    cout<< "\tRate:"<<rate[i]<<endl<<"----------------------\n";
#endif
    //v[i]=rate[i];
    //dest[i].clear();
  }
#if RG
  //create the edge representing multiple markings
  dd_edge new_marking(rg);
  //add the new markings in the MDD
  if (Init)
  {
    // JB: again using dest.size() instead of size
    // if (rg->createEdge(from,to,v,size,*mTm)!=forest::SUCCESS)
    if (rg->createEdge(from,to,v,dest.size(),*mTm)!=forest::SUCCESS)
    {
      ExceptionIO obj("Error creating a new RG path\n");
      throw (obj);
    }
    Init=false;
  }
  else
  {
    // JB: again using dest.size() instead of size
    // if (rg->createEdge(from,to,v,size,new_marking)!=forest::SUCCESS)
    if (rg->createEdge(from,to,v,dest.size(),new_marking)!=forest::SUCCESS)
    {
      ExceptionIO obj("Error creating a new RG path!\n");
      throw (obj);
    }
    else
      (*mTm)+=new_marking;
  }
#endif  
  //free memory
  source.clear();
  dest.clear();
  rate.clear();
  /*for (int i=0;i<size;i++)
    {
  //free(from[i]);
  free(to[i]);
  }
  */
  // JB: not freeing from[][] and to[][] since they are static arrays
  //free(from);
  //free(to);
  //free(v);
#if RG
  new_marking.clear();
#endif
  //free memory
  return true;


}


bool Net_Mark_pTovector(const int& npl, const Net_Mark_p& mark, vector <int>& v,int (*pf)(const int&, const Net_Mark_p&))
{
  if (!v.empty())
  {//if it is not empty then I clears all its elements
    v.clear();
  }//if it is not empty then It clears all its elements


  //to create npl+1
  v.push_back(0);
  //to create npl+1
  for(int pl = 0 ; pl < npl; pl++)
  {/* foreach place */
    if (IS_NEUTRAL(pl))
    {//neutral place
      v.push_back(net_mark[pl].total);
    }
    else
    {//colored
      if (IS_FULL(pl))
      {
        v.push_back((*pf)(pl,net_mark));        
      }
      else
      {//empty
        v.push_back(0);
      }//empty
    }//colored place
  }/* foreach place */
#if DEBUG1
  for(int pl = 0 ; pl < npl; pl++)
  {/* foreach place */
    cout<<"v["<<pl<<"]="<<v[pl+1]<<" mark["<<pl<<"]="<<mark[pl].total<<endl;
  }/* foreach place */


#endif


  if (v.size()==npl+1)
    return true;
  else
    return false;
}




int Encoding(const int& pl,const Net_Mark_p& mark )
{
  //static vector<int> num(npl);
  static int num=0;
  //static vector<map <unsigned long long,int> > mapStringtoInt(npl);
  static map <unsigned long long,int> mapStringtoInt;
  Token_p tk_p = NULL;
  unsigned long long encode=0,card=1;
  int cl,sb,lim,obj_num,index;



  //compact(net_mark[pl].different);
  if (mark==NULL)
  {
    return mapStringtoInt.size();
  }


  for(tk_p = mark[pl].marking; tk_p != NULL; tk_p = NEXT_TOKEN(tk_p))
  {/* Per ogni token della marcatura */
    encode=encode+tk_p->molt*card;
    card=card* Max_Token_Bound;;
    for (unsigned int k = 0 ; k < tabp[pl].comp_num; k++)
    {
      cl = tabp[pl].dominio[k];
      encode=encode+tk_p->id[k]*card;
      card=card*tabc[cl].card; 
    }
  }/* Per ogni token della marcatura */




  if (mapStringtoInt.find(encode)!=mapStringtoInt.end())
  {
    return mapStringtoInt[encode];
  }
  else
  {
    mapStringtoInt[encode]=(++num);
    return num;
  }
}




Tree_p RSRG::pop(list <Tree_p>&  UnExMarking)
{
int num=0;
Tree_p tmp;

if (rs->findFirstElement(*un_marking,v[0])==forest::SUCCESS);
else
	{
	ExceptionIO obj("Error MARKING IN UNEXPLORED MARKING SET\n");
      	throw (obj);
	}




//copy v in a
for (int i=MAX_ID+2;i<MAX_ID+2+npl;i++)
	{
	a[i-(MAX_ID+2)]=v[0][i];
	}
//copy v in a


for (int i=2;i<=MAX_ID+1;i++)
	{
	num=num+v[0][i]*(int)pow(2,MAX_ID+1-i);
	}
bool found=false;
list <Tree_p>::iterator iter=UnExMarking.begin();
int steps=0;
while ((iter!=UnExMarking.end())&&(!found))
	{
	if ((*iter)->marking->cont_tang==num)
		found=true;
	else
	    	iter++;
	steps++;
	}
if (found)
	tmp=*iter;
else
	{
	ExceptionIO obj("Error MARKING IN UNEXPLORED MARKING SET\n");
      	throw (obj);
	}
cout<<"\tP"<<UnExMarking.size()<<endl;
UnExMarking.erase(iter);
cout<<UnExMarking.size()<<"\t\tX"<<steps<<endl;

//remove marking 
//dd_edge new_marking(rs);
rs->createEdge(v, 1, *new_marking);
(*un_marking)-=(*new_marking);
(*marking)+=(*new_marking);

  (*new_marking).clear();

return tmp;
}



Tree_p RSRG::pop(Tree_p*  top,Tree_p*  botton)
{
int num=0;
Tree_p  tmp_top=*top,tmp_pr=NULL;

if (rs->findFirstElement(*un_marking,v[0])==forest::SUCCESS);
else
	{
	ExceptionIO obj("Error MARKING IN UNEXPLORED MARKING SET\n");
      	throw (obj);
	}




//copy v in a
for (int i=MAX_ID+2;i<MAX_ID+2+npl;i++)
	{
	a[i-(MAX_ID+2)]=v[0][i];
	}
//copy v in a


for (int i=2;i<=MAX_ID+1;i++)
	{
	num=num+v[0][i]*(int)pow(2,MAX_ID+1-i);
	}
bool found=false;

tmp_top=(Tree_p)num;
/*
while ((tmp_top!=NULL)&&(!found))
	{
	if (tmp_top->marking->cont_tang==num)
		found=true;
	else
		{
		tmp_pr=tmp_top;
	    	tmp_top=tmp_top->last;
		}
	steps++;
	}
if (!found)
	{
	ExceptionIO obj("Error MARKING IN UNEXPLORED MARKING SET\n");
      	throw (obj);
	}
else
	{
	if (tmp_pr==NULL)
		*top=tmp_top->last;
	else
		tmp_pr->last=tmp_top->last;
	}
	if (tmp_top->last==NULL)
		{
		*botton=tmp_pr;
		}
*/
//remove marking 
//dd_edge new_marking(rs);
rs->createEdge(v, 1, *new_marking);
(*un_marking)-=(*new_marking);
(*new_marking).clear();
//update id 
 int count=MAX_ID;
  int MASK = 1<<(count-1);
  int conv=tmp_top->marking->cont_tang;
int j=2;
  while(count--)
      	{
	v[0][j]=((conv & MASK ) ? 1 : 0);
#if DEBUG6
	cerr<<((conv & MASK ) ? 1 : 0);
#endif
      	conv <<= 1;
	j++;
      	}
//update id 
rs->createEdge(v, 1, *new_marking);
(*marking)+=(*new_marking);

(*new_marking).clear();

return tmp_top;
}


void testI(int F,int *v,int i,unsigned int& st,expert_forest* expertStates,bool& tro,int& type)
{
INFONODE init_info(F,i);
list <class INFONODE> qNode;

qNode.push_back(init_info);
tro=false;
while(!qNode.empty())
	{
	//update node
	//cout<<"\t"<<qNode.size()<<endl;
	qNode.begin()->get(F,i,st);
	qNode.pop_front();
	//update node
	if (i>MAX_ID+1)
		{
		if (expertStates->isFullNode(F))
			{
			if ((expertStates->getFullNodeSize(F)>=v[i-MAX_ID-2]+1)&&
			    (expertStates->getFullNodeDownPtr(F,v[i-MAX_ID-2])!=0))
				{
				init_info.set(expertStates->getFullNodeDownPtr(F,v[i-MAX_ID-2]),i-1);
				qNode.push_back(init_info);
				}
			}
		else
			{
			for (int z=0; z<expertStates->getSparseNodeSize(F);z++)
				{
				if ((expertStates->getSparseNodeIndex(F,z)==v[i-MAX_ID-2]))
					{
					init_info.set(expertStates->getSparseNodeDownPtr(F,z),i-1);
					qNode.push_back(init_info);
					}
				
				}
			
			}	
		}

	else
		{
		if (i>0)
			{
			if (expertStates->isFullNode(F))
				{
				for (int j=0;j<expertStates->getFullNodeSize(F);j++)
					{
					if ((expertStates->getFullNodeDownPtr(F,j)!=0))
						{
						if (i==1)
							type=j;
						else
						//conversion from bin to dec
							st=st|j<<-(i-MAX_ID-1);
						//conversion from bin to dec
						init_info.set(expertStates->getFullNodeDownPtr(F,j),i-1,st);
						qNode.push_back(init_info);
						}
					}
				}
			
			else
				{
				for (int j=0; j<expertStates->getSparseNodeSize(F);j++)
					{
					if (i==1)
							type=expertStates->getSparseNodeIndex(F,j);
						else
			 		//conversion from bin to dec
					st=st|expertStates->getSparseNodeIndex(F,j)<<-(i-MAX_ID-1);
// 					//conversion from bin to dec
					init_info.set(expertStates->getSparseNodeDownPtr(F,j),i-1,st);
					qNode.push_back(init_info);
					}
				}

			}
		else
			{
			if (F==-1)
					{
					tro=true;
					}
			
				}
			}
		}
	}

