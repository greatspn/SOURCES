#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <algorithm>
#include <numeric>
#include <map>
#include <stack>
#include <string.h>
#include <sys/stat.h> // check for MCL tool (mcl) in $PATH
#include <signal.h> // SIG_IGN, SIG_DFL for SIGPIPE handler

#include "rgmedd5.h"
#include "varorders.h"

// Boost graph 
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/cuthill_mckee_ordering.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/bandwidth.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/minimum_degree_ordering.hpp>
#include <boost/graph/king_ordering.hpp>
#include <boost/graph/sloan_ordering.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/profile.hpp>
#include <boost/graph/wavefront.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/graph/topological_sort.hpp>

//---------------------------------------------------------------------------------------
// Include the ViennaCL Cuthill-Mckee / Gibbs-Poole-Stockmeyer algorithms

typedef size_t vcl_size_t; // Must precede the header inclusions
typedef std::vector<std::map<int, double>> vcl_matrix_t;
#include "viennacl/cuthill_mckee.hpp"
#include "viennacl/gibbs_poole_stockmeyer.hpp"


//---------------------------------------------------------------------------------------
// Variable ordering methods that use boost::graph methods
//---------------------------------------------------------------------------------------

// Return the # of nodes in a Node_p list
inline int list_length(Node_p node) {
    int len = 0;
    while (node != nullptr) {
        len++;
        node = NEXT_NODE(node);
    }
    return len;
}

//---------------------------------------------------------------------------------------

// Max number of in-arcs * out-arcs of a transition before adding
// the transition as a 'special' node in the graph.
const int MAX_INOUT_EDGES = 100;




// Initialize a boost::graph with its edges.
// The graph is expected to have at least npl vertices.
// The idea of this function is to convert Place -> Transition -> Place relations
// of the Petri net into straight Place->Place edges in the graph.
// However, some transitions could have large in-degree and out-degree, leading to
// a cross-product of place->place edges. In that case, the transition itself
// is added as a not-a-place vertex in the graph, and the usual Place -> Transition -> Place
// relation is kept.
template<typename Graph>
void init_graph_edges_from_Petri_net(Graph &graph, bool bidirectional = false,
                                     const std::vector<int>* net_to_level = nullptr)
{
    std::set<pair<int, int>> inserted;

    // Add an edge to the graph, avoiding duplicate insertions.
    // Note: minimum_degree_ordering crashes if edges are duplicated.
    auto add_bgl_edge = [&](int source, int target) {
        if (net_to_level != nullptr) {
            if (source < npl) // source is a place, convert its index
                source = (*net_to_level)[source];
            if (target < npl) // target is a place, convert its index
                target = (*net_to_level)[target];
        }
        if (inserted.count(std::make_pair(source, target)) == 0) {
           boost::add_edge(source, target, graph);
           inserted.insert(std::make_pair(source, target));
        }
        if (bidirectional) { // add also the target -> source edge.
            if (inserted.count(std::make_pair(target, source) ) == 0) {
                boost::add_edge(target, source,  graph);
                inserted.insert(std::make_pair(target, source) );
            }
        }
    };

    // Add edges between places connected by each transition
    for (int tr=0; tr<ntr; tr++) {
        int num_in_arcs = list_length(GET_INPUT_LIST(tr));
        int num_out_arcs = list_length(GET_OUTPUT_LIST(tr));

        if (num_in_arcs * num_out_arcs < MAX_INOUT_EDGES) {
            // Build all direct edges between the places connected by transition tr.
            // Empirical observations show that a dense clique is not very good.
            // Therefore, just connect input places with output places, but do not connect
        	// input with input and output with output.
            // The goal is to build a dense clique that connects all the input/output places together.
            for (Node_p in_ptr = GET_INPUT_LIST(tr); in_ptr; in_ptr = NEXT_NODE(in_ptr)) {
                int in_plc = GET_PLACE_INDEX(in_ptr);
                // int in_mult = in_ptr->molt;

                for (Node_p out_ptr = GET_OUTPUT_LIST(tr); out_ptr; out_ptr = NEXT_NODE(out_ptr)) {
                    int out_plc = GET_PLACE_INDEX(out_ptr);
                    // int out_mult = out_ptr->molt;

                    if (in_plc != out_plc)
                        add_bgl_edge(in_plc, out_plc);
                }
            }
            // // Build direct edges between input places
            // for (Node_p in1_ptr = GET_INPUT_LIST(tr); in1_ptr; in1_ptr = NEXT_NODE(in1_ptr)) {
            // 	int in1_plc = GET_PLACE_INDEX(in1_ptr);
            // 	for (Node_p in2_ptr = NEXT_NODE(in1_ptr); in2_ptr; in2_ptr = NEXT_NODE(in2_ptr)) {
	           //  	int in2_plc = GET_PLACE_INDEX(in2_ptr);
            // 		boost::add_edge(in1_plc, in2_plc, graph);
            // 	}
            // }
            // // Build direct edges between output places
            // for (Node_p out1_ptr = GET_OUTPUT_LIST(tr); out1_ptr; out1_ptr = NEXT_NODE(out1_ptr)) {
            // 	int out1_plc = GET_PLACE_INDEX(out1_ptr);
            // 	for (Node_p out2_ptr = NEXT_NODE(out1_ptr); out2_ptr; out2_ptr = NEXT_NODE(out2_ptr)) {
	           //  	int out2_plc = GET_PLACE_INDEX(out2_ptr);
            // 		boost::add_edge(out1_plc, out2_plc, graph);
            // 	}
            // }
        }
        else {
            // Add the transition tr as a not-a-place node in the graph.
            auto vtr = boost::add_vertex(graph);

            // Add input edges
            for (Node_p in_ptr = GET_INPUT_LIST(tr); in_ptr; in_ptr = NEXT_NODE(in_ptr)) {
                int plc = GET_PLACE_INDEX(in_ptr);
                // int mult = in_ptr->molt;

                add_bgl_edge(plc, vtr);
            }

            // Add output edges
            for (Node_p out_ptr = GET_OUTPUT_LIST(tr); out_ptr; out_ptr = NEXT_NODE(out_ptr)) {
                int plc = GET_PLACE_INDEX(out_ptr);
                // int mult = out_ptr->molt;

                add_bgl_edge(vtr, plc);
            }
        }
    }

	// ofstream og("graph.dot");
	// og << "graph { " << endl;
	// typedef typename boost::graph_traits<Graph>::edge_iterator  edge_iterator;
	// edge_iterator e, end;
	// for (boost::tie(e, end) = boost::edges(graph); e != end; ++e) {
	// 	og << "v" << boost::source(*e, graph) << " -- v" << boost::target(*e, graph) << ";\n";
	// }
	// og << "}\n" << endl;
	// og.close();

    // cout << "Graph G("<<boost::num_vertices(graph)<<");\n";
    // typedef typename boost::graph_traits<Graph>::edge_iterator  edge_iterator;
    // edge_iterator e, end;
    // for (boost::tie(e, end) = boost::edges(graph); e != end; ++e) {
    //     cout << "boost::add_edge(" << boost::source(*e, graph) << ", " 
    //          << boost::target(*e, graph) << ", graph);\n";
    // }
}

//---------------------------------------------------------------------------------------

