

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <algorithm>
#include <numeric>
#include <map>
#include <stack>
#include <unistd.h> // access()

#include "rgmedd2.h"
#include "varorders.h"
// #include "nsf_subtree.h"

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
// #include <boost/graph/profile.hpp>
// #include <boost/graph/wavefront.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/graph/topological_sort.hpp>

// #undef SC
// #include <boost/accumulators/accumulators.hpp>
// #include <boost/accumulators/statistics.hpp>

//---------------------------------------------------------------------------------------
// Include the ViennaCL Cuthill-Mckee / Gibbs-Poole-Stockmeier algorithms

typedef size_t vcl_size_t; // Must precede VCL header inclusions
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
void init_graph_edges_from_Petri_net(Graph &graph, bool bidirectional = false)
{
    std::set<pair<int, int>> inserted;

    // Add an edge to the graph, avoiding duplicate insertions.
    // Note: minimum_degree_ordering crashes if edges are duplicated.
    auto add_bgl_edge = [&](int source, int target) {
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
    	// We missed some vertices. Thix may happen if those vertices are isolated.
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
void var_order_gradient_P(const VariableOrderCriteria voc, std::vector<int> &out_order) 
{
    std::vector<PSemiflow> psf;
    bool has_semiflows = load_flow_data(psf, voc);

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
				const PSemiflow *max_sf = nullptr;
				for (const PSemiflow& sf : psf) {
					double score = 0;
					int num_elems = 0;
					for (PSemiflowEntry sfe : sf) {
						int v = sfe.place_no;
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
				for (PSemiflowEntry sfe : *max_sf) {
					int v = sfe.place_no;
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

// Implements Noack and Tovchigrechko methods, as described in paper:
//   "MARCIE’s Secrets of Efficient Model Checking"
void var_order_noack_tovchigrechko(const VariableOrderCriteria voc, std::vector<int> &out_order) {
	// Precompute divisor of W: div_W(p) = |pre(p) U post(p)|, and pre/post T sets sizes
    std::vector<int> div_W(npl, 0);
 	for (int t=0; t<ntr; t++) {
		for (Node_p in = GET_INPUT_LIST(t); in; in = NEXT_NODE(in)) 
            div_W[GET_PLACE_INDEX(in)]++;
		for (Node_p out = GET_OUTPUT_LIST(t); out; out = NEXT_NODE(out)) 
            div_W[GET_PLACE_INDEX(out)]++;
	}

	std::vector<bool> S(npl, false); // already selected places
	std::vector<double> f(npl);      // intermediate weight of places
	std::vector<double> W(npl, 0);   // weight of places
	for (int pl=npl-1; pl>=0; pl--) {
		std::fill(f.begin(), f.end(), 0.0);
		for (int t=0; t<ntr; t++) {
			// Compute sizes of |pre(t) intersect S| and |post(t) intersect S|
			int n_preT_S = 0, n_postT_S = 0, n_preT = 0, n_postT = 0;
			for (Node_p in = GET_INPUT_LIST(t); in; in = NEXT_NODE(in), n_preT++) {
				if (S[GET_PLACE_INDEX(in)])
		            n_preT_S++;
	        }
			for (Node_p out = GET_OUTPUT_LIST(t); out; out = NEXT_NODE(out), n_postT++) {
	            if (S[GET_PLACE_INDEX(out)])
		            n_postT_S++;
	        }
	        double g1_t = (n_preT_S == 0) ? 0.1 : n_preT_S;
	        double h_t = (n_postT_S == 0) ? 0.2 : 2 * n_postT_S;

            switch (voc) {
                case VOC_NOACK: {
        		        // f(p) = sum_{t in pre(p)} (g1(t)/n_preT + 2*n_postT_S/n_postT) +
        		        //        sum_{t in post(p)} (h(t)/n_postT + (1+n_preT_S)/n_preT)
        		        //        only if n_preT>0 and n_postT>0
        		        if (n_preT == 0 || n_postT == 0)
        		        	continue;
        				for (Node_p in = GET_INPUT_LIST(t); in; in = NEXT_NODE(in)) {
        					if (!S[GET_PLACE_INDEX(in)])
        						f[GET_PLACE_INDEX(in)] += h_t / n_postT + (1.0 + n_preT_S) / n_preT;
        				}
        				for (Node_p out = GET_OUTPUT_LIST(t); out; out = NEXT_NODE(out)) {
        					if (!S[GET_PLACE_INDEX(out)])
        						f[GET_PLACE_INDEX(out)] += g1_t / n_preT + (2.0 * n_postT_S) / n_postT;
        				}
                    }
                    break;

                case VOC_TOVCHIGRECHKO: {
        				double g2_t = (n_postT_S == 0) ? 0.1 : 2 * n_postT_S;
        				// f(p) = sum_{t in pre(p)} (g1(t)/n_preT)           if n_preT>0
        				//      + sum_{t in pre(p)} (g2(t)/n_postT)          if n_postT>0
        				//      + sum_{t in post(p)} ((n_preT_S+1)/n_preT)   if n_preT>0
        				//      + sum_{t in post(p)} (h(t)/n_postT)          if n_postT>0
        				for (Node_p in = GET_INPUT_LIST(t); in; in = NEXT_NODE(in)) {
        					if (S[GET_PLACE_INDEX(in)])
        						continue;
        					if (n_preT > 0)
        						f[GET_PLACE_INDEX(in)] += (n_preT_S + 1.0) / n_preT;
        					if (n_postT > 0)
        						f[GET_PLACE_INDEX(in)] += h_t / n_postT;
        				}
        				for (Node_p out = GET_OUTPUT_LIST(t); out; out = NEXT_NODE(out)) {
        					if (S[GET_PLACE_INDEX(out)])
        						continue;
        					if (n_preT > 0)
        						f[GET_PLACE_INDEX(out)] += g1_t / n_preT;
        					if (n_postT > 0)
        						f[GET_PLACE_INDEX(out)] += g2_t / n_postT;
        				}
                    }
                    break;
                default:
                    throw rgmedd_exception();
			}
		}
		// Compute weights:  W(p) = f(p) / div_W(p)
		for (int p=0; p<npl; p++)
			if (!S[p])
				W[p] = f[p] / (div_W[p] == 0 ? 1 : div_W[p]);

		// Select the place with the highest weight
		int selP = std::distance(W.begin(), std::max_element(W.begin(), W.end()));
		if (S[selP])
			throw rgmedd_exception("Bug in Noack/Tovchigrechko method implementation.");
		S[selP] = true;
		W[selP] = -1000000.0;
		out_order[pl] = selP;
	}
}

//---------------------------------------------------------------------------------------

static int num_SCC = -1; // -1 means ininitialized
// SCC identifier for each place
static std::vector<int> place_components;
// Is a transition local to a component, or is cross-components?
// static std::vector<bool> trans_SCC_locality;
// should we do the variable order reshuffle using SCC info??
static bool do_varorder_reshuffle_with_SCCs = false;

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

    return num_SCC;
}

//---------------------------------------------------------------------------------------

void reorder_using_SCC(std::vector<int> &out_order, bool always_reorder) {
    compute_connected_components();

    // Move roots at the lowest levels, and BSCC at top levels
    if (num_SCC <= 1)
        return; // nothing to do, the model is (supposedly) ergodic
    if (!do_varorder_reshuffle_with_SCCs && !always_reorder)
        return;

    if (!running_for_MCC())
        cout << "Reorder variables using SCCs..." << endl;

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
        reorder[out_order[p]] = st_reorder_t{ place_components[p], p };
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

//---------------------------------------------------------------------------------------

const int* get_place_components() {
    if (num_SCC <= 1)
        throw rgmedd_exception("Should not call this method if there are no SCCs.");
    return place_components.data();
}

//---------------------------------------------------------------------------------------

void mcl_cluster_net(std::vector<PSemiflow>& out_psf, double inflation) {
    // Prepare net edges
    std::vector<pair<int, int>> edges;

    // Add edges between places connected by each transition
    for (int tr=0; tr<ntr; tr++) {
        for (Node_p in_ptr = GET_INPUT_LIST(tr); in_ptr; in_ptr = NEXT_NODE(in_ptr)) {
            int in_plc = GET_PLACE_INDEX(in_ptr);
            // int in_mult = in_ptr->molt;

            for (Node_p out_ptr = GET_OUTPUT_LIST(tr); out_ptr; out_ptr = NEXT_NODE(out_ptr)) {
                int out_plc = GET_PLACE_INDEX(out_ptr);
                // int out_mult = out_ptr->molt;

                if (in_plc != out_plc) 
                    edges.push_back(make_pair(in_plc, out_plc));
            }
        }
    }
    std::sort(edges.begin(), edges.end());
    edges.erase(std::unique(edges.begin(), edges.end()), edges.end());

    // Generate unique random file name
    const int nnlen = strlen(net_name);
    char tempname[nnlen + 16];
    sprintf(tempname, "%sXXXXXXXX", net_name);
    int fd = mkstemp(tempname);

    // Open MCL tool
    char cmd[nnlen + 128];
    sprintf(cmd, "mcl - -I %lf -o %s > /dev/null 2>&1", inflation, tempname);
    // sprintf(cmd, "cat - ");
    FILE *mcl = popen(cmd, "w");
    if (mcl == nullptr)
        throw rgmedd_exception("mcl not found in the program PATH.");

    // Write the Petri net graph in MCL format
    fprintf(mcl, "(mclheader\nmcltype matrix\ndimensions %dx%d\n)\n(mclmatrix\nbegin\n", npl, npl);

    int j=0;
    for (int row=0; row<npl; row++) {
        fprintf(mcl, "%d  ", row);
        while (j < edges.size() && edges[j].first == row)
            fprintf(mcl, "%d:1 ", edges[j++].second);
        fprintf(mcl, " $\n");
    }

    fprintf(mcl, ")\n");
    fflush(mcl);

    // Wait mcl to finish.
    if (pclose(mcl) != 0)
        throw rgmedd_exception("mcl error!");

    // Read back clusters as 'fake' P-invariants
    out_psf.clear();
    char line[256];
    FILE* cluster = fopen(tempname, "r");
    if (cluster == NULL)
        throw rgmedd_exception("could not read back clusters!");

    while (fgets(line, sizeof(line), cluster) && 0!=strcmp(line, "begin\n")) { }

    // Read clusters
    while (true) {
        fscanf(cluster, " %s", line);
        if (feof(cluster) || 0==strcmp(line, ")"))
            break; // End of file
        PSemiflow psf;
        while (!feof(cluster) && 1==fscanf(cluster, " %s", line) && 0!=strcmp(line, "$")) {
            psf.push_back(PSemiflowEntry{ atoi(line), 1 });
        }
        out_psf.emplace_back(std::move(psf));
    }
    if (!running_for_MCC())
        cout << "There are " << out_psf.size() << " place clusters." << endl;

    fclose(cluster);
    close(fd);
    remove(tempname);
}

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

    fill_out_ordering(out_order, perm, iden);
}


//---------------------------------------------------------------------------------------

