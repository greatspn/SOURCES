/***************************************************************************
 *   Copyright (C) 2006 by Marco Beccuti   *
 *   beccuti@mfn.unipmn.it   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <libxml++/libxml++.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>

#include <mdp_base.h>
#include <drawable_mdp.h>
#include <infinite_mdp.h>
#include <mdp.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <sstream>
#include <vector>
#include <graphMDP/infinite_mdp.h>
#define graphlib 0

#if !(__GEN_H__)
#define __GEN_H__
#include "general.h"
#endif

using namespace  graphMDP;
using namespace  general;

Glib::RefPtr<mdp> mMDP;

#ifdef graphlib
Glib::RefPtr<drawable_mdp> mDrawableMDP;
//*******************
static bool CanSolve;
static unsigned int Choice;
bool graph = false, TMDP = true;
bool write_flag = false, write_flag1 = false;

std::ofstream fp_write, fp_writeoar, fp1_write, fp1_writeoar;

std::vector<std::string>vec_trans;

unsigned int thorizon = 0;

void init() {
    mDrawableMDP->InitScene();
    CanSolve = true;
}

void display() {
    //mDrawableMDP->InitializeDrawing();
    mDrawableMDP->DisplayScene(Choice - 2);
    glutSwapBuffers();
}

void reshape(int w, int h) {
    mDrawableMDP->setScreen((GLsizei) w, (GLsizei) h);
    mDrawableMDP->ReshapeScene();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'i' :
        mDrawableMDP->setZoom(mDrawableMDP->getZoom() + 0.1);
        mDrawableMDP->ReshapeScene();
        glutPostRedisplay();
        break;

    case 'o' :
        mDrawableMDP->setZoom(mDrawableMDP->getZoom() - 0.1);
        mDrawableMDP->ReshapeScene();
        glutPostRedisplay();
        break;

    case 'c' :
        mDrawableMDP->ReinitializeCamera();
        mDrawableMDP->ReshapeScene();
        glutPostRedisplay();
        break;

    case 's' :
        std::cout << "State : ";
        unsigned int state_number;
        std::cin >> state_number;
        std::cout << std::endl;
        mDrawableMDP->CenterView(state_number);
        mDrawableMDP->ReshapeScene();
        glutPostRedisplay();

    case '1':
        if (CanSolve) {
            CanSolve = false;
            std::cout << "Solving the MDP..." ;
            std::cout.flush();
            mMDP->Solve();
            std::cout << "OK in " << mMDP->getResolutionTime() << " seconds" << std::endl;
            Glib::ustring solution = mMDP->getSolution();
            std::cout << solution << std::endl;
            std::cout.flush();
            CanSolve = true;
        }
        glutPostRedisplay();
        break;

    case '2' :
        mDrawableMDP->InitializeRewardsDrawing();

        Choice = 2;
        glutPostRedisplay();
        break;

    case '3' :
        mDrawableMDP->InitializeRewardsDrawing();
        mDrawableMDP->InitializeValueFunctionDrawing();
        Choice = 3;
        glutPostRedisplay();
        break;

    case '4' :
        mDrawableMDP->InitializeValueFunctionDrawing();
        mDrawableMDP->InitializePolicyDrawing();
        Choice = 4;
        glutPostRedisplay();
        break;
    case '5' :
        int i, x, y, z;
        std::cout << "\nState:";
        std::cin >> i;
        std::cout << "x:";
        std::cin >> x;
        std::cout << "y:";
        std::cin >> y;
        std::cout << "z:";
        std::cin >> z;
        system("clear");
        std::cout << "Options : \n1 - Solve the MDP\n2 - Draw the MDP with initial rewards\n3 - Draw the optimal value function\n4 - Draw the optimal policy\n5 - Set states position" << std::endl;
        std::cout << "\nCommands : \ni - Zoom IN\no - Zoom OUT\nc - Place the camera at the center of the MDP\ns - Place the camera at the center of a state\n";
        mDrawableMDP->setStateCoordinates(i, x, y, z);
        mDrawableMDP->InitializeDrawing();
        glutPostRedisplay();
        break;
    default :
        glutPostRedisplay();
        break;
    }
}


void special_keyboard(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP :
        mDrawableMDP->setLongitudinalCameraPosition(mDrawableMDP->getLongitudinalCameraPosition() + 10.0);
        break;

    case GLUT_KEY_RIGHT :
        mDrawableMDP->setLateralCameraPosition(mDrawableMDP->getLateralCameraPosition() + 10.0);
        break;

    case GLUT_KEY_DOWN :
        mDrawableMDP->setLongitudinalCameraPosition(mDrawableMDP->getLongitudinalCameraPosition() - 10.0);
        break;

    case GLUT_KEY_LEFT :
        mDrawableMDP->setLateralCameraPosition(mDrawableMDP->getLateralCameraPosition() - 10.0);
        break;

    default :
        break;
    }

    mDrawableMDP->ReshapeScene();
    glutPostRedisplay();
}
#endif

//*******************************************************
void read_data(ifstream &in, int argc, char *argv[]) {
    //InfiniteMDP::OptimizationAlgorithmType Optimization_algorithm=InfiniteMDP::LINEAR_PROGRAMMING;
    InfiniteMDP::OptimizationAlgorithmType Optimization_algorithm = InfiniteMDP::VALUE_ITERATION;
    double precision = 0.0001;
    unsigned int max_iterations_number = 500000;

    cout << "Precision: " << precision << " Max Iterations: " << max_iterations_number << endl;
    if (thorizon != 0)
        cout << "Horizon:" << thorizon << endl;
    if (thorizon == 0)
        mMDP = AverageRewardMDP::create(in, Optimization_algorithm, precision, max_iterations_number);
   //mMDP=DiscountedMDP::create(in,0.999,Optimization_algorithm,precision,max_iterations_number);
    else
        mMDP = FiniteStationaryMDP::create(in, thorizon);
    mMDP->Solve();
    Glib::ustring solution = mMDP->getSolution();


    Glib::ustring MDP_dato = mMDP->getData();
    //std::cout<<mMDP->getStateAction(3)<<" "<<mMDP->getNbActions ();
    std::cout << "\n\n*******************OPTIMAL  STRATEGY*************************\n" << solution << "\n************************************************************\n\n";

    fp_write << thorizon << endl << solution;
    fp_writeoar << MDP_dato << std::endl;

    if (thorizon == 0) {
        if (write_flag1) {
            Glib::ustring solution1 = solution.substr(solution.find("\n\n"));
            fp1_write << "\n\n******************************* OPTIMAL STRATEGY *************************************\n\n";
            std::stringstream stream;
            stream.str(solution1);
            std::string tmp_parser;
            Parser parser, parser1;
            char delim[] = " :;\n", delim1[] = "(){";

            //bool col_trans;
            while (stream) {
                //bool fine=false;
                getline(stream, tmp_parser, '\n');
                parser.update(delim, tmp_parser);
                if (parser.get(0) == "Optimal") {
                    fp1_write << "\n" << tmp_parser;
                    fp1_writeoar << parser.get(3);
                }
                else {
                    unsigned int i = 1;
                    if ((parser.get(0) != ""))
                        fp1_write << parser.get(0) << "\t:\t";
                    //int temp=parser.size()-1
                    while ((parser.size() != 0) && (i < (parser.size() - 1))) {
                        parser1.update(delim1, parser.get(i));
                        fp1_write << vec_trans[atoi(parser1.get(0).c_str())] << "(";
                        if (parser1.size() == 2)
                            if (parser1.get(1).find("}") == std::string::npos)
                                fp1_write << parser1.get(1);
                        fp1_write << ")";
                        i++;
                    }
                    fp1_write << std::endl;
                }
            }
        }
    }
}
StationaryOption  read_data(Glib::ustring filepath, int argc, char *argv[]) {
    xmlpp::DomParser xml_parser;
    xml_parser.set_validate(true);
    xml_parser.set_substitute_entities();
    xml_parser.parse_file(filepath);
    xmlpp::Document *mdp_doc = xml_parser.get_document();
    bool infinite = true;
    InfiniteCriteriumType  criterium = DISCOUNTED;
    DecompositionAlgorithmType decomposition = GRID_DECOMPOSITION;
    InfiniteMDP::OptimizationAlgorithmType Optimization_algorithm = InfiniteMDP::LINEAR_PROGRAMMING;
    double precision = 0.0001;
    unsigned int max_iterations_number = 10000;
    unsigned int horizon = 0;
    if (mdp_doc == NULL) {
        xmlException xml_mdp_error("XML document unvalidate");
        throw xml_mdp_error;
    }
    xmlpp::Element *mdp_root_node = mdp_doc->get_root_node();
    const xmlpp::Element::AttributeList &attributes = mdp_root_node->get_attributes();
    for (xmlpp::Element::AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter) {
        const xmlpp::Attribute *attribute = *iter;
        std::cout << "  Attribute " << attribute->get_name() << " = " << attribute->get_value() << std::endl;
        if (attribute->get_name() == "Infinite") {
            if (attribute->get_value() == "true") {
                infinite = true;
            }
            else {
                infinite = false;
            }
        }
        if (attribute->get_name() == "InfiniteCriteriumType") {
            if (attribute->get_value() == "AVERAGE_REWARD")
                criterium = AVERAGE_REWARD;
            else
                criterium = DISCOUNTED;
        }
        if (attribute->get_name() == "DecompositionAlgorithmType") {
            if (attribute->get_value() == "GRID_DECOMPOSITION")
                decomposition = GRID_DECOMPOSITION;
            else
                decomposition = LINEAR_PROGRAMMING_DECOMPOSITION;
        }
        if (attribute->get_name() == "Horizon")
            sscanf((*iter)->get_value().c_str(), "%u", &horizon);
        if (attribute->get_name() == "Optimization_algorithm") {
            if (attribute->get_value() == "LINEAR_PROGRAMMING")
                Optimization_algorithm = InfiniteMDP::LINEAR_PROGRAMMING;
            if (attribute->get_value() == "VALUE_ITERATION")
                Optimization_algorithm = InfiniteMDP::VALUE_ITERATION;
            if (attribute->get_value() == "POLICY_ITERATION") {
                Optimization_algorithm = InfiniteMDP::POLICY_ITERATION;
                cout << "q" << endl;
            }
        }
        if (attribute->get_name() == "Max_iterations_number")
            sscanf((*iter)->get_value().c_str(), "%u", &max_iterations_number);
        if (attribute->get_name() == "Precision")
            sscanf((*iter)->get_value().c_str(), "%lf", &precision);
    }

    xmlpp::Node::NodeList mdp_root_node_children = mdp_root_node->get_children();
    xmlpp::Node::NodeList::const_iterator Imdp_root_node_children;
    xmlpp::Element *xml_states_element = NULL;
    xmlpp::Element *xml_regions_element = NULL;
    xmlpp::Element *xml_actions_element = NULL;
    for (Imdp_root_node_children = mdp_root_node_children.begin() ; Imdp_root_node_children != mdp_root_node_children.end() ; ++Imdp_root_node_children) {
        if ((*Imdp_root_node_children)->get_name() == "STATES") {
            xml_states_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
        }

        if ((*Imdp_root_node_children)->get_name() == "REGIONS") {
            xml_regions_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
        }

        if ((*Imdp_root_node_children)->get_name() == "ACTIONS") {
            xml_actions_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
        }
    }
    if (horizon == 0) {
        mMDP = AverageRewardMDP::create(*xml_states_element, *xml_actions_element, Optimization_algorithm, precision, max_iterations_number);
        //mMDP=DiscountedMDP::create(*xml_states_element,*xml_actions_element,0.999,Optimization_algorithm,precision,max_iterations_number);
    }
    else {
        mMDP = FiniteStationaryMDP::create(*xml_states_element, *xml_actions_element, horizon);
    }
    mMDP->Solve();
    Glib::ustring solution = mMDP->getSolution();


    Glib::ustring MDP_dato = mMDP->getData();
    //std::cout<<mMDP->getStateAction(3)<<" "<<mMDP->getNbActions ();
    std::cout << "\n\n*******************OPTIMAL STRATEGY*************************\n" << solution << "\n************************************************************\n\n";

    fp_write << horizon << endl << solution;
    fp_writeoar << MDP_dato << std::endl;

    if (horizon == 0) {
        if (write_flag1) {
            Glib::ustring solution1 = solution.substr(solution.find("\n\n"));
            fp1_write << "\n\n******************************* OPTIMAL STRATEGY *************************************\n\n";
            std::stringstream stream;
            stream.str(solution1);
            std::string tmp_parser;
            Parser parser, parser1;
            char delim[] = " :;\n", delim1[] = "(){";

            //bool col_trans;
            while (stream) {
                //bool fine=false;
                getline(stream, tmp_parser, '\n');
                parser.update(delim, tmp_parser);
                if (parser.get(0) == "Optimal") {
                    fp1_write << "\n" << tmp_parser;
                    fp1_writeoar << parser.get(3);
                }
                else {
                    unsigned int i = 1;
                    if ((parser.get(0) != ""))
                        fp1_write << parser.get(0) << "\t:\t";
                    //int temp=parser.size()-1
                    while ((parser.size() != 0) && (i < (parser.size() - 1))) {
                        parser1.update(delim1, parser.get(i));
                        fp1_write << vec_trans[atoi(parser1.get(0).c_str())] << "(";
                        if (parser1.size() == 2)
                            if (parser1.get(1).find("}") == std::string::npos)
                                fp1_write << parser1.get(1);
                        fp1_write << ")";
                        i++;
                    }
                    fp1_write << std::endl;
                }
            }
        }
    }

    /*
    				while (stream)
    				{
    				bool fine=false;
    				getline(stream,tmp_parser,'\n');
    			if (tmp_parser=="")
    				{
    				fine=true;
    				getline(stream,tmp_parser,'\n');
    				if (tmp_parser!="")
    					{
    					fp1_write<<"\n"<<tmp_parser;
    					fp1_writeoar<<tmp_parser.substr(tmp_parser.find(':')+2);
    					}
    				}
    			int i=0;
    			std::string tmp_text;
    			col_trans=false;
    			while (!fine)
    				{
    				switch (tmp_parser[i])
    					{
    					case ' ':
    						if (tmp_text!="")
    						fp1_write<<tmp_text<<" : ";
    						tmp_text="";
    					break;
    					case '(':
    						fp1_write<<vec_trans[atoi(tmp_text.c_str())]<<"(";
    						tmp_text="";
    						col_trans=true;
    					break;
    					case ';':
    						if (col_trans)
    						 fp1_write<<tmp_text<<")";
    						else
    						 fp1_write<<vec_trans[atoi(tmp_text.c_str())]<<"()";
    						tmp_text="";
    						col_trans=false;
    					break;
    					case '{':
    						if (col_trans)
    						 fp_write<<tmp_text<<")"<<std::endl;
    						else
    						 fp1_write<<vec_trans[atoi(tmp_text.c_str())]<<"()"<<std::endl;
    						tmp_text="";
    						col_trans=false;
    						fine=true;
    					case ':':
    					break;
    					case ')':
    					break;
    					default:
    						tmp_text+=tmp_parser[i];
    					break;
    					}
    				i++;
    				}

    			}
    fp1_write<<"\n\n**************************************************************************************\n";
    		}
    */
