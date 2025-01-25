#ifndef __GLPK__
    #define __GLPK__
    #include <glpk.h>
#endif

#ifndef __IOS_H__
    #define __IOS_H__
    #include <iostream>
#endif

#ifndef __FSTREAM__
    #define __FSTREAM__
    #include <fstream>
#endif

#ifndef __UNORDERED_MAP__
    #define __UNORDERED_MAP__
    #include<map>
    #include <unordered_map>
#endif

#ifndef __GEN_H__
    #define __GEN_H__
    #include "general.h"
#endif

namespace FBGLPK {



    using namespace std;

    /**
     * @brief Frees the GLPK environment.
     * 
     * This function ensures that the GLPK environment is released only once. It should
     * be called at the end of the program or when GLPK resources are no longer needed.
     */
    inline void freeGLPKEnvironment() {
        static bool freed = false; // Tracks if the environment has already been freed
        if (!freed) {
            glp_free_env();
            freed = true;
            std::cout << "GLPK environment has been freed." << std::endl;
        } else {
            std::cout << "GLPK environment was already freed, skipping." << std::endl;
        }
    }
    
    
    /**
     * @brief Exception class for handling runtime errors within FBGLPK.
     */
    struct Exception {
        std::string mess; //!< Error message

        Exception() : mess("") {} //!< Default constructor
        Exception(std::string mess) : mess(mess) {} //!< Constructor with error message

        /**
         * @brief Retrieves the error message.
         * @return Error message as a string.
         */
        std::string what() const { return mess; }
    };
    

		struct SparseEntry {
				int row;
				int col;
				double val;
		};

    /**
     * @brief Class representing a linear programming problem using GLPK.
     * 
     * This class encapsulates functionalities for setting up and solving linear programming problems,
     * handling variability in solutions, and exporting results. It heavily relies on the GLPK library.
     */
    class LPprob {
    
    private:
    
        glp_prob *lp {nullptr}; //!< GLPK problem object
        int *ia {nullptr}; //!< Row indices for the LP matrix
        int *ja {nullptr}; //!< Column indices for the LP matrix
        double *ar {nullptr}; //!< Non-zero values of the LP matrix
        double *Value {nullptr}; //!< Solution values of the LP variables
        unsigned int sizeCol {0}; //!< Number of columns (variables)
        unsigned int sizeRow {0}; //!< Number of rows (constraints)
        unsigned int sizeVet {0}; //!< Total number of non-zero elements in the matrix
        bool solved {false}; //!< Flag indicating if the LP has been solved
        string filename {""}; //!< Filename of the LP problem
        unordered_map<string, unsigned int> ReactionsNamesId; //!< Mapping of reaction names to numeric IDs
        vector<string> ReactionsNamesOrd; //!< Ordered list of reaction names
        
        // Maps to keep track of the Gene / Non-gene associated reactions
				map<std::string, unsigned int> GeneAssocReactions;
				map<std::string, unsigned int> NonGeneAssocReactions;
				std::map<std::string, unsigned int> GeneAssocReactionsSplitted;
				std::map<std::string, unsigned int> NonGeneAssocReactionsSplitted;
				// 1) forwardReactions: reazioni _f (splitted)
				std::map<std::string,unsigned int> forwardReactions;
				// 2) reverseReactions: reazioni _r (splitted)
				std::map<std::string,unsigned int> reverseReactions;
				// 3) irreversibileReactions: reazioni non splitted
				std::map<std::string,unsigned int> irreversibileReactions;

				//std::unordered_map<string, unsigned int> InternalReactions; //!< Stores names and indices of internal reactions, excluding 'EX' and 'sink' reactions  //!< Stores names of internal reactions, excluding 'EX' and 'sink' reactions

        ifstream in_var; //!< Input stream for variability data
        ofstream out_var; //!< Output stream for variability results
        unsigned int flux_var; //!< Index of the flux variable considered for variability
        unsigned int pFBA_index; //!< Index of the objective flux to maximize.
        double gamma {1.0}; //!< Gamma value for variability
        
