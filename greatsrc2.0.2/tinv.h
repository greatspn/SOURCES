#ifdef ___TINV___

struct t_i_object {
    int             mult;
    struct trans_object *t_p;
    struct t_i_object *next;
};

struct t_i_l_type {
    struct t_i_object *i_l;
    struct t_i_l_type *next;
};

void ClearTinv(void);
void CollectTinvar(int);
void DehighlightTinv(void);
void ShowTinv(XButtonEvent *);
static void DisplayTinv(void);
static void HighlightTinv(struct trans_object *);

#else
#	ifndef 	__TINV__
#	define	__TINV__

extern void ClearTinv(void);
extern void CollectTinvar(int);
extern void DehighlightTinv(void);
extern void ShowTinv(XButtonEvent *);

#	endif
#endif
