#define ___LAYER___
#include "global.h"
#include <assert.h>
#include "layerviewdialog.h"
#include "layereditdialog.h"
#include <Mrm/MrmPublic.h>
#include <stdlib.h>
//#include <malloc.h>
#include <memory.h>
#include "layer.h"

/*
#define assert(expr)
*/

static int layerListElements;
static int oldElements;

static char **layer_name;
static int layersNumber;
static LayerPun viewLayerList;

static char *wholeName = "The Whole Net";

int GetLayerListElements(void) {
    return layerListElements;
}

int GetLayersNumber(void) {
    return layersNumber;
}

void SetLayer2ViewLayerList(int which) {
    SetLayer2LayerList(which, viewLayerList);
}

void ResetLayer2ViewLayerList(int which) {
    ResetLayer2LayerList(which, viewLayerList);
}

void ResetAllLayersInViewLayerList(void) {
    ResetAllLayersInList(viewLayerList);
}

void SetAllLayersInViewLayerList(void) {
    SetAllLayersInList(viewLayerList);
}

void ReleaseLayerName(int which) {
    free(layer_name[which]);
}

void ReallocViewLayerList(void) {
    viewLayerList = LayerRealloc(viewLayerList);
}



char *NewLayerName(int which, char *name) {
    layer_name[which] = emalloc((unsigned)(strlen(name) + 1));
    strcpy(layer_name[which], name);

    return layer_name[which];
}

LayerPun LayerRealloc(LayerPun layer) {
    LayerPun retval;
    /*
    retval = (LayerPun) calloc(layerListElements, sizeof(LayerListElement));
    */
    if (oldElements < layerListElements) {
        retval = (LayerPun) realloc(layer, layerListElements * sizeof(LayerListElement));
        assert(retval != NULL);
        memset(retval, 0, layerListElements * sizeof(LayerListElement));
        memcpy(retval, layer, oldElements * sizeof(LayerListElement));
    }
    else {
        retval = layer;
    }
    return retval;
}


void SetLayerName(int ln, char *lname) {
    assert(ln < layersNumber);

    layer_name[ln] = (char *) emalloc(strlen(lname) + 1);
    strcpy(layer_name[ln], lname);
}


char *GetLayerName(int ln) {
    assert(ln < layersNumber);
    return layer_name[ln];
}

void DeleteLayerName(Layer lay) {
    register int i;

    assert(lay < layersNumber);

    free(layer_name[lay]);

    if (lay != (layersNumber - 1))
        for (i = lay; i < layersNumber - 1; i++) {

            layer_name[i] = layer_name[i + 1];
        }
    layer_name[layersNumber - 1] = NULL;
}

void ReallocLayersNames(void) {
    layer_name = (char **) realloc(layer_name, layerListElements * LAYERS_PER_ELEMENT * sizeof(char *));
    assert(layer_name != NULL);
}

void ReleaseLayersNames(void) {
    register int i;

    for (i = 1; i < layersNumber; i++) {
        free(layer_name[i]);
        layer_name[i] = NULL;
    }
}


void SetLayersNumber(Layer num) {
    layersNumber = num + 1;
}


int HowManyLayers(void) {
    return layersNumber - 1;
}

void SetLayer2LayerList(int layer, LayerPun layerList) {
    unsigned bit, element;
    LayerListElement mask;

    assert(layer < layersNumber);

    bit = layer % LAYERS_PER_ELEMENT;
    element = layer / LAYERS_PER_ELEMENT;

    mask = 1 << bit;

    layerList[element] |= mask;

}

LayerPun NewLayerList(int mode, LayerPun toDuplicate) {
    LayerPun newLayerPun;

    newLayerPun = (LayerPun) calloc(layerListElements, sizeof(LayerListElement));
    assert(newLayerPun != NULL);

    switch (mode) {
    case WHOLENET:
        SetLayer2LayerList(WHOLE_NET_LAYER, newLayerPun);
        break;
    case CURRENTVIEW:
        LayerCopy(newLayerPun, viewLayerList);
        break;
    case CURRENTVIEWANDWHOLE:
        LayerCopy(newLayerPun, viewLayerList);
        SetLayer2LayerList(WHOLE_NET_LAYER, newLayerPun);
        break;
    case DUPLICATE:
        assert(toDuplicate != NULL);
        LayerCopy(newLayerPun, toDuplicate);
        break;
    }

    return newLayerPun;
}