        // Biomass parameters
        double bioMax {-1}; // Maximum biomass, default to -1 if unspecified
        double bioMin {-1}; // Minimum biomass, default to -1 if unspecified
        double bioMean {-1}; // Average biomass, default to -1 if unspecified
        int pFbaFlag {-1};
        
        //! Helper function to set the type of bounds based on string input
        int setTypeBound(string typeString);

        //! Helper function to set the optimization direction (min/max) based on string input
        int setTypeObj(string typeString){
            int type;
            if (typeString.compare("GLP_MAX")==0)
                type=GLP_MAX;
            else
                type=GLP_MIN;
            return type;
        }
        
				// Per i bound delle colonne (reazioni):
				std::vector<std::string> tmpReactions;   // dimensione = sizeCol
				std::vector<int>         tmpBoundType;   // dimensione = sizeCol
				std::vector<double>      tmpLb;          // dimensione = sizeCol
				std::vector<double>      tmpUb;          // dimensione = sizeCol

				// Per i bound delle righe (vincoli)
				std::vector<int>         tmpRowBoundType; // dimensione = sizeRow
				std::vector<double>      tmpRowLb;        // dimensione = sizeRow
				std::vector<double>      tmpRowUb;        // dimensione = sizeRow
				std::vector<double> 		 tmpObjCoeff;
				std::vector<int> 				 mapFwd;
				std::vector<int> 			   mapRev;
				unsigned int 						 sizeColBeforeSplitting; 

				// Per la matrice sparsa
				std::vector<SparseEntry> tmpMatrix; // dimensione = sizeVet

				// Flag per evitare di richiamare più volte la procedura di splitting
				bool dataCollected = false;
				
				

        //! Parses flux names from input file
        void parseFluxNames(ifstream& in, general::Parser& parser, const char* delimC);
        
         //! Parses gene associated reactions and non-associated reactions
        void parseGeneAssocLine(ifstream& in, general::Parser& parser, const char* delimC);

        //! Parses model dimensions and type from input file
        void parseModelDimensionsAndType(ifstream& in, general::Parser& parser, const char* delimC, unsigned int& sizeRow, unsigned int& sizeCol, int& typeOBJ, int variability);

        //! Parses matrix size from input file
        void parseSizeVet(ifstream& in, general::Parser& parser, const char* delimC, unsigned int& sizeVet);
        
        //! Parses BioMax and BioMean values from input file
        void parseBiomassValues(ifstream& in, general::Parser& parser, const char* delimC);

        //! Parses objective coefficients from input file
        //void parseObjectiveCoefficients(ifstream& in, general::Parser& parser, const char* delimC, int setDefaultCoefficients, int variability, unsigned int flux_var, string* var_obj_eq);
        void parseObjectiveCoefficients(ifstream& in, general::Parser& parser, const char* delimC);
        //! Sets row bounds from input data
        void setRowBounds(ifstream& in, general::Parser& parser, const char* delimC);

        //! Sets column bounds from input data
        void setColumnBounds(ifstream& in, general::Parser& parser, const char* delimC);

        //! Sets the sparse matrix elements from input data
        void setSparseMatrix(ifstream& in, general::Parser& parser, const char* delimC);

        //! Initializes the LP problem from file
        void initializeLP(const char* fileProb);

        //! Allocates memory for matrix indices and values
        void allocateMemory(unsigned int sizeVet);

        //! Handles variability management based on input data
        void manageVariability(ifstream& in, general::Parser& parser, const char* delimC, int variability, const char* FluxName);

        //! Adds a row for handling variability in the objective
        void addVariabilityRow(const string& var_obj_eq, general::Parser& parser, const char* delimC, unsigned int& sizeVet);
        
				// =================
				// Metodo che creerà col/row su GLPK DOPO lo splitting
				// =================
				void finalizeLPAndSplit(const char* fileProb, int typeOBJ);
				void debugPrintGLPKProblem(const char* fileProb);
				void debugPrintMatrix();
    public:
    
    		// Default constructor
				LPprob() = default;

				// Delete the copy constructor and copy assignment operator
				LPprob(const LPprob&) = delete;
				LPprob& operator=(const LPprob&) = delete;

