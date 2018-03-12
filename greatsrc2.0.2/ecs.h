#ifdef ___ECS___

struct t_i_object {
    struct trans_object *t_p;
    struct t_i_object *next;
};

struct t_i_l_type {
    struct t_i_object *i_l;
    struct t_i_l_type *next;
};

void ClearEcs(void);
void CollectEcsSets(int);
void DehighlightEcs(void);
void ShowEcs(XButtonEvent *);

static void EndConf(char *);
static void HighlightEcs(struct trans_object *);
static void DisplayEcs(void);

#else
#	ifndef 	__ECS__
#	define	__ECS__

extern void ClearEcs(void);
extern void CollectEcsSets(int);
extern void DehighlightEcs(void);
extern void ShowEcs(XButtonEvent *);

#	endif
#endif
