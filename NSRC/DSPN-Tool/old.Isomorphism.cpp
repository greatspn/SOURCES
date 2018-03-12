/*
 *  Isomorphism.cpp
 *
 *  Implementation of the rooted digraph isomorphism test
 *
 *  Created by Elvio Amparore
 *
 */
#include <vector>
#include <stack>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <ctime>
using namespace std;

#include "../numeric/numeric.h"
using namespace numerical;

typedef size_t  marking_t;
#include "Isomorphism.h"


//=============================================================================
// Implementations
//=============================================================================

static inline 
uint32_t ror32(uint32_t val, size_t shift) 
{
	const size_t n = shift & 32;
	return (val >> n) | (val << (32 - n));
}

//=============================================================================

static inline 
uint32_t rol32(uint32_t val, size_t shift) 
{
	const size_t n = shift & 32;
	return (val << n) | (val >> (32 - n));
}

//=============================================================================

static inline void 
unpack_double(double d, uint32_t& A, uint32_t& B) 
{
	union { double d; struct { uint32_t a, b; }; } u;
	u.d = d;
	A = u.a;
	B = u.b;
}

//=============================================================================

static inline void 
process_hash64(Signature128& g, const Signature64& vA, 
			   const uint32_t vB, const uint32_t vC) 
{
	uint32_t P = ror32(g.Q, 7) ^ vA.P;
	uint32_t Q = g.R ^ (vB | rol32(vB, 17));
	uint32_t R = g.S;
	uint32_t S = g.P + vC;
	
	g.P = P;	g.Q = Q;
	g.R = R;	g.S = S;	
}

//=============================================================================

