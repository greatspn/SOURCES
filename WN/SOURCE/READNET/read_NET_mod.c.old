/* This is a comment to test CVS */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/macros.h"

extern int parser();
extern struct MD_RATE *get_md_rate();
extern void fill_transition_data_structure();
extern void out_error();
extern void *ecalloc();
extern char *emalloc();
extern FILE *efopen();

extern int has_repetitions();
extern int get_proj_num();

extern int read_domain[];  /* In wn.yac */


static void read_transition () ;

static FILE *net_fp = NULL;
static FILE *parse_fp = NULL;
#ifdef LIBSPOT
static FILE * propA = NULL ;
#endif
static char *name_p = NULL;
static char prs[MAXSTRING];

 
static int char_read;
static int skip_layer;
static int int_val;
static float xcoord1,ycoord1;

static int ntrSave = 0 ;



extern FILE *jjin,*jjout ;

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void parse_node(type,pos,molteplicity,place_ind,string_to_parse)
 int type;
 int pos;
 int molteplicity;
 int place_ind;
 char *string_to_parse;
 {/* Init parse_node */
  int comp,ii,jj,pl;

  Node_p node = NULL;
  Coeff_p func_ptr = NULL;

  switch(type)
   {/* Tipo di posti */
    case INPUT	   : node = tabt[ntr].inptr;
		     break;
    case OUTPUT    : node = tabt[ntr].outptr;
		     break;
    case INHIBITOR : node = tabt[ntr].inibptr;
		     break;
   }/* Tipo di posti */

  for(ii=1;ii<pos;node=NEXT_NODE(node),ii++);

  if(molteplicity>=0)
   node->molt = molteplicity;  /* Init campo */
  else
   node->molt = -molteplicity;  /* Init campo */
  node->analyzed = FALSE;     /* Init campo */

  node->place_no = pl = place_ind - 1; /* Init campo */

  switch(*(string_to_parse))
   {/* Tipo di descrizione della funzione */
      case '@'  : {
	           /************************************/
  	           /*  Da implementare adattando la	   */
	           /*  grammatica affinche'riconosca   */
	           /*  come color anche descrizioni    */
	           /*  di funzioni riempiendo una ta   */
	           /*  bella			   */
	           /************************************/
  	           break;
  	          }
      case '\n' : {/* Nessuna funzione */
#ifdef SWN
                   if(IS_NEUTRAL(pl))
                    {/* Posto con dominio neutro */
                     node->arcfun = NULL;	   /* Init campo */
                    }/* Posto con dominio neutro */
                   else
                    {/* Posto con dominio colorato */
		     comp = GET_PLACE_COMPONENTS(pl);
		     if(has_repetitions(pl))
		      {/* Transizione con dominio con ripetizioni */
		       out_error(ERROR_REPETITIONS_AND_NO_FUNCTION,pl,ntr,0,0,NULL,NULL);
		      }/* Transizione con dominio con ripetizioni */
		     for(ii=0;ii<comp;ii++)
                      if(get_proj_num("?",tabp[pl].dominio[ii])!=1)
		       {/* Transizione con dominio con ripetizioni */
			out_error(ERROR_AMBIGUOUS_DEFINITION,pl,ntr,0,0,NULL,NULL);
		       }/* Transizione con dominio con ripetizioni */
		     func_ptr = (Coeff_p)emalloc(sizeof(struct ENN_COEFF));
		     func_ptr->enn_coef = 1;
		     func_ptr->card = 1;
		     func_ptr->guard = NULL;
		     func_ptr->next = NULL;
		     func_ptr->coef = (struct COEFF *)ecalloc(comp,sizeof(struct COEFF));
		     for(ii=0;ii<comp;ii++)
                      {/* Settaggio dei parametri dell'identita' */
		       func_ptr->coef[ii].xsucc_coef=(int *)ecalloc(3,sizeof(int));
		       func_ptr->coef[ii].xsucc_coef[0]=1;
		       func_ptr->coef[ii].xsucc_coef[1]=0;
		       func_ptr->coef[ii].xsucc_coef[2]=0;
		       func_ptr->coef[ii].sbc_coef=(int *)ecalloc(tabc[tabp[pl].dominio[ii]].sbc_num,sizeof(int));
		       for(jj=0;jj<tabc[tabp[pl].dominio[ii]].sbc_num ; jj++)
		        func_ptr->coef[ii].sbc_coef[jj] = 0;
                      }/* Settaggio dei parametri dell'identita' */
		     node->arcfun = func_ptr; /* Init campo */
                    }/* Posto con dominio colorato */
#endif
	           break;
	          }/* Nessuna funzione */
       default  : {/* Arco con funzione */

                     parse_DEF = FUNCTION;
                     name_p = string_to_parse;
		     sscanf(string_to_parse,"%d %f %f %n",&int_val,&xcoord1,&ycoord1,&char_read);
		     name_p = string_to_parse + char_read;
	             fun_ptr = NULL; /**** ARco con funz. in GSPN ********/
	             pl_ind = node->place_no;  /* Init campo */
                     parse_fp = efopen(prs,"w");
                     fprintf(parse_fp,"~f %s",name_p);
                     fclose(parse_fp);
                     jjin = efopen(prs,"r");
#ifdef BASTA_CON_LINUX
#ifdef LINUX
                     jjout = stderr;
#else
                     jjout = efopen("/dev/null","w");
#endif
#endif
                     parser();
                     fclose(jjin);
#ifdef BASTA_CON_LINUX
                     fclose(jjout);
#endif
		     if(fun_ptr != NULL)
	              node->arcfun = fun_ptr;	 /* Init campo */
		   break;
	          }/* Arco con funzione */
  }/* Tipo di descrizione della funzione */
}/* End parse_node */
#ifdef SWN
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int has_repetitions(p)
 int p;
 {/* Init has repetitions */
  int i,j;

  for(i=0;i<tabp[p].comp_num;i++)
   for(j=0;j<tabp[p].comp_num;j++)
    if(j != i)
     if(tabp[p].dominio[i]==tabp[p].dominio[j])
      return(TRUE);
  return(FALSE);
 }/* End has repetitions */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void init_nodes(type,num)
 int type;
 int num;
 {/* Init init_nodes */
  int ii;
  Node_p place_list = NULL;
  Node_p node_ptr = NULL;

  if(num > 0)
   {/* Almeno un nodo */
    for(ii = 0; ii < num ; ii++)
     {/* Inizializzazione */
      node_ptr = (Node_p)emalloc(sizeof(struct NODO));
      node_ptr->arcfun = NULL;
      node_ptr->molt = UNKNOWN;
      node_ptr->fun_card = UNKNOWN;
      node_ptr->place_no = UNKNOWN;
      node_ptr->skip = UNKNOWN;
      node_ptr->analyzed = UNKNOWN;
      node_ptr->type = UNKNOWN;
      node_ptr->involved_in_CC_SC = FALSE;
      node_ptr->test_arc = FALSE;
      node_ptr->next = place_list;
      place_list = node_ptr;
     }/* Inizializzazione */
   }/* Almeno un nodo */
  else
   place_list = NULL;
  switch(type)
   {/* Tipo di posti */
    case INPUT	: tabt[ntr].inptr = place_list;
		  break;
    case OUTPUT : tabt[ntr].outptr = place_list;
		  break;
    case INHIBITOR : tabt[ntr].inibptr = place_list;
		  break;
   }/* Tipo di posti */
 }/* End init_nodes */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void init_marking_parameters(num)
 int num;
 {/* Init init_marking_parameters */
  int i;

  for(i = 0; i < num; i++)
   {/* Per ogni elemento della tabella */
    tabmp[i].mark_name = NULL;
    tabmp[i].mark_val = UNKNOWN;
   }/* Per ogni elemento della tabella */
 }/* End init_marking_parameters */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void init_places(num)
 int num;
 {/* Init init_places */
  int i;

  for(i = 0; i < num; i++)
   {/* Per ogni elemento della tabella */
    tabp[i].place_name = NULL;
#ifdef SWN
    tabp[i].dominio = NULL;
#endif
    tabp[i].comp_num = UNKNOWN;
    tabp[i].card = UNKNOWN;
    tabp[i].position= UNKNOWN;
#ifdef GREATSPN
    tabp[i].tagged = TRUE;
#else
    tabp[i].tagged = FALSE;
#endif
   }/* Per ogni elemento della tabella */
 }/* End init_places */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void init_rate_parameters(num)
 int num;
 {/* Init init_rate_parameters */
  int i;

  for(i = 0; i < num; i++)
   {/* Per ogni elemento della tabella */
    tabrp[i].rate_name = NULL;
    tabrp[i].rate_val = UNKNOWN;
   }/* Per ogni elemento della tabella */
 }/* End init_rate_parameters */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void init_groups(num)
 int num;
 {/* Init init_groups */
  int i;

  for(i = 0; i < num; i++)
   {/* Per ogni elemento della tabella */
    tabg[i].name = NULL;
    tabg[i].priority = UNKNOWN;
   }/* Per ogni elemento della tabella */
 }/* End init_groups */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void init_transitions(num)
 int num;
 {/* Init init_transitions */
  int i;

  for(i = 0; i < num; i++)
   {/* Per ogni elemento della tabella */
    tabt[i].trans_name = NULL;
#ifdef SWN
    tabt[i].names = NULL;
#endif

    tabt[i].in_arc = UNKNOWN;
    tabt[i].out_arc = UNKNOWN ;
    tabt[i].inib_arc = UNKNOWN ;

#ifdef GREATSPN
    tabt[i].tagged = TRUE;
#else
    tabt[i].tagged = FALSE;
#endif
    tabt[i].reset = FALSE;
#ifdef SWN
#ifdef SIMULATION
    tabt[i].skippable = UNKNOWN;
    tabt[i].en_deg = UNKNOWN;
#endif
#endif
    tabt[i].no_serv = UNKNOWN;
    tabt[i].pri = UNKNOWN ;
    tabt[i].timing = UNKNOWN;

#ifdef SWN
    tabt[i].comp_num = UNKNOWN;
    tabt[i].dominio = NULL;
    tabt[i].comp_type = NULL;
    tabt[i].rip = NULL;
    tabt[i].off = NULL;
#ifdef SYMBOLIC
#ifdef SIMULATION
    tabt[i].split_type = NULL;
    tabt[i].pre_split = UNKNOWN;
#endif
#endif
#endif
    tabt[i].add_l = NULL;
    tabt[i].test_l = NULL;
    tabt[i].me_l = NULL;

    tabt[i].inptr = NULL;
    tabt[i].outptr = NULL ;
    tabt[i].inibptr = NULL ;

    tabt[i].guard = NULL ;
    tabt[i].mean_t = UNKNOWN;

    tabt[i].md_rate_val = NULL;

#ifdef SIMULATION

    tabt[i].instances_h = NULL;
    tabt[i].instances_t = NULL;

    tabt[i].enabl_next = NULL;

#ifndef SYMBOLIC
    tabt[i].d_instances_h = NULL;
    tabt[i].d_instances_t = NULL;
    tabt[i].dist = NULL;
    tabt[i].stages = 0;
    tabt[i].policy = UNKNOWN;
    tabt[i].deschedule_policy = UNKNOWN;
    tabt[i].reuse_policy = UNKNOWN;
#endif
#endif

   }/* Per ogni elemento della tabella */
 }/* End init_transitions */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void read_NET_file()
 {/* Init read_NET_file */
  
  char tmp[MAXSTRING];
  char trash[MAXSTRING];
  char type;
  int item,item_skip;
  float xcoord2,ycoord2;
  double float_val;

 
  sprintf(tmp,"%spar",net_name);
  sprintf(prs,"%sparse",net_name);

  net_fp = efopen(tmp,"r");
  while( TRUE )
   {/* Scanning .def file */
    if( fgets( tmp, MAXSTRING - 1, net_fp ) == NULL )
     if( feof( net_fp ) )
      break;
    if(tmp[0]=='|' && tmp[1]=='\n')
     {/* Parsing della prima riga */
      fscanf(net_fp,"%c %d %d %d %d %d %d",&type,&el[0],&el[1],&el[2],&el[4],&el[3],&el[5]);
      ntrSave = el[4];
#ifdef LIBSPOT
#ifndef ESYMBOLIC
      char tmp2 [256]; 
      sprintf (tmp2,"%stobs",net_name);
      propA = efopen (tmp2,"r");
      fscanf(propA,"%d\n",&nSpottr);
      el[4] = el[4] +nSpottr;      
#endif
#endif 
      if(el[4]>MAX_TRANS) 
       out_error(ERROR_LOW_MAX_TRANS,0,0,0,0,NULL,NULL);
      while(getc(net_fp)!='\n');
      break;
     }/* Parsing della prima riga */
   }/* Scanning .def file */
  /*********** MARKING PARAMETERS ***********/
  if(el[0] > 0)
   {/* Ci sono marking parameters */
    tabmp=(struct MARK_PAR *)ecalloc(el[0],sizeof(struct MARK_PAR));
    init_marking_parameters(el[0]);
    for(item = 1; item <= el[0]; item++)
     {/* Lettura dei Marking Parameters */
      fgets( tmp, MAXSTRING - 1, net_fp );

      sscanf(tmp,"%s %d %f %f %s",read_name,&int_val,&xcoord1,&ycoord1,&trash);
      tabmp[nmp].mark_name = (char *)ecalloc(strlen(read_name)+1,sizeof(char));
      strcpy(tabmp[nmp].mark_name,read_name);
      tabmp[nmp++].mark_val = int_val;
     }/* Lettura dei Marking Parameters */
   }/* Ci sono marking parameters */
  /*********** PLACES ***********/
  if(el[1] > 0)
   {/* Ci sono posti */
    tabp=(struct PLACES *)ecalloc(el[1],sizeof(struct PLACES));
    init_places(el[1]);
    for(item = 1; item <= el[1]; item++)
     {/* Lettura dei Posti */
      fgets( tmp, MAXSTRING - 1, net_fp );

      sscanf(tmp,"%s %d %f %f %f %f %n",read_name,&int_val,&xcoord1,&ycoord1,&xcoord2,&ycoord2,&char_read);
      tabp[npl].place_name = (char *)ecalloc(strlen(read_name)+1,sizeof(char));
      strcpy(tabp[npl].place_name,read_name);

      name_p = tmp + char_read ;
      do
       {
        sscanf(name_p,"%d%n",&skip_layer,&char_read);
        name_p+=char_read;
       }
      while (skip_layer);


      switch(*(name_p))
       {/* Tipo di dominio del posto */
        case '@'  : {
            	     /************************************/
	             /*  Da implementare adattando la	  */
	             /*  grammatica affinche'riconosca    */
	             /*  come color anche descrizioni     */
	             /*  di domini riempiendo una tabella */
	             /************************************/
	             break;
	            }
        case '\n' : {/* Dominio neutro */
            	     tabp[npl].comp_num=0;
	             break;
	            }/* Dominio neutro*/
        default   : {/*Dominio colorato da parsificare*/
                     parse_DEF = PLACE;
                     while(!isalpha(*name_p))
                      *name_p++;
                     parse_fp = efopen(prs,"w");
                     fprintf(parse_fp,"~w %s",name_p);
                     fclose(parse_fp);
                     jjin = efopen(prs,"r");
#ifdef BASTA_CON_LINUX
#ifdef LINUX
                     jjout = stderr;
#else
                     jjout = efopen("/dev/null","w");
#endif
#endif
                     parser();
                     fclose(jjin);
#ifdef BASTA_CON_LINUX
                     fclose(jjout);
#endif
#ifdef SWN
	             tabp[npl].dominio = (int *)ecalloc(tabp[npl].comp_num,sizeof(int));
	             for(item_skip = 0; item_skip < tabp[npl].comp_num ; item_skip++)
		      tabp[npl].dominio[item_skip] = read_domain[item_skip];
#endif
	            }/*Dominio colorato da parsificare*/
        }/* Tipo di dominio del posto */
#ifdef SWN
       for(item_skip=0,tabp[npl].card=1;item_skip<tabp[npl].comp_num;item_skip++)
        tabp[npl].card*=tabc[tabp[npl].dominio[item_skip]].card;
#endif
       tabp[npl].position=int_val;
       npl++;
     }/* Lettura dei Posti */
   }/* Ci sono posti */
  /*********** RATE PARAMETERS ***********/
  if(el[2] > 0)
   {/* Ci sono rate parameters */
    tabrp=(struct RATE_PAR *)ecalloc(el[2],sizeof(struct RATE_PAR));
    init_rate_parameters(el[2]);
    for(item = 1; item <= el[2]; item++)
     {/* Lettura dei Rate Parameters */
      fgets( tmp, MAXSTRING - 1, net_fp );

      sscanf(tmp,"%s %lg %f %f %s",read_name,&float_val,&xcoord1,&ycoord1,&trash);
      tabrp[nrp].rate_name = (char *)ecalloc(strlen(read_name)+1,sizeof(char));
      strcpy(tabrp[nrp].rate_name,read_name);
      tabrp[nrp++].rate_val=float_val;
     }/* Lettura dei Rate Parameters */
   }/* Ci sono rate parameters */
  /*********** PRIORITY GROUPS ***********/
  if(el[3] > 0)
   {/* Ci sono groups */
    tabg=(struct GROUP *)ecalloc(el[3],sizeof(struct GROUP));
    init_groups(el[3]);
    for(item = 1; item <= el[3]; item++)
     {/* Lettura dei Groups */
      fgets( tmp, MAXSTRING - 1, net_fp );

      sscanf(tmp,"%s %f %f %d",read_name,&xcoord1,&ycoord1,&int_val);
      tabg[ngr].name = (char *)ecalloc(strlen(read_name)+1,sizeof(char));
      strcpy(tabg[ngr].name,read_name);
      tabg[ngr++].priority=int_val;
     }/* Lettura dei Groups */
   }/* Ci sono groups */
  /*********** TRANSITIONS ***********/
  if(el[4] > 0)
   {/* Ci sono transizioni */
    tabt=(Trans_p)ecalloc(el[4],sizeof(struct TRANS));
#ifdef LIBSPOT
#ifdef ESYMBOLIC 

    tabt_sym=(Trans_p)ecalloc(el[4],sizeof(struct TRANS));
#endif
#endif

#ifdef SWN
    occ=(struct FUNBUF **)ecalloc(el[4],sizeof(struct FUNBUF *));
    for(item = 0; item < el[4]; item++)
     occ[item]=(struct FUNBUF *)ecalloc(MAX_DOMAIN,sizeof(struct FUNBUF));
#endif

    init_transitions(el[4]);
#ifdef LIBSPOT
#ifdef ESYMBOLIC 
   Trans_p tabt_store=tabt;
    tabt=tabt_sym;
    init_transitions(el[4]);
    tabt=tabt_store;
#endif
#endif
    for(item = 1; item <= ntrSave; item++)
     {
       read_transition ();
     }

#ifdef LIBSPOT
#ifndef ESYMBOLIC
    fclose(net_fp);
    net_fp = propA;
    for(item = 1; item <= nSpottr ; item++)
     {
       read_transition ();
     }
    ntr = ntrSave;
#endif
#endif
   }/* Ci sono transizioni */
  fclose(net_fp);
  /*system("rm nets/parse_temp");*/
}/* End read_NET_file */



