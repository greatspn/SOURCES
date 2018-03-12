#ifdef ___LISP___

void ShowLisp(struct lisp_object *, int);
void TypeLispDef(struct lisp_object *);
void TypeLisp(XButtonEvent *);
void NewLisp(XButtonEvent *);
void EditLisp(XButtonEvent *);
void PutLisp(struct lisp_object *lisp, int op, DrawingSurfacePun ds, int x_off, int y_off);
#else
#	ifndef 	__LISP__
#	define	__LISP__

extern 	void ShowLisp(struct lisp_object *, int);
extern void TypeLispDef(struct lisp_object *);
extern void TypeLisp(XButtonEvent *);
extern void NewLisp(XButtonEvent *);
extern void EditLisp(XButtonEvent *);
extern void PutLisp(struct lisp_object *lisp, int op, DrawingSurfacePun ds, int x_off, int y_off);

#	endif
#endif