static uint32_t s_hashes[256] = {
	0x59131E55, 0xA689C30E, 0x6D1FB43F, 0x73FDCE46, 0xB5F8ECBD, 0x2BC53A6C,
	0x9EA7266A, 0x616ED5A4, 0xC9404E2E, 0xC59E9094, 0x18E38CF3, 0x1253ABB9,
	0xD9522AA4, 0x9FB1FA12, 0xC5AC4336, 0xD5786CFF, 0x70C33863, 0x45BEEAB2,
	0x400B8D38, 0xB934DA9E, 0x0FCD33F5, 0xF40774BE, 0x72F99AB7, 0xAA5C9203,
	0x8EEE9573, 0x7DF92DE2, 0xBA7F1083, 0xC27C6E0A, 0xEB3B3B1B, 0x14DBABA8,
	0x24A1F23B, 0x49794A3D, 0x7629D129, 0xB57C8341, 0xDF7CD542, 0x4C8F726F,
	0x87907289, 0x7404E379, 0xF1C5E3E4, 0x83B62E84, 0x53CE5CC1, 0x5B649DD6,
	0xFCC19730, 0xFEDB3B89, 0xCC4C97FE, 0x063C077B, 0xD174F284, 0xF5699EDA,
	0xFE2AFF7C, 0xDC2069AE, 0xA9B7B4B9, 0xA624872C, 0x1E920D99, 0xED2DBE01,
	0x8E614C3A, 0x7006B8D4, 0x5BB52D3A, 0x4509C967, 0xEEEDA644, 0x3D721BB4,
	0x06A44527, 0x7455FA35, 0x59FCD4E9, 0x59CAEC20, 0x2A83B278, 0xF5B77CD6,
	0xFE3FC13E, 0x636C0EAB, 0x85C9CD45, 0x348A05A8, 0x1AE280FB, 0x449C0357,
	0x0ED4A4D4, 0xF1E8F690, 0xCA7F6DF8, 0x975E6FC2, 0xA27A0167, 0xA7CAD489,
	0xFD41E862, 0xEC96BC35, 0x70DF0CA2, 0x042CAED2, 0xA3C49022, 0xDEFA829D,
	0x00286D1F, 0x295245F6, 0x6BFBABC1, 0x9C09D43F, 0x0B9A0473, 0x02DDD755,
	0x6EFC336C, 0x03B72BC1, 0xD0DADC9F, 0x1C2644E0, 0x9364A02B, 0xEDCD984A,
	0x07931CC6, 0xE83F14F1, 0x86A247E0, 0x297B6225, 0x81BB6E81, 0x25F61F4D,
	0x7349CF5D, 0xED8AFD97, 0x74D5804E, 0x622E0B0B, 0xF45D8ADA, 0x8FB228AD,
	0xE6C298DA, 0x73BC5F41, 0xCEF7BCE7, 0xFBF347F9, 0x8E41DB07, 0x48437004,
	0x68E30FBD, 0x32861C5A, 0xE3C65E3B, 0xF7F3027E, 0x35F5D04B, 0x77A8AC43,
	0xCB6446B8, 0x6D2FB6B9, 0xC15F3B7D, 0x68121217, 0x304CCC0B, 0x17CE7FA6,
	0x1473F08E, 0xA8078B5E, 0x861681A4, 0x04F9F346, 0x7DC03E0E, 0x4CB23E9F,
	0x34FFA1A8, 0xFCC1D863, 0xEAD3CB25, 0x34F1ADDA, 0x53C3794B, 0x4BEC4A76,
	0xCFA0E6D9, 0x95B51DC0, 0x7C1CD633, 0xE1FCCD71, 0x2577FF0A, 0x79B0E7F4,
	0x86141023, 0x8AE894B1, 0xAB2FB8D7, 0x0E43CD16, 0x59956A15, 0x7666F898,
	0xE888A251, 0xB5614972, 0x1BA5D7F1, 0xAC34FE5D, 0x20F5C99B, 0x4FD7611C,
	0x9EC2EEBE, 0x2292D04C, 0xB5C106BC, 0x259E61A6, 0xFF4813BF, 0xD1B91A7C,
	0x38EDBC86, 0x87A4F8E1, 0x131D5D3B, 0xE9AFD6AF, 0xD577918D, 0xADE21ED7,
	0x0039982D, 0x8477F374, 0x149861CC, 0xBC1137A1, 0x9B07038F, 0x25666F25,
	0x56EE00DE, 0x05BA65C5, 0x483AD769, 0xE8408B87, 0xB64830AD, 0x11DC3BAD,
	0xB88FD1CC, 0x11DC9A39, 0x4BCA1AB6, 0x6F038C31, 0xB6B5036E, 0xABDDBA3F,
	0xB6515997, 0x0AB8E2E2, 0x53E1B20B, 0x9B1A8AB5, 0x8230074D, 0x4F0CEE20,
	0x761D6EA5, 0x984DB66C, 0xC46D9528, 0x90F34AF2, 0xA6857FE0, 0x69DBF984, 
	0x1E6D320B, 0xD8C316CE, 0xBE2C2225, 0x11253A16, 0xAE11381E, 0xFAAC8B29, 
	0xC00DF026, 0xA2209E8F, 0x3D633900, 0x66F422AA, 0xF8F07E18, 0xBA411CA9, 
	0x101B9F12, 0x0B22DB03, 0x1F8B457B, 0xF4130A3B, 0x1EEC68FB, 0x512B2660, 
	0x46DF44F5, 0x31CC515C, 0x743019C8, 0x8A8A2463, 0x2BD218FD, 0xF01E97D7, 
	0xBBA35BBA, 0xA6169D32, 0x35F71F3F, 0x7EA4BFDC, 0xC0B72867, 0xA2ED7758, 
	0xEA3C0020, 0xBF4A79F2, 0xF91E9EAA, 0xC4277522, 0xF0AE702F, 0x7B1A3413, 
	0x0FD18DBE, 0xEACD1F54, 0x15786531, 0xDA3989FD, 0xC52731E4, 0xA0996B3C, 
	0x2682B110, 0x171215E8, 0x4FBE4D01, 0x6B8C4C4B, 0xB1D92E76, 0x8E188A92, 
	0xBCB36756, 0xC5035D22, 0x0379B63E, 0xB305EFE3, 0xB8E73964, 0xF671A083, 
	0xFEC60EBF, 0x772A23AF, 0xBBE13B7F, 0xDD9C6EC2
};
static inline
uint32_t shuffle32(uint32_t in) 
{
	uint32_t out(s_hashes[0xFF & in]);
	out ^= rol32(s_hashes[0xFF & (in >>  8)], 7);
	out ^= rol32(s_hashes[0xFF & (in >> 16)], 11);
	out ^= rol32(s_hashes[0xFF & (in >> 24)], 23);
	return out;
}