void MakeLayerVisible(int ln) {
    assert(ln < layersNumber);
    SetLayer2LayerList(ln, viewLayerList);
}

void SetInitialLayerDimension(int dim) {
    dim++;
    oldElements = layerListElements;
    layerListElements = ((dim / LAYERS_PER_ELEMENT) + ((dim % LAYERS_PER_ELEMENT) != 0));
    layersNumber = dim;
    ReallocEditLayerList();
    viewLayerList = LayerRealloc(viewLayerList);
    ReallocLayersNames();
    ReallocViewLayerButton();
    SetLayer2ViewLayerList(WHOLE_NET_LAYER);

}



Boolean IsLayerListVisible(LayerPun lay) {
    register int i;

    for (i = 0; i < layerListElements; i++) {
        if (lay[i] & viewLayerList[i])
            return TRUE;
    }
    return FALSE;
}

void ResetVisibleLayers(LayerPun lay) {
    register int i;

    for (i = 0; i < layerListElements; i++)
        lay[i] &= ~viewLayerList[i];
}


void ResetAllLayersInList(LayerPun lay) {
    register int i;

    for (i = 0; i < layerListElements; i++)
        lay[i] = 0;
}

void AddVisibleLayers2LayerList(LayerPun lay) {
    register int i;

    for (i = 0; i < layerListElements; i++)
        lay[i] |= viewLayerList[i];
}


void SetAllLayersInList(LayerPun lay) {
    register int i;

    for (i = 0; i < layerListElements; i++)
        lay[i] = ~((LayerListElement) 0);
}

Boolean TestLayer(int layer, LayerPun layerList) {
    unsigned bit, element;
    LayerListElement mask;

    /* assert(layer < layersNumber); */

    bit = layer % LAYERS_PER_ELEMENT;
    element = layer / LAYERS_PER_ELEMENT;

    mask = 1 << bit;

    return ((layerList[element] & (mask)) ? TRUE : FALSE);
}

Boolean IsLayerVisible(int lay) {
    return TestLayer(lay, viewLayerList);
}


void ResetLayer2LayerList(int layer, LayerPun layerList) {
    unsigned bit, element;
    LayerListElement mask;

    assert(layer < layersNumber);

    bit = layer % LAYERS_PER_ELEMENT;
    element = layer / LAYERS_PER_ELEMENT;

    mask = 1 << bit;

    layerList[element] &= ~mask;

}

void DeleteLayerFromLayerList(int layer, LayerPun layerList) {
    unsigned bit, element;
    LayerListElement mask;
    LayerListElement mask2;
    LayerListElement carry = 0;
    int i;

    assert(layer < layersNumber);

    bit = layer % LAYERS_PER_ELEMENT;
    element = layer / LAYERS_PER_ELEMENT;
    for (i = layerListElements - 1; i > element; i--) {
        LayerListElement oldCarry = 0;

        oldCarry = layerList[i] & CARRYMASK_OUT;

        layerList[i] >>= 1;
        if (carry)
            layerList[i] |= CARRYMASK_IN;
        carry = oldCarry;
    }
    mask = ~((~((LayerListElement) 0)) << (bit));

    mask2 = layerList[i] & mask;
    layerList[i] >>= 1;
    layerList[i] &= ~mask;
    layerList[i] |= mask2;

    if (carry)
        layerList[i] |= CARRYMASK_IN;

}

void ReallocAllObjectsLayerLists(void) {
    struct place_object *place;
    struct arc_object *arc;
    struct trans_object *trans;
    struct mpar_object *mpar;
    struct rpar_object *rpar;
    struct group_object *group;

    /* ResetSelectedLayer(netobj);  */

    if ((mpar = netobj->mpars) != NULL) {
        while (mpar != NULL) {
            mpar->layer = LayerRealloc(mpar->layer);
            mpar = mpar->next;
        }
    }
    if ((place = netobj->places) != NULL) {
        while (place != NULL) {
            place->layer = LayerRealloc(place->layer);
            place = place->next;
        }
    }
    if ((rpar = netobj->rpars) != NULL) {
        while (rpar != NULL) {
            rpar->layer = LayerRealloc(rpar->layer);
            rpar = rpar->next;
        }
    }
    if ((trans = netobj->trans) != NULL) {
        while (trans != NULL) {
            trans->layer = LayerRealloc(trans->layer);
            trans = trans->next;
        }
    }
    group = netobj->groups;
    while (group != NULL) {
        for (trans = group->trans; trans != NULL; trans = trans->next) {
            trans->layer = LayerRealloc(trans->layer);
        }
        group = group->next;
    }

    if ((arc = netobj->arcs) != NULL) {
        while (arc != NULL) {
            arc->layer = LayerRealloc(arc->layer);
            arc = arc->next;
        }
    }
}

