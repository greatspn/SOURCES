#ifdef ___SC___

struct t_i_object {
    struct trans_object *t_p;
    struct t_i_object *next;
};

struct t_i_l_type {
    struct t_i_object *i_l;
    struct t_i_l_type *next;
};

void ClearSc(void);
void CollectScsSets(int);
void DehighlightSc(void);
void ShowSc(XButtonEvent *);

static void DisplaySc(struct t_i_object *);
static void HighlightSc(struct trans_object *);

#else
#	ifndef 	__SC__
#	define	__SC__

extern void ClearSc(void);
extern void CollectScsSets(int);
extern void DehighlightSc(void);
extern void ShowSc(XButtonEvent *);

#	endif
#endif