//=============================================================================

// Verifies if state s1 matches the alredy assigned adjacency of it's
// corresponding vertex in M2. Unassigned vertices don't take part in this test
bool VerifyIsomorphicStates(const ublas::compressed_matrix<double>& M1,
							const ublas::compressed_matrix<double>& M2,
							const vector<int>& tab1to2, 
							const vector<int>& tab2to1,
							size_t s1)
{
	typedef ublas::matrix_row< const ublas::compressed_matrix<double> > matrix_row_t;
	typedef matrix_row_t::const_iterator row_iterator_t;
	
	const size_t s2 = tab1to2[s1];
	assert(s2 >= 0);
	
	// Verify M1 -> M2 isomorphism
	matrix_row_t M1_row(M1, s1);
	row_iterator_t M1_rowIt = M1_row.begin();
	row_iterator_t M1_rowItEnd = M1_row.end();
	while (M1_rowIt != M1_rowItEnd) {
		const int e1 = M1_rowIt.index();
		const int e2 = tab1to2[e1];
		if (e2 >= 0) {
			if (*M1_rowIt != M2(s2, e2))
				return false; // This edge does not coincide
		}
		++M1_rowIt;
	}
	
	// Verify M2 -> M1 isomorphism
	matrix_row_t M2_row(M2, s2);
	row_iterator_t M2_rowIt = M2_row.begin();
	row_iterator_t M2_rowItEnd = M2_row.end();
	while (M2_rowIt != M2_rowItEnd) {
		const int e2 = M2_rowIt.index();
		const int e1 = tab2to1[e2];
		if (e1 >= 0) {
			if (*M2_rowIt != M1(s1, e1))
				return false; // This edge does not coincide
		}
		++M2_rowIt;
	}
	return true;
}

//=============================================================================

// Verifies isomorphism if two rooted . This method should be called
// when tab1to2 and tab2to1 are fully assigned, with no -1 left.
bool VerifyIsomorphism(const ublas::compressed_matrix<double>& M1,
					   const ublas::compressed_matrix<double>& M2,
					   const vector<int>& tab1to2, const vector<int>& tab2to1)
{
	const size_t N = tab1to2.size();
	
	for (size_t n=0; n<N; n++) {
		assert(tab2to1[ tab1to2[n] ] == (int)n);
		if (!VerifyIsomorphicStates(M1, M2, tab1to2, tab2to1, n))
			return false;
	}
	return true;
}

//=============================================================================