// Same as above, for vcl matrix
void init_graph_edges_from_Petri_net(vcl_matrix_t &graph) {
    graph.resize(npl);

        // Add edges between places connected by each transition
    for (int tr=0; tr<ntr; tr++) {
        int num_in_arcs = list_length(GET_INPUT_LIST(tr));
        int num_out_arcs = list_length(GET_OUTPUT_LIST(tr));

        if (num_in_arcs * num_out_arcs < MAX_INOUT_EDGES) {
            // Build all direct edges between the places connected by transition tr.
            for (Node_p in_ptr = GET_INPUT_LIST(tr); in_ptr; in_ptr = NEXT_NODE(in_ptr)) {
                int in_plc = GET_PLACE_INDEX(in_ptr);
                for (Node_p out_ptr = GET_OUTPUT_LIST(tr); out_ptr; out_ptr = NEXT_NODE(out_ptr)) {
                    int out_plc = GET_PLACE_INDEX(out_ptr);
                    if (in_plc != out_plc) {
                        graph[in_plc][out_plc] = 1.0;
                        graph[out_plc][in_plc] = 1.0;
                    }
                }
            }
        }
        else {
            // Add the transition tr as a not-a-place node in the graph.
            size_t vtr = graph.size();
            graph.resize(vtr + 1);

            // Add input edges
            for (Node_p in_ptr = GET_INPUT_LIST(tr); in_ptr; in_ptr = NEXT_NODE(in_ptr)) {
                int plc = GET_PLACE_INDEX(in_ptr);

                graph[plc][vtr] = 1.0;
                graph[vtr][plc] = 1.0;
            }

            // Add output edges
            for (Node_p out_ptr = GET_OUTPUT_LIST(tr); out_ptr; out_ptr = NEXT_NODE(out_ptr)) {
                int plc = GET_PLACE_INDEX(out_ptr);

                graph[plc][vtr] = 1.0;
                graph[vtr][plc] = 1.0;
            }
        }
    }
}

//---------------------------------------------------------------------------------------

// Convert a boost::graph permutation into a variable ordering for Meddly
template<typename VertexIndexMap, typename VectorPerm>
void fill_out_ordering(std::vector<int> &out_order, 
                       const VectorPerm& permutation,
                       const VertexIndexMap& vertex_index_map) 
{
	typedef typename VectorPerm::size_type  size_type;
    // Print the ordering
    // cout << permutation.size() << " VALUES: ";
    // for (size_type c = 0; c != permutation.size(); ++c) {
    //     size_type index = vertex_index_map[ permutation[c] ];
    //     if (index >= npl) // the boost::graph vertex is not a place
    //         cout << "<> ";
    //     else
    //         cout << index << " ";
    //         // cout << tabp[ index ].place_name << " ";
    // }
    // cout << endl << endl;
    // for (size_type var=0; var<npl; var++) {
    // 	if (permutation.end() != std::find(permutation.begin(), permutation.end(), var))
    // 		continue;
    // 	cout << "missing variable " << var << " in ordering..." << endl;
    // }

    // Copy the ordering into out_order[]
    std::fill(out_order.begin(), out_order.end(), -1);
    int count = 0;
    for (size_type c = 0; c != permutation.size(); ++c) {
        size_type index = vertex_index_map[ permutation[c] ];
        if (index >= npl) // the boost::graph vertex is not a place
            continue;
        if (index < 0)
            throw rgmedd_exception("Negative index!!");
        if (out_order[index] != -1)
        	throw rgmedd_exception("Ordering contains repetitions.");
        out_order[ index ] = count++;
        // out_order[ count++ ] = index ].index;
    }
    if (count != npl)
    	throw rgmedd_exception("Ordering is not contiguous.");
}

//---------------------------------------------------------------------------------------
// Cuthill McKee ordering (from boost, modified)
//---------------------------------------------------------------------------------------

// Use the boost::cuthill_mckee_ordering() method to derive a variable ordering.
void var_order_cuthill_mckee(const VariableOrderCriteria voc, 
                             std::vector<int> &out_order) 
{
    typedef boost::adjacency_list<boost::vecS, 
    /**/                          boost::vecS, 
    /**/                          boost::undirectedS> Graph; // Must be undirected!
    Graph G(npl);
    boost::property_map<Graph, boost::vertex_index_t>::type index_map = boost::get(boost::vertex_index, G);
    init_graph_edges_from_Petri_net(G);

    // Build the reverse Cuthill-McKee ordering from the vertex_queue visit
    std::vector<int> inv_perm(boost::num_vertices(G));
    auto end_iter = boost::cuthill_mckee_ordering(G, inv_perm.rbegin(), index_map);
    if (end_iter != inv_perm.rend()) 
    	throw rgmedd_exception("Access violation in CM method");

    // Return the direct ordering
    fill_out_ordering(out_order, inv_perm, index_map);
}

//---------------------------------------------------------------------------------------

// // Use the boost::minimum_degree_ordering() method to derive a variable ordering.
// void var_order_minimum_degree_ordering(const VariableOrderCriteria voc, 
//                                        std::vector<int> &out_order) 
// {
// 	typedef boost::adjacency_list<boost::vecS, 
// 	/**/                          boost::vecS, 
// 	/**/                          boost::directedS> Graph;
// 	Graph G(npl);
//     boost::property_map<Graph, boost::vertex_index_t>::type index_map = boost::get(boost::vertex_index, G);
//     init_graph_edges_from_Petri_net(G);

//     // minimum_degree_ordering seems to be a bit bugged, and sometimes 
//     // it overwrites randomly its buffer sizes. Just use larger buffer, and then
//     // check the validity of the ordering
// 	std::vector<int> inverse_perm(2 * boost::num_vertices(G), 0);
// 	std::vector<int> perm(2 * boost::num_vertices(G), 0);
// 	std::vector<int> supernode_sizes(2 * boost::num_vertices(G), 1); // must be initialized with 1
// 	std::vector<int> degree(2 * boost::num_vertices(G), 0);
// 	int delta = 0;
// 	std::fill(inverse_perm.begin() + boost::num_vertices(G), inverse_perm.end(), -1);
// 	std::fill(perm.begin() + boost::num_vertices(G), perm.end(), -1);
// 	std::fill(supernode_sizes.begin() + boost::num_vertices(G), supernode_sizes.end(), -1);
// 	std::fill(degree.begin() + boost::num_vertices(G), degree.end(), -1);

// 	boost::minimum_degree_ordering(G,
// 		boost::make_iterator_property_map(&degree[0], index_map, degree[0]),
// 		&inverse_perm[0],
// 		&perm[0],
// 		boost::make_iterator_property_map(&supernode_sizes[0], index_map, supernode_sizes[0]), 
// 		delta, index_map);

// 	// for (int i=0; i<boost::num_vertices(G); i++)
// 	// 	cout << perm[i] << " ";
// 	// cout << endl;
// 	// for (int i=boost::num_vertices(G); i<perm.size(); i++)
// 	// 	cout << perm[i] << " ";
// 	// cout << endl;

//     // The produced order could contain repetitions. Remove them before getting the out_order
//     std::vector<bool> inserted(boost::num_vertices(G), false);
//     std::vector<int> new_perm(boost::num_vertices(G), 0);
//     int k=0;
//     for (int i=0; i<boost::num_vertices(G); i++) {
//         if (perm[i] < 0 || perm[i] >= boost::num_vertices(G))
//             throw rgmedd_exception("index out-of-bound in MDO.");
//         if (!inserted[perm[i]]) {
//             new_perm[k++] = perm[i];
//             inserted[perm[i]] = true;
//         }
//         else {
//             cerr << "repetition: perm["<<i<<"] = " << perm[i] << endl;
//             // throw rgmedd_exception("vertex index repetition in MDO!");
//         }
//     }
//     // Check if some vertex is missing the permutation
//     if (k != boost::num_vertices(G)) {
// 		for (int v=0; v<boost::num_vertices(G); v++) {
// 			if (!inserted[v]) {
// 				new_perm[k++] = v;
// 				cerr << "missing: v="<<v<<endl;
// 			}
// 		}
// 	}

