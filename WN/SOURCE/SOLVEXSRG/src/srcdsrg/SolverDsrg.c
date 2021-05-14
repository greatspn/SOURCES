#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "const.h"
#include "SCONSSPOT.h"
#include "struct.h"
#include "SSTRUCTSPOT.h"
#include "var_ext.h"
#include "SVAR_EXTSPOT.h"
#include "macros.h"
#include "structSolv.h"
#include "service.h"
#include "gspnlib.h"

#include "../../../DSRG/src/refDsrg.h"

#undef  NORMAL

#define true 1
#define false 0
#define MAXSTR 256


extern void write_on_srg();
extern void create_canonical_data_structure();
extern TO_MERGEP   *TO_MERGE();
extern void   GROUPING_ALL_STATICS();
extern STORE_STATICS_ARRAY CREATE_STORE_STATIC_STRUCTURE();
extern void   NEW_SIM_STATIC_CONF();
extern void       FREE_ALL_LMS_ELEMS();
extern void compose_name(char  *name,  const char  *path,  const char  *postfix);
extern int perf_index_parser(FILE* f);
extern void CHAR_LOAD_COMPACT(unsigned long *);
extern TYPE_P AllocateBV();
extern void   POP_STATIC_CONF();
extern void string_to_marking();
extern STORE_STATICS_ARRAY
NewTabc__(TYPE_P **PM, TYPE_P NbElPM, char *** MTCL);
extern void SPECIAL_CANISATION();
extern void DynDistOfObjects(TYPE_P **PM, TYPE *NbElPM,
                 int *** Card, int **num, int **DynDis);
extern CToken_p ParcourtArbreProp(TreeNode_p root, int **DynDis);
extern int my_get_ordinary_tokens(CToken_p tk_p);
extern void FreeList(CToken_p tk);
extern int INCLUDE(TYPE_P CONJ1, TYPE_P CONJ2);
extern int       get_refined_eventualities_prob(TYPE_P **PM_mark ,
        TYPE_P NbElPM_mark,
        TYPE_P **PM , TYPE_P NbElPM,
        pDecomp   *EVENT_ARRAY_ ,
        int *SIZE_OF_EVENT_ARR_);
extern void my_garbage_collect(Tree_Esrg_p fire_ptr);
extern void free_mtrx(mtrx *U, unsigned long nb);


typedef struct SR_SM {
  int* ord;
  int* sm;
  unsigned long* fstpos;
  unsigned long  fsrpos;
  int nb_sm;
}sr_sm;

sr_sm* arr_sr_sm=NULL;
int* vanishing=NULL;
int nb_sr=0;
int nb_sm=0;
int nb_va=0;
int nb_dead=0;
int nb_v=0; 
double normal =0.0;
TYPE_P** OR_PM=NULL;
TYPE_P NB_OR_PM=NULL;
int indices=FALSE;
FILE*  dsrg ;
FILE* off_ev=NULL;
FILE* off_sr=NULL;
FILE* resultat=NULL;

/********************** extern function ************************/
extern char* strtok();
extern Result_p trait_gurded_transitions (Result_p enabled_head, 
					  PART_MAT_P s_prop_Part);
extern int      adjust_priority          (int cur_priority,
					  Result_p enabled_head);
extern Event_p get_new_event             (int tr);
extern double* gauss_seidel              (double* mprob, 
					  mtrx * column,
					  unsigned long top_tan );

/***************************************************************/

/************** exetrn variables ***********/
extern char cache_string [];
extern TO_MERGEP* MERG_ALL;
extern STORE_STATICS_ARRAY Sym_StaticConf;
extern Result_p enabled_head;
extern int cur_priority;
/******************************************/


char net[MAXSTRING];
Tree_Esrg_p  sr_ordinary=NULL; 
int sr_size_ordinary=0;
int counter=0;

void 
free_temp_ord(Tree_Esrg_p  sr_ordinary, 
		   int sr_size_ordinary){
  int i; 
  for(i=0;i<sr_size_ordinary;i++)
    if(sr_ordinary[i].length_for_prob!=0)
      free(sr_ordinary[i].marking_for_prob);
  free(sr_ordinary);
}

int 
seach_string_marking(Tree_Esrg_p  sm,
		     int sm_size,char* s_string,
		     int s_size ){

  int i;
  unsigned long ii,op1,op2;
  char* cp;
  char* pp;

  for(i=0;i< sm_size;i++)
    if(s_size == sm[i].length_for_prob){
      cp=sm[i].marking_for_prob ;
      pp=s_string ;
      for(ii=s_size;ii;ii--) {
	op1= *cp; cp++;
	op2= *pp; pp++;
	if(op1 != op2 )
	  break;
      }
      if(ii==0){
	return EQUAL;
      }
    }
  return NOT_EQUAL;; 
}