// Computes a "possibly good" hash value for each vertex of the MRP process,
// described by Q and Qbar (Delta is not used in this computation). These hashes
// may be shared between different SMC, because are computed taking into account
// only the Q adjacency informations. For a better SMC hashing function, use
// ComputeVertexInvariants, which also considers the distance between vertices
// and the digraph root.
void ComputeMRP_VertexHashes(const ublas::compressed_matrix<double>& Q,
							 const ublas::compressed_matrix<double>& Qbar,
							 vector<Signature64>& inv) 
{
	const size_t N = Q.size1();
	inv.resize(N);
	vector<uint32_t> codes;
	
	static const uint32_t masks[3][2] = {
		{ 0x6A0C53D8, 0xF6BAD0E0 }, 
		{ 0xF5C5EDB4, 0xD850FB38 },
		{ 0x9B82694D, 0x84A92096 }
	};
	
	typedef ublas::compressed_matrix<double>::const_iterator1 iter1_t;
	typedef ublas::compressed_matrix<double>::const_iterator2 iter2_t;
	
	iter1_t Q_iRow(Q.begin1()), Qbar_iRow(Qbar.begin1());	
	for (size_t n=0; n<N; n++) {
		uint32_t arcCount = 0;
		
		iter2_t Q_ij(Q_iRow.begin()), Q_end(Q_iRow.end());
		for (; Q_ij != Q_end; ++Q_ij) {
			uint32_t C1, C2;
			unpack_double(*Q_ij, C1, C2);
			codes.push_back(C1 ^ masks[0][0]);
			codes.push_back(C2 ^ masks[1][0]);
			arcCount++;
		}
		codes.push_back(rol32(arcCount, 18) ^ arcCount ^ masks[2][0]);
		
		iter2_t Qbar_ij(Qbar_iRow.begin()), Qbar_end(Qbar_iRow.end());
		for (; Qbar_ij != Qbar_end; ++Qbar_ij) {
			uint32_t C1, C2;
			unpack_double(*Qbar_ij, C1, C2);
			codes.push_back(C2 ^ masks[0][1]);
			codes.push_back(C1 ^ masks[1][1]);
			arcCount++;
		}
		codes.push_back(rol32(arcCount, 15) ^ arcCount ^ masks[2][1]);
		
		++Q_iRow;
		++Qbar_iRow;
		
		// Sort the codes (to make them order-independent)
		sort(codes.begin(), codes.end());
		
		// Pack codes into a single Signature64, with an hash-like algorithm
		uint32_t P = shuffle32(arcCount), Q = 0;
		for (size_t k=0; k<codes.size(); k++) {
			uint32_t C = codes[k];
			uint32_t nP = ror32(P, 7) ^ (Q | C);
			uint32_t nQ = rol32(Q, 3+k) ^ (P & C);
			P = nP;
			Q = nQ;
		}		
		codes.resize(0);
		
		inv[n].P = P;
		inv[n].Q = Q;
	}
	
	// Now mix together the invariant codes; use P as the constant
	// base in the iteration, and Q as the accumulator
	const size_t NUM_STEPS = min(size_t(15), N); // TODO: determinare il massimo cammino minimo
	for (size_t s=0; s<NUM_STEPS; s++) {
		
		Q_iRow = Q.begin1();
		Qbar_iRow = Qbar.begin1();
		
		// Accumulate in Q hashes of the vertex neighborhoods
		for (size_t n=0; n<N; n++) {
			
			iter2_t Q_ij(Q_iRow.begin()), Q_end(Q_iRow.end());
			for (; Q_ij != Q_end; ++Q_ij) {
				const size_t j = Q_ij.index2();
				inv[n].Q ^= rol32(inv[j].P, 3*(s+1));
			}
			
			iter2_t Qbar_ij(Qbar_iRow.begin()), Qbar_end(Qbar_iRow.end());
			for (; Qbar_ij != Qbar_end; ++Qbar_ij) {
				const size_t j = Qbar_ij.index2();
				inv[n].Q ^= ror32(inv[j].P, s+2);
			}
			
			++Q_iRow;
			++Qbar_iRow;
		}
		
		// Combine P and Q hashes
		for (size_t n=0; n<N; n++) {
			uint32_t tmpP = inv[n].P;
			inv[n].P ^= ror32(inv[n].Q, 3*(s+3));
			inv[n].Q ^= rol32(tmpP, 1+(s&3)) ^ shuffle32(ror32(inv[n].P, 5));
		}		
	}	
	
	/*for (size_t n=0; n<N; n++) {
		cout << "  VERTEX["<<n<<"] -> "<<inv[n]<<endl;
	}
	cout << endl;//*/
}

//=============================================================================

