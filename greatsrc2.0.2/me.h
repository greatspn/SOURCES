#ifdef ___ME___

struct t_i_object {
    struct trans_object *t_p;
    struct t_i_object *next;
};

struct t_i_l_type {
    struct t_i_object *i_l;
    struct t_i_l_type *next;
};

void ClearMe(void);
void CollectMesets(int);
void DehighlightMe(void);
void ShowMe(XButtonEvent *);

static void DisplayMe(struct t_i_object *);
static void HighlightMe(struct trans_object *);

#else
#	ifndef 	__ME__
#	define	__ME__

extern	void ClearMe(void);
extern void CollectMesets(int);
extern void DehighlightMe(void);
extern void ShowMe(XButtonEvent *);

#	endif
#endif
