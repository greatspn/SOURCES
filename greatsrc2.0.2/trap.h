#ifdef ___TRAP___

struct p_i_object {
    struct place_object *p_p;
    struct p_i_object *next;
};
struct p_i_l_type {
    struct p_i_object *i_l;
    struct p_i_l_type *next;
};
void ClearTrap(void);
void CollectTrap(int);
void DehighlightTrap(void);
void ShowTrap(XButtonEvent *);

static void DisplayTrap(void);
static void HighlightTrap(struct place_object *);


#else
#	ifndef 	__TRAP__
#	define	__TRAP__

extern 	void ClearTrap(void);
extern void CollectTrap(int);
extern void DehighlightTrap(void);
extern void ShowTrap(XButtonEvent *);

#	endif
#endif
