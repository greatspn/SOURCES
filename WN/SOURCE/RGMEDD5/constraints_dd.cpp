#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <climits>
// #include <math.h>
#include <algorithm>
// #include <numeric>
#include <memory>
#include <map>
#include <unordered_set>
// #include <set>
#include <stack>
// #include <cstdint>

#include "rgmedd5.h"
#include "varorders.h"
// #include "bufferless_sort.h"
// #include "optimization_finder.h"
// #include "utils/thread_pool.h"
// #include "utils/mt64.h"
// #include "dlcrs_matrix.h"

#include "irank.h"


//---------------------------------------------------------------------------------------

struct edge_t {
    int value;
    size_t node_ref;
};

//---------------------------------------------------------------------------------------

class node_t {
    int lvl; // can be negative (terminal levels)
public:
    std::vector<int> psums; // partial sums of the constraints
    std::vector<edge_t> ee; // value/node-ref pairs (edges)
    size_t precomputed_hash;

    inline node_t(int _lvl=int(-100), size_t _num_constr=0) 
    : lvl(_lvl), psums(_num_constr, -100) { }

    node_t(const node_t& ps) = default;
    node_t(node_t&&) = default;
    node_t& operator=(const node_t&) = default;
    node_t& operator=(node_t&&) = default;

    // join two partial sums into a single empty node
    inline node_t(const node_t& ps1, const node_t& ps2) 
    : lvl(max(ps1.lvl, ps2.lvl)), psums(ps1.num_psums() + ps2.num_psums())
    {
        size_t j=0;
        for (size_t i=0; i<ps1.num_psums(); i++)
            psums[j++] = ps1.psums[i];
        for (size_t i=0; i<ps2.num_psums(); i++)
            psums[j++] = ps2.psums[i];
    }

    inline ~node_t() { }

    void swap(node_t& ps);

    inline size_t num_psums() const { return psums.size(); }
    inline int level() const { return lvl; }
    inline bool is_terminal() const { return lvl<0; }
    inline bool is_true() const { return lvl == -1; }
    inline bool is_false() const { return lvl == -2; }

    void precompute_hash();

    // inline bool operator<(const node_t& rhs) const {
    //     assert(num_psums() == rhs.num_psums());
    //     if (lvl < rhs.lvl)
    //         return true;
    //     else if (lvl > rhs.lvl)
    //         return false;
    //     for (size_t cc=0; cc<num_psums(); cc++) {
    //         if (at(cc) < rhs.at(cc))
    //             return true; // *this < rhs
    //         else if (at(cc) > rhs.at(cc))
    //             return false; // *this > rhs
    //     }
    //     return false; // *this == rhs
    // }

    bool psums_equal(const node_t& rhs) const;
};
ostream& operator<<(ostream& os, const node_t& psum);

//---------------------------------------------------------------------------------------

void node_t::swap(node_t& ps) {
    std::swap(lvl, ps.lvl);
    std::swap(psums, ps.psums);
    std::swap(ee, ps.ee);
    std::swap(precomputed_hash, ps.precomputed_hash);
}

//---------------------------------------------------------------------------------------

ostream& operator<<(ostream& os, const node_t& node) {
    cout <<"lvl=";
    if (node.is_terminal())
        cout << (node.is_true() ? "T:" : "F:");
    else
        cout << node.level() << ":";
    for (size_t i=0; i<node.num_psums(); i++)
        os << (i==0?"":",") << node.psums[i];
    os << "[";
    for (size_t i=0; i<node.ee.size(); i++)
        os << (i==0?"":",") << node.ee[i].value;
    os << "]";

    return os;
}

//---------------------------------------------------------------------------------------

bool node_t::psums_equal(const node_t& rhs) const {
    // cout << "psums_equal " << (*this) << "   " << rhs << endl;
    assert(num_psums() == rhs.num_psums());
    if (lvl != rhs.lvl)
        return false;
    for (size_t cc=0; cc<num_psums(); cc++)
        if (psums[cc] != rhs.psums[cc])
            return false;
    return true;
}

//---------------------------------------------------------------------------------------

template<typename T>
inline T bit_rotl(const T value, int shift) {
    return (value << shift) | (value >> (sizeof(value)*CHAR_BIT - shift));
}

inline size_t hash_combine(size_t seed) { return seed; }

template <typename T, typename... Rest>
inline size_t hash_combine(size_t seed, T v, Rest&&... rest) {
    seed = std::hash<T>{}(v) + 0xd2911b3ffc2dd383 + bit_rotl(seed, 6);
    return hash_combine(seed, std::forward<Rest>(rest)...);
}

//---------------------------------------------------------------------------------------

void node_t::precompute_hash() {
    precomputed_hash = 0x3ea4a8cdab71bde9;
    for (size_t cc=0; cc<num_psums(); cc++)
        precomputed_hash = hash_combine(precomputed_hash, size_t(psums[cc]));
}

