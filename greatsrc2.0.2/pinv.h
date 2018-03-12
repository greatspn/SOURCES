#ifdef ___PINV___

struct p_i_object {
    int             mult;
    struct place_object *p_p;
    struct p_i_object *next;
};

struct p_i_l_type {
    struct p_i_object *i_l;
    struct p_i_l_type *next;
};

void ClearPinv(void);
void CollectPinvar(int);
void DehighlightPinv(void);
void ShowPinv(XButtonEvent *);

static void DisplayPinv(void);
static void HighlightPinv(struct place_object *);

#else
#	ifndef 	__PINV__
#	define	__PINV__

extern void ClearPinv(void);
extern void CollectPinvar(int);
extern void DehighlightPinv(void);
extern void ShowPinv(XButtonEvent *);

#	endif
#endif
