// Interface to OGDF
// Read a net file (in gml format), apply a layout algorithm,
// and save the result in an easy-to-parse format for the JavaGUI.
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/orthogonal/OrthoLayout.h>
#include <ogdf/planarity/EmbedderMinDepthMaxFaceLayers.h>
#include <ogdf/planarity/PlanarSubgraphFast.h>
#include <ogdf/planarity/PlanarizationLayout.h>
#include <ogdf/planarity/SubgraphPlanarizer.h>
#include <ogdf/planarity/VariableEmbeddingInserter.h>
#include <ogdf/planarlayout/PlanarStraightLayout.h>
#include <ogdf/planarity/PlanarizationGridLayout.h>

#include <ogdf/energybased/GEMLayout.h>
#include <ogdf/energybased/FMMMLayout.h>
#include <ogdf/energybased/FastMultipoleEmbedder.h>
#include <ogdf/misclayout/BalloonLayout.h>
#include <ogdf/misclayout/CircularLayout.h>
#include <ogdf/energybased/DavidsonHarelLayout.h>
// // #include <ogdf/energybased/StressMajorizationSimple.h>
#include <ogdf/energybased/MultilevelLayout.h>
#include <ogdf/layered/SugiyamaLayout.h>
#include <ogdf/layered/OptimalRanking.h>
#include <ogdf/layered/OptimalHierarchyLayout.h>
#include <ogdf/upward/DominanceLayout.h>
// #include <ogdf/upward/UpwardPlanarizationLayout.h>
#include <ogdf/upward/VisibilityLayout.h>
#include <ogdf/energybased/SpringEmbedderFRExact.h>
#include <ogdf/energybased/SpringEmbedderKK.h>
// #include <ogdf/tree/TreeLayout.h>
// #include <ogdf/tree/RadialTreeLayout.h>
// #include <ogdf/planarlayout/MixedModelLayout.h>
#include <ogdf/planarlayout/PlanarDrawLayout.h>
// #include <ogdf/planarity/PlanarizationLayout.h>
// #include <ogdf/planarity/PlanarizationGridLayout.h>
// // new with v2012.06
#include <ogdf/planarlayout/FPPLayout.h>
// #include <ogdf/planarlayout/SchnyderLayout.h>

// #include <ogdf/energybased/multilevelmixer/MMMExampleFastLayout.h>
// #include <ogdf/energybased/multilevelmixer/MMMExampleNiceLayout.h>
// #include <ogdf/energybased/multilevelmixer/MMMExampleNoTwistLayout.h>
// #include <ogdf/energybased/multilevelmixer/MixedForceLayout.h>

#include <iostream>
#include <fstream>
#include <cstdlib>
// #include <signal.h>
// #include <unistd.h>

using namespace ogdf;
using namespace std;

// void alarm_handler(int) {
//     exit(25);
// }

