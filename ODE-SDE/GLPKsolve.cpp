/***************************************************************************
 *   Copyright (C) 2021 by Marco Beccuti				                   *
 *   marco.beccuti@unito.it						                           *
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

#ifndef __CGLPK_H__
	#define __CGLPK_H__
	#include "GLPKsolve.hpp"
#endif


namespace FBGLPK{


	/**
	 * @brief Parses flux names from an input stream and updates internal mappings.
	 * 
	 * This method reads a single line from the provided input stream, which is expected to contain flux names separated by a space.
	 * Each flux name is then stored in two internal structures: one mapping the names to their corresponding indices, and another 
	 * maintaining the order of these names as they appear in the input. If the line of flux names cannot be read or if no names are found,
	 * the method throws an exception.
	 *
	 * @param in Reference to an input file stream from which flux names are read.
	 * @param parser A parser object used to split the input line into names based on provided delimiters.
	 * @param delimC A C-style string containing delimiter characters that separate flux names in the input.
	 * 
	 * @throws Exception If the line of flux names cannot be read or if it contains no recognizable names.
	 * 
	 */
	void LPprob::parseFluxNames(std::ifstream& in, general::Parser& parser, const char* delimC) {
		  std::string buffer;
		  if (!std::getline(in, buffer)) {
		      throw Exception("FLUX BALANCE: Failed to read flux names line.");
		  }
		  parser.update(delimC, buffer);
		  if (parser.size() == 0) {
		      throw Exception("FLUX BALANCE: No flux names found.");
		  }
		  for (unsigned int i = 0; i < parser.size(); ++i) {
       	string reactionName = parser.get(i);
        // Store all reactions in the ordered list and ID map
        ReactionsNamesId[reactionName] = i + 1;
        ReactionsNamesOrd.push_back(reactionName);

        //if (reactionName.rfind("EX_", 0) != 0 && reactionName.rfind("sink_", 0) != 0) {
        //    InternalReactions[reactionName] = i + 1;  // Add to InternalReactions map
        //}
		  }
	}
	
	
	void LPprob::parseGeneAssocLine(std::ifstream& in, general::Parser& parser, const char* delimC){
		  std::string buffer;
		  // Prova a leggere la riga che contiene i flag di gene assoc (0/1)
		  if(!std::getline(in, buffer)){
		  	throw Exception("FLUX BALANCE: Failed to read gene-association line.");
		  }

		  // Esegui il parsing
		  parser.update(delimC, buffer);

		  // Deve avere la stessa dimensione di ReactionsNamesOrd
		  if(parser.size() != ReactionsNamesOrd.size()){
		  	throw Exception("FLUX BALANCE: gene-association flags do not match the number of reactions.");
		  }

		  // Scorri tutti i flag e assegna alle mappe
		  for(unsigned int i = 0; i < parser.size(); ++i){
		  	int flag = std::stoi(parser.get(i));  // 0 o 1
		    const std::string& rxnName = ReactionsNamesOrd[i];

		    if(flag == 1){
		    	// Se gene-associated
		      GeneAssocReactions[rxnName] = i+1;
		    }else{
		      // Se non gene-associated
		      NonGeneAssocReactions[rxnName] = i+1;
		    }
		  }
		  
		// Debug Gene/Non Gene Reactions:
		/*
    std::cout << "=== Gene-Associated Reactions ===" << std::endl;
    for (const auto& kv : GeneAssocReactions) {
        // kv.first  è il nome della reazione
        // kv.second è l’indice corrispondente (es. colonna in GLPK)
        std::cout << kv.first  << std::endl;
    }
    
    std::cout << "=== Non-Gene-Associated Reactions ===" << std::endl;
    for (const auto& kv : NonGeneAssocReactions) {
        std::cout <<  kv.first << std::endl;
    }
    */
	}



	/**
	 * @brief Parses the matrix dimensions and objective type from an input stream.
	 * 
	 * This method reads a line from the provided input stream expected to contain three parts: number of rows, number of columns,
	 * and a string representing the objective type of the optimization problem (GLP_MAX,). 
	 * It then updates the provided sizeRow and sizeCol parameters with these values. 
	 * If variability is not flagged (variability == 0), it also sets the typeOBJ parameter.
	 *
	 * @param in Reference to an input file stream from which the dimensions and type are read.
	 * @param parser A parser object used to split the input line into parts based on the provided delimiters.
	 * @param delimC A C-style string containing delimiter characters that separate the data items in the input.
	 * @param sizeRow Reference to an unsigned int where the number of rows will be stored.
	 * @param sizeCol Reference to an unsigned int where the number of columns will be stored.
	 * @param typeOBJ Reference to an integer where the objective type of the LP problem will be stored.
	 * @param variability An integer flag indicating whether the objective type should be set (0 if it should be set).
	 * 
	 * @throws Exception If the line cannot be read, if it does not contain exactly three parts, or if the parts cannot be
	 * correctly converted and interpreted.
	 *
	 */

	void LPprob::parseModelDimensionsAndType(std::ifstream& in, general::Parser& parser, const char* delimC, unsigned int& sizeRow, unsigned int& sizeCol, int& typeOBJ, int variability) {
		  std::string buffer;
		  if (!std::getline(in, buffer)) {
		      throw Exception("FLUX BALANCE: Failed to read dimensions and type line.");
		  }
		  parser.update(delimC, buffer);
		  if (parser.size() != 4) {
		      throw Exception("FLUX BALANCE: Incorrect format for dimensions and type line.");
		  }
		  sizeRow = std::stoul(parser.get(0));
		  sizeCol = std::stoul(parser.get(1));
		  if(!variability)
		  	typeOBJ = setTypeObj(parser.get(2));
		  	
		  pFbaFlag = std::stoul(parser.get(3));
	}


	/**
	 * @brief Parses and sets the objective coefficients for an LP problem from an input stream.
	 *
	 * This method reads a line of objective coefficients from the specified input stream, expecting them to be 
	 * separated by the given delimiters. It then checks if the number of coefficients matches the expected number of columns.
	 * If `setDefaultCoefficients` is true, it directly sets these coefficients in the LP problem. If `variability` is true,
	 * it instead sets a specific coefficient (related to `flux_var`) to 1.0, making the corresponding flux the focus of optimization.
	 *
	 * @param in Reference to the input stream from which coefficients are read.
	 * @param parser Parser object used to split the input string.
	 * @param delimC String of delimiter characters for parsing the input.
	 * @param setDefaultCoefficients Flag indicating if coefficients should be directly set.
	 * @param variability Flag indicating if the setup should adapt based on varying conditions.
	 * @param flux_var Index specifying which flux's coefficient to set to 1.0 under variability.
	 * @param var_obj_eq Optional pointer to a string where the raw input line is stored if variability is active.
	 *
	 * @throws Exception If unable to read the line or if the number of parsed coefficients does not match the expected number.
	 */
	 
	/* ORIGINAL
	void LPprob::parseObjectiveCoefficients(std::ifstream& in, general::Parser& parser, const char* delimC, 
		                                      int setDefaultCoefficients, int variability, 
		                                      unsigned int flux_var, std::string* var_obj_eq) {
		  std::string buffer;
		  if (!std::getline(in, buffer)) {
		      throw Exception("FLUX BALANCE: Failed to read objective coefficients.");
		  }
		  parser.update(delimC, buffer);
		  if (parser.size() != sizeCol) {
		      throw Exception("FLUX BALANCE: Incorrect number of objective coefficients.");
		  }
		  if (setDefaultCoefficients) {
		      for (unsigned int i = 0; i < parser.size(); ++i) {
		      		if(std::stof(parser.get(i)) == 1)
		      			pFBA_index = i+1;
		      			//cout << "Valore da ottimizzare --> " << ReactionsNamesOrd[i] << "index --> " <<  i << endl;
		      		
		          glp_set_obj_coef(lp, i + 1, std::stof(parser.get(i)));
		      }
		  } else if (variability) {
		      if (var_obj_eq != nullptr) {
		          *var_obj_eq = buffer;  // Modifica sicura solo se var_obj_eq non è nullptr
		      }
		      for (unsigned int i = 0; i < sizeCol; ++i) {
		          glp_set_obj_coef(lp, i + 1, (i + 1 == flux_var) ? 1.0 : 0.0);
		      }
		  }
	}
	
	*/