				// Move constructor
				LPprob(LPprob&& other) noexcept 
						: lp(other.lp), ia(other.ia), ja(other.ja), ar(other.ar), Value(other.Value),
							sizeCol(other.sizeCol), sizeRow(other.sizeRow), sizeVet(other.sizeVet),
							solved(other.solved), filename(std::move(other.filename)),
							ReactionsNamesId(std::move(other.ReactionsNamesId)), ReactionsNamesOrd(std::move(other.ReactionsNamesOrd)), /*InternalReactions(std::move(other.InternalReactions)),*/
							GeneAssocReactions(std::move(other.GeneAssocReactions)),
							NonGeneAssocReactions(std::move(other.NonGeneAssocReactions)),
							GeneAssocReactionsSplitted(std::move(other.GeneAssocReactionsSplitted)),
							NonGeneAssocReactionsSplitted(std::move(other.NonGeneAssocReactionsSplitted)),
							forwardReactions(std::move(other.forwardReactions)),
							reverseReactions(std::move(other.reverseReactions)),
							irreversibileReactions(std::move(other.irreversibileReactions)),							
							in_var(std::move(other.in_var)), out_var(std::move(other.out_var)),
							flux_var(other.flux_var), pFBA_index(other.pFBA_index), gamma(other.gamma),
							bioMax(other.bioMax), bioMin(other.bioMin), bioMean(other.bioMean), pFbaFlag(other.pFbaFlag) {

						// Nullify the moved-from object to prevent double freeing
						other.lp = nullptr;
						other.ia = nullptr;
						other.ja = nullptr;
						other.ar = nullptr;
						other.Value = nullptr;
				}

				// Move assignment operator
				LPprob& operator=(LPprob&& other) noexcept {
						if (this != &other) {
								//cout << "Move assignment called for: " << other.filename << endl;

								// Free existing resources
								if (lp) {
								    glp_delete_prob(lp);
								}
								free(ia);
								free(ja);
								free(ar);
								free(Value);

								// Transfer ownership of resources
								lp = other.lp;
								ia = other.ia;
								ja = other.ja;
								ar = other.ar;
								Value = other.Value;
								sizeCol = other.sizeCol;
								sizeRow = other.sizeRow;
								sizeVet = other.sizeVet;
								solved = other.solved;
								filename = std::move(other.filename);
								ReactionsNamesId = std::move(other.ReactionsNamesId);
								ReactionsNamesOrd = std::move(other.ReactionsNamesOrd);
			//				InternalReactions = std::move(other.InternalReactions);
								GeneAssocReactions = std::move(other.GeneAssocReactions);
								NonGeneAssocReactions = std::move(other.NonGeneAssocReactions);
								GeneAssocReactionsSplitted = std::move(other.GeneAssocReactionsSplitted);
								NonGeneAssocReactionsSplitted = std::move(other.NonGeneAssocReactionsSplitted);
								forwardReactions = std::move(other.forwardReactions);
								reverseReactions = std::move(other.reverseReactions);
								irreversibileReactions = std::move(other.irreversibileReactions);
								in_var = std::move(other.in_var);
								out_var = std::move(other.out_var);
								flux_var = other.flux_var;
								pFBA_index = other.pFBA_index;
								gamma = other.gamma;
								bioMax = other.bioMax;
								bioMin = other.bioMin;
								bioMean = other.bioMean;
								pFbaFlag = other.pFbaFlag;
								// Nullify other’s pointers
								other.lp = nullptr;
								other.ia = nullptr;
								other.ja = nullptr;
								other.ar = nullptr;
								other.Value = nullptr;
						}
						return *this;
				}

				// Additional constructors
				LPprob(const char * FileProb);
				LPprob(const char * FileProb, const char* FileInVar, const char* FileOutVar, int typeOBJ, const char* FluxName, int gamma);