// 	// Translate the computed permutation into the variable ordering
//     fill_out_ordering(out_order, new_perm, index_map);
// }

//---------------------------------------------------------------------------------------

// Use the boost::king_ordering() method to derive a variable ordering.
void var_order_king_ordering(const VariableOrderCriteria voc, 
                             std::vector<int> &out_order) 
{
    typedef boost::adjacency_list<boost::vecS, 
    /**/                          boost::vecS, 
    /**/                          boost::undirectedS> Graph;

    Graph G(npl);
    boost::property_map<Graph, boost::vertex_index_t>::type index_map = boost::get(boost::vertex_index, G);
    init_graph_edges_from_Petri_net(G);

    // Build the reverse Cuthill-McKee ordering
    std::vector<int> inv_perm(boost::num_vertices(G));
    boost::king_ordering(G, inv_perm.rbegin());

    // Return the direct ordering
    fill_out_ordering(out_order, inv_perm, index_map);
}

//---------------------------------------------------------------------------------------

// Weights of the parametric priority function of the Sloan method
extern int g_sloan_W1, g_sloan_W2;

// Use the boost::sloan_ordering() method to derive a variable ordering.
void var_order_sloan(const VariableOrderCriteria voc, std::vector<int> &out_order) 
{
    if (npl == 1) {
        out_order = {0};
        return;
    }
    if (ntr == 0) {
        throw rgmedd_exception("var_order_sloan() cannot be used with 0 transitions.");
    }

    // The graph type for the sloan method
    typedef boost::adjacency_list<boost::setS, 
    /**/                          boost::vecS, 
    /**/                          boost::undirectedS, 
    /**/                          boost::property<boost::vertex_color_t, 
    /**/                                          boost::default_color_type,
    /**/                          boost::property<boost::vertex_degree_t,
    /**/                                          int,
    /**/                          boost::property<boost::vertex_priority_t,
    /**/                                          double>>>> Graph;

    typedef boost::graph_traits<Graph>::vertex_descriptor  Vertex;
    typedef boost::graph_traits<Graph>::vertices_size_type size_type;

    Graph G(npl);
    boost::property_map<Graph, boost::vertex_index_t>::type index_map = boost::get(boost::vertex_index, G);
    boost::property_map<Graph, boost::vertex_degree_t>::type degree_map = boost::get(boost::vertex_degree, G);
    init_graph_edges_from_Petri_net(G);

    // Setup vertex degrees
    boost::graph_traits<Graph>::vertex_iterator ui, ui_end;
    for (boost::tie(ui, ui_end) = boost::vertices(G); ui != ui_end; ++ui)
        degree_map[*ui] = boost::degree(*ui, G);

    // Support vector needed by sloan_ordering  
    std::vector<Vertex> sloan_order(boost::num_vertices(G));
    std::vector<size_type> perm(boost::num_vertices(G));
    std::vector<Vertex>::iterator inserted_vars;
    int W1 = g_sloan_W1, W2 = g_sloan_W2;
    if (voc == VOC_SLOAN_1_16) { W1 = 1; W2 = 16; }

    inserted_vars = boost::sloan_ordering(G, sloan_order.begin(), 
				                          boost::get(boost::vertex_color, G),
				                          boost::make_degree_map(G), 
				                          boost::get(boost::vertex_priority, G),
                                          W1, W2);

    if (inserted_vars != sloan_order.end()) {
    	// We missed some vertices. This may happen if those vertices are isolated.
    	// Complete the ordering by finding those vertices
    	typedef boost::property_map<Graph, boost::vertex_color_t>::type ColorMap;
	    typedef boost::property_traits<ColorMap>::value_type ColorValue;
	    typedef boost::color_traits<ColorValue> Color;
	    ColorMap color_map = boost::get(boost::vertex_color, G);
	    for (boost::tie(ui, ui_end) = boost::vertices(G); ui != ui_end; ++ui) {
    		if (color_map[*ui] == Color::white()) {
    			*inserted_vars++ = *ui;
    			color_map[*ui] = Color::green();
    		}
    	}
    }

    fill_out_ordering(out_order, sloan_order, index_map);
}

//---------------------------------------------------------------------------------------

// TODO: remove
extern double GP_WEIGHT, GP_EXP, GP_GRAD, GP_WEIGHT2;
extern bool GP_SIGN;
double sign(double v) { return v<0 ? -1 : +1; }