#ifdef graphlib
    if (graph == true) {
        //Glib::RefPtr<mdp>drawmdp;
        //mdp  =*mdp
        //InfiniteDrawableMDP<AverageRewardMDP> drawmdp(mdp1);
        //drawmdp=InfiniteDrawableMDP::create(mdp1);

        std::cout << "Options : \n1 - Solve the MDP\n2 - Draw the MDP with initial rewards\n3 - Draw the optimal value function\n4 - Draw the optimal policy\n5 - Set states position" << std::endl;
        std::cout << "\nCommands : \ni - Zoom IN\no - Zoom OUT\nc - Place the camera at the center of the MDP\ns - Place the camera at the center of a state\n";

        mDrawableMDP = InfiniteDrawableMDP<AverageRewardMDP>::create(Glib::RefPtr<AverageRewardMDP>::cast_dynamic(mMDP));
        srand(time(NULL));
        unsigned int num_states = mMDP->getNbStates();
        int x = 0, y = 0;
        //use to draw set position states
        for (unsigned int i = 0; i < num_states; i++) {
            x = (x + 50) % 200;
            if (x == 0) y = y + 50;
            mDrawableMDP->setStateCoordinates(i, x, y, 0);
        }
        //use to draw set position states
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
        glutInitWindowSize(3000, 3000);
        glutInitWindowPosition(0, 0);
        glutCreateWindow("Drawable MDP");
        init();
        glutDisplayFunc(display);
        glutReshapeFunc(reshape);
        glutKeyboardFunc(keyboard);
        glutSpecialFunc(special_keyboard);
        glutMainLoop();
    }