				// Update method for LP problem
				void updateLP(const char * FileProb, int variability = 0, int typeOBJ = -1, const char* FluxName = "");
				    //! Solves the LP problem using simplex method
				void solve() {
						// Inizializza i parametri del metodo del simplesso
						glp_smcp param;
						glp_init_smcp(&param);

						// 1) Imposta il livello di messaggi (per debugging). 
						//    Può essere: GLP_MSG_OFF, GLP_MSG_ON, GLP_MSG_ERR, GLP_MSG_DBG, etc.
						param.msg_lev = GLP_MSG_OFF; 

						// 2) Attiva il presolve (opzionale, aiuta a ridurre il problema)
					  //param.presolve = GLP_ON;

						// 3) Scegli il metodo del simplesso: 
						//    - GLP_PRIMAL (default)
						//    - GLP_DUAL
						//    - GLP_DUALP (dual con partial pricing, a volte riduce il ciclaggio)
						//param.meth = GLP_DUAL; 

						// 4) Imposta eventualmente un limite sulle iterazioni (ad es. 1 milione)
						//    Se GLPK supera questa soglia, interrompe e restituisce un codice di stato.
						param.it_lim = 100000; // o un valore adeguato

						// Esegui il solver
						int status = glp_simplex(lp, &param);

						// Controlla se la soluzione è stata trovata
						if (status == 0) {
								solved = true;
						} else {
								solved = false;

								// Eventuale gestione degli errori
								if (status == GLP_EBADB) {
								    // ...
								} else if (status == GLP_ESING) {
								    // ...
								} else if (status == GLP_ECOND) {
								    // ...
								} else if (status == GLP_EBOUND) {
								    // ...
								} else {
								    // ...
								}
						}
				}




        //! Handles solution variability
        void solveVariability();

        //! Returns the objective function value
        inline double getOBJ(){
            if (!solved) solve();
            return glp_get_obj_val(lp);
        };
        
        inline int getPFBA_index(){
            return pFBA_index;
        };

        //! Returns the solution values for variables
        inline double* getVariables(){
            if (!solved) solve();
				   // std::cout << "Flussi delle Reazioni:" << std::endl;
					//	std::cout << "----------------------" << std::endl;

						for (unsigned int i = 1; i <= sizeCol; ++i){
								Value[i] = glp_get_col_prim(lp, i);
								/*
								if (i-1 < ReactionsNamesOrd.size()) {
										std::cout << ReactionsNamesOrd[i-1] << " : " << Value[i] << std::endl;
								} else {
										std::cout << "Reazione " << i << " : " << Value[i] << " (Nome non disponibile)" << std::endl;
								}*/
						}
						//std::cout << "----------------------" << std::endl;
						return Value;
        };
        
        //! Returns the lower bound for a specified variable
        inline double getLwBounds(int indexR){
						double LB = glp_get_col_lb(lp, indexR);
            return LB;
        };

        //! Returns the upper bound for a specified variable
        inline double getUpBounds(int indexR){
	    			double UB = glp_get_col_ub(lp, indexR);
            return UB;
        };

        //! Prints the last GLPK solution to standard output
        void print(){
            if (!solved) solve();
            cout<<"Obj value:"<< getOBJ()<<endl<<endl;
            getVariables();
           auto it=ReactionsNamesOrd.begin();
            for (unsigned int i=1;i<=sizeCol;++i,++it){
                cout<<*it<<":"<<Value[i]<<endl;

            }
        };

        //! Writes variable values to an output file stream
        inline void printValue(ofstream& out){
            getVariables();
            for (unsigned int i=1;i<=sizeCol;++i){
                out<<" "<<Value[i];
            }
        };

        //! Writes the objective value to an output file stream
        inline void printObject(ofstream& out){
            out<<" "<<getOBJ();
        }

        //! Writes the upper bounds of variables to an output file stream
        inline void printUpper(ofstream& out){
            for (unsigned int i=1;i<=sizeCol;++i){
                out<<" "<<glp_get_col_ub(lp, i);
            }

        }

        //! Writes the lower bounds of variables to an output file stream
        inline void printLower(ofstream& out){
            for (unsigned int i=1;i<=sizeCol;++i){
                out<<" "<<glp_get_col_lb(lp, i);
            }

        }

        //! Writes the bounds of variables to an output file stream
        inline void printLowerMax(ofstream& out){
            for (unsigned int i=1;i<=sizeCol;++i){
                out<<" "<<glp_get_col_lb(lp, i)<<" "<<glp_get_col_ub(lp, i);
            }

        }

