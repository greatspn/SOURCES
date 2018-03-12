#ifndef __SOLVER_STRUCT_H__
#define __SOLVER_STRUCT_H__

// representation of an entry  
// of the transition matrix
typedef struct mtrx_itm {
  unsigned long fm;
  double rt;
}itm ;

// representation of a column 
// of the transition matrix
typedef struct mtrx_col {
  unsigned long ninn;
  itm * first;
}mtrx ;



#endif /* __SOLVER_STRUCT_H__ */