//---------------------------------------------------------------------------------------




//---------------------------------------------------------------------------------------

class CDD_t;
struct hashed_node_t {
    size_t node_id; // node id or -1
    union {
        CDD_t* p_dd;          // when node_id !=-1
        const node_t* p_node; // when node_id == -1
    };

    inline const node_t& get_node() const;
    bool operator==(const hashed_node_t& rhs) const;
};

namespace std {
    template<> struct hash<hashed_node_t> {
        std::size_t operator()(const hashed_node_t& k) const;
    };
}

//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// Simple parial sum for single constraint initialization
struct single_psum_t {
    int lvl;
    int psum;

    inline bool operator==(const single_psum_t& sps) const {
        return lvl==sps.lvl && psum==sps.psum;
    }
};
namespace std {
    template<> struct hash<single_psum_t> {
        std::size_t operator()(const single_psum_t& sps) const {
            return hash_combine(sps.lvl, sps.psum);
        }
    };
}
//---------------------------------------------------------------------------------------







//---------------------------------------------------------------------------------------
// Constraint Decision Diagram
//---------------------------------------------------------------------------------------
class CDD_t {
    // Constraints table
    const flow_basis_metric_t& fbm;
    // Constraint indices in the footprint matrix fbm.B
    std::vector<size_t> constrs; 

    // Node table
    std::vector<node_t> forest;
    // Node cache
    std::unordered_set<hashed_node_t> cache;

    static const size_t T; // terminal True
    static const size_t F; // terminal False

    // Root node
    size_t root_node_id;
public:
    CDD_t(const flow_basis_metric_t& _fbm, size_t cc) : fbm(_fbm), constrs{cc} { }

    inline size_t num_nodes() const { return forest.size(); }
    size_t num_edges() const;

    // initialize for a single constraint
    void initialize();

    // remove dangling nodes
    bool collect_unused_nodes();

    void show(ostream& os) const;

    // Next node after assigning value to the variable at node.level()
    node_t next(const node_t& node, int value) const;

    // intersection between two cdds
    void intersection(const CDD_t& c1, const CDD_t& c2);

    // get the domain of the variables for the active levels
    std::map<size_t, std::vector<bool>> mark_var_domains() const;

    // remove variable values from all nodes
    void intersect_var_domains(const std::vector<std::vector<bool>>& var_doms);

    // count nodes/edges by level
    void count_nodes_edges(std::vector<size_t>& nodes, std::vector<size_t>& edges) const;

    // count assigned variable values per level
    // void count_values_per_level(std::vector<std::map<int, size_t>>& vpl) const;

private:
    typedef std::map<std::pair<size_t, size_t>, size_t> intersection_op_cache_t;
    size_t intersect_recursive(const CDD_t& c1, const CDD_t& c2,
                               const size_t node_id1, const size_t node_id2,
                               intersection_op_cache_t& op_cache);

    typedef std::unordered_map<single_psum_t, size_t> initialize_op_cache_t;
    size_t initialize_recursive(node_t&& node,
                                initialize_op_cache_t& op_cache);

    typedef std::map<size_t, size_t> intersect_var_domains_cache_t;
    size_t intersect_var_domains_recursive(const std::vector<std::vector<bool>>& var_doms,
                                           const size_t node_id,
                                           intersect_var_domains_cache_t& op_cache);

    size_t add_node(node_t&& node);

    void mark_recursively(std::vector<bool>& in_use, size_t node_id) const;

    friend struct std::hash<hashed_node_t>;
    friend struct hashed_node_t;
};

//---------------------------------------------------------------------------------------

const size_t CDD_t::T = 1;
const size_t CDD_t::F = 0;

//---------------------------------------------------------------------------------------

namespace std {
    std::size_t hash<hashed_node_t>::operator()(const hashed_node_t& k) const {
        return k.get_node().precomputed_hash;
    }
};

const node_t& hashed_node_t::get_node() const { 
    return (node_id != -1 ? p_dd->forest[node_id] : *p_node); 
}

bool hashed_node_t::operator==(const hashed_node_t& rhs) const {
    return get_node().psums_equal(rhs.get_node());
}

//---------------------------------------------------------------------------------------

void CDD_t::show(ostream& os) const {
    os << "CONSTRAINTS";
    for (size_t cc : constrs) 
        os << " " << cc;
    os << endl;

    for (int lvl=npl-1; lvl>=-1; lvl--) {
        int count = 0;

        for (const node_t& node : forest) {
            if (node.level() != lvl)
                continue;

            if (count==0) {
                const char* lvl_name;
                if (lvl >= 0)
                    lvl_name = tabp[ fbm.level_to_net[node.level()] ].place_name;
                else
                    lvl_name = "TERM";
                os << " @" << left << setw(5) << lvl_name;
            }
            os << " ";
            // print the psum
            for (size_t i=0; i<node.num_psums(); i++)
                os << (i==0?"":",") << node.psums[i];
            os << "[";
            // print the allowed assignments
            for (size_t i=0; i<node.ee.size(); i++)
                os << (i==0?"":",") << node.ee[i].value;
            os << "]";  
            count++;
        }
        if (count)
            os << endl;
    }
}