#endif
    return make_stationary_option(infinite, criterium, decomposition, horizon);
}


int main(int argc, char *argv[]) {
    std::string Criterious;
    time_t time_1, time_2;
    time(&time_1);


    Glib::ustring filepath;
    Glib::RefPtr<mdp> my_mdp;
//std::ifstream input_file(argv[1]);
    if (argv[1] == NULL) {
        std::cerr << "\nUSE: MDP file [Options]\n\nOptions:\n\t -o\t enable output\n\t -g\t graphical representation\n\t -t\t mdp in textual format\n\n";
        exit(EXIT_FAILURE);
    }
    std::cout << "\n\n =========================================================\n";
    std::cout << "|	               MDP solver                         |\n";
    std::cout << " =========================================================\n";
    std::cout << "\n If you find any bug, send an email to beccuti@di.unito.it\n";
    std::cout << "\n =========================================================\n\n";

    if (argc > 2) {
        for (int i = 2; i < argc; i++) {
            switch (argv[i][1]) {
            case 'g':
            case 'G':
                graph = true;
                std::cout << "\n\n******************Graphical representation****************\n\n";
                break;
            case 'o':
                write_flag = true;
                break;
            case 'O':
                write_flag1 = true;
                break;
            case 't':
            case 'T':
                TMDP = true;
                break;
            case 'h':
            case 'H':
                if (argc >= (i + 1))
                    thorizon = atoi(argv[++i]);
                break;
            default:
                std::cerr << "\n\nUSE: MDP file [option]\n\t -o\t enable output\n\t -g\t graphical representation\n\t -t\t mdp in textual format\n\n";
                exit(EXIT_FAILURE);
                break;
            }
        }
    }

    if (write_flag) {
        std::string tmp = std::string(argv[1]) + ".otp";
        fp_write.open(tmp.c_str(), std::ofstream::out);
        tmp = std::string(argv[1]) + ".oar";
        fp_writeoar.open(tmp.c_str(), std::ofstream::out);
        if (!fp_write) {
            std::cerr << "\n\n the file " << argv[1] << ".otp is not created\n\n";
            exit(EXIT_FAILURE);
        }
        if (!fp_writeoar) {
            std::cerr << "\n\n the file " << argv[1] << ".oar is not createdt\n\n";
            exit(EXIT_FAILURE);
        }
    }
    if (write_flag1) {
        std::string tmp = std::string(argv[1]) + ".Otp";
        fp1_write.open(tmp.c_str(), std::ofstream::out);
        tmp = std::string(argv[1]) + ".Oar";
        fp1_writeoar.open(tmp.c_str(), std::ofstream::out);
        if (!fp1_write) {
            std::cerr << "\n\n the file " << argv[1] << ".Otp is not created\n\n";
            exit(EXIT_FAILURE);
        }
        if (!fp1_writeoar) {
            std::cerr << "\n\n the file " << argv[1] << ".Oar is not createdt\n\n";
            exit(EXIT_FAILURE);
        }
    }
//read transition
    std::ifstream fp_trans;
    std::string tmp1 = std::string(argv[1]) + ".trans";
    fp_trans.open(tmp1.c_str(), std::ifstream::in);
    if (!fp_trans) {
        std::cerr << "\n\n the file " << argv[1] << ".trans does not exist\n\n";
        exit(EXIT_FAILURE);
    }
    std::string trans_name;
    int i = 0;
    while (fp_trans) {
        getline(fp_trans, trans_name, '\n');
        vec_trans.push_back(trans_name);
        i++;
    }
//read transition

    if (!TMDP) {
        filepath = std::string(argv[1]) + ".xml";
        StationaryOption paramiter = read_data(filepath, argc, argv);
    }
    else {
        std::string tmp2 = std::string(argv[1]) + ".tmdp";
        std::ifstream in_mdp(tmp2.c_str(), std::ifstream::in);
        if (!in_mdp) {
            std::cerr << "\n\n the file " << argv[1] << ".tmpd does not exist\n\n";
            exit(EXIT_FAILURE);
        }
        read_data(in_mdp, argc, argv);
        in_mdp.close();
    }
    if (write_flag) {
        fp_write.close();
        fp_writeoar.close();
    }
    if (write_flag1) {
        fp1_write.close();
        fp1_writeoar.close();
    }
    fp_trans.close();
    time(&time_2);
    cout << "\n=============================== TIME ================================\n\n\t";
    cout << "Total time required: " << (time_2 - time_1) << "s";
    cout << "\n\n=============================== TIME ================================\n\n";
    return EXIT_SUCCESS;
}