Layer NewLayer(char *layerName) {
    char *temp;

#ifdef DEBUG_LAYERS
    printf("NewLayer Entering ....\n");
#endif

    if (layersNumber++ / LAYERS_PER_ELEMENT == layerListElements) {

#ifdef DEBUG_LAYERS
        printf("NewLayer Realloc ....\n");
#endif

        oldElements = layerListElements;
        layerListElements++;
        ReallocEditLayerList();
        viewLayerList = LayerRealloc(viewLayerList);
        ReallocLayersNames();
        ReallocAllObjectsLayerLists();
        ReallocViewLayerButton();
    }
    temp = emalloc(strlen(layerName) + 1);
    strcpy(temp, layerName);
    layer_name[layersNumber - 1] = temp;

#ifdef DEBUG_LAYERS
    printf("NewLayer Entering ....\n");
#endif

    return layersNumber - 1;
}

void DeleteLayers(void) {
    int ln, i, oldnum;
    int correct = 0;

    /*ResetSelectedLayer(netobj);*/
    for (i = 1, oldnum = layersNumber; i < oldnum; i++)
        if (TestLayerInEditLayerList(i)) {
            struct place_object *place;
            struct arc_object *arc;
            struct trans_object *trans;
            struct mpar_object *mpar;
            struct rpar_object *rpar;
            struct group_object *group;

            ln = i - correct++;

            if ((mpar = netobj->mpars) != NULL) {
                while (mpar != NULL) {
                    DeleteLayerFromLayerList(ln, mpar->layer);
                    mpar = mpar->next;
                }
            }
            if ((place = netobj->places) != NULL) {
                while (place != NULL) {
                    DeleteLayerFromLayerList(ln, place->layer);
                    place = place->next;
                }
            }
            if ((rpar = netobj->rpars) != NULL) {
                while (rpar != NULL) {
                    DeleteLayerFromLayerList(ln, rpar->layer);
                    rpar = rpar->next;
                }
            }
            if ((trans = netobj->trans) != NULL) {
                while (trans != NULL) {
                    DeleteLayerFromLayerList(ln, trans->layer);
                    trans = trans->next;
                }
            }
            group = netobj->groups;
            while (group != NULL) {
                for (trans = group->trans; trans != NULL; trans = trans->next) {
                    DeleteLayerFromLayerList(ln, trans->layer);
                }
                group = group->next;
            }

            if ((arc = netobj->arcs) != NULL) {
                while (arc != NULL) {
                    DeleteLayerFromLayerList(ln, arc->layer);
                    arc = arc->next;
                }
            }
            DeleteLayerName(ln);
            layersNumber--;
            puts("Decremento");
        }
    puts("Esco");
}

void ReinitializeLayers(void) {
    ReleaseLayersNames();

    oldElements = layerListElements;
    layerListElements = 1;
    layersNumber = 1;
    ReinitializeViewDialog();
    ReinitializeEditDialog();

    ReallocLayersNames();
}

void ResetSelectedLayer(struct net_object *net) {
    struct place_object *place;
    struct arc_object *arc;
    struct trans_object *trans;
    struct mpar_object *mpar;
    struct rpar_object *rpar;
    struct group_object *group;

    if ((mpar = net->mpars) != NULL) {
        while (mpar != NULL) {
            ResetVisibleLayers(mpar->layer);
            mpar = mpar->next;
        }
    } if ((place = net->places) !=
            NULL) {
        while (place != NULL) {
            ResetVisibleLayers(place->layer);
            place =
                place->next;
        }
    } if ((rpar = net->rpars) != NULL) {
        while (rpar !=
                NULL) {
            ResetVisibleLayers(rpar->layer);
            rpar = rpar->next;
        }
    } if
    ((trans = net->trans) != NULL) {
        while (trans != NULL) {
            ResetVisibleLayers(trans->layer);
            trans = trans->next;
        }
    } group = net->groups;
    while
    (group != NULL) {
        for (trans = group->trans; trans != NULL; trans =
                    trans->next) {
            ResetVisibleLayers(trans->layer);
        } group = group->next;
    }

    if ((arc = net->arcs) != NULL) {
        while (arc != NULL) {
            ResetVisibleLayers(arc->layer);
            arc = arc->next;
        }
    }

}


