#ifdef ___DEADL___

struct p_i_object {
    struct place_object *p_p;
    struct p_i_object *next;
};

struct p_i_l_type {
    struct p_i_object *i_l;
    struct p_i_l_type *next;
};

void ClearDeadl(void);
void CollectDeadl(int);
void DehighlightDeadl(void);
void ShowDeadl(XButtonEvent *);

static void DisplayDeadl(void);
static void HighlightDeadl(struct place_object *);

#else
#	ifndef 	__DEADL__
#	define	__DEADL__

extern 	void ClearDeadl(void);
extern void CollectDeadl(int);
extern void DehighlightDeadl(void);
extern void ShowDeadl(XButtonEvent *);

#	endif
#endif