//---------------------------------------------------------------------------------------

size_t CDD_t::num_edges() const {
    size_t cnt = 0;
    for (const node_t& node : forest)
        cnt += node.ee.size();
    return cnt;
}

//---------------------------------------------------------------------------------------

void CDD_t::initialize() {
    assert(constrs.size() == 1);
    size_t icc = constrs[0];
    const int_lin_constr_t& constr = fbm.B[icc];
    forest.clear();

    // initialize the terminal elements
    forest.push_back(node_t(-2, 1)); // node 0
    forest[F].psums[0] = -1000;
    forest.push_back(node_t(-1, 1)); // node 1
    forest[T].psums[0] = constr.const_term;

    if (constr.coeffs.nonzeros() == 0)
        root_node_id = T;
    else {
        // initialize the root element
        node_t root_node(constr.coeffs.trailing(), 1);
        root_node.psums[0] = 0;

        initialize_op_cache_t op_cache;

        // Fill the DD
        root_node_id = initialize_recursive(std::move(root_node), op_cache);
    }
}

//---------------------------------------------------------------------------------------

size_t CDD_t::initialize_recursive(node_t&& node, initialize_op_cache_t& op_cache) {
    // cout << "initialize_recursive: " << node << endl;
    assert(constrs.size() == 1);
    if (node.is_true())
        return node.psums_equal(forest[T]) ? T : F;
    if (node.is_false())
        return F;

    // search in operation cache
    single_psum_t op_key{.lvl=node.level(), .psum=node.psums[0]};
    auto op_it = op_cache.find(op_key);
    if (op_it != op_cache.end())
        return op_it->second;

    // search in node cache
    node.precompute_hash();
    hashed_node_t hn {.node_id=size_t(-1), .p_node=&node};
    auto it = cache.find(hn);
    if (it != cache.end()) // node exists
        return it->node_id;
    
    size_t icc = constrs[0];
    const int_lin_constr_t& constr = fbm.B[icc];

    const int plc = fbm.level_to_net[node.level()];
    const int bound = fbm.get_bound(plc);
    const int coeff = constr.coeffs[node.level()];

    // Add the links to downward nodes
    for (int v=0; v<=bound; v++) {
        node_t next_node = next(node, v);
        assert(next_node.num_psums() == 1);

        size_t new_node_id = initialize_recursive(std::move(next_node), op_cache);
        if (new_node_id != F)
            node.ee.push_back(edge_t{.value=v, .node_ref=new_node_id});
    }
    // Add the node to the forest and to the cache
    size_t node_id = add_node(std::move(node));
    
    // Add to the operation cache
    op_cache.insert(make_pair(op_key, node_id));

    return node_id;
}

//---------------------------------------------------------------------------------------

size_t CDD_t::add_node(node_t&& node) {
    // cout << "add_node " << node << endl;
    if (node.ee.empty())
        return F;
    assert(!node.is_terminal());

    // Start by appending at the end of the forest
    size_t node_id = forest.size();
    node.precompute_hash();
    forest.emplace_back(node);
    // Search the node in the cache
    hashed_node_t hn {.node_id=node_id, .p_dd=this};
    auto it = cache.find(hn);
    if (it != cache.end()) { // node exists
        forest.pop_back();
        node_id = it->node_id;
        // cout << "USING " << node_id << " as " << forest[node_id] << endl;
    }
    else { // New node
        cache.emplace(std::move(hn));
        // cout << "NEW " << node_id << " as " << forest[node_id] << endl;
    }

    return node_id;
}

//---------------------------------------------------------------------------------------