void LPprob::parseObjectiveCoefficients(std::ifstream& in, general::Parser& parser, const char* delimC) {
    std::string buffer;
    if (!std::getline(in, buffer)) {
        throw Exception("FLUX BALANCE: Failed to read objective coefficients.");
    }
    parser.update(delimC, buffer);
    if (parser.size() != sizeCol) {
        throw Exception("FLUX BALANCE: Incorrect number of objective coefficients.");
    }

    tmpObjCoeff.resize(sizeCol);

    for (unsigned int i = 0; i < sizeCol; ++i) {
        float val = std::stof(parser.get(i));
        tmpObjCoeff[i] = val;
        if (val == 1.0f) {
            pFBA_index = i+1; // 1-based
        }
    }
}


	/**
	 * @brief Sets the bounds for each row (constraint) in the linear programming problem.
	 *
	 * This method reads bounds for each row from an input stream, expecting each line to specify the bound type and limits for one row.
	 * The bounds are defined by three parts: the bound type, lower limit, and upper limit, separated by specified delimiters. 
	 * If the format of any line does not match the expected format (three parts), it throws an exception. 
	 * The bounds types are converted from string identifiers to GLPK-specific constants using the `setTypeBound` method.
	 *
	 * @param in Reference to the input file stream from which row bounds are read.
	 * @param parser Parser object used to split the input line into components.
	 * @param delimC String of delimiter characters for parsing the input.
	 *
	 * @throws Exception If any line does not contain exactly three parts or fails to meet format expectations.
	 *
	 */
	 
	/* ORIGINAL
	void LPprob::setRowBounds(std::ifstream& in, general::Parser& parser, const char* delimC) {
		  std::string buffer;
		  for (unsigned int i = 0; i < sizeRow && std::getline(in, buffer); ++i) {
		      parser.update(delimC, buffer);
		      if (parser.size() != 3) {
		          throw Exception("FLUX BALANCE: Incorrect row bounds format.");
		      }
		      glp_set_row_bnds(lp, i + 1, setTypeBound(parser.get(0)), std::atof(parser.get(1).c_str()), std::atof(parser.get(2).c_str()));
		  }
	}
	*/
	
	void LPprob::setRowBounds(std::ifstream& in, general::Parser& parser, const char* delimC) {
		  // Allochiamo i vector in base a sizeRow (già noto)
		  tmpRowBoundType.resize(sizeRow);
		  tmpRowLb.resize(sizeRow);
		  tmpRowUb.resize(sizeRow);

		  std::string buffer;
		  for (unsigned int i = 0; i < sizeRow && std::getline(in, buffer); ++i) {
		      parser.update(delimC, buffer);
		      if (parser.size() != 3) {
		          throw Exception("FLUX BALANCE: Incorrect row bounds format.");
		      }

		      tmpRowBoundType[i] = setTypeBound(parser.get(0));     
		      tmpRowLb[i]        = std::atof(parser.get(1).c_str());
		      tmpRowUb[i]        = std::atof(parser.get(2).c_str());
		  }
	}



	/**
	 * @brief Sets the bounds for each column (variable) in the linear programming problem.
	 *
	 * This method reads bounds for each column from an input stream, with each line expected to specify the bound type, 
	 * lower limit, and upper limit for one column. These parts should be separated by specified delimiters. The method processes 
	 * each line to set the bounds in the LP problem using GLPK functions. If a line does not match the expected format (three parts), 
	 * an exception is thrown. Special handling is implemented for GLP_DB type bounds: if both lower and upper bounds are zero, 
	 * the column is set as fixed (GLP_FX), meaning its value cannot vary.
	 *
	 * @param in Reference to the input file stream from which column bounds are read.
	 * @param parser Parser object used to split the input line into components.
	 * @param delimC String of delimiter characters for parsing the input.
	 *
	 * @throws Exception If any line does not contain exactly three parts or fails to meet format expectations.
	 *
	 */
	 
	/* ORIGINAL
	void LPprob::setColumnBounds(std::ifstream& in, general::Parser& parser, const char* delimC) {
		  std::string buffer;
		  for (unsigned int i = 0; i < sizeCol && std::getline(in, buffer); ++i) {
		      parser.update(delimC, buffer);
		      if (parser.size() != 3) {
		          throw Exception("FLUX BALANCE: Incorrect column bounds format.");
		      }
		      double lb = std::atof(parser.get(1).c_str());
		      double ub = std::atof(parser.get(2).c_str());
		      if (parser.get(0) == "GLP_DB" && lb == ub) {
		          glp_set_col_bnds(lp, i + 1, GLP_FX, lb, lb);
		      }else{
		          glp_set_col_bnds(lp, i + 1, setTypeBound(parser.get(0)), lb, ub);
		      }
		  }
	}*/
	
