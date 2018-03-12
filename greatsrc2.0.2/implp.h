#ifdef ___IMPLP___

struct p_i_object {
    int             mult;
    struct place_object *p_p;
    struct p_i_object *next;
};

struct p_i_l_type {
    struct p_i_object *i_l;
    struct p_i_l_type *next;
};

void DehighlightImplp(void);
void ShowImplp(XButtonEvent *);
void DisplayImplp(void);

static void ClearImplp(void);
static void CollectImplp(int);
static void HighlightImplp(struct place_object *);

#else
#	ifndef 	__IMPLP__
#	define	__IMPLP__

extern void DehighlightImplp(void);
extern void ShowImplp(XButtonEvent *);
extern void DisplayImplp(void);

#	endif
#endif