node_t CDD_t::next(const node_t& node, int value) const {
    // cout <<"next "<< node <<"  value="<<value<<endl;
    assert(constrs.size() == 1);
    assert(node.num_psums() == 1);
    const int_lin_constr_t& constr = fbm.B[constrs[0]];
    const int next_psum = node.psums[0] + constr.coeffs[node.level()] * value;

    if (node.level() <= constr.coeffs.leading()) { // leading term
        return (next_psum == constr.const_term) ? forest[T] : forest[F];
    }
    // determine the next level
    const int ii = constr.coeffs.lower_bound_nnz(node.level());
    const int lvl_below = constr.coeffs.ith_nonzero(ii - 1).index;
    int next_lvl = lvl_below;

    node_t next_node(next_lvl, 1);
    next_node.psums[0] = next_psum;
    // cout << node <<" next value="<<value<<" is "<< next_node<<endl;
    return next_node;

    // // determine the next level
    // int next_lvl = -1;
    // for (size_t cc=0; cc<constrs.size(); cc++) {
    //     const int_lin_constr_t& constr = fbm.B[constrs[cc]];
    //     if (node.level() <= constr.coeffs.leading()) { // leading term
    //         next_lvl = max(next_lvl, -1);
    //     }
    //     else if (node.level() > constr.coeffs.trailing()) { // trailing term
    //         next_lvl = max(next_lvl, int(constr.coeffs.trailing()));
    //     }
    //     else {
    //         const int ii = constr.coeffs.lower_bound_nnz(node.level());
    //         const int lvl_below = constr.coeffs.ith_nonzero(ii - 1).index;
    //         next_lvl = max(next_lvl, lvl_below);
    //     }
    // }

    // node_t next_node(next_lvl, node.num_psums());
    // for (size_t cc=0; cc<constrs.size(); cc++) {
    //     const int_lin_constr_t& constr = fbm.B[constrs[cc]];
    //     next_node.psums[cc] = node.psums[cc] + constr.coeffs[node.level()] * value;
    // }
    // // cout << node <<" next value="<<value<<" is "<< next_node<<endl;
    // return next_node;
}

//---------------------------------------------------------------------------------------

void CDD_t::mark_recursively(std::vector<bool>& in_use, size_t node_id) const {
    if (in_use[node_id])
        return; // already visited and marked recursively
    in_use.at(node_id) = true;
    if (node_id == T || node_id == F)
        return;
    const node_t& node = forest[node_id];
    for (const edge_t& e : node.ee) {
        mark_recursively(in_use, e.node_ref);
    }
}

//---------------------------------------------------------------------------------------

bool CDD_t::collect_unused_nodes() {
    std::vector<bool> in_use(forest.size(), false);
    std::fill(in_use.begin(), in_use.end(), false);

    // Mark nodes in use
    mark_recursively(in_use, root_node_id);
    // Anyway, always mark the root and the terminals to avoid removals
    in_use[T] = in_use[F] = in_use[root_node_id] = true;

    if (std::find(in_use.begin(), in_use.end(), false) == in_use.end())
        return false; // nothing to be removed

    std::vector<size_t> remap(forest.size());
    size_t count = 0;
    for (size_t i=0; i<forest.size(); i++) {
        if (in_use[i]) {
            remap[i] = count;
            if (count != i)
                std::swap(forest[count], forest[i]);
            count++;
        }
    }
    forest.resize(count);

    // Remap edges
    for (node_t& node : forest) {
        for (edge_t& edge : node.ee)
            edge.node_ref = remap[edge.node_ref];
    }

    // Rebuild the cache
    cache.clear();
    for (size_t i=0; i<forest.size(); i++) {
        cache.insert({.node_id=i, .p_dd=this});
    }

    // Remap the root node
    root_node_id = remap[root_node_id];

    return true;
}

//---------------------------------------------------------------------------------------

void CDD_t::intersection(const CDD_t& c1, const CDD_t& c2) {
    forest.clear();
    cache.clear();
    constrs.clear();
    constrs.insert(constrs.end(), c1.constrs.begin(), c1.constrs.end());
    constrs.insert(constrs.end(), c2.constrs.begin(), c2.constrs.end());

    // create the terminal nodes
    forest.push_back(node_t(c1.forest[c1.F], c2.forest[c2.F])); // node 0
    forest.push_back(node_t(c1.forest[c1.T], c2.forest[c2.T])); // node 1

    // create the root nodes
    node_t root_node(c1.forest[c1.root_node_id], c2.forest[c2.root_node_id]);

    // do the intersection
    intersection_op_cache_t op_cache;
    root_node_id = intersect_recursive(c1, c2, c1.root_node_id, c2.root_node_id, op_cache);
}

//---------------------------------------------------------------------------------------

