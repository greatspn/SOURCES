typedef  unsigned long LayerListElement;
typedef LayerListElement *LayerPun;
typedef  unsigned Layer;

/***************************  graphical objects  *************************/

#define ZEROINIT	0
#define CURRENTVIEW	1
#define CURRENTVIEWANDWHOLE	2
#define DUPLICATE	3
#define WHOLENET	4

#define                          INV_PAINT_OP           GXxor
#define                          PAINT                  GXor


#define				 DRAW			1
#define				 ERASE			0

/***************************  global variables   *************************/

#define WHOLE_NET_LAYER ((Layer)0)

/**********************  canvas variables  ************************/


/****************************  Variables for Active Drawing  *************/


/**************************  Default Variables for Fixed sized objects  ******/


/****************************  MACROS  ****************************************/
/*
#define encode_layer(N) ((LAYER)( ((LAYER)1) << (N) ))
#define test_layer(N,L) ( (int)((encode_layer(N)) & L) )
*/
#define IN_TO_PIX(x)    ((x) * PIX_PER_INCH )	/* inches to pixels */
#define PIX_TO_IN(x)    (((float)(x)) / PIX_PER_INCH)	/* pixels to inches */

#define ABS(X)		(((X) >= 0 ) ?  (X) : -(X))

#define sign(X)		(((X) >= 0)  ?   1  :  -1 )



#ifndef MIN
#define MIN(A,B)	(((A) < (B)) ?  (A) :  (B))
#endif
#ifndef MAX
#define MAX(A,B)	(((A) < (B)) ?  (B) :  (A))
#endif