// Computes a 128-bit hash for the initial vertices of a SMC. This function is
// intended to be used to estimate which SMC may be isomorphic. inv should
// be initialized with the root-agnostic vertex invariants computed
// by ComputeMRP_VertexHashes.
void ComputeSMC_IsoHash(size_t initialState, 
						const ublas::compressed_matrix<double>& Q,
						const ublas::compressed_matrix<double>& Qbar, 
						const vector<Signature64>& inv, double delay,
						Signature128& hash, const uint32_t MAX_VISITED_NODES)
{
	typedef ublas::matrix_row< const ublas::compressed_matrix<double> >  MatRow;
	typedef MatRow::const_iterator MatRowIter;
	
	const size_t N = Q.size1();
	deque<marking_t> visitQueue, nextVisitQueue;
	vector<Signature64> signs;
	size_t totalVisitCount = 0;
	vector<bool> alreadyVisited(N, false);
	
	
	Signature64 delaySig;
	unpack_double(delay, delaySig.P, delaySig.Q);
	hash.P = (delaySig.P ^ delaySig.Q);
	hash.Q = hash.P;
	hash.R = ~ delaySig.P;
	hash.S = shuffle32(hash.P);
	
	uint32_t visitLevel = 1;
	visitQueue.push_back(initialState);
	while (!visitQueue.empty() && totalVisitCount < MAX_VISITED_NODES) {
		while (!visitQueue.empty()) {
			marking_t currMark = visitQueue.front();
			visitQueue.pop_front();
			alreadyVisited[currMark] = true;	
			totalVisitCount++;
			signs.push_back(inv[currMark]);
			
			// Visit adjacent vertices
			MatRow Q_iRow(Q, currMark);
			MatRowIter Q_ij = Q_iRow.begin();
			for (; Q_ij != Q_iRow.end(); ++Q_ij) {
				const size_t j = Q_ij.index();
				if (!alreadyVisited[j])
					nextVisitQueue.push_back(j);
			}
			MatRow Qbar_iRow(Qbar, currMark);
			MatRowIter Qbar_ij = Qbar_iRow.begin();
			for (; Qbar_ij != Qbar_iRow.end(); ++Qbar_ij) {
				// For preemption exp arcs we are not interested in their
				// hash code, but only in the arc rate
				Signature64 preemArcSig;
				unpack_double(-(*Qbar_ij), preemArcSig.P, preemArcSig.Q);
				signs.push_back(preemArcSig);
			}
		}
		
		sort(signs.begin(), signs.end());
		size_t visitLevelHash = shuffle32(visitLevel);
		for (size_t i=0; i<signs.size(); i++) {
			process_hash64(hash, signs[i], visitLevelHash, i);
		}
		process_hash64(hash, delaySig, visitLevelHash, 100);
		
		visitLevel++;
		nextVisitQueue.swap(visitQueue);
	}
	
	//cout << "ISOHASH = " << hash << endl;
}

//=============================================================================