size_t CDD_t::intersect_recursive(const CDD_t& c1, const CDD_t& c2,
                                  const size_t node_id1, const size_t node_id2,
                                  intersection_op_cache_t& op_cache)
{
    if (node_id1 == c1.T)
        return node_id2 == c2.T ? T : F;
    else if (node_id1 == c1.F)
        return F;

    // Search the operation cache
    auto cache_key = make_pair(node_id1, node_id2);
    auto cache_id = op_cache.find(cache_key);
    if (cache_id != op_cache.end())
        return cache_id->second;

    node_t new_node(c1.forest[node_id1], c2.forest[node_id2]);
    const node_t& node1 = c1.forest[node_id1];
    const node_t& node2 = c2.forest[node_id2];

    // Perform edge intersection
    if (new_node.level() != node2.level()) {
        for (const edge_t& ee1 : node1.ee) {
            size_t next_id = intersect_recursive(c1, c2, ee1.node_ref, node_id2, op_cache);
            if (next_id != F)
                new_node.ee.push_back(edge_t{.value=ee1.value, .node_ref=next_id});
        }
    }
    else if (new_node.level() != node1.level()) {
        for (const edge_t& ee2 : node2.ee) {
            size_t next_id = intersect_recursive(c1, c2, node_id1, ee2.node_ref, op_cache);
            if (next_id != F)
                new_node.ee.push_back(edge_t{.value=ee2.value, .node_ref=next_id});
        }
    }
    else {
        size_t i1 = 0, i2 = 0;
        while (i1 < node1.ee.size() && i2 < node2.ee.size()) {
            if (node1.ee[i1].value < node2.ee[i2].value)
                i1++;
            else if (node1.ee[i1].value > node2.ee[i2].value)
                i2++;
            else {
                // Generate the intersection node
                size_t next_id = intersect_recursive(c1, c2, node1.ee[i1].node_ref, node2.ee[i2].node_ref, op_cache);
                if (next_id != F)
                    new_node.ee.push_back(edge_t{.value=node1.ee[i1].value, .node_ref=next_id});
                i1++;
                i2++;
            }
        }
    }

    // Add the node to the forest and to the cache
    size_t node_id = add_node(std::move(new_node));
    op_cache.insert(make_pair(cache_key, node_id));

    return node_id;
}

//---------------------------------------------------------------------------------------

std::map<size_t, std::vector<bool>> CDD_t::mark_var_domains() const {
    // initialize the domain
    std::map<size_t, std::vector<bool>> level_doms;
    // mark all domain values used by the DD edges
    for (const node_t& node : forest) {
        if (node.is_terminal())
            continue;
        if (level_doms.count(node.level()) == 0) {
            // initialize the level
            const int plc = fbm.level_to_net[node.level()];
            const int bound = fbm.get_bound(plc);
            std::vector<bool> ld(bound + 1, false);
            level_doms[node.level()] = ld;
        }
        std::vector<bool>& doms = level_doms[node.level()];
        // cout << "marking node: " << node << endl;
        for (const edge_t& edge : node.ee)
            doms[edge.value] = true;
    }
    return level_doms;
}

//---------------------------------------------------------------------------------------

void CDD_t::intersect_var_domains(const std::vector<std::vector<bool>>& var_doms) {
    intersect_var_domains_cache_t op_cache;
    root_node_id = intersect_var_domains_recursive(var_doms, root_node_id, op_cache);
    collect_unused_nodes();
}

//---------------------------------------------------------------------------------------

size_t CDD_t::intersect_var_domains_recursive(const std::vector<std::vector<bool>>& var_doms,
                                              const size_t node_id,
                                              intersect_var_domains_cache_t& op_cache)
{
    // cout << "intersect_var_domains_recursive node_id="<<node_id<<"  "<<forest[node_id]<<endl;
    if (node_id == T || node_id == F)
        return node_id; // nothing to do

    size_t cache_key = node_id;
    auto cache = op_cache.find(cache_key);
    if (cache != op_cache.end())
        return cache->second;

    node_t& node = forest[node_id];

    std::vector<edge_t> new_edges;
    for (const edge_t& edge : node.ee) {
        if (var_doms[node.level()][edge.value]) {
            size_t next_id = intersect_var_domains_recursive(var_doms, edge.node_ref, op_cache);
            if (next_id != F)
                new_edges.push_back(edge_t{.value=edge.value, .node_ref=next_id});
        }
    }
    std::swap(new_edges, node.ee);

    size_t new_node_id = node_id;
    if (node.ee.empty())
        new_node_id = F;
    
    // cout << "intersect dom: " << node_id << "->" << new_node_id << "  " << node << endl;
    op_cache.insert(make_pair(cache_key, new_node_id));

    return new_node_id;
}

//---------------------------------------------------------------------------------------

void CDD_t::count_nodes_edges(std::vector<size_t>& nodes, std::vector<size_t>& edges) const {
    nodes.clear();
    edges.clear();
    nodes.resize(npl, 0);
    edges.resize(npl, 0);
    for (const node_t& node : forest) {
        if (node.is_terminal())
            continue;

        nodes[node.level()]++;
        edges[node.level()] += node.ee.size();
    }
}

//---------------------------------------------------------------------------------------

// void CDD_t::count_values_per_level(std::vector<std::map<int, size_t>>& vpl) const {
//     vpl.clear();
//     vpl.resize(npl);
//     for (const node_t& node : forest) {
//         if (node.is_terminal())
//             continue;

//         for (const edge_t& edge : node.ee) {
//             auto& vmap = vpl[node.level()];
//             auto it = vmap.find(edge.value);
//             if (it == vmap.end())
//                 vmap[edge.value] = 1;
//             else
//                 it->second++;
//         }
//     }
// }