void read_transition () {

  char tmp [MAXSTRING] ;
  int int_val1,int_val2,int_val3,int_val4;
  float xcoord2,ycoord2;
  float xcoord3,ycoord3;
  int item_arc,item_skip;
  int rte;
  double float_val;

#ifdef LIBSPOT
#ifdef ESYMBOLIC
  int old_pos,new_pos;
  Trans_p tabt_store=NULL;
  old_pos=ftell(net_fp);
#endif
#endif

  /* Lettura delle transizioni */
  fgets( tmp, MAXSTRING - 1, net_fp );
  
  sscanf(tmp,"%s %lg %d %d %d %d %f %f %f %f %f %f %n",
	 read_name,
	 &float_val,
	 &int_val1,&int_val2,&int_val3,&int_val4,
	 &xcoord1,&ycoord1,&xcoord2,&ycoord2,&xcoord3,&ycoord3,
	 &char_read);
  tabt[ntr].trans_name = (char *)ecalloc(strlen(read_name)+1,sizeof(char));
  strcpy(tabt[ntr].trans_name,read_name);
  

#ifdef ESYMBOLIC
  /***************** For_ESRG ************/
  tabt[ntr].trans_type=SYM_T;
  /***************************************/
#endif
  
  if(float_val==-510)
    { /*tabt[ntr].md_rate_val=get_md_rate(ntr);*/}
  else
    if(float_val<0)
      {
	rte = -float_val - 1;
	tabt[ntr].mean_t=tabrp[rte].rate_val;
      }
    else
      tabt[ntr].mean_t = float_val;
  tabt[ntr].pri=int_val2;
  if(tabt[ntr].pri == 0)
    tabt[ntr].timing = TIMING_EXPONENTIAL;
  else
    tabt[ntr].timing = TIMING_IMMEDIATE;
  tabt[ntr].no_serv=int_val1;
  if(int_val1 == INFINITE_SERVER)
    tabt[ntr].no_serv = 0;
  tabt[ntr].in_arc=int_val3;
  comp_check=0;
  
  name_p = tmp + char_read;
  do
    {
      sscanf(name_p,"%d%n",&skip_layer,&char_read);
      name_p+=char_read;
    }
  while (skip_layer);
  
  switch(*(name_p))
    {/* Tipo di dominio della transizione */
    case '@'  : {
      /************************************/
      /*  Da implementare adattando la	   */
      /*  grammatica affinche'riconosca   */
      /*  come color anche descrizioni    */
      /*  di domini riempiendo una tabella*/
      /************************************/
      break;
    }
    case '\n' : {/* Dominio neutro*/
      break;
    }/* Dominio neutro*/
    default   : {/*Dominio colorato da parsificare*/
      parse_DEF = TRANSITION;
      while((*name_p)!='[' && (*name_p)!='#' )
	*name_p++;
      parse_fp = efopen(prs,"w");
      fprintf(parse_fp,"~k %s",name_p);
      fclose(parse_fp);
      jjin = efopen(prs,"r");
#ifdef BASTA_CON_LINUX
#ifdef LINUX
      jjout = stderr;
#else
      jjout = efopen("/dev/null","w");
#endif
#endif
      parser();


      fclose(jjin);
#ifdef BASTA_CON_LINUX
      fclose(jjout);
#endif
    }/*Dominio colorato da parsificare*/
    }/* Tipo di dominio della transizione */
  /*********** INPUT ARCS *************/
  input_flag=FALSE;
  init_nodes(INPUT,int_val3);
  if(tabt[ntr].in_arc!=0)
    {/* Ci sono archi di ingresso */
      input_flag=TRUE;
      for(item_arc = 1; item_arc <= tabt[ntr].in_arc; item_arc++)
	{/* Lettura di ogni arco di ingresso */
          fgets( tmp, MAXSTRING - 1, net_fp );
	  
          sscanf(tmp,"%d %d %d %n",
		 &int_val1,&int_val2,&int_val3,
		 &char_read);
          name_p = tmp + char_read;
          do
	    {
	      sscanf(name_p,"%d%n",&skip_layer,&char_read);
	      name_p+=char_read;
	    }
          while (skip_layer);
          parse_node(INPUT,item_arc,int_val1,int_val2,name_p);
          for(item_skip = 1; item_skip <= int_val3; item_skip++)
	    fgets( tmp, MAXSTRING - 1, net_fp ); 
	}/* Lettura di ogni arco di ingresso */
    }/* Ci sono archi di ingresso */
  /*********** OUTPUT ARCS *************/
  fgets( tmp, MAXSTRING - 1, net_fp ); 
  sscanf(tmp,"%d",&int_val3);
  
  input_flag=FALSE;
  tabt[ntr].out_arc=int_val3;
  init_nodes(OUTPUT,int_val3);
  if(tabt[ntr].out_arc!=0)
    {/* Ci sono archi di uscita */
      for(item_arc = 1; item_arc <= tabt[ntr].out_arc; item_arc++)
	{/* Lettura di ogni arco di ingresso */
          fgets( tmp, MAXSTRING - 1, net_fp );
	  
          sscanf(tmp,"%d %d %d %n",
		 &int_val1,&int_val2,&int_val3,
		 &char_read);
          name_p = tmp + char_read;
          do
	    {
	      sscanf(name_p,"%d%n",&skip_layer,&char_read);
	      name_p+=char_read;
	    }
          while (skip_layer);
          parse_node(OUTPUT,item_arc,int_val1,int_val2,name_p);
          for(item_skip = 1; item_skip <= int_val3; item_skip++)
	    fgets( tmp, MAXSTRING - 1, net_fp ); 
	}/* Lettura di ogni arco di ingresso */
    }/* Ci sono archi di uscita */
  /*********** INHIBITOR ARCS *************/
  fgets( tmp, MAXSTRING - 1, net_fp ); 
  sscanf(tmp,"%d",&int_val3);
  
  input_flag=FALSE;
  tabt[ntr].inib_arc=int_val3;
  init_nodes(INHIBITOR,int_val3);
  if(tabt[ntr].inib_arc!=0)
    {/* Ci sono archi inibitori */
      for(item_arc = 1; item_arc <= tabt[ntr].inib_arc; item_arc++)
	{/* Lettura di ogni arco di ingresso */
          fgets( tmp, MAXSTRING - 1, net_fp );
	  
          sscanf(tmp,"%d %d %d %n",
		 &int_val1,&int_val2,&int_val3,
		 &char_read);
          name_p = tmp + char_read ;
          do
	    {
	      sscanf(name_p,"%d%n",&skip_layer,&char_read);
	      name_p+=char_read;
	    }
          while (skip_layer);
          parse_node(INHIBITOR,item_arc,int_val1,int_val2,name_p);
          for(item_skip = 1; item_skip <= int_val3; item_skip++)
	    fgets( tmp, MAXSTRING - 1, net_fp ); 
	}/* Lettura di ogni arco di ingresso */
    }/* Ci sono archi inibitori */
  fill_transition_data_structure(ntr);
#ifdef LIBSPOT
#ifdef ESYMBOLIC
  new_pos=ftell(net_fp);
  tabt_store=tabt;
  tabt=tabt_sym;
  read_transition_sym ( ntr,old_pos);
  tabt=tabt_store;
  fseek(net_fp,new_pos,0);  
#endif
#endif

  ntr++;
}/* Lettura delle transizioni */
  
 