void LPprob::setColumnBounds(std::ifstream& in, general::Parser& parser, const char* delimC) {
    // Allochiamo i vector in base a sizeCol (già noto)
    tmpReactions.resize(sizeCol);   
    tmpBoundType.resize(sizeCol);
    tmpLb.resize(sizeCol);
    tmpUb.resize(sizeCol);

    std::string buffer;
    for (unsigned int i = 0; i < sizeCol && std::getline(in, buffer); ++i) {
        parser.update(delimC, buffer);
        if (parser.size() != 3) {
            throw Exception("FLUX BALANCE: Incorrect column bounds format.");
        }
        double lb = std::atof(parser.get(1).c_str());
        double ub = std::atof(parser.get(2).c_str());

        tmpBoundType[i] = setTypeBound(parser.get(0));
        tmpLb[i]        = lb;
        tmpUb[i]        = ub;

        // Copiamo il nome della reazione corrispondente (assumendo che ReactionsNamesOrd sia già pieno)
        tmpReactions[i] = ReactionsNamesOrd[i];
    }
}


	/**
	 * @brief Sets the sparse matrix coefficients for the linear programming problem.
	 *
	 * This method reads matrix entries from an input stream, with each line expected to specify the row index, 
	 * column index, and value of a non-zero element in the sparse matrix. These entries are used to construct the 
	 * matrix in the LP problem. If a line does not conform to the expected format (three components), an exception is thrown.
	 * This method populates three arrays: ia, ja, and ar, which represent the row indices, column indices, and values of the
	 * non-zero elements, respectively.
	 *
	 * @param in Reference to the input file stream from which matrix entries are read.
	 * @param parser Parser object used to split the input line into components.
	 * @param delimC String of delimiter characters for parsing the input.
	 *
	 * @throws Exception If any line does not contain exactly three parts or fails to meet format expectations.
	 *
	 */
	 
	/* ORIGINAL
	void LPprob::setSparseMatrix(std::ifstream& in, general::Parser& parser, const char* delimC) {
		  std::string buffer;
		  for (unsigned int i = 0; i < sizeVet && std::getline(in, buffer); ++i) {
		      parser.update(delimC, buffer);
		      if (parser.size() != 3) {
		          throw Exception("FLUX BALANCE: Incorrect matrix format.");
		      }
		      ia[i + 1] = std::atoi(parser.get(0).c_str());
		      ja[i + 1] = std::atoi(parser.get(1).c_str());
		      ar[i + 1] = std::atof(parser.get(2).c_str());
		  }
	}*/
	
		void LPprob::setSparseMatrix(std::ifstream& in, general::Parser& parser, const char* delimC) {
				// Puliamo il vettore e lo prepariamo a contenere sizeVet elementi
				tmpMatrix.clear();
				tmpMatrix.reserve(sizeVet);

				std::string buffer;
				for (unsigned int i = 0; i < sizeVet && std::getline(in, buffer); ++i) {
				    parser.update(delimC, buffer);
				    if (parser.size() != 3) {
				        throw Exception("FLUX BALANCE: Incorrect matrix format.");
				    }
				    SparseEntry se;
				    se.row = std::atoi(parser.get(0).c_str());
				    se.col = std::atoi(parser.get(1).c_str());
				    se.val = std::atof(parser.get(2).c_str());

				    tmpMatrix.push_back(se);
				}
		}

	
	/**
	 * @brief Parses the maximum and average biomass values directly from an input stream and stores them in class attributes.
	 * 
	 * This method reads two lines from the provided input stream, expected to contain the maximum and average biomass values.
	 * It converts these lines to double and directly updates the class attributes bioMax and bioMean.
	 *
	 * @param in Reference to an input file stream from which the biomass values are read.
	 * 
	 * @throws Exception If either of the biomass lines cannot be read or if the values are not properly formatted.
	 */
		void LPprob::parseBiomassValues(std::ifstream& in, general::Parser& parser, const char* delimC) {
				std::string buffer;

				// Parse the BioMax value
				if (!std::getline(in, buffer)) {
				    throw Exception("FLUX BALANCE: Failed to read BioMax value.");
				}
				parser.update(delimC, buffer);
				if (parser.size() < 1) {
				    throw Exception("FLUX BALANCE: No valid BioMax value found.");
				}
				try {
				    bioMax = std::stod(parser.get(0));
				} catch (const std::invalid_argument& ia) {
				    throw Exception("FLUX BALANCE: Invalid BioMax value format.");
				}

				// Parse the BioMean value
				if (!std::getline(in, buffer)) {
				    throw Exception("FLUX BALANCE: Failed to read BioMean value.");
				}
				parser.update(delimC, buffer);
				if (parser.size() < 1) {
				    throw Exception("FLUX BALANCE: No valid BioMean value found.");
				}
				try {
				    bioMean = std::stod(parser.get(0));
				} catch (const std::invalid_argument& ia) {
				    throw Exception("FLUX BALANCE: Invalid BioMean value format.");
				}
				
				// Parse the BioMean value
				if (!std::getline(in, buffer)) {
				    throw Exception("FLUX BALANCE: Failed to read BioMean value.");
				}
				parser.update(delimC, buffer);
				if (parser.size() < 1) {
				    throw Exception("FLUX BALANCE: No valid BioMean value found.");
				}
				try {
				    bioMin = std::stod(parser.get(0));
				} catch (const std::invalid_argument& ia) {
				    throw Exception("FLUX BALANCE: Invalid BioMean value format.");
				}
		}


	/**
	 * @brief Parses the count of non-zero elements (NZE) from an input stream.
	 * 
	 * This method reads a single line from the provided input stream which should contain the count of non-zero elements 
	 * in the sparse matrix. The count is then converted to an unsigned integer and stored in the sizeVet parameter.
	 *
	 * @param in Reference to an input file stream from which the NZE count is read.
	 * @param parser A parser object used to process the input line and extract the NZE count.
	 * @param delimC A C-style string containing delimiter characters that separate the data items in the input.
	 * @param sizeVet Reference to an unsigned int where the NZE count will be stored.
	 * 
	 * @throws Exception If the NZE count line cannot be read or if the count is not properly formatted.
	 * 
	 * Usage note: This function assumes the NZE count is the only item on the read line, correctly formatted as a numeric string.
	 */

	void LPprob::parseSizeVet(std::ifstream& in, general::Parser& parser, const char* delimC, unsigned int& sizeVet) {
		  std::string buffer;
		  if (!std::getline(in, buffer)) {
		      throw Exception("FLUX BALANCE: Failed to read non-zero elements count.");
		  }
		  parser.update(delimC, buffer);
		  sizeVet = std::stoul(parser.get(0));
	}

	/**
	 * @brief Initializes the linear programming (LP) problem with GLPK.
	 * 
	 * This function sets up a new LP problem instance using the GNU Linear Programming Kit (GLPK). It creates a new problem object
	 * and assigns a name to it.
	 *
	 * @param fileProb The name of the file, which will also be used as the name of the LP problem for identification.
	 * 
	 */
	void LPprob::initializeLP(const char* fileProb) {
		  lp = glp_create_prob();
		  glp_set_prob_name(lp, fileProb);
	}

	/**
	 * @brief Allocates memory for matrix indices and values based on the non-zero elements count.
	 * 
	 * This function dynamically allocates memory for arrays that will store the row indices (ia), column indices (ja),
	 * non-zero element values (ar), and additional values (Value), which are all used in constructing the sparse matrix
	 * for the linear programming problem.
	 * If any allocation fails, all allocated memory is freed, and a std::bad_alloc exception is thrown to signal the error.
	 *
	 * @param sizeVet The number of non-zero elements in the sparse matrix, used to determine the amount of memory to allocate.
	 * 
	 * @throws std::bad_alloc If memory allocation for any of the arrays fails.
	 */
	void LPprob::allocateMemory(unsigned int sizeVet){

		  ia = (int*)malloc(sizeof(int) * (sizeVet + 1));
		  ja = (int*)malloc(sizeof(int) * (sizeVet + 1));
		  ar = (double*)malloc(sizeof(double) * (sizeVet + 1));
		  Value = (double*)malloc(sizeof(double) * (sizeVet + 1));

		  if (!ia || !ja || !ar || !Value) {
		      free(ia); free(ja); free(ar); free(Value);  
		      throw std::bad_alloc(); 
		  }
	}


	/**
	 * @brief Manages the adjustment of objective coefficients based on variability and a specified flux name.
	 *
	 * This method adjusts the linear programming problem's focus based on the presence of variability and a specific flux name.
	 * If variability is enabled, it verifies whether the provided FluxName is a valid flux name present in the internal mapping
	 * of reaction names to indices (ReactionsNamesId). If the flux name is found, it sets the index of this flux as the target
	 * for variability adjustments in the optimization problem. If the flux name is not found, an exception is thrown indicating
	 * the invalid flux name.
	 *
	 * @param in Reference to the input file stream, used if additional information needs to be read (not used in the current implementation).
	 * @param parser A parser object potentially used for further input parsing (not used in the current implementation).
	 * @param delimC A C-style string containing delimiter characters; provided for potential parsing needs (not used in the current implementation).
	 * @param variability An integer indicating if variability adjustments are active (non-zero means active).
	 * @param FluxName A C-string representing the name of the flux to focus on if variability is active.
	 *
	 * @throws Exception If the flux name provided is not found in the internal mappings, indicating an invalid or unspecified flux.
	 *
	 */
	void LPprob::manageVariability(ifstream& in, general::Parser& parser, const char* delimC, int variability, const char* FluxName) {
		  if (variability) {
		      auto it = ReactionsNamesId.find(string(FluxName));
		      if (it == ReactionsNamesId.end())
		          throw Exception(string(FluxName) + " is not a valid flux name");
		      flux_var = it->second;
		  }
	}


	/**
	 * @brief Adds a new row to the sparse matrix to account for variability in the objective function.
	 *
	 * This method integrates additional objective coefficients into the sparse matrix as a new row, 
	 * representing variability adjustments specified in the optimization problem. It reads coefficients
	 * from a given string, checks their count against the expected number of columns, and appends them
	 * as a new row at the end of the existing matrix. This allows for dynamic adjustments to the model
	 * based on specific scenarios or requirements. If the number of coefficients does not match the number 
	 * of columns, an exception is thrown.
	 *
	 * @param var_obj_eq A string containing the coefficients for the variability row, separated by delimiters.
	 * @param parser A parser object used to split the variability coefficients string.
	 * @param delimC A C-style string containing delimiter characters that separate the coefficients.
	 * @param sizeVet Reference to an unsigned integer tracking the total number of non-zero elements in the matrix,
	 *                which will be updated to include the new row's elements.
	 *
	 * @throws Exception If the number of coefficients parsed does not match the expected number of columns.
	 *
	 */
	void LPprob::addVariabilityRow(const std::string& var_obj_eq, general::Parser& parser, const char* delimC, unsigned int& sizeVet) {
		  parser.update(delimC, var_obj_eq);

		  if (parser.size() != sizeCol) {
		      throw Exception("FLUX BALANCE: Incorrect number of coefficients for variability objective.");
		  }

		  int baseIndex = sizeVet + 1;  // Starting index for new entries in ia, ja, ar
		  for (unsigned int j = 0; j < parser.size(); ++j) {
		      int index = baseIndex + j;
		      ia[index] = sizeRow + 1;  // Index for the new variability row
		      ja[index] = j + 1;        // Column index
		      ar[index] = atof(parser.get(j).c_str());
		  }

		  sizeVet += parser.size();  // Update sizeVet to reflect added elements
	}

	/**
	 * @brief Constructs an LPprob object by setting up a linear programming problem.
	 * 
	 * This constructor initializes the linear programming problem by reading from a specified file.
	 * The file should contain all necessary details of the problem including flux names, matrix dimensions,
	 * objective coefficients, and bounds for rows and columns. It sets up the environment, parses and loads
	 * data into the GLPK solver for further operations like optimization. The process involves multiple steps:
	 * opening the file, initializing the problem, parsing the file contents, allocating memory, and setting up 
	 * the GLPK matrix with the provided data.
	 *
	 * @param fileProb Path to the file containing the problem specifications.
	 * 
	 * @exception std::exception Throws if the file cannot be opened, or if any parsing or initialization step fails.
	 * 
	 * Example usage:
	 * @code
	 *   LPprob lpProblem("path/to/problem/file.txt");
	 * @endcode
	 */
	LPprob::LPprob(const char* fileProb){
		  try{
		      ifstream in(fileProb, std::ifstream::in);
		      if (!in) {
		          throw Exception("FLUX BALANCE: error opening input file:" + string(fileProb));
		      }
		      initializeLP(fileProb);

		      general::Parser parser;
		      char delimC[] = "\t, ;\"";
		      int typeOBJ;

		      parseFluxNames(in, parser, delimC);
		      parseGeneAssocLine(in, parser, delimC);
		      parseModelDimensionsAndType(in, parser, delimC, sizeRow, sizeCol, typeOBJ, 0);
		      parseSizeVet(in, parser, delimC, sizeVet);
		      parseBiomassValues(in, parser, delimC);

		      allocateMemory(sizeVet);

		      glp_set_obj_dir(lp, typeOBJ);
		      glp_add_rows(lp, sizeRow);
		      glp_add_cols(lp, sizeCol);

		      parseObjectiveCoefficients(in, parser, delimC);//, 1, 0, 0, nullptr);
		      setRowBounds(in, parser, delimC);
		      setColumnBounds(in, parser, delimC);
		      setSparseMatrix(in, parser, delimC);
		      
		      glp_load_matrix(lp, sizeVet, ia, ja, ar);

		  }catch(const std::exception& e){
		      std::cerr << "Exception: " << e.what() << std::endl;
		      exit(EXIT_FAILURE);
		  }
	}
	
	
	// TODO: Adapt for the splitted problem version
	LPprob::LPprob(const char* FileProb, const char* FileInVar, const char* FileOutVar, int typeOBJ, const char* FluxName, const int gamma) {
		  // Creating LP problem
			cout << "init problem in update --->  " << FileProb << endl;
		  updateLP(FileProb, 1, typeOBJ, FluxName);
		  try {
		      // Opening input file
		      in_var.open(FileInVar, std::ifstream::in);
		      if (!in_var)
		          throw Exception("FLUX BALANCE: error opening input file:" + string(FileInVar));
		      if (in_var.eof())
		          throw Exception("FLUX BALANCE: error input file:" + string(FileInVar) + " is empty");

		      this->gamma = gamma;

		      string buffer;
		      getline(in_var, buffer);
		      // Reading flux names
		      // cout << buffer << endl

		      // Opening output file
		      out_var.open(FileOutVar, std::ofstream::out);
		      if (!out_var)
		          throw Exception("FLUX BALANCE: error opening output file:" + string(FileOutVar));
		      out_var.precision(16);
		  } catch (exception& e) {
		      cout << "\nException: " << e.what() << endl;
		      exit(EXIT_FAILURE);
		  } catch (Exception& e) {
		      cout << "\nException: " << e.what() << endl;
		      exit(EXIT_FAILURE);
		  }
	}

 


	/**
	 * @brief Updates or initializes a linear programming problem based on file input, with optional variability adjustments.
	 *
	 * This method configures or updates the settings of an LP problem by reading specifications from a given file.
	 * It involves setting flux names, problem dimensions, objective coefficients, and bounds. The process is sensitive to
	 * the `variability` parameter, which dictates whether additional adjustments are made to cater to dynamic aspects of the
	 * problem setup, such as changing objective coefficients or constraints based on a specific flux name.
	 *
	 * If variability is enabled, an additional row may be added, and specific objective coefficients are adjusted to focus on a designated flux variable.
	 *
	 * @param fileProb The file path from which to read the LP problem settings.
	 * @param variability An integer flag indicating whether to apply variability-specific settings (1 if true).
	 * @param typeOBJ The objective type (maximization or minimization) of the LP problem.
	 * @param FluxName The name of the flux variable to focus on if variability is enabled.
	 *
	 * @throws Exception If the file cannot be opened or parsed correctly, or if any setup step fails due to format issues or
	 *                   logical errors in configuration.
	 *
	 * Usage example:
	 * @code
	 *   LPprob lp;
	 *   lp.updateLP("path/to/problem.txt", 1, GLP_MAX, "targetFlux");
	 * @endcode
	 */
	 
	/* Original
	void LPprob::updateLP(const char* fileProb, int variability, int typeOBJ, const char* FluxName) {
		  try {
		      string var_obj_eq = "";
		      ifstream in(fileProb, std::ifstream::in);
		      if (!in) {
		          throw Exception("FLUX BALANCE: error opening input file:" + string(fileProb));
		      }

		      initializeLP(fileProb);
		      general::Parser parser;
		      char delimC[] = "\t, ;\"";

		      parseFluxNames(in, parser, delimC);
		      parseGeneAssocLine(in, parser, delimC);		      
		      parseModelDimensionsAndType(in, parser, delimC, sizeRow, sizeCol, typeOBJ, variability);
		      parseSizeVet(in, parser, delimC, sizeVet);
		     	parseBiomassValues(in, parser, delimC);

		      if (variability) {
		          manageVariability(in, parser, delimC, variability, FluxName);
		      }

		      allocateMemory(sizeVet + (variability * sizeCol));
		      glp_set_obj_dir(lp, typeOBJ);
		      glp_add_rows(lp, sizeRow + variability);
		      glp_add_cols(lp, sizeCol);
		      parseObjectiveCoefficients(in, parser, delimC, !variability, variability, flux_var, &var_obj_eq);

		      setRowBounds(in, parser, delimC);
		      setColumnBounds(in, parser, delimC);
		      setSparseMatrix(in, parser, delimC);

		      if (variability) {
		          addVariabilityRow(var_obj_eq, parser, delimC, sizeVet);
		      }

		      glp_load_matrix(lp, sizeVet, ia, ja, ar);

		      filename = string(fileProb);
		      
		  } catch (exception& e) {
		      cout << "Exception: " << e.what() << endl;
		      exit(EXIT_FAILURE);
		  } catch (Exception& e) {
		      cout << "Exception: " << e.what() << endl;
		      exit(EXIT_FAILURE);
		  }
	}*/

	void LPprob::updateLP(const char* fileProb, int variability, int typeOBJ, const char* FluxName) {
		  try {
		      string var_obj_eq;
		      ifstream in(fileProb, std::ifstream::in);
		      if (!in) {
		          throw Exception("FLUX BALANCE: error opening input file:" + string(fileProb));
		      }

				 // If not split:
		     // initializeLP(fileProb);

		      general::Parser parser;
		      char delimC[] = "\t, ;\"";

		      // parse
		      parseFluxNames(in, parser, delimC);
		      parseGeneAssocLine(in, parser, delimC);		      
		      parseModelDimensionsAndType(in, parser, delimC, sizeRow, sizeCol, typeOBJ, variability);
		      parseSizeVet(in, parser, delimC, sizeVet);
		      parseBiomassValues(in, parser, delimC);

		      if (variability) {
		          manageVariability(in, parser, delimC, variability, FluxName);
		      }

					// Variability (TODO)
		      parseObjectiveCoefficients(in, parser, delimC);

		      setRowBounds(in, parser, delimC);
		      setColumnBounds(in, parser, delimC);
		      setSparseMatrix(in, parser, delimC);

					// Variability (TODO)
		      if (variability) {
		          // addVariabilityRow(...);
		      }

		      filename = string(fileProb);
		      finalizeLPAndSplit(fileProb, typeOBJ);
		  } 
		  catch (exception& e) {
		      cout << "Exception: " << e.what() << endl;
		      exit(EXIT_FAILURE);
		  } 
		  catch (Exception& e) {
		      cout << "Exception: " << e.what() << endl;
		      exit(EXIT_FAILURE);
		  }
	}



		void LPprob::solveVariability(){
				string buffer;
				out_var<<"Time Obj"<<endl;
				try{
				    while (!in_var.eof()){
				    getline(in_var,buffer); 
				    class general::Parser par(" ",buffer);
				    //cout<<"tot:"<<par.size()<<" sizeCol"<<sizeCol<<endl;
				    if (par.size()!=0){
				     for (unsigned int i=sizeCol+2,j=1; i<par.size();i=i+2,++j){ //+2 is due to time and obj
				        //updating bound
				        update_bound(j,get_bound_type(j),atof(par.get(i).c_str()),atof(par.get(i+1).c_str()));
				        }
				     //updating new equation based on old obj
				     glp_set_row_bnds(lp,sizeRow+1, GLP_LO, atof(par.get(0).c_str())*gamma, atof(par.get(0).c_str())*gamma);
				     //glp_set_col_bnds(lp,sizeRow+1, GLP_LO, atof(par.get(0).c_str())*gamma, atof(par.get(0).c_str())*gamma);
				     solve();
				     out_var<<par.get(0)<<" "<< glp_get_obj_val(lp)<<endl;   
				    }
				    }
				 }
		 catch (exception& e){
				cout << "\n Exception: " << e.what() << endl;
				exit(EXIT_FAILURE);
				}
		 catch (Exception& e){
				cout << "\n Exception: " << e.what() << endl;
				exit(EXIT_FAILURE);
				}       
		}

		int LPprob::setTypeBound(string typeString){
				int type;
				if (typeString.compare("GLP_FR")==0)
				    type=GLP_FR;
				else  if (typeString.compare("GLP_LO")==0)
				    type=GLP_LO;
				else  if (typeString.compare("GLP_UP")==0)
				    type=GLP_UP;
				else  if (typeString.compare("GLP_DB")==0)
				    type=GLP_DB;
				else
				    type=GLP_FX;
				return type;
		}
		
		void LPprob::finalizeLPAndSplit(const char* fileProb,  int typeOBJ)
	{
		  // Evita di rieseguire due volte
		  if (dataCollected) {
		      std::cerr << "[finalizeLPAndSplit] data are already collected. Skipping.\n";
		      return;
		  }
		  dataCollected = true;
		  sizeColBeforeSplitting = sizeCol;
		  // =========================
		  // 1) Prepara i vettori “finali”
		  // =========================
		  std::vector<std::string> newReactions;
		  std::vector<int>         newBoundType;
		  std::vector<double>      newLb, newUb;

		  // Mappature per capire come la colonna i-esima (old) diventa 1 o 2 colonne (new)
		  std::vector<int> mapFwd(sizeCol, -1);
		  std::vector<int> mapRev(sizeCol, -1);

		  int newIndexCount = 0; // contatore per le "nuove" colonne
		  int forwardCount = 0;  // Contatore per reazioni forward
		  int reverseCount = 0;  // Contatore per reazioni reverse

		  // Splitting colonna per colonna
		  for(unsigned int c = 0; c < sizeCol; c++)
		  {
		      double LB = tmpLb[c];
		      double UB = tmpUb[c];

		      // Se la reazione c ha lb < 0 < ub => splitted
		      if(LB < 0.0 && UB > 0.0) {
		          // Forward
		          {
		              std::string fwdName = tmpReactions[c] + "_f";
		              newReactions.push_back(fwdName);
		              newBoundType.push_back(GLP_DB); // double-bounded => [0, UB]
		              newLb.push_back(0.0);
		              newUb.push_back(UB);

		              mapFwd[c] = newIndexCount;
		              newIndexCount++;
		             	forwardCount++;  // Incrementa forward
		          }
		          // Reverse
		          {
		              std::string revName = tmpReactions[c] + "_r";
		              newReactions.push_back(revName);
		              newBoundType.push_back(GLP_DB); // [0, -LB]
		              newLb.push_back(0.0);
		              newUb.push_back(-LB);

		              mapRev[c] = newIndexCount;
		              newIndexCount++;
		              reverseCount++;  // Incrementa forward
		          }
		      }
		      else {
		          // Non splitted => 1 colonna invariata
		          newReactions.push_back(tmpReactions[c]);
		          newBoundType.push_back(tmpBoundType[c]);
		          newLb.push_back(LB);
		          newUb.push_back(UB);

		          mapFwd[c] = newIndexCount;
		          mapRev[c] = -1;
		          newIndexCount++;
		      }
		  }
		  
		 // std::cout << "[finalizeLPAndSplit: " << fileProb << " ]" << "Reazioni aggiunte come forward (_f): " << forwardCount << std::endl;
		 // std::cout << "[finalizeLPAndSplit " << fileProb << " ]" << " Reazioni aggiunte come reverse (_r): " << reverseCount << std::endl;

		  unsigned int finalCols = (unsigned int)newIndexCount;

		  // =========================
		  // 2) Ricostruire la matrice sparsa
		  // =========================
		  std::vector<SparseEntry> newMatrix;
		  newMatrix.reserve(tmpMatrix.size() * 2);

		  for(const auto& e : tmpMatrix) {
		      int oldCol   = e.col - 1; // 1-based -> 0-based
		      int oldRow   = e.row;     // la riga possiamo lasciarla 1-based
		      double coeff = e.val;

		      if(mapRev[oldCol] >= 0) {
		          // splitted => abbiamo col forward e col reverse
		          int cF = mapFwd[oldCol]; 
		          int cR = mapRev[oldCol]; 

		          // parte forward: coeff invariato
		          SparseEntry eF;
		          eF.row = oldRow;    
		          eF.col = cF + 1;    // back to 1-based
		          eF.val = coeff;
		          newMatrix.push_back(eF);

		          // parte reverse: coeff con segno invertito
		          SparseEntry eR;
		          eR.row = oldRow;
		          eR.col = cR + 1;
		          eR.val = -coeff;
		          newMatrix.push_back(eR);
		      }
		      else {
		          // non splitted
		          int cN = mapFwd[oldCol];
		          SparseEntry eN;
		          eN.row = oldRow;
		          eN.col = cN + 1;
		          eN.val = coeff;
		          newMatrix.push_back(eN);
		      }
		  }

		  unsigned int finalNonZeros = (unsigned int)newMatrix.size();

		  // =========================
		  // 3) Costruisci davvero l'oggetto GLPK
		  // =========================
		  // Se c’era già qualcosa, pulisci
		  if (lp) {
		      glp_delete_prob(lp);
		      lp = nullptr;
		  }
		  lp = glp_create_prob();
		  glp_set_prob_name(lp, fileProb);
		  glp_set_obj_dir(lp, typeOBJ); 
		  
		  // 3a) Crea le righe
		  glp_add_rows(lp, sizeRow);
		  for(unsigned int r = 0; r < sizeRow; r++) {
		      glp_set_row_bnds(lp, r+1,
		                       tmpRowBoundType[r],
		                       tmpRowLb[r],
		                       tmpRowUb[r]);
		  }

		  // 3b) Crea le colonne (splittate o no)
		  glp_add_cols(lp, finalCols);

		  // Svuotiamo i vecchi ReactionsNamesOrd/Id e li rigeneriamo
		  ReactionsNamesOrd.clear();
		  ReactionsNamesId.clear();

		  for(unsigned int c = 0; c < finalCols; c++) {
		      glp_set_col_bnds(lp, c+1,
		                       newBoundType[c],
		                       newLb[c],
		                       newUb[c]);

		      // Salviamo i nuovi nomi
		      std::string rxnName = newReactions[c];
		      ReactionsNamesOrd.push_back(newReactions[c]);
		      ReactionsNamesId[newReactions[c]] = c+1;
		      
		      if(rxnName.size() >= 2 && rxnName.substr(rxnName.size()-2) == "_f") {
		          forwardReactions[rxnName] = c+1;
		      }
		      else if(rxnName.size() >= 2 && rxnName.substr(rxnName.size()-2) == "_r") {
		          reverseReactions[rxnName] = c+1;
		      }
		      else {
		          irreversibileReactions[rxnName] = c+1;
		      }
		  }

		  // 3c) Prepara ia,ja,ar e carica la matrice
		  free(ia);  free(ja);  free(ar);  free(Value);

		  sizeCol = finalCols;
		  sizeVet = finalNonZeros;

		  ia    = (int*)malloc(sizeof(int)*(sizeVet+1));
		  ja    = (int*)malloc(sizeof(int)*(sizeVet+1));
		  ar    = (double*)malloc(sizeof(double)*(sizeVet+1));
		  Value = (double*)malloc(sizeof(double)*(sizeVet+1));
		  if(!ia || !ja || !ar || !Value) {
		      throw std::bad_alloc();
		  }

		  for(unsigned int i = 0; i < sizeVet; i++) {
		      ia[i+1] = newMatrix[i].row;  // 1-based
		      ja[i+1] = newMatrix[i].col;  // 1-based
		      ar[i+1] = newMatrix[i].val;
		  }

		  glp_load_matrix(lp, sizeVet, ia, ja, ar);
		  
		   for(unsigned int oldC = 0; oldC < sizeColBeforeSplitting; oldC++) {
		      double cObj = tmpObjCoeff[oldC]; // coeff letto dal file (o 0 se non c’era)

		      if (mapRev[oldC] >= 0) {
		          // splitted => forward e reverse
		          int newF = mapFwd[oldC]; // colonna forward (0-based)
		          int newR = mapRev[oldC]; // colonna reverse (0-based)
		          // esempio: assegno lo stesso coeff a entrambe
		          glp_set_obj_coef(lp, newF+1, cObj);
		          glp_set_obj_coef(lp, newR+1, cObj);
		      } else {
		          // non splitted
		          int newN = mapFwd[oldC];
		          glp_set_obj_coef(lp, newN+1, cObj);
		      }
		  }
		  

			for (unsigned int i=0; i < ReactionsNamesOrd.size(); i++) {
					std::string splittedName = ReactionsNamesOrd[i]; // es. "R1_f" o "R2"
					unsigned int colIdx = i+1;

					// Estrai la “baseName”
					std::string baseName = splittedName;
					if (baseName.size()>=2) {
						 if (baseName.substr(baseName.size()-2)=="_f" || baseName.substr(baseName.size()-2)=="_r") {
						    baseName = baseName.substr(0, baseName.size()-2);
						 }
					}
					// Check gene
					bool wasGene = (GeneAssocReactions.find(baseName) != GeneAssocReactions.end());
					if (wasGene) {
						  GeneAssocReactionsSplitted[splittedName] = colIdx;
					} else {
						  NonGeneAssocReactionsSplitted[splittedName] = colIdx;
					}
			}
			
		  std::string oldObjName;
		  if (pFBA_index > 0 && pFBA_index <= tmpReactions.size()) {
		      oldObjName = tmpReactions[pFBA_index-1]; 
		  }

		  // Una volta popolato ReactionsNamesOrd col new set:
		  // Re-inizializzo pFBA_index = 0
		  pFBA_index = 0;

		  // Ora cerco se la reazione è splitted
		  //   - se era irr, trovo directly oldObjName
		  //   - se era rev, trovo oldObjName + "_f" (ad es.)
		  // E setto pFBA_index
		  for (unsigned int c=0; c<ReactionsNamesOrd.size(); c++){
		      std::string rName = ReactionsNamesOrd[c];
		      // Caso tipico, biomassa irreversibile
		      if (rName == oldObjName){
		          pFBA_index = c+1;
		          break;
		      }
		      // Caso splitted
		      else if (rName == oldObjName + "_f"){
		          pFBA_index = c+1;
		          break;
		      }
		  }
			/*
			// Debug: stampa i contenuti di GeneAssocReactions
			std::cout << "\n[DEBUG] GeneAssocReactions contents:" << std::endl;
			for (const auto& pair : GeneAssocReactions) {
					std::cout << "  " << pair.first << " => " << pair.second << std::endl;
			}
			
			// Debug: stampa i contenuti di GeneAssocReactions
			std::cout << "\n[DEBUG] NonGeneAssocReactions contents:" << std::endl;
			for (const auto& pair : NonGeneAssocReactions) {
					std::cout << "  " << pair.first << " => " << pair.second << std::endl;
			}

			// Debug: stampa i contenuti di GeneAssocReactionsSplitted
			std::cout << "\n[DEBUG] GeneAssocReactionsSplitted contents:" << std::endl;
			for (const auto& pair : GeneAssocReactionsSplitted) {
					std::cout << "  " << pair.first << " => " << pair.second << std::endl;
			}

			// Debug: stampa i contenuti di NonGeneAssocReactionsSplitted
			std::cout << "\n[DEBUG] NonGeneAssocReactionsSplitted contents:" << std::endl;
			for (const auto& pair : NonGeneAssocReactionsSplitted) {
					std::cout << "  " << pair.first << " => " << pair.second << std::endl;
			}

			std::cout << "[finalizeLPAndSplit] oldObjName = " << oldObjName 
						    << " => new pFBA_index = " << pFBA_index << std::endl;*/
			//debugPrintGLPKProblem(fileProb);
	}

	void LPprob::debugPrintGLPKProblem(const char* fileProb)
	{
		  std::cout << "\n=== DEBUG: GLPK Problem " << fileProb << "  ===" << std::endl;
		  int nrows = glp_get_num_rows(lp);
		  int ncols = glp_get_num_cols(lp);
		  std::cout << "Rows: " << nrows << "   Cols: " << ncols << std::endl;

		  // Stampa bounds delle righe
		  for (int r = 1; r <= nrows; r++) {
		      int type = glp_get_row_type(lp, r);
		      double lb = glp_get_row_lb(lp, r);
		      double ub = glp_get_row_ub(lp, r);
		      std::cout << "Row " << r << " => type=" << type
		                << "  LB=" << lb << "  UB=" << ub << std::endl;
		  }

		  // Stampa bounds delle colonne
		  for (int c = 1; c <= ncols; c++) {
		      int type = glp_get_col_type(lp, c);
		      double lb = glp_get_col_lb(lp, c);
		      double ub = glp_get_col_ub(lp, c);

		      // Se hai le reazioni in ReactionsNamesOrd (c-1)
		      std::string rxnName = ReactionsNamesOrd[c-1]; 
		      std::cout << "Col " << c << " (" << rxnName << ") => type=" << type
		                << "  LB=" << lb << "  UB=" << ub << std::endl;
		  }

		  // Stampa la matrice
		  debugPrintMatrix();

		  std::cout << "====================================\n" << std::endl;
	}

	void LPprob::debugPrintMatrix() 
	{
		  std::cout << "\n=== DEBUG: GLPK Matrix (row by row) ===\n";

		  int nrows = glp_get_num_rows(lp);
		  int ncols = glp_get_num_cols(lp);

		  // GLPK richiede array 1-based per indici e valori
		  // (ind[0] e val[0] non vengono usati).
		  std::vector<int>    ind(ncols + 1);
		  std::vector<double> val(ncols + 1);

		  // Per ogni riga
		  for (int r = 1; r <= nrows; ++r)
		  {
		      // glp_get_mat_row restituisce quanti non-zero ci sono in questa riga
		      // e scrive in ind[] e val[] le colonne e i rispettivi coefficienti
		      int len = glp_get_mat_row(lp, r, ind.data(), val.data());

		      // Stampa solo se ci sono elementi non-zero
		      std::cout << "Row " << r << " has " << len << " non-zero(s):\n";
		      for (int i = 1; i <= len; ++i)
		      {
		          int colIndex    = ind[i];
		          double coeff    = val[i];
		          // Se vuoi, puoi anche stampare il nome della colonna: ReactionsNamesOrd[colIndex-1] 
		          std::string colName = ReactionsNamesOrd[colIndex - 1];
		          
		          std::cout << "   col " << colIndex 
		                    << " (" << colName << "), val = " << coeff << "\n";
		      }
		  }

		  std::cout << "====================================\n" << std::endl;
	}



}