//---------------------------------------------------------------------------------------








//---------------------------------------------------------------------------------------

std::vector<std::vector<bool>> initialize_level_domains(const flow_basis_metric_t& fbm) 
{
    std::vector<std::vector<bool>> dom(npl);
    for (size_t lvl=0; lvl<dom.size(); lvl++) {
        const int plc = fbm.level_to_net[lvl];
        const int bound = fbm.get_bound(plc);
        dom[lvl].resize(bound + 1, true);
    }
    return dom;
}

//---------------------------------------------------------------------------------------

bool intersect_level_domains(std::vector<std::vector<bool>>& dom,
                             const std::map<size_t, std::vector<bool>>& level_doms) 
{
    bool changed = false;
    for (auto iter : level_doms) {
        const size_t lvl = iter.first;
        assert(iter.second.size() == dom[lvl].size());
        for (size_t i=0; i<iter.second.size(); i++)
            if (!iter.second[i] && dom[lvl][i]) {
                dom[lvl][i] = false;
                changed = true;
                // cout << "removing " << i << " from level " << lvl << endl;
            }
    }
    return changed;
}

//---------------------------------------------------------------------------------------

void show_level_domains(std::vector<std::vector<bool>>& dom) {
    for (ssize_t lvl = dom.size()-1; lvl >=0; lvl--) {
        cout << "LEVEL " << setw(2) << lvl << ": ";
        for (size_t i=0; i<dom[lvl].size(); i++) {
            // cout << " " << (dom[lvl][i] ? "T" : "F");
            if (dom[lvl][i])
                cout << " " << i;
        }
        cout << endl;
    }
}

//---------------------------------------------------------------------------------------

void propagate(std::vector<size_t>& values, size_t leading, size_t trailing) {
    assert(0 <= leading && leading <= trailing && trailing < npl);
    size_t memory = 0;
    for (size_t l=leading; l<=trailing; l++) {
        if (values[l] != 0)
            memory = values[l];
        else
            values[l] = memory;
    }
}

//---------------------------------------------------------------------------------------