// scale & translate
inline double sctr(double v, double translate, double scale, bool do_round) {
    double ret = (v  * scale) - translate;
    if (do_round)
        ret = std::round(ret);
    return ret;
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        cout << "Missing arguments." << endl;
        return 1;
    }
    const char* in_file = argv[1];
    const char* out_file = argv[2];
    const char* layout_algo = argv[3];
    // const double separation = atof(argv[3]);
    // const double overhang = atof(argv[4]);
    const double separation = 4.0;
    const double overhang = 1.0;

    double scaleX = 0.1, scaleY = 0.1;
    bool do_round = false;

    // signal(SIGALRM, alarm_handler);
    // alarm(5);

	Graph G;
	GraphAttributes GA(G,
	  GraphAttributes::nodeGraphics | GraphAttributes::nodeType |
	  GraphAttributes::edgeGraphics | GraphAttributes::edgeType);

	if (!GraphIO::read(GA, G, in_file, GraphIO::readGML)) {
		std::cerr << "Could not read " << in_file << std::endl;
		return 1;
	}

    if (0==strcmp(layout_algo, "FMMM")) {
        FMMMLayout pl;
        pl.unitEdgeLength(separation*5);
        pl.call(GA);
    }
    else if (0==strcmp(layout_algo, "FMME")) {
        FastMultipoleMultilevelEmbedder pl;
        pl.call(GA);
    }
    else if (0==strcmp(layout_algo, "FPP")) {
        FPPLayout pl;
        pl.call(GA);
    }
    else if (0==strcmp(layout_algo, "GEM")) {
        GEMLayout pl;
        pl.minDistCC(separation);
        pl.call(GA);
    }
    // else if (0==strcmp(layout_algo, "Mixed Force Layout")) {
    //     MixedForceLayout pl;
    //     pl.call(GA);
    // }
    // else if (0==strcmp(layout_algo, "Mixed Model Layout")) {
    //     MixedModelLayout pl;
    //     pl.call(GA);
    // }
    // else if (0==strcmp(layout_algo, "Nice")) {
    //     MMMExampleNiceLayout pl;
    //     pl.call(GA);
    // }
    // else if (0==strcmp(layout_algo, "Fast")) {
    //     MMMExampleFastLayout pl;
    //     pl.call(GA);
    // }
    // else if (0==strcmp(layout_algo, "NoTwist")) {
    //     MMMExampleNoTwistLayout pl;
    //     pl.call(GA);
    // }
    else if (0==strcmp(layout_algo, "Basic Planarization")) {
        PlanarizationLayout pl;
        // SubgraphPlanarizer crossMin;

        // auto* ps = new PlanarSubgraphFast<int>;
        // ps->runs(100);
        // VariableEmbeddingInserter *ves = new VariableEmbeddingInserter;
        // ves->removeReinsert(RemoveReinsertType::All);

        // crossMin.setSubgraph(ps);
        // crossMin.setInserter(ves);

        // EmbedderMinDepthMaxFaceLayers *emb = new EmbedderMinDepthMaxFaceLayers;
        // pl.setEmbedder(emb);

        OrthoLayout *ol = new OrthoLayout;
        ol->separation(separation);
        ol->cOverhang(overhang);
        pl.setPlanarLayouter(ol);
        do_round = true;
        scaleX = scaleY = 0.2;

        pl.call(GA);
    }
    else if (0==strcmp(layout_algo, "Grid Planarization")) {
        PlanarizationGridLayout pl;
        do_round = true;
        pl.call(GA);
    }
    else if (0==strcmp(layout_algo, "Straight Planarization")) {
        PlanarStraightLayout pl;
        do_round = true;
        pl.call(GA);
    }
    // else if (0==strcmp(layout_algo, "Draw Planarization")) {
    //     PlanarDrawLayout pl;
    //     do_round = true;
    //     pl.call(GA);
    // }
    // else if (0==strcmp(layout_algo, "Spring Layout")) {
    //     SpringEmbedderFRExact pl;
    //     pl.minDistCC(separation);
    //     pl.call(GA);
    // }
    // else if (0==strcmp(layout_algo, "SpringKK Layout")) {
    //     SpringEmbedderKK pl;
    //     pl.setDesLength(separation);
    //     pl.call(GA);
    // }
#ifdef HAS_COIN_LIB
    else if (0==strcmp(layout_algo, "Sugiyama")) {
        SugiyamaLayout pl;
        pl.setRanking(new OptimalRanking());
        pl.call(GA);
    }