// Selects P-invariants following a Sloan-like gradient between two extreme 
// vertices of the graph.
void var_order_gradient_P(const VariableOrderCriteria voc, 
                          const flow_basis_t& psf,
                          std::vector<int> &out_order) 
{
    typedef boost::adjacency_list<boost::setS, 
    /**/                          boost::vecS, 
    /**/                          boost::undirectedS, 
    /**/                          boost::property<boost::vertex_color_t, 
    /**/                                          boost::default_color_type,
    /**/                          boost::property<boost::vertex_degree_t,
    /**/                                          int,
    /**/                          boost::property<boost::vertex_priority_t,
    /**/                                          double>>>> Graph;

    typedef boost::graph_traits<Graph>::vertex_descriptor  Vertex;
    typedef boost::graph_traits<Graph>::edge_descriptor  Edge;
    typedef boost::graph_traits<Graph>::vertices_size_type size_type;
    typedef boost::graph_traits<Graph>::adjacency_iterator AdjacencyIterator;
	typedef boost::property_map<Graph, boost::vertex_color_t>::type ColorMap;
    typedef boost::property_traits<ColorMap>::value_type ColorValue;
    typedef boost::color_traits<ColorValue> Color;

    Graph G(npl);
    boost::property_map<Graph, boost::vertex_index_t>::type index_map = boost::get(boost::vertex_index, G);
    boost::property_map<Graph, boost::vertex_degree_t>::type degree_map = boost::get(boost::vertex_degree, G);
    init_graph_edges_from_Petri_net(G);

    // Setup vertex degrees
    boost::graph_traits<Graph>::vertex_iterator ui, ui_end;
    for (boost::tie(ui, ui_end) = boost::vertices(G); ui != ui_end; ++ui)
        degree_map[*ui] = boost::degree(*ui, G);
    std::vector<int> gradient_l2f(boost::num_vertices(G)); // last to first
    std::fill(gradient_l2f.begin(), gradient_l2f.end(), -1);
    std::vector<int> gradient_f2l(boost::num_vertices(G)); // first to last
    std::fill(gradient_f2l.begin(), gradient_f2l.end(), -1);
    std::vector<double> gradient(boost::num_vertices(G)); // first to last
    std::fill(gradient_f2l.begin(), gradient_f2l.end(), -1.0);
    std::vector<int> representative(boost::num_vertices(G), -1);
    std::vector<bool> inserted(boost::num_vertices(G), false);

    // Initialize vertex color to white
    ColorMap color_map = boost::get(boost::vertex_color, G);
    BGL_FORALL_VERTICES_T(v, G, Graph) put(color_map, v, Color::white());

    Vertex last_vert, first_vert, last_vert2;
    std::vector<Vertex> comp_verts, vert_comp_num(boost::num_vertices(G), -1);
    comp_verts.reserve(boost::num_vertices(G));
	// Visitor object that finds the last vertex, while adding all the visited vertices
	// in a vector. Visited vertices form a connected component.
	struct dfs_visit_mark_verts : public boost::default_bfs_visitor {
    	Vertex& vert;
    	std::vector<Vertex>& comp_verts;
    	dfs_visit_mark_verts(Vertex& v, std::vector<Vertex>& c) 
    	/**/ : vert(v), comp_verts(c) { }
    	void start_vertex(Vertex v, Graph const& G) { 
    		// cout <<"("<<v<<") "; 
    		vert = v; 
    		comp_verts.clear();
    	}
    	void discover_vertex(Vertex v, Graph const& G) { 
    		// cout <<v<<" "; 
    		vert = v;
    		comp_verts.push_back(v);
    	}
    	void back_edge(Edge e, Graph const& G) { }
    	void forward_or_cross_edge(Edge e, Graph const& G) { }
    } visit_mark_verts(last_vert, comp_verts);

    // Set to v_last the last visited vertex
    struct dfs_visit_simple : public boost::default_bfs_visitor {
        Vertex& vert;
        dfs_visit_simple(Vertex& v) : vert(v) { }
        void start_vertex(Vertex v, Graph const& G) { vert = v; }
        void discover_vertex(Vertex v, Graph const& G) { vert = v; }
        void back_edge(Edge e, Graph const& G) { }
        void forward_or_cross_edge(Edge e, Graph const& G) { }
    } visit_simple_verts(last_vert);

    // Visitor object that enstablish a gradient from the start vertex
    // the the end vertex, and returns the end vertex.
    struct dfs_visit_gradient : public boost::default_bfs_visitor {
    	Vertex& vert;
    	std::vector<int>& gradient;
    	dfs_visit_gradient(Vertex& v, std::vector<int>& g) 
    	/**/ : vert(v), gradient(g) { }
    	void start_vertex(Vertex v, Graph const& G) { 
    		// cout <<"("<<v<<":1) "; 
    		vert = v; 
    		gradient[v] = 1; 
    	}
    	void discover_vertex(Vertex v, Graph const& G) { 
    		AdjacencyIterator a, ae;
    		for (boost::tie(a, ae) = boost::adjacent_vertices(v, G); a != ae; ++a)
    			gradient[v] = std::max(gradient[v], gradient[*a] + 1);
    		// cout <<v<<":"<<gradient[v]<<" "; 
    		if (gradient[v] > gradient[vert])
	    		vert = v; 
    	}
    	void back_edge(Edge e, Graph const& G) { }
    	void forward_or_cross_edge(Edge e, Graph const& G) { }
    } visit_gradient_l2f(first_vert, gradient_l2f), visit_gradient_f2l(last_vert2, gradient_f2l);


	// Repeat for every connected component
	int comp_num=0, k=0;
    BGL_FORALL_VERTICES_T(start, G, Graph) {
		if (get(color_map, start) == Color::white()) {
			comp_num++; // connected component id
			// cout << endl;
			boost::depth_first_visit(G, start, visit_mark_verts, color_map);
			// cout << endl << "last_vert = " << last_vert << endl;
            Vertex v0_max = start;
			for (Vertex v : comp_verts) {
				put(color_map, v, Color::white());
				vert_comp_num[v] = comp_num;
                if (boost::out_degree(v, G) > boost::out_degree(v0_max, G))
                    v0_max = v; // select the vertex with the maximum in/out-degree
			}
            // v0_max = comp_verts[ rand() % comp_verts.size() ];
            if (v0_max != start) {
                boost::depth_first_visit(G, v0_max, visit_simple_verts, color_map);
                for (Vertex v : comp_verts)
                    put(color_map, v, Color::white());
            }

			// Enstablish the double gradient from first_vert to last_vert
			boost::depth_first_visit(G, last_vert, visit_gradient_l2f, color_map);
			// cout << endl << "first_vert = " << first_vert << endl;
			for (Vertex v : comp_verts) 
				put(color_map, v, Color::white());
			boost::depth_first_visit(G, first_vert, visit_gradient_f2l, color_map);
            // cout << "first place: " << tabp[first_vert].place_name << endl;
            // cout << "last place: " << tabp[last_vert].place_name << endl;
			// cout << endl << "last_vert2 = " << last_vert2 << endl;
			// Combine the two gradients together into gradient_l2f[]
			//  gradient_f2l = first_vert:1   v:2  v:3  v:3   last_vert:4
			//  gradient_l2f = first_vert:4   v:3  v:3  v:2   last_vert:1   max_grad_l2f=4
			//      gradient = first_vert:10  v:21 v:31 v:32  last_vert:43
			int max_grad_l2f = gradient_l2f[ comp_verts[0] ];
			for (Vertex v : comp_verts) 
				max_grad_l2f = std::max(max_grad_l2f, gradient_l2f[v]);
			// cout << "max_grad_l2f = " << max_grad_l2f << endl;
			for (Vertex v : comp_verts) {
				gradient[v] = gradient_f2l[v] + GP_GRAD * double(max_grad_l2f + 1 - gradient_l2f[v]) / max_grad_l2f;
				// cout << v << ":" << gradient[v] << " ";
                // cout << "   gradient["<<tabp[v].place_name<<"] = "<<gradient[v]<<endl;
			}
			// cout << endl;

			// Find the P-invariant with the highest score.
			while (true) {
				double max_score = 0;
				const sparse_vector_t *max_sf = nullptr;
				for (const sparse_vector_t& sf : psf) {
					double score = 0;
					int num_elems = 0;
					for (auto& sfe : sf) {
						int v = sfe.index;
						if (comp_num != vert_comp_num[v])
							continue; // not part of this SCC
						if (!inserted[v]) {
							score -= gradient[v];
							num_elems++;
						}
						else 
							score += /*0.25*/ GP_WEIGHT * gradient[v];
					}
					if (num_elems == 0)
						continue;
					// Dividere lo score sembra essere peggiorativo.
					// score /= num_elems + 1;
					// score *= num_elems * num_elems;
                    score *= pow(num_elems, GP_EXP * (GP_SIGN ? sign(score) : 1));

                    score += GP_WEIGHT2 * num_elems;
                    
					// score /= num_elems;//*num_elems;
					if (max_sf == nullptr || score > max_score) {
						max_score = score;
						max_sf = &sf;
					}
                    // cout << max_sf << "   score="<<score<<endl;
				}
				if (max_sf == nullptr)
					break;
				// Insert all variables of *max_sf in the out-order, in gradient order
				std::vector<pair<int, int>> var_list;
				for (auto& sfe : *max_sf) {
					int v = sfe.index;
					if (comp_num == vert_comp_num[v] && !inserted[v])
						var_list.push_back(std::make_pair(gradient[v], v));
				}
				// cout << "score = " << max_score << " : ";
				std::sort(var_list.begin(), var_list.end());
				for (auto&& vl : var_list) {
					out_order[k++] = vl.second;
					// cout << vl.second << ":" << gradient[vl.second] <<  " ";
					inserted[vl.second] = true;
				}
				// cout << endl;
			}

			// Append all vertices of this SCC that were not covered by any P-invariant
			for (int v=0; v<boost::num_vertices(G); v++) {
				if (comp_num == vert_comp_num[v] && !inserted[v]) {
					// Remember that G could also contain pseudo-vertices (transitions)
					// that are not places. Those vertices should not end up in out_order[]
					if (v < npl)
						out_order[k++] = v;
					// cout << " [" << v << "] ";
					inserted[v] = true;
				}
			}
		}
    }
}

//---------------------------------------------------------------------------------------

static int num_SCC = -1; // -1 means uninitialized
// SCC identifier for each place
static std::vector<int> place_components;
// Is a transition local to a component, or is cross-components?
// static std::vector<bool> trans_SCC_locality;
// should we do the variable order reshuffle using SCC info??
// static bool do_varorder_reshuffle_with_SCCs = false;
// Order of the SCC in bottom-up BFS order
// static std::vector<int> bfs_scc_order;
// static std::vector<int> weights;

