#ifdef ___CC___

struct t_i_object {
    struct trans_object *t_p;
    struct t_i_object *next;
};

struct t_i_l_type {
    struct t_i_object *i_l;
    struct t_i_l_type *next;
};

void CollectCcSets(int);
void DehighlightCc(void);
void ShowCc(XButtonEvent *);
static void DisplayCc(struct t_i_object *);
static void HighlightCc(struct trans_object *);
static void ClearCc(void);

#else
#	ifndef 	__CC__
#	define	__CC__

extern void CollectCcSets(int);
extern void DehighlightCc(void);
extern void ShowCc(XButtonEvent *);

#	endif
#endif