// Compute a good hash for a rooted digraph, taking into account the
// distance between each vertex and the root (vertex at index 0).
static
void ComputeRootedDigraphVertexHashes(const ublas::compressed_matrix<double>& M,
									  vector<Signature64>& inv) 
{
	typedef ublas::matrix_row< const ublas::compressed_matrix<double> > matrix_row_t;
	typedef matrix_row_t::const_iterator row_iterator_t;

	const size_t N = M.size1();
	inv.resize(N);
	vector<double> codes;
	vector<int> bfsVisitOrder(N, -1);
	
	// Visit vertices in BFS order, from the root vertex 0
	deque<int> bfsVisit, nextBfsVisit;
	bfsVisit.push_back(0);
	int visitDepth = 1;
	bfsVisitOrder[0] = 0;
	while (!bfsVisit.empty()) {
		// Visit a depth level
		while (!bfsVisit.empty()) {
			int i = bfsVisit.front();			
			bfsVisit.pop_front();
			
			// Visit adjacent vertices of i
			size_t arcCount = 0;
			matrix_row_t M_iRow(M, i);
			row_iterator_t M_ij(M_iRow.begin()), M_iRowEnd(M_iRow.end());
			while (M_ij != M_iRowEnd) {
				const int j = (int)M_ij.index();
				
				// Save the hash code
				codes.push_back(*M_ij);
				arcCount++;
				
				// Add j to the frontier (not yet visited)
				if (bfsVisitOrder[j] == -1) {
					bfsVisitOrder[j] = visitDepth;
					nextBfsVisit.push_back(j);
				}			
				++M_ij;
			}
			
			// Compute an hash for vertex i at level visitDepth
			sort(codes.begin(), codes.end());
			inv[i].P = shuffle32(arcCount + rol32(visitDepth, 19));
			inv[i].Q = shuffle32(visitDepth + 1);
			for (size_t k=0; k<codes.size(); k++) {
				double val = codes[k];
				uint32_t C1, C2;
				unpack_double(val, C1, C2);
				uint32_t nP = inv[i].Q ^ C1;
				uint32_t nQ = rol32(inv[i].P, 3*(k&7)+2) ^ (C2 & inv[i].Q);
				inv[i].P = ror32(nP, visitDepth);
				inv[i].Q = nQ;
			}			
			codes.resize(0);
		}
		
		// Prepare the next visited level
		visitDepth++;
		bfsVisit.swap(nextBfsVisit);
	}
	
	/*for (size_t n=0; n<N; n++) {
		cout << "  BASE VERTEX["<<n<<"] -> "<<inv[n]<<endl;
	}
	cout << "\n\n";	*/

	/*/ Mix hashes together, taking into account also the BFS depth level
	typedef ublas::compressed_matrix<double>::const_iterator1 iter1_t;
	typedef ublas::compressed_matrix<double>::const_iterator2 iter2_t;
	const size_t NUM_STEPS = 1;
	for (size_t s=0; s<NUM_STEPS; s++) 
	{		
		iter1_t M_iRow(M.begin1()), M_endRow(M.end1());
		
		// Accumulate in Q hashes of the vertex neighbourhoods
		for(; M_iRow != M_endRow; ++M_iRow) {
			const size_t i = M_iRow.index1();
			const int vi = bfsVisitOrder[i];
			iter2_t M_ij(M_iRow.begin()), M_end(M_iRow.end());
			for (; M_ij != M_end; ++M_ij) {
				const size_t j = M_ij.index2();
				if (i == j)
					continue;
				const int vj = bfsVisitOrder[j];
				uint32_t C1, C2;
				unpack_double(*M_ij, C1, C2);
				inv[i].Q ^= rol32(inv[j].P, 3*(s+vi)) ^ C1;
				inv[j].Q ^= ror32(inv[i].P, 5*(s+vj)) ^ C2;
			}
		}
		
		// Mix P with Q
		for (size_t i=0; i<N; i++) {
			uint32_t oP = inv[i].P;
			uint32_t oQ = inv[i].Q;
			int vd = bfsVisitOrder[i];
			inv[i].P ^= ror32(oQ, 3*(s+3));
			inv[i].Q ^= rol32(oP, vd+(s&3)) ^ shuffle32(ror32(inv[i].P, 5)+vd);
		}
	}//*/
	
	/*for (size_t n=0; n<N; n++) {
		cout << "  VERTEX["<<n<<"] -> "<<inv[n]<<endl;
	}
	cout << endl;//*/
}

//=============================================================================

struct BackTrackStackEntry {
	int orbit, orbitEnd; // Interval of candidate vertices
	int sel2;			 // Current candidate associated vertex in M2
};
typedef struct BackTrackStackEntry  BackTrackStackEntry;