void AddSelectedLayer(void) {
    struct place_object *place;
    struct arc_object *arc;
    struct trans_object *trans;
    struct mpar_object *mpar;
    struct rpar_object *rpar;
    struct group_object *group;

    if ((mpar = selected.mpars) != NULL) {
        while (mpar != NULL) {
            AddVisibleLayers2LayerList(mpar->layer);
            mpar = mpar->next;
        }
    }
    if ((place = selected.places) != NULL) {
        while (place != NULL) {
            AddVisibleLayers2LayerList(place->layer);
            place = place->next;
        }
    }
    if ((rpar = selected.rpars) !=  NULL) {
        while (rpar != NULL) {
            AddVisibleLayers2LayerList(rpar->layer);
            rpar =
                rpar->next;
        }
    }
    if ((trans = selected.trans) != NULL) {
        while (trans !=   NULL) {
            AddVisibleLayers2LayerList(trans->layer);
            trans = trans->next;
        }
    }
    group = selected.groups;
    while (group != NULL) {
        for (trans = group->trans; trans != NULL; trans = trans->next) {
            AddVisibleLayers2LayerList(trans->layer);
        }
        group = group->next;
    }

    if ((arc = selected.arcs) != NULL) {
        while (arc != NULL) {
            AddVisibleLayers2LayerList(arc->layer);
            arc = arc->next;
        }
    }
}

void InitLayer(void) {

    layer_name = (char **) calloc(LAYERS_PER_ELEMENT, sizeof(char *));
    assert(layer_name != NULL);
    layer_name[0] = wholeName;

    layersNumber = 1;
    layerListElements = 1;
    oldElements = 1;

    viewLayerList = NewLayerList(ZEROINIT, NULL);
    SetLayer2LayerList(WHOLE_NET_LAYER, viewLayerList);

    ResetUpdate();

}

/*
 * void SetLayersToCurrentVisibleList(lay) LayerPun lay; { LayerCopy(lay,
 * viewLayerList); }
 *
 * void LayerReset(layerDest, layerSource) LayerPun layerDest; LayerPun
 * layerSource; { register int i;
 *
 * for (i = 0; i < layerListElements; i++) { layerDest[i] &= ~layerSource[i]; }
 * }
 *
 * void LayerAdd(layerDest, layerSource) LayerPun layerDest; LayerPun
 * layerSource; { register int i;
 *
 * for (i = 0; i < layerListElements; i++) { layerDest[i] |= layerSource[i]; } }
 * update_layer_dialog() { Arg args[1]; int ln;
 *
 * for (ln = 0; ln <= layersNumber; ln++) { int tf = (test_layer(ln,
 * viewLayerList)) ? TRUE : FALSE; XmToggleButtonGadgetSetState(layers[ln], tf, 1);
 * } }
 *
 * static open_layer_proc(item, event) Widget item; XEvent *event; {
 *
 * window_fit(layer_sw); window_fit(layer_f); xv_set(layer_f, WIN_SHOW, TRUE,
 * 0);
 *
 * }
 * static layer_close_proc(item, event) Widget item; XEvent *event; {
 * xv_set(layer_f, WIN_SHOW, FALSE, 0); redisplay_canvas(); }
 *
 * static layer_redraw_proc(item, event) Widget item; XEvent *event; {
 * redisplay_canvas(); }
 *
 * reset_layer() { int i; Arg args[1]; XmString XmName;
 *
 * for (i = 0; i < MAX_LAYER; i++) { if (i <= layersNumber) { XmName =
 * XmStringCreate(layer_name[i], XmSTRING_DEFAULT_CHARSET); XtSetArg(args[0],
 * XmNlabelString, XmName); XtSetValues(layers[i], args, 1);
 * XmStringFree(XmName); XtManageChild(layers[i]); } else
 * XtUnmanageChild(layers[i]);
 *
 * } update_layer(); } update_layer() { int ln;
 *
 * updating = 1; for (ln = 0; ln <= layersNumber; ln++) { int tf =
 * (test_layer(ln, viewLayerList)) ? TRUE : FALSE;
 * XmToggleButtonGadgetSetState(layers[ln], tf, 1); } updating = 0; }
 */