int 
copy_cache_string( char** new_string){
  int i;

  (*new_string)=(char*)calloc(length,sizeof(char));

  for(i=0; i<length;i++)
    (*new_string)[i]=cache_string[i];
  return length;
}

// This function is used 
// to print a symbolic state
// in "res" file 
int 
print_symbolic_marking (FILE* res,
			int ord,
			int marking, 
			char** st,
			double pr) {
 
  FILE *fd; 
  int pos;
  char*  mark="states.mark"; 
  fd=fopen(mark,"w+");

  write_on_srg(fd, 1 );
  pos=ftell(fd);
  *st=malloc((pos+1)*sizeof(char));
  fseek(fd, 0,SEEK_SET);
  fread(*st, sizeof(char),pos*sizeof(char),fd);
  (*st)[pos]='\0';
  fclose(fd);
  if(!ord)
    fprintf(res, "\n Symbolic marking [%d] [prob=%lf]:\n %s",
	    marking,pr,(*st));
  else
    fprintf(res, "\n Ordinary marking [%d] [prob=%lf]:\n %s",
	    marking,pr,(*st)); 
   
  system("rm -f states.mark");  
  return 0;
}

// closes all files at the end
// of a session. 
void 
finalize_session(FILE* fp) {
  fclose (fp);  
  fclose(off_sr);
  fclose (off_ev);
  fclose(resultat);
}

// Initialisation of global variables
void 
init_global_variables(char* net_name) {
 
  create_canonical_data_structure();    
  MERG=MERG_ALL = (TO_MERGEP*) TO_MERGE(ncl,tabc);
  GROUPING_ALL_STATICS(MERG_ALL,tabc,num);
  Sym_StaticConf= (STORE_STATICS_ARRAY)
                  CREATE_STORE_STATIC_STRUCTURE();
  NEW_SIM_STATIC_CONF (&Sym_StaticConf);
  FreeMerg(MERG_ALL);
  FREE_ALL_LMS_ELEMS();

  // output params. of the 
  // dsrg construction (with options: -o, -m) 
  compose_name(net,net_name,"cmst");
  FILE* stat= fopen(net,"r");  
  fscanf(stat,"%d %d %d %d",
	 &nb_sr,&nb_sm,
	 &nb_va,&nb_dead);
  fclose(stat);
 
 
  compose_name(net,net_name,"prob");
  resultat= fopen(net,"w+");   
  compose_name(net,net_name,"esrg");
  dsrg=fopen(net,"r");  
  compose_name(net,net_name,"off_sr");
  off_sr= fopen(net,"r");
  compose_name(net,net_name,"off_ev");
  off_ev= fopen(net,"r");

  if(indices){
    compose_name(net,net_name,"ind");
    FILE* INDICES= fopen(net,"r");
    perf_index_parser(INDICES);
  }
 
  arr_sr_sm=(sr_sm*)calloc(nb_sr,sizeof(sr_sm));
  vanishing=(int*)calloc(nb_va,sizeof(int));  
}



// load the symmetrical representation 
// from the ".mark" file using the traditional 
// params. stored  in the ".off_sr" file.
unsigned long
load_sym_rep(FILE* fd, int pos,
	     unsigned long fpos,
             unsigned long *  mark_pos,
	     unsigned long * length,
	     unsigned long * d_ptr) {
  int i,ps=0;

 
    fseek(fd,fpos,SEEK_SET);
    fread(CACHE_STRING_ESRG,1,100,fd);
    LP_ESRG = CACHE_STRING_ESRG;
    CHAR_LOAD_COMPACT( mark_pos );
    CHAR_LOAD_COMPACT( length );
    CHAR_LOAD_COMPACT( d_ptr );
  
  return fpos;    
 
}

// load an eventuality from the .event
// file, using the "pos" position in the "off_ev" 
// file to get the position "d_ptr" and the length  
// "length" of the eventuality.   
unsigned long
load_event( FILE* fd,
	    int pos, 
	    unsigned long fpos,
	    unsigned long * sr,  
	    unsigned long * llength, 
	    unsigned long * ld_ptr,
	    TYPE_P*** PM,  
	    TYPE_P*   NB_PM ) {

  int i,cl,sc,k, ps=0;

  (*PM)    = (TYPE_P**) calloc(ncl,sizeof(TYPE_P*));
  (*NB_PM) = (TYPE_P) calloc(ncl,sizeof(TYPE));


    fseek(fd,fpos,SEEK_SET);
    fread(CACHE_STRING_ESRG,1,100,fd); 
    LP_ESRG = CACHE_STRING_ESRG;

    CHAR_LOAD_COMPACT( sr );

    for(cl=0;cl<ncl;cl++) {
      unsigned long nbst;
      CHAR_LOAD_COMPACT(&nbst);
      (*NB_PM)[cl]=(TYPE)nbst;
      (*PM)[cl]=(TYPE_P*)calloc((*NB_PM)[cl],sizeof(TYPE_P));
      for(sc=0;sc<(int)(*NB_PM)[cl];sc++){
	(*PM)[cl][sc]=(TYPE_P)AllocateBV();
	for(k=0;k<MAX_IND;k++){
	  unsigned long v;
	  CHAR_LOAD_COMPACT(&v);
	  (*PM)[cl][sc][k]= (TYPE)v;
	}
      }
    }
    CHAR_LOAD_COMPACT( ld_ptr );
    CHAR_LOAD_COMPACT( llength );
  

  return fpos; 
}

