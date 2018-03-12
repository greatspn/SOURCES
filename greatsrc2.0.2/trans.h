#ifdef ___TRANS___

void NewImtrans(XButtonEvent *ie);
void NewTitrans(XButtonEvent          *ie);
void ChangeTransKind(XButtonEvent  *ie);
void RotateTrans(XButtonEvent  *ie);
void DeleteTrans(XButtonEvent *ie);
void RemoveTrans(struct trans_object *trans);

#else
#	ifndef 	__TRANS__
#	define	__TRANS__

void NewImtrans(XButtonEvent *);
void NewTitrans(XButtonEvent *);
void ChangeTransKind(XButtonEvent *);
void RotateTrans(XButtonEvent *);
void DeleteTrans(XButtonEvent *);
void RemoveTrans(struct trans_object *);

#	endif
#endif