        //! Writes the flux names to an output file stream
        inline void printFluxName(ofstream& out){
            for (auto it=ReactionsNamesOrd.begin();it!=ReactionsNamesOrd.end();++it){
                out<<" "<<*it;
            }
        };

        //! Writes flux names with "_Lb" and "_Ub" suffixes to an output file stream
        inline void printFluxNameMinMax(ofstream& out){
            for (auto it=ReactionsNamesOrd.begin();it!=ReactionsNamesOrd.end();++it){
                out<<" "<<*it<<"_Lb"<<" "<<*it<<"_Ub";
            }
        };
        
        vector<std::string> getReactions() const {
            return ReactionsNamesOrd;
        }

        //! Updates the bounds for a specified variable
        inline void update_bound(int indexR, string TypeBound, double Lb, double Ub){
            glp_set_col_bnds(lp, indexR, setTypeBound(TypeBound) , Lb, Ub);
           // cout<<"Bounds of "<< indexR <<" is updated as: ["<<Lb<<";"<<Ub<<"]"<<endl;
        };

        inline void update_bound(int indexR, int TypeBound, double Lb, double Ub){
            glp_set_col_bnds(lp, indexR, TypeBound , Lb, Ub);          
            //cout<<"Bounds of "<< indexR <<" is updated as: ["<<Lb<<";"<<Ub<<"]"<<endl;         
        };
        
				/**
				 * @brief Getter method to retrieve the filename of the LP problem.
				 * @return A string representing the filename.
				 */
				string getFilename() const {
				    return filename;
				}
				
				/**
				 * @brief Gets the maximum biomass value.
				 * @return Maximum biomass as a double.
				 */
				double getBioMax() const {
				    return bioMax;
				}

				/**
				 * @brief Gets the average biomass value.
				 * @return Average biomass as a double.
				 */
				double getBioMean() const {
				    return bioMean;
				}
				
				/**
				 * @brief Gets the minimum biomass value.
				 * @return Minimum biomass as a double.
				 */
				double getBioMin() const {
				    return bioMin;
				}
				
				inline int getPFbaFlag() const {
						return pFbaFlag;
				}
				
				// Getter per forwardReactions
				const std::map<std::string, unsigned int>& getForwardReactions() const {
				    return forwardReactions;
				}

				// Getter per reverseReactions
				const std::map<std::string, unsigned int>& getReverseReactions() const {
				    return reverseReactions;
				}

				// Getter per irreversibileReactions
				const std::map<std::string, unsigned int>& getIrreversibileReactions() const {
				    return irreversibileReactions;
				}
				
				std::string getFilenameWithoutExtension() const {
				    size_t lastDot = filename.rfind('.');  // Find the last occurrence of a dot in the filename
				    
				    // Check if the dot is the first character or not found
				    if (lastDot == 0 || lastDot == std::string::npos) {
				        return filename;  // Return the full filename if no valid dot is found
				    }
				    
				    // Return the filename up to the position of the last dot
				    return filename.substr(0, lastDot);
				}

        //!Returns the type of j-th column, i.e. the type of corresponding structural variable, as follows: GLP_FR — free (unbounded) variable; GLP_LO — variable with lower bound; GLP_UP — variable with upper bound; GLP_DB — double-bounded variable; GLP_FX — fixed variable.
        inline int  get_bound_type(int indexR){
            return glp_get_col_type(lp, indexR);
        }

        //! Converts a flux name to its numeric ID
        inline int fromNametoid(const string& name){
            auto it=ReactionsNamesId.find(name);
            if (it!=ReactionsNamesId.end())
                return it->second;
            else
                return -1;

        }

        //! Writes the solution to a specified file
				inline void writeSolutionToFile(const std::string& filename) {
						if (!solved) {
								throw Exception("LP problem not solved yet.");
						}

						std::ofstream outFile(filename);
						if (!outFile) {
								throw Exception("Could not open file for writing: " + filename);
						}

						// Write the objective value
						outFile << "Objective Value: " << getOBJ() << std::endl;

						// Fetch and write each variable's value
						getVariables();  // Ensure variables are updated
						outFile << "Variable Values:" << std::endl;
						for (unsigned int i = 1; i <= sizeCol; ++i) {
								outFile << ReactionsNamesOrd[i - 1] << ": " << Value[i] << std::endl;
						}

						outFile.close();
				}
				