#endif
    else if (0==strcmp(layout_algo, "Visibility")) {
        VisibilityLayout pl;
        pl.setMinGridDistance(separation);
        pl.call(GA);
    }
    // else if (0==strcmp(layout_algo, "Tree Layout")) {
    //     TreeLayout pl;
    //     pl.siblingDistance(separation);
    //     pl.subtreeDistance(separation);
    //     pl.treeDistance(separation);
    //     pl.levelDistance(separation);
    //     pl.call(GA);
    // }
    // else if (0==strcmp(layout_algo, "Orthogonal Tree Layout")) {
    //     TreeLayout pl;
    //     pl.orthogonalLayout(true);
    //     pl.siblingDistance(separation);
    //     pl.subtreeDistance(separation);
    //     pl.treeDistance(separation);
    //     pl.levelDistance(separation);
    //     pl.call(GA);
    // }
    // else if (0==strcmp(layout_algo, "Upward Planarization")) {
    //     UpwardPlanarizationLayout pl;
    //     pl.call(GA);
    // }
    // else if (0==strcmp(layout_algo, "Schnyder")) {
    //     SchnyderLayout pl;
    //     pl.separation(separation);
    //     pl.call(GA);
    // }
    else if (0==strcmp(layout_algo, "Davidson-Harel")) {
        DavidsonHarelLayout pl;
        pl.setPreferredEdgeLength(separation);
        scaleX = scaleY = 10.0;
        pl.call(GA);
    }
    else if (0==strcmp(layout_algo, "Dominance")) {
        DominanceLayout pl;
        pl.setMinGridDistance(separation);
        pl.call(GA);
    }
    else if (0==strcmp(layout_algo, "Balloon")) {
        BalloonLayout pl;
        pl.call(GA);
    }
    else if (0==strcmp(layout_algo, "Circular")) {
        CircularLayout pl;
        pl.call(GA);
    }
    else {
        cerr << "Unknown graph algorithm " << layout_algo << endl;
        return 1;
    }

	// PlanarizationLayout pl;
	// SubgraphPlanarizer crossMin;

	// auto* ps = new PlanarSubgraphFast<int>;
	// ps->runs(100);
	// VariableEmbeddingInserter *ves = new VariableEmbeddingInserter;
	// ves->removeReinsert(RemoveReinsertType::All);

	// crossMin.setSubgraph(ps);
	// crossMin.setInserter(ves);

	// EmbedderMinDepthMaxFaceLayers *emb = new EmbedderMinDepthMaxFaceLayers;
	// pl.setEmbedder(emb);

	// OrthoLayout *ol = new OrthoLayout;
	// ol->separation(separation);
	// ol->cOverhang(overhang);
	// pl.setPlanarLayouter(ol);

	// pl.call(GA);

    // PlanarStraightLayout pl;

    // DavidsonHarelLayout pl;
    // pl.setPreferredEdgeLength (4000);
    // pl.call(GA);

    // Find min x/y
    double minX = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::max();
    for (node v : G.nodes) {
        minX = std::min(minX, GA.x(v) * scaleX);
        minY = std::min(minY, GA.y(v) * scaleY);
	}
    for (edge e : G.edges) {
        const DPolyline &dpl = GA.bends(e);
        for (const DPoint &point : dpl) {
            minX = std::min(minX, point.m_x * scaleX);
            minY = std::min(minY, point.m_y * scaleY);
        }
    }
    // Set the new net to start in (4,4)
    minX -= 4;
    minY -= 4;

    // Write the output
	// GraphIO::write(GA, out_file, GraphIO::writeGML);
    ofstream fout(out_file);

    // write nodes positions
    for (node v : G.nodes) {
        fout << sctr(GA.x(v), minX, scaleX, do_round) << " " 
             << sctr(GA.y(v), minY, scaleY, do_round) << endl;
	}

    // write edges positions
    for (edge e : G.edges) {
        fout << e->source() << " " << e->target() << " ";
        const DPolyline &dpl = GA.bends(e);
        if (dpl.size() > 0) {
            node v = e->source();
            int has_front = (dpl.front().m_x < GA.x(v) - GA.width (v)/2 ||
            			     dpl.front().m_x > GA.x(v) + GA.width (v)/2 ||
            			     dpl.front().m_y < GA.y(v) - GA.height(v)/2 ||
            			     dpl.front().m_y > GA.y(v) + GA.height(v)/2);
            v = e->target();
            int has_back = (dpl.back().m_x < GA.x(v) - GA.width (v)/2 ||
            			    dpl.back().m_x > GA.x(v) + GA.width (v)/2 ||
            			    dpl.back().m_y < GA.y(v) - GA.height(v)/2 ||
            			    dpl.back().m_y > GA.y(v) + GA.height(v)/2);
            int num_points = dpl.size() - 2 + has_front + has_back;


            fout << num_points << "  ";

            int pos = 0;
            for (const DPoint &point : dpl) {
                pos++;
                if (pos == 1 && !has_front)
                    continue;
                if (pos == dpl.size() && !has_back)
                    continue;
                fout << sctr(point.m_x, minX, scaleX, do_round) << " " 
                     << sctr(point.m_y, minY, scaleY, do_round) << "  ";
            }
        }
        else {
            fout << "0";
        }
        fout << endl;
	}
    fout.close();

	return 0;
}