void experiment_cdd(const flow_basis_metric_t& fbm) {
    bool verbose = false;
    std::vector<std::vector<bool>> dom = initialize_level_domains(fbm);
    std::vector<std::unique_ptr<CDD_t>> constr_dd(fbm.B.size());

    for (size_t i=0; i<fbm.B.size(); i++) {
        constr_dd[i] = make_unique<CDD_t>(fbm, i);
        cout << "initialize constraint " << i << "/" << fbm.B.size() << endl;
        constr_dd[i]->initialize();
        constr_dd[i]->collect_unused_nodes();
        cout << "nodes="<<constr_dd[i]->num_nodes()<<" edges="<<constr_dd[i]->num_edges()<<endl;
        // constr_dd[i]->show(cout);
        // cout << endl;

        std::map<size_t, std::vector<bool>> level_doms;
        level_doms = constr_dd[i]->mark_var_domains();
        // for (auto iter : level_doms) {
        //     cout << "LVL " << iter.first << " : ";
        //     for (size_t i=0; i<iter.second.size(); i++)
        //         if (iter.second[i])
        //             cout << " " << i;
        //     cout << endl;
        // }
        // cout << endl;

        intersect_level_domains(dom, level_doms);
        // show_level_domains(dom);
        // cout << endl;
    }

    bool repeat = true;
    while (repeat) {
        cout << "\n\nVARIABLE DOMAINS:" << endl;
        show_level_domains(dom);
        cout << endl;
        repeat = false;
        for (size_t i=0; i<fbm.B.size(); i++) {
            size_t sz1 = constr_dd[i]->num_nodes();
            constr_dd[i]->intersect_var_domains(dom);
            size_t sz2 = constr_dd[i]->num_nodes();
            // cout << "constraint " << i << " from " << sz1 << " to " << sz2 << endl;
            // constr_dd[i]->show(cout);
            // cout << endl;
        }

        for (size_t i=0; i<fbm.B.size(); i++) {
            std::map<size_t, std::vector<bool>> level_doms;
            level_doms = constr_dd[i]->mark_var_domains();
            if (intersect_level_domains(dom, level_doms))
                repeat = true;
        }
    }
    cout << "\n\n";
    // exit(0);

    std::vector<std::vector<size_t>> node_counts(fbm.B.size());
    std::vector<size_t> nodes, edges;
    for (size_t i=0; i<fbm.B.size(); i++) {
        if (fbm.B[i].coeffs.nonzeros() > 0) {
            constr_dd[i]->count_nodes_edges(node_counts[i], edges);
            propagate(node_counts[i], fbm.B[i].coeffs.leading(), fbm.B[i].coeffs.trailing());
        }
    }

    // std::vector< std::vector<std::map<int, size_t>> > all_vpl(fbm.B.size());
    // for (size_t i=0; i<fbm.B.size(); i++) {
    //     constr_dd[i]->count_values_per_level(all_vpl[i]);
    // }

    // std::vector<double> discount_factors(npl, 1.0);

    
    // for (size_t i=0; i<fbm.B.size() - 1; i++) {
    //     CDD_t isect_pair(fbm, 0);
    //     isect_pair.intersection(*constr_dd[i], *constr_dd[i+1]);
    //     isect_pair.collect_unused_nodes();
    //     isect_pair.count_nodes_edges(nodes, edges);
    //     propagate(nodes, 
    //               min(fbm.B[i].coeffs.leading(), fbm.B[i+1].coeffs.leading()), 
    //               max(fbm.B[i].coeffs.trailing(), fbm.B[i+1].coeffs.trailing()));
    //     for (size_t lvl=0; lvl<npl; lvl++) {      
    //         size_t prod = node_counts[i][lvl] * node_counts[i+1][lvl];
    //         if (prod > 0) {
    //             double discount = double(nodes[lvl]) / prod;
    //             discount_factors[lvl] *= discount;
    //             // cout << "discount i="<<i<<" lvl="<<lvl<<"  f="<<discount<<endl;
    //         }
    //     }
    // }

    // Compute discount factors
    /*
    verbose = false;
    cout << "SCORES WITH DISCOUNT FACTORS:\n";
    for (size_t K=1; K<=fbm.B.size(); K++) {
        clock_t timeDF = clock();
        std::vector<double> discount_factors;
        if (K>=2) {
            discount_factors.resize(npl, 1.0);
            for (size_t start=0; start<fbm.B.size()-1; start+=K) {
                size_t end = min(start+K, fbm.B.size());
                if (verbose) {
                    cout << "DISCOUNT FACTORS start="<<start<<" K="<<K<<" end="<<end<<"  seq=";
                    for (size_t j=start; j<end; j++)
                        cout << j << " ";
                    cout << endl;
                }

                unique_ptr<CDD_t> isect_n = make_unique<CDD_t>(fbm, 0);
                isect_n->intersection(*constr_dd[start], *constr_dd[start+1]);
                isect_n->collect_unused_nodes();
                for (size_t j=start+2; j<end; j++) {
                    unique_ptr<CDD_t> isect_swap = make_unique<CDD_t>(fbm, 0);
                    isect_swap->intersection(*isect_n, *constr_dd[j]); 
                    isect_swap->collect_unused_nodes();
                    isect_n = std::move(isect_swap);
                }
                isect_n->count_nodes_edges(nodes, edges);

                size_t leading = npl, trailing = 0;
                for (size_t j=start; j<end; j++) {
                    leading = min(leading, fbm.B[j].coeffs.leading());
                    trailing = max(trailing, fbm.B[j].coeffs.trailing());
                }
                propagate(nodes, leading, trailing);

                for (size_t lvl=0; lvl<npl; lvl++) {      
                    size_t prod = 1;
                    for (size_t j=start; j<end; j++) {
                        if (node_counts[j][lvl] > 0)
                            prod *= node_counts[j][lvl];
                    }
                    if (prod > 0 && nodes[lvl] > 0) {
                        double discount = double(nodes[lvl]) / prod;
                        discount_factors[lvl] *= discount;
                        // cout << "discount start="<<start<<" lvl="<<lvl<<"  f="<<discount<<endl;
                    }
                }
            }
        }

        // Compute the N(K) score
        cardinality_t NK=0;
        if (verbose)
            cout << "\nSCORE K="<<K<<endl;
        for (ssize_t lvl=npl-1; lvl>=0; lvl--) {
            double prod = 1;
             if (verbose)
                cout << " lvl "<<setw(3)<<lvl<<setw(5)<<tabp[fbm.level_to_net[lvl]].place_name<<":";
            for (size_t i=0; i<fbm.B.size(); i++) {
                if (fbm.B[i].coeffs.nonzeros() == 0)
                    continue;
                if (node_counts[i][lvl] > 0) {
                    prod *= node_counts[i][lvl];
                    if (verbose)
                        cout << " " << node_counts[i][lvl];
                }
            }
            if (!discount_factors.empty()) {
                prod *= discount_factors[lvl];
                if (verbose)
                    cout << " * " << discount_factors[lvl];
            }
            if (verbose)
                cout << " = " << prod << endl;

            NK += cardinality_t(std::round(prod));
        }
        timeDF = clock() - timeDF;

        cout << "  N("<<K<<") = " << setw(20) << NK << (verbose?"\n\n":"") 
             << "\ttime: " << (timeDF/double(CLOCKS_PER_SEC)) << endl;
 
        // // Count the E(K) score
        // for (ssize_t lvl=npl-1; lvl>=0; lvl--) {
        //     double prod = 1;
        //     //  if (verbose)
        //     //     cout << " lvl "<<setw(3)<<lvl<<setw(5)<<tabp[fbm.level_to_net[lvl]].place_name<<":";
        //     for (size_t i=0; i<fbm.B.size(); i++) {
        //         if (node_counts[i][lvl] > 0) {
        //             prod *= node_counts[i][lvl];
        //             if (verbose)
        //                 cout << " " << node_counts[i][lvl];
        //         }
        //     }
        // }

    }*/


    cout << "\n\n---------------------\n";
    // exit(0);
}

