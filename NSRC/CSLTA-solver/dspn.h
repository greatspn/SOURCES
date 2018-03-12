/*
 *  dspn.h
 *  CslTA-Solver
 *
 *  Definitions of a Deterministic and Stochastic Petri Net
 *
 */

//-----------------------------------------------------------------------------
#ifndef __DSPN_H__
#define __DSPN_H__
//-----------------------------------------------------------------------------

#ifdef __cplusplus

//-----------------------------------------------------------------------------
// DSPN: Deterministic & Stochastic Petri Net
//-----------------------------------------------------------------------------
struct DSPN {
public:
    enum TrKind  { IMM, EXP, DET };
    enum TrSrvc  { SingleSrv, InfSrv /*,MarkDepSrv*/ };
    enum ArcKind { INPUT, OUTPUT, INHIB };

    static const size_t FIXED_ARC_MULTIPLICITY;

    struct Place {
        Place(const string &, size_t, const string &);

        string	name;		// Name of this place
        size_t	initMark;	// # of tokens in the initial marking
        string  cluster;	// Graphic cluster
        dPoint  pos;		// Graphic position in 2D space
    };

    struct Transition {
        Transition(const string &, TrKind, double, size_t, TrSrvc, const string &);

        string	name;		// Name of this transition
        TrKind  kind;		// Immediate, Exponential or Deterministic
        double	delay;		// Delay (Exp,Det) or firing weight (Imm)
        size_t  priority;	// Priority of Immediate transitions
        TrSrvc  service;	// Service type
        string  cluster;	// Graphic cluster name
        dPoint  pos;		// Graphic position in 2D space
    };

    struct Arc {
        Arc(size_t, size_t, ArcKind, size_t, size_t);

        size_t		place;	// Place index (source or target)
        size_t		tran;	// Trasition index (source or target)
        ArcKind		kind;   // Input, Output, Inhibitor
        size_t		mult;	// Multiplicity
        dPolyLine	line;	// Graphic line from the place to the transition

        size_t		mdPlace; // #mdPlace mark-dep multiplicity or FIXED_ARC_MULT

        bool isSourceAPlace() const			{ return kind != OUTPUT; }
        bool isTargetAPlace() const			{ return kind == OUTPUT; }
        size_t getSource() const   { return isSourceAPlace() ? place : tran; }
        size_t getTarget() const   { return isTargetAPlace() ? place : tran; }
    };

    // Add a new place
    size_t AddPlace(const string &name, size_t initMark,
                    const string &cluster = "");

    // Add a new transition
    size_t AddImmTran(const string &name, double weight,
                      size_t priority, const string &cluster = "");
    size_t AddExpTran(const string &name, double delay,
                      TrSrvc srv = SingleSrv, const string &cluster = "");
    size_t AddDetTran(const string &name, double delay,
                      TrSrvc srv = SingleSrv, const string &cluster = "");

    // Create a new arc between a place and a transition
    size_t AddInputArc(size_t srcPlace, size_t dstTran, size_t mult = 1);
    size_t AddOutputArc(size_t srcTran, size_t dstPlace, size_t mult = 1);
    size_t AddInhibArc(size_t srcPlace, size_t dstTran, size_t mult = 1);
    // Creates a couple of input/output arcs (i.e. a "test" arc)
    pair<size_t, size_t> AddInputOutputArcs(size_t place, size_t tran, size_t mult = 1);

    // Merges with another DSPN, doing place & transition superposition
    void MergeWithPlaceSuperpos(const DSPN &dspn, const vector<string> &spPlcs);

    void WriteAsGmlFile(ostream &os, const char *name, bool writeUID) const;
    void WriteAsGmlFile(const char *filename, const char *name = "",
                        bool writeUID = false) const;
    void PrintDSPN() const;

    // Save as a vaid DSPNexpress Network file
    void WriteAsDSPNexpressFile(ostream &net, ostream &def,
                                const char *name, const char *measures = "") const;

    // Save as a TimeNET DSPN file
    void WriteAsTimeNetFile(ostream &net, const char *name) const;

    // Save as a SPNica definition file
    void WriteAsSPNicaFile(ostream &net, const char *name) const;

    // Save as a GreatSPN Petri net file
    void WriteAsGreatSPNFile(ostream &net, ostream &def,
                             const char *name, const char *measures = "") const;

    // DSPN Transformations
    size_t AddInitialTangibleMark(double lambda = 1000.0);
    void AddAbsorbingLoops(const char *absorbingPlaceNames[], size_t numAbsPlaces);
    bool CloseBSCCs(ifstream &bscc, const char *placeName,
                    double expLambda, size_t startPlace);
    void RemoveDoubleArcs(size_t *Nplaces, size_t numNplaces);

    // Prepare a very generic (X,Y) graphical positioning of the DSPN
    void SimpleGraphicPlanarization();
    // Import (x,Y) coords from a GML file (written with writeUID=true)
    void ImportGmlCoords(istream &gml);
    // Planarize using external graph planarizer
    bool Planarize();
    // Fix coords by rescale & translation ops
    void RescaleAndFixCoords(double rescale);
    // Reset all coords to (0,0)
    void ClearAllCoords();

    // Returns the index of a place named *name, or -1
    size_t FindPlaceByName(const char *name) const;

public:
    typedef vector<Place>		VectorOfPlaces;
    typedef vector<Transition>  VectorOfTransitions;
    typedef vector<Arc>			VectorOfArcs;

    VectorOfPlaces		 places;	  // Place set
    VectorOfTransitions	 transitions; // Transition set
    VectorOfArcs		 arcs;		  // Arc set

    // Prepares a name for a DSPNexpress model
    string GetDSPNexprName(const string &str, char pre, size_t count) const;

    // Write out a GreatSPN transition
    void WriteGreatSPNTransition(ostream &net, const Transition &trn,
                                 size_t trnCnt, size_t enabl, size_t knd,
                                 const multimap<size_t, const Arc *> &arcsByTrn) const;

    vector<string> _ReadGmlTokens(istream &gml) const;
};



//-----------------------------------------------------------------------------
#endif  // __cplusplus
#endif  // __DSPN_H__