// Load a symbolic state in the 
// internal structures  of 
// GreatSPN and print it
void 
load_symbolic_marking( FILE* fsr,
		       FILE* fev, 
		       int sour,
		       int ev,
		       double prob) {
  unsigned long sr;
  unsigned long llength; 
  unsigned long ld_ptr;
  unsigned long mark_pos;
  TYPE_P** PM=NULL;
  TYPE_P NB_PM=NULL; 
  STORE_STATICS_ARRAY old_StaticConf=NULL;
               
  load_event(fev,0, arr_sr_sm[sour].fstpos[ev],
	     &sr,&llength,&ld_ptr,&PM,&NB_PM );

  /********** load the i'th eventuality from the file ************************/
  GET_EVENTUALITIE_FROM_FILE(ld_ptr,llength);
  COPY_CACHE(llength);         
  STRING_TO_EVENTUALITIE(Sym_StaticConf); 
  /****************************************************************************/

  load_sym_rep(fsr, sr ,arr_sr_sm[sour].fsrpos, &mark_pos, &llength,  &ld_ptr);

  /**********  change the current tabc by the one of s and load s state ********/
  POP_STATIC_CONF(Sym_StaticConf,&tabc); 
  string_to_marking(mark_pos,ld_ptr,llength);
  /*****************************************************************************/

  /********* load partion of the symbolic marking s ***************************/      
  old_StaticConf=(STORE_STATICS_ARRAY )NewTabc__(PM,NB_PM,MTCL);  
  POP_STATIC_CONF(old_StaticConf,&tabc); 
  MERG= MERG_ALL = (TO_MERGEP*)TO_MERGE(ncl,tabc);
  GROUPING_ALL_STATICS(MERG_ALL,tabc,num);
  /*****************************************************************************/

  /************* instanciate the marking s with the eventuality i **************/
  NEW_ASYM_MARKING(& tabc,&net_mark,&card,&num,&tot,
		   MERG_ALL,old_StaticConf,RESULT);
  /*****************************************************************************/
  SPECIAL_CANISATION();

  char* st;
   print_symbolic_marking (resultat,0,arr_sr_sm[sour].sm[ev],&st,prob);
  free(st);

  FreeStoreStructs(old_StaticConf);
  FreeMerg(MERG_ALL); 
  FREE_ALL_LMS_ELEMS();
  FreePartMAt(PM, NB_PM);
} 


// Store of symbolic states
// in the arr_sr_sm array structure 
void 
add_sms(int sr_source,unsigned long ofsr, 
	int source,unsigned long ofst, 
	int sord,
	int sr_dest, unsigned long ofsrd, 
	int dest,unsigned long ofstd, 
	int dord) {
  int i;

  arr_sr_sm[sr_source].fsrpos=ofsr;

  for(i=0 ; 
      (i < arr_sr_sm[sr_source].nb_sm) && 
      (arr_sr_sm[sr_source].sm[i]!=source) ; 
      i++);

  if(i==arr_sr_sm[sr_source].nb_sm) 
    {
      arr_sr_sm[sr_source].sm=
	(int*)realloc(arr_sr_sm[sr_source].sm,(i+1)*sizeof(int));
      arr_sr_sm[sr_source].ord=
	(int*)realloc(arr_sr_sm[sr_source].ord,(i+1)*sizeof(int));
      arr_sr_sm[sr_source].fstpos=
	(unsigned long*)realloc(arr_sr_sm[sr_source].fstpos,
				(i+1)*sizeof(unsigned long));
      arr_sr_sm[sr_source].sm[i]=source;
      arr_sr_sm[sr_source].ord[i]=sord;
      arr_sr_sm[sr_source].fstpos[i]=ofst;
      arr_sr_sm[sr_source].nb_sm++;
    }

  for(i=0 ; 
      (i<arr_sr_sm[sr_dest].nb_sm) && 
      (arr_sr_sm[sr_dest].sm[i]!=dest );
      i++);
  
  arr_sr_sm[sr_dest].fsrpos=ofsrd;
  
  if(i==arr_sr_sm[sr_dest].nb_sm) 
    {
      arr_sr_sm[sr_dest].sm=
	(int*)realloc(arr_sr_sm[sr_dest].sm,(i+1)*sizeof(int));
      arr_sr_sm[sr_dest].ord=
	(int*)realloc(arr_sr_sm[sr_dest].ord,(i+1)*sizeof(int));
      arr_sr_sm[sr_dest].fstpos=
	(unsigned long*)realloc(arr_sr_sm[sr_dest].fstpos,
				(i+1)*sizeof(unsigned long));
      arr_sr_sm[sr_dest].sm[i]=dest;
      arr_sr_sm[sr_dest].ord[i]=dord;
      arr_sr_sm[sr_dest].fstpos[i]=ofstd;
      arr_sr_sm[sr_dest].nb_sm++;
    }    
  
}




