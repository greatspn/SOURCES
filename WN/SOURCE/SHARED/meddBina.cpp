#include "medd.h"

extern bool GORDER;
extern map <int,int> OrdPlaces;
extern ifstream fin;
extern ofstream fout;
extern int Max_Token_Bound;


static int passo=0;

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
    	bounds[i]=2;
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
  // JB: disabling PESSIMISTIC_DELETION
  rs->setNodeDeletion(forest::OPTIMISTIC_DELETION);
  //Initialize the marking
  static dd_edge new_marking(rs);
  marking=&new_marking;


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
  return true;
}

bool RSRG::init_RS(const int& npl,const Net_Mark_p& net_mark,const int&num,int (*pf)(const int&, const Net_Mark_p&))
{
  stringstream s1;//use to map id marking
  int** v = (int **) malloc(1 * sizeof(int*));
  v[0] = (int *) malloc((npl+2+MAX_ID) * sizeof(int));
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
  rs->createEdge(v, 1, *marking);
  //Initialize the marking
  //dd_edge new_marking(states);
  //states->createEdge(v, 1, new_marking);
  //(*marking)+=new_marking;
#if DEBUG3
for (int i=0;i<npl+MAX_ID+2;i++)
		{
		cout<<"["<<i<<"]"<<v[0][i]<<" ";
		}
cout<<endl<<endl;

#endif

  //free memory
  free(v[0]);
  free(v);
  //free memory
#if DEBUG
  cout<<"\nInitial Marking\n";
  (*marking).show(stdout,true);
#endif
  return true;
}


int** RSRG::en_marking(const int& npl,const Net_Mark_p& net_mark,int (*pf)(const int&, const Net_Mark_p&))
{
int** v = (int **) malloc(1 * sizeof(int*));
v[0] = (int *) malloc((npl+MAX_ID+2) * sizeof(int));
int j=MAX_ID+2;
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
return v;
}

bool RSRG::add_marking(const int& npl,int** v,const int& num,const int& type,int (*pf)(const int&, const Net_Mark_p&))
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

  dd_edge new_marking(rs);
  rs->createEdge(v, 1, new_marking);
  (*marking)+=new_marking;


#if DEBUG3
for (int i=0;i<npl+MAX_ID+2;i++)
		{
		cout<<"["<<i<<"]"<<v[0][i]<<" ";
		}
cout<<endl;
#endif

  //free memory
  free(v[0]);
  free(v);
  //free memory

  new_marking.clear();
  return true;
}