#ifdef LIBSPOT
#ifdef ESYMBOLIC
void read_transition_sym (int ntr,int pos ){

  char tmp [MAXSTRING] ; 
  int int_val1,int_val2,int_val3,int_val4;
  float xcoord2,ycoord2;
  float xcoord3,ycoord3;
  int item_arc,item_skip;
  int rte;
  double float_val;

  /* Lettura delle transizioni */
  fseek(net_fp,pos,0);  
  fgets( tmp, MAXSTRING - 1, net_fp );
  
  sscanf(tmp,"%s %lg %d %d %d %d %f %f %f %f %f %f %n",
	 read_name,
	 &float_val,
	 &int_val1,&int_val2,&int_val3,&int_val4,
	 &xcoord1,&ycoord1,&xcoord2,&ycoord2,&xcoord3,&ycoord3,
	 &char_read);
  tabt[ntr].trans_name = (char *)ecalloc(strlen(read_name)+1,sizeof(char));
  strcpy(tabt[ntr].trans_name,read_name);
    
  if(float_val==-510)
    { }
  else
    if(float_val<0)
      {
	rte = -float_val - 1;
	tabt[ntr].mean_t=tabrp[rte].rate_val;
      }
    else
      tabt[ntr].mean_t = float_val;
  tabt[ntr].pri=int_val2;
  if(tabt[ntr].pri == 0)
    tabt[ntr].timing = TIMING_EXPONENTIAL;
  else
    tabt[ntr].timing = TIMING_IMMEDIATE;
  tabt[ntr].no_serv=int_val1;
  if(int_val1 == INFINITE_SERVER)
    tabt[ntr].no_serv = 0;
  tabt[ntr].in_arc=int_val3;
  comp_check=0;
  
  name_p = tmp + char_read;
  do
    {
      sscanf(name_p,"%d%n",&skip_layer,&char_read);
      name_p+=char_read;
    }
  while (skip_layer);
  
  switch(*(name_p))
    {/* Tipo di dominio della transizione */
    case '@'  : {
      /************************************/
      /*  Da implementare adattando la	   */
      /*  grammatica affinche'riconosca   */
      /*  come color anche descrizioni    */
      /*  di domini riempiendo una tabella*/
      /************************************/
      break;
    }
    case '\n' : {/* Dominio neutro*/
      break;
    }/* Dominio neutro*/
    default   : {/*Dominio colorato da parsificare*/
      parse_DEF = TRANSITION;
      while((*name_p)!=']'  )
	*name_p++;
   
    }/*Dominio colorato da parsificare*/
    }/* Tipo di dominio della transizione */
  /*********** INPUT ARCS *************/
  input_flag=FALSE;
  init_nodes(INPUT,int_val3);
  if(tabt[ntr].in_arc!=0)
    {/* Ci sono archi di ingresso */
      input_flag=TRUE;
      for(item_arc = 1; item_arc <= tabt[ntr].in_arc; item_arc++)
	{/* Lettura di ogni arco di ingresso */
          fgets( tmp, MAXSTRING - 1, net_fp );
	  
          sscanf(tmp,"%d %d %d %n",
		 &int_val1,&int_val2,&int_val3,
		 &char_read);
          name_p = tmp + char_read;
          do
	    {
	      sscanf(name_p,"%d%n",&skip_layer,&char_read);
	      name_p+=char_read;
	    }
          while (skip_layer);
          parse_node(INPUT,item_arc,int_val1,int_val2,name_p);
          for(item_skip = 1; item_skip <= int_val3; item_skip++)
	    fgets( tmp, MAXSTRING - 1, net_fp ); 
	}/* Lettura di ogni arco di ingresso */
    }/* Ci sono archi di ingresso */
  /*********** OUTPUT ARCS *************/
  fgets( tmp, MAXSTRING - 1, net_fp ); 
  sscanf(tmp,"%d",&int_val3);
  
  input_flag=FALSE;
  tabt[ntr].out_arc=int_val3;
  init_nodes(OUTPUT,int_val3);
  if(tabt[ntr].out_arc!=0)
    {/* Ci sono archi di uscita */
      for(item_arc = 1; item_arc <= tabt[ntr].out_arc; item_arc++)
	{/* Lettura di ogni arco di ingresso */
          fgets( tmp, MAXSTRING - 1, net_fp );
	  
          sscanf(tmp,"%d %d %d %n",
		 &int_val1,&int_val2,&int_val3,
		 &char_read);
          name_p = tmp + char_read;
          do
	    {
	      sscanf(name_p,"%d%n",&skip_layer,&char_read);
	      name_p+=char_read;
	    }
          while (skip_layer);
          parse_node(OUTPUT,item_arc,int_val1,int_val2,name_p);
          for(item_skip = 1; item_skip <= int_val3; item_skip++)
	    fgets( tmp, MAXSTRING - 1, net_fp ); 
	}/* Lettura di ogni arco di ingresso */
    }/* Ci sono archi di uscita */
  /*********** INHIBITOR ARCS *************/
  fgets( tmp, MAXSTRING - 1, net_fp ); 
  sscanf(tmp,"%d",&int_val3);
  
  input_flag=FALSE;
  tabt[ntr].inib_arc=int_val3;
  init_nodes(INHIBITOR,int_val3);
  if(tabt[ntr].inib_arc!=0)
    {/* Ci sono archi inibitori */
      for(item_arc = 1; item_arc <= tabt[ntr].inib_arc; item_arc++)
	{/* Lettura di ogni arco di ingresso */
          fgets( tmp, MAXSTRING - 1, net_fp );
	  
          sscanf(tmp,"%d %d %d %n",
		 &int_val1,&int_val2,&int_val3,
		 &char_read);
          name_p = tmp + char_read ;
          do
	    {
	      sscanf(name_p,"%d%n",&skip_layer,&char_read);
	      name_p+=char_read;
	    }
          while (skip_layer);
          parse_node(INHIBITOR,item_arc,int_val1,int_val2,name_p);
          for(item_skip = 1; item_skip <= int_val3; item_skip++)
	    fgets( tmp, MAXSTRING - 1, net_fp ); 
	}/* Lettura di ogni arco di ingresso */
    }/* Ci sono archi inibitori */
  fill_transition_data_structure(ntr);


 
}/* Lettura delle transizioni */
  
#endif
#endif
 