//---------------------------------------------------------------------------------------

const int* get_place_components() {
    if (num_SCC <= 1)
        throw rgmedd_exception("Should not call this method if there are no SCCs.");
    return place_components.data();
}

int get_num_components() {
    if (num_SCC < 0)
        throw rgmedd_exception("compute_connected_components() has not been called yet.");
    return num_SCC;
}

//---------------------------------------------------------------------------------------

int compute_connected_components() {
    typedef boost::adjacency_list<boost::vecS, 
    /**/                          boost::vecS, 
    /**/                          boost::directedS> Graph;
    if (num_SCC >= 0)
        return num_SCC; // already computed
    Graph G(npl);
    place_components.resize(npl);

    // Setup the graph from the incidence matrix
    std::map<int, int> incid; // incidence matrix row for transition tr
    // collect statistics on input, test and output arcs
    static int num_input_arcs = 0, num_test_arcs = 0, num_output_arcs = 0;

    for (int tr=0; tr<ntr; tr++) {
        // Get the incidence matrix row for transition tr
        incid.clear();
        for (Node_p in_ptr = GET_INPUT_LIST(tr); in_ptr; in_ptr = NEXT_NODE(in_ptr))
            incid[GET_PLACE_INDEX(in_ptr)] -= in_ptr->molt;
        for (Node_p out_ptr = GET_OUTPUT_LIST(tr); out_ptr; out_ptr = NEXT_NODE(out_ptr))
            incid[GET_PLACE_INDEX(out_ptr)] += out_ptr->molt;

        for (auto&& it=incid.begin(); it != incid.end(); ++it) {
            if (it->second > 0)
                num_output_arcs++;
            else if (it->second < 0)
                num_input_arcs++;
            else
                num_test_arcs++;
        }

        for (auto&& it1=incid.begin(); it1 != incid.end(); ++it1) {
            if (it1->second >= 0)
                continue; // it1 should be an input or a test arc, not an output
            for (auto&& it2=incid.begin(); it2 != incid.end(); ++it2) {
                if (it1->first == it2->first || it2->second < 0)
                    continue; // it1 should be an output arc, not an input or a test arc
                boost::add_edge(it1->first, it2->first, G);
            }
        }
    }

    // Build the components using Tarjan's algorithm
    typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
    typedef boost::graph_traits<Graph>::edge_descriptor Edge;
    std::vector<int> discover_time(boost::num_vertices(G));
    std::vector<boost::default_color_type> color(boost::num_vertices(G));
    std::vector<Vertex> root(boost::num_vertices(G));
  
    num_SCC = boost::strong_components(G, &place_components[0], 
                                       boost::root_map(&root[0]).
                                       color_map(&color[0]).
                                       discover_time_map(&discover_time[0]));

    if (!running_for_MCC() && num_SCC > 1)
        cout << "Number of components: " << num_SCC << endl;

    // // Determine transition locality
    // trans_SCC_locality.resize(ntr, true);
    // for (int tr=0; tr<ntr; tr++) {
    //     int comp = -1;
    //     bool is_local = true;
    //     for (Node_p in_ptr = GET_INPUT_LIST(tr); in_ptr && is_local; in_ptr = NEXT_NODE(in_ptr)) {
    //         if (comp == -1)
    //             comp == place_components[GET_PLACE_INDEX(in_ptr)];
    //         else if (comp != place_components[GET_PLACE_INDEX(in_ptr)]) {
    //             is_local = false;
    //         }
    //     }
    //     for (Node_p out_ptr = GET_OUTPUT_LIST(tr); out_ptr && is_local; out_ptr = NEXT_NODE(out_ptr)) {
    //         if (comp == -1)
    //             comp == place_components[GET_PLACE_INDEX(out_ptr)];
    //         else if (comp != place_components[GET_PLACE_INDEX(out_ptr)]) {
    //             is_local = false;
    //         }            
    //     }
    //     trans_SCC_locality[tr] = is_local;
    // }

    // TEST: 

    // for (int i=0; i<npl; i++) {
    //     cout << place_components[i] << " ";
    // }
    // cout << endl;

#if 0
    // Build the condensation graph of the Strongly connected components
    typedef typename boost::graph_traits<Graph>::edge_descriptor  Edge;
    typedef typename boost::graph_traits<Graph>::edge_iterator  edge_iterator;
    typedef typename boost::graph_traits<Graph>::out_edge_iterator  out_edge_iterator;
    edge_iterator e, end;
    std::vector<std::pair<Vertex, Vertex>> cgedges;
    for (boost::tie(e, end) = boost::edges(G); e != end; ++e) {
        if (place_components[boost::source(*e, G)] == place_components[boost::target(*e, G)])
            continue; // self loop
        cgedges.push_back(make_pair(place_components[boost::source(*e, G)], 
                                    place_components[boost::target(*e, G)]));
    }
    std::sort(cgedges.begin(), cgedges.end());
    cgedges.erase(std::unique(cgedges.begin(), cgedges.end()), cgedges.end());
    
    Graph CDAG = Graph(num_SCC);
    for (auto&& it : cgedges)
        boost:add_edge(it.first, it.second, CDAG);

    // cout << "SAVING dag.dot ..." << endl;
    // ofstream dot("dag.dot");
    // dot << "digraph {\n";
    // for (int c=0; c<num_SCC; c++) {
    //     dot << "  c"<<c<<" [label=\"c"<<c<<": ";
    //     for (int i=0; i<npl; i++) {
    //         if (place_components[i] == c)
    //             dot << tabp[i].place_name << " ";
    //     }
    //     dot << "\"];\n";
    // }
    // for (auto&& it : cgedges)
    //     dot << "  c" << it.first << " -> c" << it.second << ";\n";
    // dot << "}\n";
    // Discover DAG roots
    std::vector<bool> dag_roots(num_SCC, true);
    for (boost::tie(e, end) = boost::edges(CDAG); e != end; ++e)
        dag_roots[boost::target(*e, CDAG)] = false;

    // Weight individual components
    weights.resize(num_SCC, 0);
    // std::vector<int> weights(num_SCC, 0);
    for (int pl=0; pl<npl; pl++)
        weights[place_components[pl]]++;

    std::vector<bool> visited(num_SCC, false);
    std::function<int(int)> rec_visit;
    rec_visit = [&visited,/*&weights,*/&CDAG,&rec_visit](int c) -> int {
        if (visited[c])
            return weights[c];
        visited[c] = true;
        out_edge_iterator e, end;
        for (boost::tie(e, end) = boost::out_edges(c, CDAG); e != end; ++e) {
            weights[c] += rec_visit(boost::target(*e, CDAG));
        }
        return weights[c];
    };
    for (int c=0; c<num_SCC; c++)
        if (dag_roots[c])
            rec_visit(c);

    bfs_scc_order.resize(num_SCC, -1);
    std::fill(visited.begin(), visited.end(), false);
    int count=0;
    std::function<void(int)> rec_visit_ord;
    rec_visit_ord = [&](int c) {
        if (visited[c])
            return;
        visited[c] = true;
        std::priority_queue<pair<int, int>> nextQ;
        out_edge_iterator e, end;
        for (boost::tie(e, end) = boost::out_edges(c, CDAG); e != end; ++e)
            nextQ.push(make_pair(weights[boost::target(*e, CDAG)], boost::target(*e, CDAG)));

        while (!nextQ.empty()) {
            rec_visit_ord(nextQ.top().second);
            nextQ.pop();
        }
        bfs_scc_order[c] = count++;
    };
    for (int c=0; c<num_SCC; c++)
        if (dag_roots[c])
            rec_visit_ord(c);
#endif
    // struct weight_visit : public boost::bfs_visitor<> {
    //     weight_visit(std::vector<int>& _ord) : ord(_ord) { }
        
    //     void finish_vertex(int v, const Graph& G) {
    //         ord[v] = cnt++;
    //     }

    //     // void examine_edge(const Edge& e, const Graph& G) 
    //     // { w[boost::target(e, G)] = std::max(w[boost::target(e, G)], w[boost::source(e, G)] - 1); }
    //     // { w[boost::target(e, G)] += w[boost::source(e, G)]; }
        
    //     int cnt = 0;
    //     std::vector<int>& ord;
    // } wv(bfs_scc_order);
    // for (int c=0; c<num_SCC; c++)
    //     if (dag_roots[c])
    //         boost::breadth_first_search(CDAG, c, boost::visitor(wv));

#if 0
    // cout << "Weights1: ";
    // for (int c=0; c<num_SCC; c++)
    //     cout << "c"<<c<<"="<<comp_weights[c] << " ";
    // cout << endl;

    // Make components inherit the weight of their predecessors.
    // std::vector<int> weights = comp_weights;
    std::vector<int> weights(num_SCC, 0);
    std::vector<bool> visited(num_SCC, false);
    // std::function<int(int)> rec_visit;
    // rec_visit = [&visited,&weights,&CDAG,&rec_visit](int c) -> int {
    //     if (visited[c])
    //         return weights[c];
    //     visited[c] = true;
    //     out_edge_iterator e, end;
    //     for (boost::tie(e, end) = boost::out_edges(c, CDAG); e != end; ++e) {
    //         weights[c] += rec_visit(boost::target(*e, CDAG));
    //     }
    //     return weights[c];
    // };
    // for (int c=0; c<num_SCC; c++)
    //     if (dag_roots[c])
    //         rec_visit(c);
    struct weight_visit : public boost::dfs_visitor<> {
        weight_visit(std::vector<int>& _w) : w(_w) { }
        
        void examine_edge(const Edge& e, const Graph& G) 
        { w[boost::target(e, G)] = std::max(w[boost::target(e, G)], w[boost::source(e, G)] - 1); }
        // { w[boost::target(e, G)] += w[boost::source(e, G)]; }
        
        std::vector<int>& w;
    } wv(weights);
    boost::depth_first_search(CDAG, boost::visitor(wv));

    cout << "Weights: ";
    for (int c=0; c<num_SCC; c++)
        cout << "c"<<c<<"="<<weights[c] << " ";
    cout << endl;

    // visit using a priority queue
    std::vector<Vertex> toposort(num_SCC, -1);
    int K = 0;
    std::priority_queue<pair<int, int>> pq;
    for (int c=0; c<num_SCC; c++)
        if (dag_roots[c]) 
            pq.push(make_pair(weights[c], c));
    std::fill(visited.begin(), visited.end(), false);
    while (!pq.empty()) {
        int c = pq.top().second;
        pq.pop();
        if (!visited[c]) {
            visited[c] = true;
            toposort[K++] = c;
            out_edge_iterator e, end;
            for (boost::tie(e, end) = boost::out_edges(c, CDAG); e != end; ++e) {
                int c2 = boost::target(*e, CDAG);
                if (!visited[c2])
                    pq.push(make_pair(weights[c2], c2));
            }
        }
    }

    // boost::topological_sort(CDAG, std::back_inserter(toposort));

    cout << "A topological ordering: ";
    int H=0;
    for (std::vector<Vertex>::reverse_iterator ii=toposort.rbegin(); ii!=toposort.rend(); ++ii) {
        cout << "c"<< *ii << " ";
        place_components[*ii] = H++;
    }
    cout << endl;
#endif
    return num_SCC;
}

