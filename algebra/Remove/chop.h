#ifdef ___CHOP___

void ChopPlace(float *, float *, float , float);
void ChopTrans(float *, float *t_y, float , float , int);
void FNormTrans(struct trans_object *, float , float , float *, float *);
void RNormTrans(struct trans_object *, float , float , float *, float *);

#else
#	ifndef 	__CHOP__
#	define	__CHOP__

extern void ChopPlace(float *, float *, float , float);
extern void ChopTrans(float *, float *t_y, float , float , int);
extern void FNormTrans(struct trans_object *, float , float , float *, float *);
extern void RNormTrans(struct trans_object *, float , float , float *, float *);

#	endif
#endif
