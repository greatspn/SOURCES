/*
 *  Isomorphism.h
 *
 *  Rooted digraph isomorphism test
 *
 *  Created by Elvio Amparore
 *
 */
//=============================================================================
#ifndef __ISOMORPHISM_H__
#define __ISOMORPHISM_H__
//=============================================================================

struct Signature64;
typedef Signature64  Signature64;
struct Signature128;
typedef Signature128  Signature128;

#ifdef WIN32
typedef unsigned __int32 uint32_t;
#endif

//=============================================================================

/// 64-bit hash used to classify graph vertices
struct Signature64 {
    uint32_t   P;
    uint32_t   Q;

    inline void reset(uint32_t val = 0) { P = Q = val; }

    inline bool operator < (const Signature64 &g) const {
        return ((P < g.P) || (P == g.P && Q < g.Q));
    }

    inline bool operator == (const Signature64 &g) const {
        return (P == g.P && Q == g.Q);
    }
};
template<class ostream_t>
inline ostream_t &operator << (ostream_t &os, const Signature64 &g) {
    os << setfill('0') << setw(8) << g.P << "-";
    os << setfill('0') << setw(8) << g.Q << setfill(' ');
    return os;
}

//=============================================================================

/// 128-bit hash for SMC isomorphism hashes
struct Signature128 {
    uint32_t   P;
    uint32_t   Q;
    uint32_t   R;
    uint32_t   S;

    inline void reset(uint32_t val = 0) { P = Q = R = S = val; }

    inline bool operator < (const Signature128 &g) const {
        return ((P < g.P) ||
                (P == g.P && Q < g.Q) ||
                (P == g.P && Q == g.Q && R < g.R) ||
                (P == g.P && Q == g.Q && R == g.R && S < g.S));
    }

    inline bool operator == (const Signature128 &g) const {
        return (P == g.P && Q == g.Q && R == g.R && S == g.S);
    }
};
template<class ostream_t>
inline ostream_t &operator << (ostream_t &os, const Signature128 &g) {
    os << setfill('0') << setw(8) << g.P << "-";
    os << setfill('0') << setw(8) << g.Q << "-";
    os << setfill('0') << setw(8) << g.R << "-";
    os << setfill('0') << setw(8) << g.S << setfill(' ');
    return os;
}


//=============================================================================
//    Function Definitions:
//=============================================================================

void ComputeMRP_VertexHashes(const ublas::compressed_matrix<double> &Q,
                             const ublas::compressed_matrix<double> &Qbar,
                             vector<Signature64> &inv);

void ComputeSMC_IsoHash(size_t initialState,
                        const ublas::compressed_matrix<double> &Q,
                        const ublas::compressed_matrix<double> &Qbar,
                        const vector<Signature64> &inv, double delay,
                        Signature128 &hash, const uint32_t MAX_VISITED_NODES = 50);

bool FindRootedDigraphIsomorphism(const ublas::compressed_matrix<double> &M1,
                                  const ublas::compressed_matrix<double> &M2,
                                  vector<int> &tab1to2, VerboseLevel verboseLvl);

//=============================================================================
#endif   // __ISOMORPHISM_H__