//---------------------------------------------------------------------------------------

void reorder_using_SCC(std::vector<int> &out_order) {
    // compute_connected_components();

    // Move roots at the lowest levels, and BSCC at top levels
    if (get_num_components() <= 1)
        return; // nothing to do, the model is (supposedly) ergodic
    // if (!do_varorder_reshuffle_with_SCCs && !always_reorder)
    //     return;

    // if (!running_for_MCC())
    //     cout << "Reording variables using SCCs..." << endl;

    // {
    //     std::vector<int> inv(npl);
    //     for (int p=0; p<npl; p++)
    //         inv[out_order[p]] = p;
    //     cout << "\nBefore reorder: ";
    //     for (int p=0; p<npl; p++)
    //         cout << tabp[inv[p]].place_name << " ";
    //     cout << endl;
    // }

    struct st_reorder_t {
        int comp;   // component number (topological order)
        int place;  // place number
        st_reorder_t() { }
        st_reorder_t(int c, int p) : comp(c), place(p) { }
        // Compare for reorder: consider only the component field.
        inline bool operator<(const st_reorder_t& p) const { return comp < p.comp; }
    };

    // Support vector for the reorder phase. Note that reorder is done on the place
    // sequence, while net_to_mddLevel is the inverse sequence. Therefore, we need
    // to invert before and after the reorder.
    std::vector<st_reorder_t> reorder(npl);
    for (int p=0; p<npl; p++) {
        reorder[out_order[p]] = st_reorder_t{ place_components[p], p }; // BEFORE
        // reorder[out_order[p]] = st_reorder_t{ bfs_scc_order[place_components[p]], p };
        // cout << "weight["<<tabp[p].place_name << "] = " << weights[place_components[p]] << endl;
    }
    // Reorder preserving the order of entries with equal sytr value.
    std::stable_sort(reorder.begin(), reorder.end());

    // Copy back the reordered vector.
    for (int p=0; p<npl; p++) {
        out_order[reorder[p].place] = p;
    }

    // {
    //     std::vector<int> inv(npl);
    //     for (int p=0; p<npl; p++)
    //         inv[out_order[p]] = p;
    //     cout << "\nAfter reorder: ";
    //     for (int p=0; p<npl; p++)
    //         cout << tabp[inv[p]].place_name << " ";
    //     cout << endl << endl;
    // }
}


// bool is_transition_SCC_local(int tr) {
//     compute_connected_components();
//     if (num_SCC <= 1)
//         return true;

//     return trans_SCC_locality[tr];
// }

//---------------------------------------------------------------------------------------

void metrics_bandwidth(const std::vector<int>& net_to_level, uint64_t *BW, uint64_t *PROF,
                       double *AVGWF, double *MAXWF, double *RMSWF) 
{
    typedef boost::adjacency_list<boost::vecS, 
    /**/                          boost::vecS, 
    /**/                          boost::bidirectionalS> Graph;
    Graph G(npl);
    init_graph_edges_from_Petri_net(G, false, &net_to_level);

    if (BW)     *BW    = boost::bandwidth(G);
    //The profile is the sum of all the maximum distances between the i-th vertex and any of 
    // its neighbors with an index j>i :  B(G) = max { |index[u] - index[v]|  | (u,v) in E }
    if (PROF)   *PROF  = boost::profile(G);
    // Calculates the maximum/average/root mean square wavefront of a graph.
    if (AVGWF)  *AVGWF = boost::aver_wavefront(G);
    if (MAXWF)  *MAXWF = boost::max_wavefront(G);
    if (RMSWF)  *RMSWF = boost::rms_wavefront(G);
}

//---------------------------------------------------------------------------------------