				void printInitialFluxForReaction(const string& reactionName) {
					int reactionId = fromNametoid(reactionName);
						if(reactionId != -1){
							double* variables = getVariables();
							cout << "Initial flux value for " << reactionName << " : " << variables[reactionId] << endl;
						}else{
							cout << "Reaction " << reactionName << " not found in the problem." << endl;
						}
				}
				
				
				

				// Parsimonious Flux Balance Analysis (PFBA) Support Functions:
				
				// Method to set optimization mode to maximization
				void setMaximizeMode() {
				    glp_set_obj_dir(lp, GLP_MAX);
				//    cout << "Objective mode set to maximize." << endl;
				}

				// Method to set optimization mode to minimization
				void setMinimizeMode() {
				    glp_set_obj_dir(lp, GLP_MIN);
				//    cout << "Objective mode set to minimize." << endl;
				}
		  	
				/**
				 * @brief Sets the objective coefficients to minimize fluxes,
				 *        depending on the geneOption:
				 *        0 = penalizes all reactions,
				 *        1 = penalizes only gene-associated reactions,
				 *        2 = penalizes only non-gene-associated reactions.
				 *
				 * @param biomassIndex The index of the biomass reaction, which is excluded from penalization.
				 * @param geneOption   0, 1, or 2 as in the COBRA Toolbox.
				 */

				void setMinimizeFluxObjective(int biomassIndex) {
					//std::cout << "[DEBUG setMinimizeFluxObjective] geneOption = " << pFbaFlag 
					//	        << ", biomassIndex = " << biomassIndex << std::endl;

					// Azzera tutti i coefficienti
					int numCols = glp_get_num_cols(lp);
					for (int col = 1; col <= numCols; ++col) {
						  glp_set_obj_coef(lp, col, 0.0);
					}

					// Per contare quante colonne penalizziamo
					int countObjCol = 0;

					// Stampo per debug la dimensione delle map
					/*std::cout << "    [DEBUG] GeneAssocReactionsSplitted.size() = " 
						        << GeneAssocReactionsSplitted.size() << std::endl;
					std::cout << "    [DEBUG] NonGeneAssocReactionsSplitted.size() = " 
						        << NonGeneAssocReactionsSplitted.size() << std::endl;*/

					switch(pFbaFlag) {
						  case 0:
						     // std::cout << "    Minimizzo TUTTE le reazioni (tranne biomassa)!" << std::endl;
						      for (int c = 1; c <= numCols; c++) {
						          if (c != biomassIndex) {
						              glp_set_obj_coef(lp, c, 1.0);
						              countObjCol++;
						          }
						      }
						      break;

						  case 1:
						     // std::cout << "    Minimizzo SOLO reazioni gene-associated." << std::endl;
						      for (auto &kv : GeneAssocReactionsSplitted){
						          unsigned int col = kv.second;
						          if ((int)col != biomassIndex) {
						              glp_set_obj_coef(lp, col, 1.0);
						              countObjCol++;
						          }
						      }
						      break;

						  case 2:
						     // std::cout << "    Minimizzo SOLO reazioni NON-gene-associated." << std::endl;
						      for(auto &kv : NonGeneAssocReactionsSplitted){
						          unsigned int col = kv.second;
						          if((int)col != biomassIndex){
						            glp_set_obj_coef(lp, col, 1.0);
						          	countObjCol++;
						          }
						      }
						      break;

						  default:
						      //std::cout << "    geneOption non riconosciuto, fallback = 0 (TUTTO)" << std::endl;
						      for (int c = 1; c <= numCols; c++) {
						          if (c != biomassIndex) {
						              glp_set_obj_coef(lp, c, 1.0);
						              countObjCol++;
						          }
						      }
						      break;
					}

					// Debug: quante reazioni effettivamente penalizzate?
					//std::cout << "    [DEBUG] Reazioni (colonne) con coefficiente=1: " << countObjCol << std::endl;

					// Ora settiamo la minimizzazione
					glp_set_obj_dir(lp, GLP_MIN);
				//	std::cout << "    [DEBUG] Obiettivo impostato a MINIMIZE" << std::endl;
					
					int numCols2 = glp_get_num_cols(lp);
					//double pfbaObjective = glp_get_obj_val(lp);
					double sumFluxes = 0.0;
					for (int col = 1; col <= numCols2; ++col) {
							double fluxVal = glp_get_col_prim(lp, col);
							sumFluxes += fluxVal;
					}
					//std::cout << "[DEBUG pFBA] objective_value = " << pfbaObjective
					//					<< ", sum(|fluxes|) = " << sumFluxes << std::endl;

						/*std::cout << "[DEBUG pFBA SUPER SOMMA FLUSSI]"
								      << ",  sum(|fluxes|) = " << sumFluxes
								      << std::endl;*/
			}


					