// Find an isomorphism ordering between two rooted digraphs. Vertices at index 0
// are assumed to be corresponding (this correspondence is tested anyway).
bool FindRootedDigraphIsomorphism(const ublas::compressed_matrix<double>& M1,
								  const ublas::compressed_matrix<double>& M2,
								  vector<int>& tab1to2, VerboseLevel verboseLvl)
{
	typedef ublas::matrix_row< const ublas::compressed_matrix<double> > matrix_row_t;
	typedef matrix_row_t::const_iterator row_iterator_t;
	
	const size_t N = M1.size1();
	if (N!=M1.size2() || N!=M2.size1() || N!=M2.size2())
		return false;
	tab1to2.clear();
	tab1to2.resize(N, -1);
	vector<int> tab2to1(N, -1);
	
	// Compute vertex invariants and the V2->INV2 map
	vector<Signature64> inv1, inv2;
	ComputeRootedDigraphVertexHashes(M1, inv1);
	ComputeRootedDigraphVertexHashes(M2, inv2);
	vector< pair<Signature64, size_t> > mapInv2to2;
	mapInv2to2.reserve(N-1); // Exclude the first vertex
	for (size_t n=1; n<N; n++)
		mapInv2to2.push_back(make_pair(inv2[n], n));
	sort(mapInv2to2.begin(), mapInv2to2.end());
	
	// Roots must have the same hash code
	if (!(inv1[0] == inv2[0]))
		return false;

	
	/* // Print the two graphs in DOT format
	for (size_t s=0; s<2; s++) {
		const ublas::compressed_matrix<double>& M = (s==0 ? M1 : M2);
		const vector<Signature64>& inv = (s==0 ? inv1 : inv2);
		cout << "digraph g"<<s<<" {\n";
		cout << "  rankdir=TB;\n  node [shape=ellipse];\n";
		for (size_t r=0; r<N; r++) {
			cout << "  V"<<(r+1)<<" [ label=\"SMC("<<(r+1)<<")\\n";
			cout << inv[r].P<<"\\n" << inv[r].Q<<"\" ];\n";
		}
		for (size_t r=0; r<N; r++) {
			matrix_row_t M_row(M, r);
			row_iterator_t M_rowIt = M_row.begin();
			row_iterator_t M_rowItEnd = M_row.end();
			while (M_rowIt != M_rowItEnd) {
				size_t adj = M_rowIt.index();
				if (adj != r) {
					cout << "  V"<<(r+1)<<" -> V"<<(adj+1)<<" [ label=\"";
					cout << *M_rowIt << "\" ];\n";
				}
				++M_rowIt;
			}
		}
		cout << "}\n";
	}*/
	
	

	// Visit the first graph in DFS order - note that the DFS visit allows us to
	// explore new vertices while having at least one in-edge towards them: this
	// is particularly useful for absorbing vertices.
	vector<int> ord1(N, -1);
	stack<size_t> dfsVisitStack;
	dfsVisitStack.push(0);
	ord1[0] = 0;
	size_t count = 1;
	while (!dfsVisitStack.empty()) {
		size_t v = dfsVisitStack.top();
		dfsVisitStack.pop();
		
		matrix_row_t M1_row(M1, v);
		row_iterator_t M1_rowIt = M1_row.begin();
		row_iterator_t M1_rowItEnd = M1_row.end();
		while (M1_rowIt != M1_rowItEnd) {
			uint32_t adj = M1_rowIt.index();
			if (ord1[adj] == -1) { // Not yet seen in the visit
				ord1[adj] = count++;
				dfsVisitStack.push(adj);
			}
			++M1_rowIt;
		}		
	}
	assert(count == N);
	
	if (verboseLvl >= VL_VERBOSE) {
		cout << "ISOMORPHISM TEST:\n";
		cout << "  DFS VISIT: ";
		for (size_t n=0; n<N; n++)
			cout << ord1[n] << " ";
		cout << endl;
	}
	
	// Prepare the Back-tracking stack
	vector<BackTrackStackEntry> btStack(N);
	btStack[0].orbit    = 0;
	btStack[0].orbitEnd = 0;
	btStack[0].sel2     = -1;
	tab1to2[0] = 0;
	tab2to1[0] = 0;
	
	// Verify isomorphism of the digraph roots
	if(!VerifyIsomorphicStates(M1, M2, tab1to2, tab2to1, 0))
		return false;
	
	const Signature64& vi1 = inv1[ord1[1]];// inv1[ord1[1]];
	btStack[1].orbit    = (lower_bound(mapInv2to2.begin(), mapInv2to2.end(),
									   make_pair(vi1, size_t(0))) - 
						   mapInv2to2.begin());
	btStack[1].orbitEnd = (upper_bound(mapInv2to2.begin(), mapInv2to2.end(),
									   make_pair(vi1, N)) - 
						   mapInv2to2.begin());
	btStack[1].sel2     = -1;
	
	// At this point try the back-tracking mapping
	size_t level = 1;
	while (level > 0) {
		BackTrackStackEntry& BTS = btStack[level];
		const size_t sel1 = ord1[level];
		if (verboseLvl >= VL_VERBOSE) {
			cout << "    ISOTEST(level="<<level<<", ";
			cout << "orbit="<<BTS.orbit<<"/"<<BTS.orbitEnd<<") ";		
		}
		if (BTS.orbit == BTS.orbitEnd) {
			if (verboseLvl >= VL_VERBOSE)
				cout << "ROLL BACK ONE LEVEL" << endl;
			// No associations can be found in this orbit: roll back one level
			if (BTS.sel2 != -1) {
				tab1to2[sel1] = -1;
				tab2to1[BTS.sel2] = -1;
				BTS.sel2 = -1;
			}
			level--;
			continue; 
		}
		
		const size_t candidateSel2 = mapInv2to2[BTS.orbit].second;
		++BTS.orbit;
		
		// Check if the association has already been used in a previous level
		if (tab2to1[candidateSel2] != -1) {
			if (verboseLvl >= VL_VERBOSE)
				cout << candidateSel2 << " IS ALREADY ASSOCIATED!" << endl;
			continue;
		}
		
		// Make the new association (after having cleared up the previous)
		if (BTS.sel2 != -1) {
			tab1to2[sel1] = -1;
			tab2to1[BTS.sel2] = -1;
		}
		BTS.sel2 = candidateSel2;
		assert(tab1to2[sel1] == -1 && tab2to1[BTS.sel2] == -1);
		tab1to2[sel1] = BTS.sel2;
		tab2to1[BTS.sel2] = sel1;
		if (verboseLvl >= VL_VERBOSE) {
			cout << "ASSOC "<<sel1<<" => "<<BTS.sel2<<"  ";
			cout << "tab1to2:";
			for (size_t n=0; n<N; n++) {
				if (tab1to2[n] != -1)
					cout << tab1to2[n];
				else cout << "-";
				cout << (n != N-1?",":"");
			}
			cout << "  tab2to1:";
			for (size_t n=0; n<N; n++) {
				if (tab2to1[n] != -1)
					cout << tab2to1[n];
				else cout << "-";
				cout << (n != N-1?",":"");
			}
			cout << "   ";
		}
		
		// Verify that this new association is compatible with the adjacency
		if (!VerifyIsomorphicStates(M1, M2, tab1to2, tab2to1, sel1)) {
			// Try the next vertex in the same orbit
			tab1to2[sel1] = -1;
			tab2to1[BTS.sel2] = -1;
			BTS.sel2 = -1;
			if (verboseLvl >= VL_VERBOSE)
				cout << "  Isomorphic Test Failed." << endl;
			continue;
		}
		
		if (verboseLvl >= VL_VERBOSE)
			cout << "  MATCHES!" << endl;
		
		// Final check if we have associated every vertex of M2 into M1
		if (level == N-1) {
			if (VerifyIsomorphism(M1, M2, tab1to2, tab2to1)) {
				if (verboseLvl >= VL_VERBOSE)
					cout << "  ISOMORPHISM DETECTED.\n\n" <<endl;
				return true;
			}
			
			// Otherwise, try onther association or roll back.
			if (verboseLvl >= VL_VERBOSE)
				cout << "   VerifyIsomorphism failed." << endl;
			tab1to2[sel1] = -1;
			tab2to1[BTS.sel2] = -1;
			BTS.sel2 = -1;
			continue;
		}
		
		// Explore the next backtracking level
		
		const Signature64& nvi1 = inv1[ord1[level+1]];
		btStack[level+1].orbit    = (upper_bound(mapInv2to2.begin(), mapInv2to2.end(),
												 make_pair(nvi1, size_t(0))) 
									 - mapInv2to2.begin());
		btStack[level+1].orbitEnd = (upper_bound(mapInv2to2.begin(), mapInv2to2.end(),
												 make_pair(nvi1, N)) 
									 - mapInv2to2.begin());
		btStack[level+1].sel2     = -1;
		level++;
	}
	
	if (verboseLvl >= VL_VERBOSE) {
		cout << "  ISOMORPHISM TEST FAILED: no strong isomorphism found.\n" << endl;
	}
	return false;
}

//=============================================================================












