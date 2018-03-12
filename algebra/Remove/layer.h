#ifdef ___LAYER___

#	define WHOLE_NET_LAYER_MASK	1
#	define LAYERS_PER_ELEMENT (sizeof(LayerListElement) << 3)
#	define CARRYMASK_IN	(1<<(LAYERS_PER_ELEMENT - 1))
#	define CARRYMASK_OUT	((Layer)1)
#	define LayerCopy(d,s) 		memcpy(d,s,sizeof(LayerListElement)*layerListElements)

void ReleaseLayerName(int);
char *NewLayerName(int , char *);
LayerPun LayerRealloc(LayerPun);
void SetLayerName(int ln, char *);
char *GetLayerName(int);
void DeleteLayerName(Layer);
void ReallocLayersNames(void);
void ReleaseLayersNames(void);
void SetLayersNumber(Layer);
int HowManyLayers(void);
void SetLayer2LayerList(int , LayerPun);
LayerPun NewLayerList(int , LayerPun);
void MakeLayerVisible(int);
void SetInitialLayerDimension(int);
//	Boolean IsLayerListVisible(LayerPun );
void ResetVisibleLayers(LayerPun);
void ResetAllLayersInList(LayerPun);
void AddVisibleLayers2LayerList(LayerPun);
void SetAllLayersInList(LayerPun);
int TestLayer(int , LayerPun);
//	Boolean IsLayerVisible(int );
void ResetLayer2LayerList(int , LayerPun);
void DeleteLayerFromLayerList(int , LayerPun);
void ReallocAllObjectsLayerLists(void);
Layer NewLayer(char *);
void DeleteLayers(void);
void ReinitializeLayers(void);
void ResetSelectedLayer(struct net_object *);
void AddSelectedLayer(void);
void InitLayer(void);
void ReallocViewLayerButton(void);
void SetLayer2ViewLayerList(int);
void ResetLayer2ViewLayerList(int);
void ResetAllLayersInViewLayerList(void);
void SetAllLayersInViewLayerList(void);
int GetLayersNumber(void);
void ReallocViewLayerList(void);
int GetLayerListElements(void);

#else
#	ifndef 	__LAYER__
#	define	__LAYER__

#		define LAYERS_PER_ELEMENT (sizeof(LayerListElement) << 3)

extern void ReleaseLayerName(int);
extern char *NewLayerName(int , char *);
extern LayerPun LayerRealloc(LayerPun);
extern void SetLayerName(int ln, char *);
extern char *GetLayerName(int);
extern void DeleteLayerName(Layer);
extern void ReallocLayersNames(void);
extern void ReleaseLayersNames(void);
extern void SetLayersNumber(Layer);
extern int HowManyLayers(void);
extern void SetLayer2LayerList(int , LayerPun);
extern LayerPun NewLayerList(int , LayerPun);
extern void MakeLayerVisible(int);
extern void SetInitialLayerDimension(int);
//		extern Boolean IsLayerListVisible(LayerPun );
extern void ResetVisibleLayers(LayerPun);
extern void ResetAllLayersInList(LayerPun);
extern void AddVisibleLayers2LayerList(LayerPun);
extern void SetAllLayersInList(LayerPun);
//		extern Boolean TestLayer(int ,LayerPun );
//		extern Boolean IsLayerVisible(int );
extern void ResetLayer2LayerList(int , LayerPun);
extern void DeleteLayerFromLayerList(int , LayerPun);
extern void ReallocAllObjectsLayerLists(void);
extern Layer NewLayer(char *);
extern void DeleteLayers(void);
extern void ReinitializeLayers(void);
extern void ResetSelectedLayer(struct net_object *);
extern void AddSelectedLayer(void);
extern void InitLayer(void);
extern void ReallocViewLayerButton(void);
extern void SetLayer2ViewLayerList(int);
extern void ResetAllLayersInViewLayerList(void);
extern int GetLayersNumber(void);
extern void ResetLayer2ViewLayerList(int);
extern void SetAllLayersInViewLayerList(void);
extern void ReallocViewLayerList(void);
extern int GetLayerListElements(void);

int TestLayer(int layer, LayerPun layerList);
void SetLayer2LayerList(int layer, LayerPun layerList);
void SetLayersNumber(Layer num);

#	endif
#endif