//no more update!!!!
bool RSRG::add_marking(const int& npl,const Net_Mark_p& net_mark,const int& num,const int& type,int (*pf)(const int&, const Net_Mark_p&))
{
  int** v = (int **) malloc(1 * sizeof(int*));
  v[0] = (int *) malloc((npl+MAX_ID+2) * sizeof(int));
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
  dd_edge new_marking(rs);
  rs->createEdge(v, 1, new_marking);
  (*marking)+=new_marking;


//#if DEBUG3
if (num==53565)
{
for (int i=0;i<npl+MAX_ID+2;i++)
		{
		cout<<"["<<i<<"]"<<v[0][i]<<" ";
		}
cout<<endl;
}
//#endif


  //free memory
  free(v[0]);
  free(v);
  //free memory


#if DEBUG
  cout<<"\nNew Marking\n";
  new_marking.show(stdout,true);
#endif



  new_marking.clear();


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

  stringstream s1;//use to map id marking
  int tro=NEW_MARKING;
  expert_forest* expertStates = static_cast<expert_forest*>(rs);
  // JB: expand when needed
  // int* k = (int *) malloc((npl+1) * sizeof(int));
  if (k.size() < (npl)) k.resize(npl);
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
  int* kArray = reinterpret_cast<int*>(&k[0]);
  //rs->evaluate(*marking,kArray,tmp);
  unsigned int st=0;
  int type=0;
  bool test=false;

  testI(marking->getNode(),kArray,npl+MAX_ID+1,st,expertStates,test,type);
  // JB: k is static, so do not free it
  //free memory
  //free(k);

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
    card=card*Max_Token;
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



//RSHASH class


bool RSHASH::add_marking(const int& npl, const Net_Mark_p& net_mark)
{


  static int num=0;


  //insert the mark in cache
  vector <int> tmp_v;
  std::string st_mark("");
  stringstream s1;
  for (int pl=0;pl<npl; pl++)
  {
    if (GORDER)
    {
      tmp_v.push_back(net_mark[pl].total);
      s1<<"*"<<net_mark[pl].total;
    }
    else
    {
      tmp_v.push_back(net_mark[OrdPlaces[pl]].total);
      s1<<"*"<<net_mark[OrdPlaces[pl]].total; 
    }
  }
  tmp_RS[s1.str()]=tmp_v;
  //insert the mark in cache
  //cout<<"\t"<<s1.str()<<endl;


  //fout<<s1.str()<<" "<<(tang+1)<<"\n";//update .mapping


  if (tmp_RS.size() == sizeCache)
  {//copy all the cache in the MDD
#if DEBUG1
    cout<<"\n\n";
    num++;
    cout<<num<<"\n\n";
#endif
    // JB: another static vector; ugly but works for now;
    // should make it a class member.
    static vector< vector<int> > v;
    if (v.size() < sizeCache) v.resize(sizeCache);
    // int** v = (int **) malloc(sizeCache * sizeof(int*));
    map <std::string, vector <int> >::iterator iter=tmp_RS.begin();
    for (int i=0;i<sizeCache;i++,iter++)
    {
      // JB: no need to malloc, we're using the static vector declared above
      // v[i] = (int *) malloc((npl+1) * sizeof(int));
      if (v[i].size() < (npl+1)) v[i].resize(npl+1);
      //v[i][0]=0;
      for(int pl = 0 ; pl < npl; pl++)
      {/* foreach place */
        v[i][pl+1]=iter->second[pl];
        //cout<< "*"<<iter->second[pl];
      }/* foreach place */
      //cout<<endl;
      iter->second.clear();
    }
    //create the edge representing multiple markings
    dd_edge new_marking(rs);

    // JB: another static array; again should probably be made class member.
    static int** vArray = 0;
    static int vSize = 0;
    if (vSize < sizeCache) {
      vSize = sizeCache;
      vArray = (int **) realloc(vArray, vSize * sizeof(int*));
      assert(vArray != 0);
      assert(v.size() == vSize);
      for (int i = 0; i < vSize; i++)
        vArray[i] = reinterpret_cast<int*>(&v[i][0]);
    }

    //rs->createEdge(v,sizeCache, new_marking);
    rs->createEdge(vArray,sizeCache, new_marking);


    //add the new markings in the MDD
    (*marking)+=new_marking;


    //free cache
    tmp_RS.clear();

    // JB: new marking will get cleared when it goes out of scope.
    //new_marking.clear();
    //tmp_v.clear();

    // JB: no need to free this memory when using the vectors above
    //free memory
    /*
    for (int i=0;i<sizeCache;i++)
    {
      free(v[i]);
    }
    free(v);
    //free memory
    */
    return true;
  }//copy all the cache in the MDD
}



int RSHASH::test_marking(const int& npl,const Net_Mark_p& net_marking, const Tree_p& reached_marking,int (*pf)(const int&, const Net_Mark_p&))
{
  stringstream s1;
  for (int pl=0;pl<npl; pl++)
  {
    if (GORDER)
      s1<<"*"<<net_mark[pl].total;
    else
      s1<<"*"<<net_mark[OrdPlaces[pl]].total;
  }
  if (tmp_RS.find(s1.str())==tmp_RS.end())
  {//the new marking is not in cache
    return  this->RSRG::test_marking(npl,net_marking,reached_marking,pf);
  }//the new marking is not in cache
  else    
  {//the new marking is  in cache
    //now we are using only Timed Transitions
    reached_marking->marking->pri=-1;
    //now I'm using only Timed
    //cont_tang=FindInFile(s1,fin);
    tro = TANGIBLE_OLD;
    if (cont_tang<0)
    {
      ExceptionIO obj("Error mapping not find\n");
      throw (obj);
    }
    //now we are using only Timed Transitions


    /*if(IS_VANISHING(reached_marking->marking->pri))
      {// Vanishing marking
      if(reached_marking->enabled_head == NULL)
      tro = VANISHING_OLD;
      else
      tro = VANISHING_LOOP;
      }// Vanishing marking
      else if(IS_TANGIBLE(reached_marking->marking->pri))
      {
      tro = TANGIBLE_OLD;
      cont_tang = reached_marking->marking->cont_tang;
      }
      else
      {
      cont_tang = reached_marking->marking->cont_tang;
      tro = DEAD_OLD;
      }
      */
    return tro;
  }//the new marking is  in cache
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