				// Method to reset the biomass objective after pFBA
				void setBiomassObjective(int biomassIndex) {
				    // Reset all coefficients to zero
				    for (int i = 1; i <= glp_get_num_cols(lp); ++i) {
				        glp_set_obj_coef(lp, i, 0.0);
				    }
				    
				    // Set the coefficient for the biomass reaction to 1 to maximize it
				    glp_set_obj_coef(lp, biomassIndex, 1.0);
				 //   cout << "Objective set to maximize biomass for reaction index " << biomassIndex << "." << endl;
				}

				// Method to remove an equality constraint on a variable by setting it to unbounded
				void removeConstraint(int index) {
				    glp_set_col_bnds(lp, index, GLP_FR, 0.0, 0.0); // Set to free (unbounded) variable
				 //   cout << "Removed constraint on variable " << index << endl;
				}
    
				void resetMaximizationObjectiveForBiomass(int biomassIndex) {
						// Reset all objective coefficients to zero
						for (int i = 1; i <= glp_get_num_cols(lp); ++i) {
								glp_set_obj_coef(lp, i, 0.0);
						}

						// Set the coefficient for the biomass reaction back to 1 for maximization
						glp_set_obj_coef(lp, biomassIndex, 1.0);
						setMaximizeMode(); // Set the objective mode back to maximization
				}
		
				void removeConstraint(int index, int originalType, double originalLb, double originalUb) {
						// Set the bounds back to their original state, using the saved constraint type and bounds
						glp_set_col_bnds(lp, index, originalType, originalLb, originalUb);
				}
								
				inline void debugPFBA() {
						// Assicuriamoci che il problema sia stato risolto.
						if (!solved) {
								solve();
						}

						// 1) Leggiamo l'obiettivo restituito da GLPK (che, in pFBA, è la somma dei flussi penalizzati)
						double pfbaObjective = glp_get_obj_val(lp);

						// 2) Calcoliamo a mano la somma dei valori assoluti dei flussi
						int numCols = glp_get_num_cols(lp);
						double sumFluxes = 0.0;
						for (int col = 1; col <= numCols; ++col) {
								double fluxVal = glp_get_col_prim(lp, col);
								sumFluxes += fluxVal;
						}

						// 3) Stampiamo a video per confrontare
						std::cout << "[DEBUG pFBA] objective_value (GLPK) = " << pfbaObjective
								      << ",  sum(|fluxes|) = " << sumFluxes
								      << ",  difference = " << (pfbaObjective - sumFluxes)
								      << std::endl;
				}

				// Destructor:
				~LPprob() {
						//cout << "Destroying LPprob object for file: " << filename << endl;

						// Free dynamically allocated resources if they are not nullptr
						if (lp) {
								glp_delete_prob(lp);
								lp = nullptr;  // Prevent future issues
						}
						if (ia) {
								free(ia);
								ia = nullptr;
						}
						if (ja) {
								free(ja);
								ja = nullptr;
						}
						if (ar) {
								free(ar);
								ar = nullptr;
						}
						if (Value) {
								free(Value);
								Value = nullptr;
						}
				}
    };
}