/************************** FOR PERF. INDICES COMPUTATION ************************/

int get_ordinary_tokens_perfs(Token_p tk_p, int pl)
 {/* Init get_ordinary_tokens */
  int ret_value = 1;
  int ii = GET_PLACE_COMPONENTS(pl);
  int cl,ss,ds;

  for( ; ii ; ii--)
   {/* Per ogni componente del dominio del posto */
    cl = GET_COLOR_COMPONENT(ii - 1,pl);
    if(IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
     {/* Classe non ordinata */
      ds = tk_p->id[ii - 1];
      ss = get_static_subclass(cl,ds);
      ret_value *= GET_CARD(cl,ss,ds);
     }/* Classe non ordinata */
   }/* Per ogni componente del dominio del posto */
  return(ret_value);
 }/* End get_ordinary_tokens */


void printToken(int* token, int size)
{
  int k;
  for (k=0; k <size;k++)
    printf("%d ", token[k]);
  
}

int COUNT_FUNCT_PLACES(PerfIndex_P p, 
		       TYPE_P**    PM, 
		       TYPE*   NbElPM,
		       int***  Card, 
		       int**   num,
		       int**   DynDis){ 
  int i=0;
  if (p->cond) {
    CToken_p ptr,ptr1,ptr2;
    DynDistOfObjects(PM, NbElPM,Card,num,DynDis );
    ptr1= ParcourtArbreProp((p->cond)->filsg,DynDis);
    ptr2= ParcourtArbreProp((p->cond)->filsd,DynDis);
    ptr=ptr1;
 
   while(ptr){
       
      if(memcmp((ptr->tok_dom).Token,
		(ptr2->tok_dom).Token,
		(ptr2->tok_dom).NbCl*sizeof(int)
		)==0
	 ){

	i += ((ptr->mult)*my_get_ordinary_tokens(ptr));
      }
      ptr=ptr->next;
    }
    FreeList(ptr1);
    FreeList(ptr2);
  }
  else {
    
    int place=p->pt_index ; 
    Token_p marking=net_mark[place].marking;
    while(marking){ 
      i += ((marking->molt)* get_ordinary_tokens_perfs(marking, place));        
      marking=marking->next;
    } 
  }  
  return i;
}

int 
COUNT_FUNCT_TRANS(PerfIndex_P p, 
		  PART_MAT_P mark_part, 
		  Event_p list)  {
  Event_p ptr=list;
  int i,sbc=UNKNOWN; 
  TYPE_P var=(TYPE_P)AllocateBV();

  if (p->cond) {
    CToken_p Token=p->cond->FONCT;
    int cl=tabt[list->trans].dominio[(Token->mult)-1];
    int dy=list->npla[(Token->mult)-1];
    
    for(i=0;i<(Token->tok_dom).NbCl ;i++) Add (var,(Token->tok_dom).Token[i]) ;   

    if(INCLUDE(mark_part->PART_MAT[cl][get_static_subclass(cl,dy)],var)){
      free(var);  
      return (list->ordinary_instances* list->enabling_degree);
    }
    free(var);
    return 0;
      
  }
 
 free(var);
 return (list->ordinary_instances*list->enabling_degree);
}   

void ParcourtStructuretrans(PART_MAT_P mark_part,  Event_p ev){
  int i;
  
  for (i=0 ; i<NbPind ;i++)     
    if (PERFINDICES[i]->type==TR && ev->trans==PERFINDICES[i]->pt_index)
      PERFINDICES[i]->sum += COUNT_FUNCT_TRANS(PERFINDICES[i],mark_part,ev); 
      
}


void ParcourtStructureplaces(PART_MAT_P mark_part){
  int i;

  for (i=0 ; i<NbPind ;i++)     
    if (PERFINDICES[i]->type == PL){
	PERFINDICES[i]->sum = COUNT_FUNCT_PLACES(PERFINDICES[i], 
						 mark_part->PART_MAT, 
						 mark_part->NbElPM,
						 card,num,DynDis);

    }
}

void CollectInfo(double prob){

  int i;
  for (i=0 ; i<NbPind ;i++) 
    if(PERFINDICES[i]->sum){
      if(PERFINDICES[i]->type==TR)
	PERFINDICES[i]->mean += PERFINDICES[i]->sum * 
	                        tabt[PERFINDICES[i]->pt_index].mean_t * 
	                        prob;
      else 
	PERFINDICES[i]->mean += PERFINDICES[i]->sum * prob;
      
      PERFINDICES[i]->sum=0;     
    }
}

char* GET_FUNC(int t, int ind)
 {
   return tabt[t].names[ind];
 }

void PrintCondition(int i){
  int k;
  if (PERFINDICES[i]->type==TR) {
    printf("%s={",GET_FUNC(PERFINDICES[i]->pt_index, 
                  (PERFINDICES[i]->cond->FONCT->mult)-1));
    for(k=0;k<(PERFINDICES[i]->cond->FONCT->tok_dom).NbCl;k++)
      printf("%s ", GetObjFrInd(((PERFINDICES[i]->cond->FONCT->tok_dom).Token[k])-1 ,MTCL,
				(PERFINDICES[i]->cond->FONCT->tok_dom).Domain[k] )); 
    printf("}");
  }
  else{
    printf("[");
    for(k=0;k<(PERFINDICES[i]->cond->filsg->FONCT->tok_dom).NbCl;k++)
      if((PERFINDICES[i]->cond->filsg->FONCT->tok_dom).Token[k]==ELIMINATE)
	printf("-");
      else     
	printf("*");

    printf("]=<"); 

    for(k=0;k<(PERFINDICES[i]->cond->filsd->FONCT->tok_dom).NbCl;k++)
      printf("%s ", GetObjFrInd(((PERFINDICES[i]->cond->FONCT->tok_dom).Token[k])-1 ,MTCL,
			       (PERFINDICES[i]->cond->FONCT->tok_dom).Domain[k] )); 
    
    printf(">"); 
     
  }
    

}


void PrintIndicies(double normal){
  int i;
     for (i=0 ; i<NbPind ;i++) 
       if(PERFINDICES[i]->type==TR){
	 if (!PERFINDICES[i]->cond)
	   printf(" The throughput of the transtion \"%s\" = %g\n", 
		  tabt[PERFINDICES[i]->pt_index].trans_name,
		  PERFINDICES[i]->mean/normal);
	 else{
	   printf(" The throughput of the transtion \"%s\"", 
		  tabt[PERFINDICES[i]->pt_index].trans_name);
	   printf(" w.r.t. conditon \"");   
	   PrintCondition(i);
	   printf("\" = %g\n",PERFINDICES[i]->mean/normal);
	 }
	   
	 
       }
       else{
	 if (!PERFINDICES[i]->cond)
	   printf(" The mean number of tokens in the place \"%s\" = %g\n", 
		  tabp[PERFINDICES[i]->pt_index].place_name,
		  PERFINDICES[i]->mean/normal);
	 else{
	   printf(" The mean number of tokens in the place \"%s\" ", 
		  tabp[PERFINDICES[i]->pt_index].place_name);
	   printf(" w.r.t. conditon \"");   
	   PrintCondition(i);
	   printf("\" = %g\n",PERFINDICES[i]->mean/normal);

	 }
       }

}
/*****************************************************************************************/



// construction of the color partition 
// needed to get ordinary states
void 
get_worst_partition(TYPE_P*** pm, 
		    TYPE_P* nbpm ) {
  int i,j;

  (*nbpm)= (TYPE_P)calloc(ncl,sizeof(TYPE));
  (*pm) = (TYPE_P**)calloc(ncl,sizeof(TYPE_P*));
  
  for(i=0;i<ncl;i++)
    {
      (*pm)[i]=(TYPE_P*)calloc(tabc[i].card,sizeof(TYPE_P));
      for(j=0;j<tabc[i].card;j++)
	(*pm)[i][j]=(TYPE_P)AllocateBV();
    }
  
  for(i=0;i<ncl;i++)
    {
      for(j=0;j<tabc[i].card;j++)
	Add ((*pm)[i][j], j+1);
      (*nbpm)[i]=tabc[i].card;
    }    
}



// Computation of ordinary states 
// represented by a symbolic state 
void 
get_ordinary_markings( FILE* fsr,
		       FILE* fev, 
		       int sour,
		       int ev,
		       double prob) { 
  unsigned long sr;
  unsigned long llength; 
  unsigned long ld_ptr;
  unsigned long mark_pos;
  TYPE_P** PM=NULL;
  TYPE_P NB_PM=NULL;
  Tree_Esrg_p  ordinary=NULL     ; 
  int size_ordinary=0,i,j;
  
  /********** load the i'th eventuality from the file ********************/
  load_event(fev,arr_sr_sm[sour].sm[ev],arr_sr_sm[sour].fstpos[ev], 
	     &sr,&llength,&ld_ptr,&PM,&NB_PM );
  GET_EVENTUALITIE_FROM_FILE(ld_ptr,llength);
  COPY_CACHE(llength);         
  STRING_TO_EVENTUALITIE(Sym_StaticConf); 
  /***********************************************************************/ 

  get_refined_eventualities_prob( PM ,NB_PM ,OR_PM ,NB_OR_PM,
				  &ordinary ,&size_ordinary );
 
  for(i=0;i< size_ordinary;i++) {
    for(j=0;j<sr_size_ordinary ;j++)
      if(NEW_COMPARE_EVENTUALITIES(
				   sr_ordinary[j].marking->marking_as_string,
				   sr_ordinary[j].marking->length,
				   ordinary[i].marking->marking_as_string,
				   ordinary[i].marking->length,
				   TEMP_EVENT
				   ) == EQUAL
	 ){
	  sr_ordinary[j].input_tr[0] +=prob;
	  break;
	}
       
       if(j==sr_size_ordinary) {
	  sr_ordinary=realloc( sr_ordinary,(sr_size_ordinary+1)*sizeof(struct TREE_ESRG));
	  sr_ordinary[sr_size_ordinary].marking=malloc(sizeof(struct MARKING_INFO));
	  sr_ordinary[sr_size_ordinary].marking->marking_as_string= 
	                                ordinary[i].marking->marking_as_string;
	  sr_ordinary[sr_size_ordinary].marking->length=ordinary[i].marking->length;
	  sr_ordinary[sr_size_ordinary].enabled_head=NULL;
	  sr_ordinary[sr_size_ordinary].marking->pri=UNKNOWN;
	  sr_ordinary[sr_size_ordinary].input_tr = (double*) calloc(1,sizeof(double)); 
	  sr_ordinary[sr_size_ordinary].input_tr[0]= prob;
	  sr_ordinary[sr_size_ordinary].marking_for_prob=NULL;
	  sr_ordinary[sr_size_ordinary].length_for_prob=0;
	  sr_size_ordinary++;	
	}
      free(ordinary[i].marking);
    }

  free(ordinary);
  FreePartMAt(PM, NB_PM);
}


// Computation of steady state space  
// probabilities for the ordinary CTMC
void 
compute_oridinary_probs (double* prob) {
  int i,j,k,tr,ord=0;
  char* st; 
  unsigned long sr;
  unsigned long llength; 
  unsigned long ld_ptr;
  unsigned long mark_pos;
  STORE_STATICS_ARRAY old_StaticConf=NULL  ;
  PART_MAT_P s_prop_Part  = (PART_MAT_P) malloc(sizeof(PART_MAT));
  Event_p  nev_p ,ev_p,copy_of_ev_p;
  Result_p current_transition;
  Result_p next_transition;

  /********* load partion of the symbolic marking s ***************************/      
  old_StaticConf=(STORE_STATICS_ARRAY )NewTabc__(OR_PM,NB_OR_PM,MTCL);  
  POP_STATIC_CONF(old_StaticConf,&tabc); 
  MERG= MERG_ALL = (TO_MERGEP*)TO_MERGE(ncl,tabc);
  GROUPING_ALL_STATICS(MERG_ALL,tabc,num);
  /*****************************************************************************/

  s_prop_Part->PART_MAT=OR_PM;
  s_prop_Part->NbElPM=NB_OR_PM;

  for(i=0;i<nb_sr;i++) {
    counter=0;
    sr_ordinary=NULL;
    sr_size_ordinary=0;

    int desc= fileno(TEMP_EVENT);
    ftruncate(desc,0);
    
    load_sym_rep(off_sr,i, arr_sr_sm[i].fsrpos, &mark_pos ,&llength ,&ld_ptr);

    /**********  change the current tabc by the one of s and load s state ********/ 
    POP_STATIC_CONF(Sym_StaticConf,&tabc); 
    string_to_marking(mark_pos,ld_ptr,llength);
    /*****************************************************************************/ 
      
    for(j=0;j<arr_sr_sm[i].nb_sm;j++)
      get_ordinary_markings(off_sr, off_ev, i,j, 
			    prob[ arr_sr_sm[i].sm[j] ] / arr_sr_sm[i].ord[j]);
	
  
    for(k=0; k<sr_size_ordinary; k++)
      {
	  
	GET_TEMP_EVENTUALITIE_FROM_FILE(sr_ordinary[k].marking->marking_as_string,
					sr_ordinary[k].marking->length ,TEMP_EVENT);
	COPY_CACHE(sr_ordinary[k].marking->length);         
	STRING_TO_EVENTUALITIE(Sym_StaticConf); 
	   
	/************* instanciate the marking s with the eventuality i **********/ 
	NEW_ASYM_MARKING(& tabc,&net_mark,&card,&num,&tot,
			 MERG_ALL,old_StaticConf,RESULT);
	SPECIAL_CANISATION();
	/*************************************************************************/
 
	if( seach_string_marking(sr_ordinary,k,cache_string,length) != EQUAL ){ 
	  counter ++; ord++;

	  sr_ordinary[k].length_for_prob=copy_cache_string( & sr_ordinary[k].marking_for_prob); 
	 
	  /************** computation of ordinary tokens in  each place ************/
	  if (indices) ParcourtStructureplaces(s_prop_Part);
	  /*************************************************************************/

	  enabled_head=NULL;	    
	  int pri;
	  my_initialize_en_list();  	
	  enabled_head= trait_gurded_transitions(enabled_head,s_prop_Part);
	  //  printf("before:%d",cur_priority);
	  if(enabled_head){ 
	    pri=adjust_priority(0/*not signif*/,enabled_head);
	  }

	  double rate=0;  
	  current_transition= enabled_head;
	  sr_ordinary[k].enabled_head=enabled_head;
	  while(current_transition != NULL)
	    {
	      next_transition = current_transition->next;
	      tr = GET_TRANSITION_INDEX(current_transition->list);
	  
	      if(tabt[tr].pri == pri){ 
		copy_of_ev_p = get_new_event(tr);
		ev_p = current_transition->list;
		    
		while(ev_p != NULL){
		  nev_p = ev_p->next;
		  copy_event(copy_of_ev_p,ev_p);
		     
		  if(tabt[tr].timing!=TIMING_IMMEDIATE)
		    rate += ev_p->ordinary_instances * (ev_p->enabling_degree* tabt[tr].mean_t);
		  else 
		    rate =0;
		  /************** computation of ordinary tokens in  each event ************/
		  if (indices) ParcourtStructuretrans(s_prop_Part, ev_p);
		  /*************************************************************************/
		  ev_p = nev_p;
		}
		dispose_old_event(copy_of_ev_p);
	      }
	      current_transition = next_transition;
	    } 
	  
	  if(rate != 0) {
	    sr_ordinary[k].input_tr[0] *= (1/rate);
	    /****************  computation of the correct indicies ************************/
	    if (indices) CollectInfo(sr_ordinary[k].input_tr[0]);
	    /******************************************************************************/
	  }
	  else
	    sr_ordinary[k].input_tr[0]=0.0;

	  normal +=sr_ordinary[k].input_tr[0]; 
	  print_symbolic_marking (resultat,1,ord, &st, sr_ordinary[k].input_tr[0]);  
	  my_garbage_collect( &sr_ordinary[k]);
	  free(st);
	}
	free(sr_ordinary[k].marking);
	free(sr_ordinary[k].input_tr);
	POP_STATIC_CONF(Sym_StaticConf,&tabc); 
	string_to_marking(mark_pos,ld_ptr,llength); 
      }
    free_temp_ord(sr_ordinary,sr_size_ordinary);
  }
 
  free(s_prop_Part);  
  FreeStoreStructs (old_StaticConf);
}



void solver_usage (void) {

 fprintf(stdout,"\n\n------------------------------------------------------\n");
 fprintf(stdout,"USE : solverDsrg netname [options]                        \n");
 fprintf(stdout,"      options :                                           \n");
 fprintf(stdout,"        -s : to solve the symbolic DTMC                   \n");
 fprintf(stdout,"        -o : to solve the ordinary CTMC                   \n");
 fprintf(stdout,"        -p : to solve the ordinary CTMC with                ");
 fprintf(stdout,"             perf. indices computation                    \n");
 fprintf(stdout,"----------------------------------------------------------\n");

}

void 
output_dtmc_solution (double* pi){
  
  unsigned long i,j;

  fprintf(resultat,"\n /*********** Symbolic markings steady probabilities"); 
  fprintf(resultat,"(Symbolic DTMC) ***********/ \n");

  for(i=0 ; i< nb_sr ; i++)   
    for (j=0 ; j< arr_sr_sm[i].nb_sm  ; j++)   
      load_symbolic_marking(off_sr,off_ev, i,j, pi[ arr_sr_sm[i].sm[j]] );

  fprintf(resultat,"\n /*********** End symbolic markings steady probabilities");
  fprintf(resultat," (Symbolic DTMC) ***********/ \n");

}


// Parse of  params. (in order) :
// sym. rep. position of source
// source 
// nb ordinary of the source
// type of source (tangible or vanishing)
// out put prob.
// sym. rep. position of destination
// destination
// nb ordinary of the destination

parse_binding(char* string,
		   int* sr_source, 
		   unsigned long* ofsr, 
		   int* source ,
		   unsigned long* ofst, 
		   int*sord,
		   int* type, 
		   int* sr_dest, 
		   unsigned long* ofsrd, 
		   int* dest ,
		   unsigned long* ofstd, 
		   int* dord,
		   double* prob  ) { 
  
  sscanf(string, "%d %lu %d %lu %d %d %lf %d %lu %d %lu %d",
	 sr_source, ofsr,  source , ofst,  sord,
	 type, prob, sr_dest,  ofsrd, dest , ofstd,  dord);

  return true;
}

// Load of the transition matrix U
// from the dsrg ASCII file (netname.dsrg).
mtrx* 
load_mat(unsigned long nb_sm, FILE*  dsrg ) {
  int sr_source, source, sord,
      sr_dest,   dest,   dord, type;
  double rate;
  char tmp[MAXSTR];
  mtrx* U=NULL;
  unsigned long ofsr, ofst, ofsrd, ofstd; 

 
  /*************** Allocation of the matrix of transition U *****/
  U   = (mtrx*)calloc(nb_sm+1,sizeof(mtrx));
  /************** End allocation ********************************/

  /*************** upload of the U matrix * *********************/
  while( fgets(tmp, MAXSTR - 1, dsrg ) ) { 
  
    (void) parse_binding(tmp,
			 &sr_source,&ofsr,  
			 &source,&ofst,
			 &sord,
			 &type,
			 &sr_dest,&ofsrd,
			 &dest,&ofstd, 
			 &dord,&rate);

    add_sms( sr_source,ofsr, 
	     source,ofst,
	     sord, 
	     sr_dest,ofsrd, 
	     dest,ofstd,
	     dord);

    dest++;
    source++; 
    U[dest].ninn++;
    U[dest].first= (itm*)realloc (U[dest].first,U[dest].ninn *sizeof(itm));  
    U[dest].first[(U[dest].ninn)-1].fm=source;
    U[dest].first[(U[dest].ninn)-1].rt=rate;
  }  
  /*************************************************************/
  return U;
}

// The pricipal function of the solver.
// To use the solver, use this function 
// with the number of dead states (nb_dead),
// the number of states (nb_sm),
// the dsrg file (netname.dsrg). 
double* 
symbolic_solv(unsigned long nb_dead, 
	      unsigned long nb_sm,
	      FILE* dsrg ){

  mtrx* U=NULL;
  double* prob=NULL;
  unsigned long i;

  if(nb_dead){ 
    printf("THE MARCOV CHAIN IS NOT ERGODIC"); 
    return NULL;
  }

  U=load_mat(nb_sm,dsrg);
  prob = (double*)calloc(nb_sm+1,sizeof(double));
  for (i = nb_sm  ; i-- ; prob[i] = 1.0 /(double)nb_sm );
  
  prob = gauss_seidel(prob, U ,nb_sm);

  free_mtrx(U,nb_sm+1);

  return prob;
 
}



int 
main(int argc, char* argv[] ) {
  int i,ordi=FALSE;
  double* pi1 ; 
  double* pi; 
  char** arg=(char**)calloc(3,sizeof(char*));

  if (argc < 2){
    solver_usage();
    exit(1);
  }

  arg[2]=(char*)calloc(2,sizeof(char));
  arg[0]=argv[0];
  arg[1]=argv[1];
  arg[2][0]='-';  
  arg[2][1]='r';

  if(argc > 2)
     switch(argv[2][1]) {
     case 's' : break;
     case 'o' : ordi = TRUE; break;
     case 'p' : ordi = TRUE; indices=TRUE; break;
     default  : solver_usage(); exit(1);
    }
  printf("CIO\n");
  initialize (3, arg);
  init_global_variables (arg[1]);
  
  pi1=symbolic_solv(nb_dead,nb_sm,dsrg);
  pi= pi1+1;

  output_dtmc_solution (pi);
  
  if(ordi) {
    get_worst_partition(&OR_PM,&NB_OR_PM); 
   
    fprintf(resultat,"\n /*********** Ordinary markings steady probabilities");
    fprintf(resultat,"(Ordinary CTMC)***********/ \n");

    compute_oridinary_probs (pi);

    fprintf(resultat,"\n The final CTMC probabilities are obtained by");
    fprintf(resultat," normalisation: dividing by  %f ) \n", normal);
    fprintf(resultat,"\n /******** End ordinary markings steady probabilities");
    fprintf(resultat,"(Ordinary CTMC)***********/ \n");
    if (indices) PrintIndicies(normal);
  }
 
  free(arg[2]);
  free(arg);
  free(pi1);
  finalize_session(dsrg);
}


