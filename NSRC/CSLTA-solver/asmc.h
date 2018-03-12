/*
 *  asmc.h
 *  CslTA-Solver
 *
 *  Definitions of the Action and State -labelled Markovian Chain
 *
 */

//-----------------------------------------------------------------------------
#ifndef __ASMC_H__
#define __ASMC_H__
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// ASMC Declaration API
//-----------------------------------------------------------------------------
PUBLIC_API void       AsmcState_Delete(AsmcState *st);
PUBLIC_API AsmcState *AsmcState_New(const char *name, IDValList *idvl);

PUBLIC_API void            AsmcTransition_Delete(AsmcTransition *aarc);
PUBLIC_API AsmcTransition *AsmcTransition_New(const char *from, const char *to,
        const char *act, double lambda);

PUBLIC_API void	 Asmc_Delete(ASMC *asmc);
PUBLIC_API ASMC *Asmc_New();
PUBLIC_API ASMC *Asmc_AddState(ASMC *asmc, AsmcState *ast);
PUBLIC_API ASMC *Asmc_AddTransition(ASMC *asmc, AsmcTransition *aarc);


//-----------------------------------------------------------------------------
#ifdef __cplusplus

//-----------------------------------------------------------------------------
// ASMC Helper structures
//-----------------------------------------------------------------------------
struct AsmcState {
    AsmcState(IDValList *a, const char *n) : apl(a), name(n) {}
    std::shared_ptr<IDValList> apl;
    string name;
};
struct AsmcTransition {
    AsmcTransition(const char *f, const char *t, const char *a, double l) :
/**/           from(f), to(t), action(a), lambda(l) {}
    string from, to, action;
    double lambda;
};



//-----------------------------------------------------------------------------
// ASMC: Action & State labelled Markovian Chain
//-----------------------------------------------------------------------------
struct ASMC : public MCObject {
public:
    // Vertex properties for the Markovian chain node
    struct VertexP {
        string			vertexName;	// name
        vector<int	>	labelVals;	// atomic proposition values
    };
    // Edge properties for the ASMC
    struct EdgeP {
        ActionID		actionID;	// action activated following this edge
        double			lambda;		// rate
    };
    struct VertexPropertyKind { typedef vertex_property_tag kind; };
    typedef property<VertexPropertyKind, VertexP> VertexProperty;
    struct EdgePropertyKind { typedef edge_property_tag kind; };
    typedef property<EdgePropertyKind, EdgeP> EdgeProperty;

    // Type definitions for the adjacency list of the ASMC graph
    typedef adjacency_list<listS, vecS, bidirectionalS,
            /**/				   VertexProperty, EdgeProperty> ASMCGraph;
    typedef graph_traits<ASMCGraph>::vertex_iterator     vertex_iter_t;
    typedef graph_traits<ASMCGraph>::edge_iterator       edge_iter_t;
    typedef graph_traits<ASMCGraph>::vertex_descriptor   vertex_t;
    typedef graph_traits<ASMCGraph>::edge_descriptor     edge_t;


    ASMC();

    AtomicPropID AddAtomicProposition(const string &name);
    const SymbolTable &GetAtomicPropositions() const { return AtomicProps; }
    bool AddState(const string &stName, const list<IdValuePair> &APs);
    bool AddEdge(const string &s0, const string &s1,
                 double lambda, const string &action);

    MCObjectType GetObjectType() const { return MCOT_ASMC; }
    bool isValidObject(string &reason) const;
    bool WriteAsGmlFile(ostream &os, const char *name, const char *args);

    // Find all the BSCC of the graph e return the number of BSCC found;
    // if a state s is in the n-th BSCC, then outBSCCs[s]==n,
    // otherwise outBSCCs[s]==-1
    int FindBSCCs(vector<int> &outBSCCs) const;

public:
    SymbolTable		AtomicProps;  // List of APs (state labels)
    SymbolTable		Actions;	  // List of actions (edge labels)

    // Graph data
    ASMCGraph		graph;		  // The ASMC graph

    // Accessors for vector & edge mapped data in the ASMC graph
    property_map<ASMCGraph, VertexPropertyKind>::type  vertPMap;
    property_map<ASMCGraph, EdgePropertyKind>::type    edgePMap;

    // Map between vertex names and indices (for fast FindVertex() calls)
    map<string, vertex_t>   vertexName2Index;
private:
    vertex_iter_t FindVertex(const string &stName);
    string GetGmlNameForVertex(const VertexP &vp) const;
};



//-----------------------------------------------------------------------------
#endif  // __cplusplus
#endif  // __ASMC_H__