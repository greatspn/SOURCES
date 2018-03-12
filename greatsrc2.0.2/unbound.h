#ifdef ___UNBOUND___

struct t_i_object {
    int             mult;
    struct trans_object *trans;
    struct t_i_object *next;
};

struct p_i_object {
    struct place_object *place;
    struct p_i_object *next;
};

struct t_i_l_type {
    struct t_i_object *i_l;
    struct p_i_object *p_l;
    struct t_i_l_type *next;
};
void ClearUnbound(void);
void CollectUnbound(int);
void DehighlightUnbound(void);
void ShowUnbound(XButtonEvent *);

static void DisplayUnbound(void);
static void HighlightUnbound(struct position);

#else
#	ifndef 	__UNBOUND__
#	define	__UNBOUND__

extern	void ClearUnbound(void);
extern void CollectUnbound(int);
extern void DehighlightUnbound(void);
extern void ShowUnbound(XButtonEvent *);

#	endif
#endif