// void generate_netgraph_metrics(std::vector<double>& out_metrics, bool pinv_metrics, bool bandwidth_metrics)
// {
//     typedef boost::adjacency_list<boost::vecS, 
//     /**/                          boost::vecS, 
//     /**/                          boost::bidirectionalS> Graph;

//     Graph G(npl);
//     boost::property_map<Graph, boost::vertex_index_t>::type index_map = boost::get(boost::vertex_index, G);
//     init_graph_edges_from_Petri_net(G);

//     clock_t start = clock();

//     // cout << "edges/vertex ratio:" << boost::num_edges(G) / double(boost::num_vertices(G)) << endl;

//     // Mean/variance of the in & out degree of the vertices
//     typedef boost::accumulators::accumulator_set<double, 
// 	/**/		boost::accumulators::features<boost::accumulators::tag::mean, 
// 	/**/									  boost::accumulators::tag::min, 
// 	/**/									  boost::accumulators::tag::max, 
// 	/**/									  boost::accumulators::tag::count, 
// 	/**/									  boost::accumulators::tag::variance>> perf_t;

//     auto print_perf = [&out_metrics](const perf_t& p){
//     	out_metrics.push_back(boost::accumulators::mean(p));
//     	out_metrics.push_back(sqrt(boost::accumulators::variance(p)));
//     	out_metrics.push_back(boost::accumulators::min(p));
//     	out_metrics.push_back(boost::accumulators::max(p));
//     };

//     // In and out degree on the boost::graph used to compute CM/SLO/KING/MDO/... orderings
//     perf_t in_deg_ratio, out_deg_ratio;
// 	boost::graph_traits<Graph>::vertex_iterator ui, ui_end;
// 	int max_inout_deg = 0;
//     for (boost::tie(ui, ui_end) = boost::vertices(G); ui != ui_end; ++ui) {
//     	int in = boost::in_degree(*ui, G);
//     	int out = boost::out_degree(*ui, G);
//     	max_inout_deg = max(max(max_inout_deg, in), out);
//         in_deg_ratio(in);
//         out_deg_ratio(out);
//     }
//     print_perf(in_deg_ratio);
//     print_perf(out_deg_ratio);

//     // Normalized in/out degrees in graph G
//     out_metrics.push_back(boost::accumulators::mean(in_deg_ratio) / max_inout_deg);
//     out_metrics.push_back(sqrt(boost::accumulators::variance(in_deg_ratio)) / max_inout_deg);
//     out_metrics.push_back(boost::accumulators::mean(out_deg_ratio) / max_inout_deg);
//     out_metrics.push_back(sqrt(boost::accumulators::variance(out_deg_ratio)) / max_inout_deg);

//     //---------------------------
//     // Petri-net metrics
//     //---------------------------

//     // Place/transitions ratio
//     out_metrics.push_back(double(npl)/ntr);

//     // Token quantity transported on each transitions by input/output/inhibitor arcs
//     perf_t in_trn_quant, out_trn_quant, inhib_trn_quant;
//     for (int t=0; t<ntr; t++) {
//     	int in_quant = 0, out_quant = 0, inhib_quant = 0;
//     	for (Node_p list = GET_INPUT_LIST(t); list != nullptr; list = NEXT_NODE(list))
//     		in_quant += list->molt;
//     	for (Node_p list = GET_OUTPUT_LIST(t); list != nullptr; list = NEXT_NODE(list))
//     		out_quant += list->molt;
//     	for (Node_p list = GET_INHIBITOR_LIST(t); list != nullptr; list = NEXT_NODE(list))
//     		inhib_quant += list->molt;
//     	in_trn_quant(in_quant);
//     	out_trn_quant(out_quant);
//     	inhib_trn_quant(inhib_quant);
//     }
//     print_perf(in_trn_quant);
//     print_perf(out_trn_quant);
//     print_perf(inhib_trn_quant);

//     // Token distribution in (non-zero) marked initial places
//     perf_t avg_tokens_nnz_m0; // tokens in non-zero places in m0
//     perf_t avg_tokens_m0;     // tokens in all places of m0
//     for(int pl=0; pl < npl; pl++) {
//     	avg_tokens_m0(net_mark[pl].total);
//     	if (net_mark[pl].total > 0)
//     		avg_tokens_nnz_m0(net_mark[pl].total);
//     }
//     print_perf(avg_tokens_nnz_m0);
//     print_perf(avg_tokens_m0);
//     // average # of marked places in m0, normalized by npl
//     out_metrics.push_back(double(boost::accumulators::count(avg_tokens_m0)) / npl);
//     // average # tokens in m0, normalized by npl
//     out_metrics.push_back(double(boost::accumulators::sum(avg_tokens_m0)) / npl);
//     // out_metrics.push_back("mean(tokens in non-empty m0 places): " << boost::accumulators::mean(avg_tokens_nnz_m0));
//     // out_metrics.push_back("mean(tokens in m0 places): " << boost::accumulators::mean(avg_tokens_m0));
//     // out_metrics.push_back("devstd(tokens in m0 places): " << sqrt(boost::accumulators::variance(avg_tokens_m0)));

//     // logarithm of places, transitions, arcs
//     out_metrics.push_back(log(npl));
//     out_metrics.push_back(log(ntr));
//     out_metrics.push_back(log(boost::accumulators::count(in_trn_quant) + 
// 		    			      boost::accumulators::count(out_trn_quant) + 
// 		    			      boost::accumulators::count(inhib_trn_quant)));

// 	if (pinv_metrics) {
// 	    // Check if we have P-invariants
// 	    std::string pinv_name(net_name);
// 	    pinv_name += "pin";
// 	    ifstream pif(pinv_name.c_str());
// 	    if (pif) {
// 		    int num_pinv;
// 		    pif >> num_pinv;
// 		    pif.close();

// 		    // Write the # of P-invars, and the ratio of P-invars and places
// 		    out_metrics.push_back(num_pinv);
// 		    out_metrics.push_back((double(num_pinv)/npl));
// 		}
// 		else { // No P-invariants availables
// 			out_metrics.push_back(0);
// 			out_metrics.push_back(0);
// 		}
// 	}

//     // Complex metrics - NOT ROBUST TO NET SHUFFLING
//     if (bandwidth_metrics) {
// 	    // The bandwidth of a graph is the maximum distance between two adjacent vertices, with distance 
// 	    // measured on a line upon which the vertices have been placed at unit intervals. To put it another way, 
// 	    // if the vertices of a graph G=(V,E) are each assigned an index from zero to |V| - 1 given by index[v], 
// 	    // then the bandwidth of G is: B(G) = max { |index[u] - index[v]|  | (u,v) in E }
// 	    out_metrics.push_back(boost::bandwidth(G));
// 	    //The profile is the sum of all the maximum distances between the i-th vertex and any of 
// 	    // its neighbors with an index j>i :  B(G) = max { |index[u] - index[v]|  | (u,v) in E }
// 	    out_metrics.push_back(boost::profile(G));
// 	    // Calculates the maximum/average/root mean square wavefront of a graph.
// 	    out_metrics.push_back(boost::max_wavefront(G));
// 	    out_metrics.push_back(boost::aver_wavefront(G));
// 	    out_metrics.push_back(boost::rms_wavefront(G));
// 	}

//     // cout << "Time to compute metrics: " << (double(clock() - start) / CLOCKS_PER_SEC) << endl;

// 	return;
// }

//---------------------------------------------------------------------------------------

// const flow_basis_t&
// mcl_cluster_net(size_t timeout, double inflation) {
//     static flow_basis_t mcl_psf;
//     static bool has_clusters = false;
//     if (has_clusters)
//         return mcl_psf;