//---------------------------------------------------------------------------------------

std::vector<sparse_vector_t> transpose(const int_lin_constr_vec_t& B) {
    if (B.empty())
        return std::vector<sparse_vector_t>();
    std::vector<sparse_vector_t> T(B[0].coeffs.size(), sparse_vector_t(B.size()));

    for (size_t r=0; r<B.size(); r++) {
        const sparse_vector_t& row = B[r].coeffs;
        for (size_t nz=0; nz<row.nonzeros(); nz++) {
            auto c = row.ith_nonzero(nz);
            T[c.index].add_element(r, c.value);
        }
    }
    return T;
}

//---------------------------------------------------------------------------------------







//---------------------------------------------------------------------------------------

int
compute_variable_groups(const int_lin_constr_vec_t& B, 
                        std::vector<int>& varsets, 
                        int excluded_var=-1) 
{
    std::vector<sparse_vector_t> TB = transpose(B);

    // Find all the connected sets
    varsets.clear();
    varsets.resize(npl, -1);
    std::vector<bool> constr_visited(B.size(), false);
    int set_id = 0;
    size_t start_var = 0;
    while (start_var < varsets.size()) {
        if (varsets[start_var] != -1 || start_var==excluded_var) {
            ++start_var;
            continue;
        }

        std::stack<size_t> visit_constr;
        for (auto it : TB[start_var]) {
            if (!constr_visited[it.index]) {
                visit_constr.push(it.index);
                constr_visited[it.index] = true;
            }
        }
        // visit the constraints in the visit queue
        while (!visit_constr.empty()) {
            size_t cc = visit_constr.top();
            visit_constr.pop();

            for (auto it : B[cc].coeffs) {
                size_t vv = it.index; // variables in constraint cc
                if (varsets[vv] == -1 && vv != excluded_var) {
                    varsets[vv] = set_id;
                    // transitively mark all constraints that share vv
                    for (auto it : TB[vv]) {
                        if (!constr_visited[it.index]) {
                            visit_constr.push(it.index);
                            constr_visited[it.index] = true;
                        }
                    }
                }
            }
        }
        ++set_id;
        ++start_var;
    }

    // for (int grp : varsets)
    //     cout << " " << grp;
    // cout << endl;

    return set_id;
}

//---------------------------------------------------------------------------------------

void experiment_footprint_chaining(const std::vector<int>& net_to_mddLevel) {
    int_lin_constr_vec_t B = get_int_constr_problem();
    reorder_basis(B, net_to_mddLevel);
    reduced_row_footprint_form(B);

    // cout << "\n\nB=\n";
    // print_flow_basis(B);
    // cout << "\n\nTB=\n";
    // print_flow_basis(TB);
    // cout << endl;

    /*/ Find all the connected sets
    std::vector<int> varsets;
    int n_groups = compute_variable_groups(B, varsets);

    for (int id=0; id<n_groups; id++) {
        cout << "GROUP " << id << ": ";
        for (size_t pl=0; pl<npl; pl++) {
            int lvl = net_to_mddLevel[pl];
            if (varsets[lvl] == id) {
                cout << tabp[pl].place_name << " ";
            }
        }
        cout << endl;
    }

    // search if there is a separator for two chainings
    for (size_t pl=0; pl<npl; pl++) {
        int lvl = net_to_mddLevel[pl];
        std::vector<int> varsets2;
        int n_groups2 = compute_variable_groups(B, varsets2, lvl);
        if (n_groups2 != n_groups) {
            cout << "Variable " << tabp[pl].place_name << " is a separator." << endl;
        }
    }*/

    // // try random reorders
    // std::vector<int> reorder = net_to_mddLevel;
    // for (size_t i=0; i<100; i++) {
    //     std::swap(reorder[genrand64_int63() % reorder.size()],
    //               reorder[genrand64_int63() % reorder.size()]);
    //     B = get_int_constr_problem();
    //     reorder_basis(B, reorder);
    //     reduced_row_footprint_form(B);

    //     std::vector<int> varsets2;
    //     int n_groups2 = compute_variable_groups(B, varsets);

    //     // bool same = 
    // }
}

//---------------------------------------------------------------------------------------