//     // Build the clusters using the mcl command
//     has_clusters = true;

//     // Prepare net edges
//     std::vector<pair<int, int>> edges;

//     // Add edges between places connected by each transition
//     for (int tr=0; tr<ntr; tr++) {
//         for (Node_p in_ptr = GET_INPUT_LIST(tr); in_ptr; in_ptr = NEXT_NODE(in_ptr)) {
//             int in_plc = GET_PLACE_INDEX(in_ptr);
//             // int in_mult = in_ptr->molt;

//             for (Node_p out_ptr = GET_OUTPUT_LIST(tr); out_ptr; out_ptr = NEXT_NODE(out_ptr)) {
//                 int out_plc = GET_PLACE_INDEX(out_ptr);
//                 // int out_mult = out_ptr->molt;

//                 if (in_plc != out_plc) 
//                     edges.push_back(make_pair(in_plc, out_plc));
//             }
//         }
//     }
//     std::sort(edges.begin(), edges.end());
//     edges.erase(std::unique(edges.begin(), edges.end()), edges.end());

//     // Generate unique random file name
//     const int nnlen = strlen(net_name);
//     char tempname[nnlen + 16];
//     sprintf(tempname, "%sXXXXXXXX", net_name);
//     int fd = mkstemp(tempname);

// 	char cmd[nnlen + 128];
// 	sprintf(cmd, "/bin/bash -c 'timeout %ld mcl - -I %lf -o %s' > /dev/null 2>&1", timeout, inflation, tempname);

// 	// ignore SIGPIPEs generated by mcl failures
// 	signal(SIGPIPE, SIG_IGN);

// 	// start MCL tool
//     FILE *mcl = popen(cmd, "w");
//     if (mcl == nullptr)
// 		throw rgmedd_exception("unable to start MCL tool");

//     // Write the Petri net graph in MCL format
//     fprintf(mcl, "(mclheader\nmcltype matrix\ndimensions %dx%d\n)\n(mclmatrix\nbegin\n", npl, npl);

//     int j=0;
//     for (int row=0; row<npl; row++) {
//         fprintf(mcl, "%d  ", row);
//         while (j < edges.size() && edges[j].first == row)
//             fprintf(mcl, "%d:1 ", edges[j++].second);
//         fprintf(mcl, " $\n");
//     }

//     fprintf(mcl, ")\n");
//     fflush(mcl);

//     // Wait mcl to finish.
//     if (pclose(mcl) != 0) {
//         close(fd);
//         remove(tempname);
//         throw rgmedd_exception("mcl error!");
//     }
// 	// restore SIGPIPE default handler
// 	signal(SIGPIPE, SIG_DFL);

//     // Read back clusters as 'fake' P-invariants
//     mcl_psf.clear();
//     char line[256];
//     FILE* cluster = fopen(tempname, "r");
//     if (cluster == NULL) {
//         close(fd);
//         remove(tempname);
//         throw rgmedd_exception("could not read back clusters!");
//     }

//     while (fgets(line, sizeof(line), cluster) && 0!=strcmp(line, "begin\n")) { }

//     // Read clusters
//     while (true) {
//         if (0 == fscanf(cluster, " %s", line))
//             break;
//         if (feof(cluster) || 0==strcmp(line, ")"))
//             break; // End of file
//         sparse_vector_t psf;
//         while (!feof(cluster) && 1==fscanf(cluster, " %s", line) && 0!=strcmp(line, "$")) {
//             psf.add_element(atoi(line), 1);
//         }
//         mcl_psf.emplace_back(std::move(psf));
//     }
//     if (!running_for_MCC()) {
//         cout << "There are " << mcl_psf.size() << " place clusters." << endl;
//         // for (auto&& cl : mcl_psf) {
//         //     cout << cl.size() << ":  ";
//         //     for (auto&& p : cl)
//         //         cout << tabp[p.place_no].place_name << " ";
//         //     cout << endl;
//         // }
//     }

//     fclose(cluster);
//     close(fd);
//     remove(tempname);
//     return mcl_psf;
// }

// //---------------------------------------------------------------------------------------

// double
// get_cluster_isolation_coeff(const flow_basis_t& mcl_psf) {
//     ssize_t cluster_of[npl];
//     const size_t NCL = mcl_psf.size(); // number of clusters
//     for (size_t p=0; p<npl; p++)
//         cluster_of[p] = -1;
//     for (size_t c=0; c<mcl_psf.size(); c++)
//         for (auto&& p : mcl_psf[c])
//             cluster_of[p.place_no] = c;

//     std::set<pair<size_t, size_t>> connections;
//     for (int tr=0; tr<ntr; tr++) {
//         for (Node_p in_ptr = GET_INPUT_LIST(tr); in_ptr; in_ptr = NEXT_NODE(in_ptr)) {
//             size_t p1 = GET_PLACE_INDEX(in_ptr);
//             ssize_t cl1 = cluster_of[p1];
//             if (cl1 < 0)
//                 continue;
//             for (Node_p out_ptr = GET_OUTPUT_LIST(tr); out_ptr; out_ptr = NEXT_NODE(out_ptr)) {
//                 size_t p2 = GET_PLACE_INDEX(out_ptr);
//                 ssize_t cl2 = cluster_of[p2];
//                 if (cl2 < 0)
//                     continue;
//                 if (cl1 != cl2) // cluster 1 is connected to cluster 2
//                     connections.insert(make_pair(std::min(cl1, cl2), std::max(cl1, cl2)));
//             }
//         }
//     }

//     if (!running_for_MCC()) {
//         cout << "Connections: " << connections.size()*2 << " / " << NCL*(NCL-1) << " clusters: " << NCL << endl;
//     }

//     double N = connections.size() * 2.0;
//     double D = double(NCL) * double(NCL-1);
//     return N / D;
// }

//---------------------------------------------------------------------------------------

// Use the ViennaCL methods
void var_order_vcl(const VariableOrderCriteria voc, std::vector<int> &out_order) {
    vcl_matrix_t matrix;
    init_graph_edges_from_Petri_net(matrix);

    std::vector<int> perm;
    switch (voc) {
        case VOC_VCL_CUTHILL_MCKEE:
            perm = viennacl::reorder(matrix, viennacl::cuthill_mckee_tag());
            break;
        case VOC_VCL_ADVANCED_CUTHILL_MCKEE:
            perm = viennacl::reorder(matrix, viennacl::advanced_cuthill_mckee_tag());
            break;
        case VOC_VCL_GIBBS_POOLE_STOCKMEYER:
            perm = viennacl::reorder(matrix, viennacl::gibbs_poole_stockmeyer_tag());
            break;
        default:
            throw rgmedd_exception();
    }

    struct identity_t {
        size_t operator[] (size_t k) const { return k; }
    } iden;

    // Sometimes the VCL algorithms are a bit bugged. fix them.
    const size_t N = perm.size();
    std::vector<bool> found(N), inserted(N);
    std::fill(found.begin(), found.end(), false);
    std::fill(inserted.begin(), inserted.end(), false);

    for (size_t i=0; i<N; i++) {
        if (perm[i] >= N || perm[i] < 0)
            perm[i] = 0;
        found[ perm[i] ] = true;
    }
    size_t j = 0;
    for (size_t i=0; i<N; i++)
        if (found[perm[i]] && !inserted[perm[i]]) {
            perm[j++] = perm[i];
            inserted[perm[i]] = true;
        }
    for (size_t i=0; i<N; i++)
        if (!inserted[i])
            perm[j++] = i;

    fill_out_ordering(out_order, perm, iden);
}

//---------------------------------------------------------------------------------------

