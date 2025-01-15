package editor.domain.io;

import editor.domain.Node;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Transition;
import editor.domain.grammar.ExpressionLanguage;
import editor.domain.grammar.ParserContext;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Set;
import java.util.StringJoiner;

/**
 *
 * @author Irene 
 * Write che c++ file with the expressions of the transitions
 */
 
public class CppFormat {

    public static void export(File file, GspnPage gspn, boolean fluxBalance, ParserContext context, Set<String> filenames, Set<String> rnames, String baseName) throws Exception {
        try (PrintWriter out = new PrintWriter(new BufferedOutputStream(new FileOutputStream(file)))) {
        
            if (!filenames.isEmpty()) {
                StringJoiner joiner = new StringJoiner(",");
                int i = 0;
                for (String name : filenames) {
                    out.print("#define ");
                    String defineString = name.substring(1, name.length() - 1).replace('.', '_');
                    out.println(defineString + " " + i);
                    joiner.add(name);
                    i++;
                }
                out.println("vector<string> name_file = {" + joiner + "};");
                out.println("vector<Table> class_files(" + Integer.toString(filenames.size()) + ", Table());\n");
            } else {
                out.println("vector<string> name_file = {};");
            }

            if(fluxBalance){
							if (baseName.endsWith("_unf")) {
									baseName = baseName.substring(0, baseName.length() - "_unf".length());
							}
							out.println("#define FBA_INFO_FILE \"./" + baseName + ".fbainfo\"");
              out.println("\n\n/**FBAProcessor Class Definition**/\n\n");
           		out.println(generateFBAClass());
           	 	out.println("\n\n/**General Transitions function definitions**/\n\n");
           	}
           		
            for (Node node : gspn.nodes) {
                if (node instanceof Transition) {
                    Transition trn = (Transition) node;
                    ArrayList<String> double_constant_log = new ArrayList<>();
                    String cppDelayExpr = trn.convertDelayLang(context, null, ExpressionLanguage.CPP);
                    GreatSpnFormat.realOrRpar(cppDelayExpr, "", gspn, double_constant_log);
                    if (!double_constant_log.isEmpty()) {
                        out.println(generateFunction(trn, cppDelayExpr, fluxBalance));
                    }
                }
            }
            
        }
    }

    private static String generateFunction(Transition trn, String cppDelayExpr, boolean fluxBalance) {
        StringBuilder sb = new StringBuilder();
        sb.append("double ").append(trn.getUniqueName()).append("_general(double *Value, ");
        if (fluxBalance) {
            sb.append("vector<class FBGLPK::LPprob>& vec_fluxb, ");
        }
        sb.append("map <string,int>& NumTrans, ");
        sb.append("map <string,int>& NumPlaces, ");
        sb.append("const vector<string> & NameTrans, ");
        sb.append("const struct InfTr* Trans, ");
        sb.append("const int T, ");
        sb.append("const double& time) {\n");
        sb.append("    double rate = ").append(cppDelayExpr).append(";\n");
        sb.append("    return (rate);\n");
        sb.append("}\n");
        return sb.toString();
    }
    
    
    
	/**
	 * @Author: Chiabrando
	 *
	 *
	 * Generates the complete class definition for FBAProcessor in C++ syntax.
	 * This method constructs a string that includes the entire class definition, encompassing
	 * documentation comments, methods, and member variables. The class is designed using the Singleton
	 * pattern to manage flux balance analysis processes efficiently.
	 *
	 * @return A string containing the formatted class definition for FBAProcessor, ready to be used in C++ code.
	 */

		private static String generateFBAClass() {
				StringBuilder sb = new StringBuilder();
				
				sb.append("/**\n");
				sb.append(" * @class FBAProcessor\n");
				sb.append(" * @brief A singleton class that manages flux balance analysis processes.\n");
				sb.append(" *\n");
				sb.append(" * The FBAProcessor class is designed using the Singleton design pattern to ensure\n");
				sb.append(" * that only one instance of this class is created throughout the lifetime of the application.\n");
				sb.append(" * This class handles the initialization, management, and computation of flux balance analysis\n");
				sb.append(" * based on metabolic reaction data.\n");
				sb.append(" */\n");
				sb.append("class FBAProcessor {\n");

				sb.append("		public:\n");
				sb.append("    /**\n");
				sb.append("     * @brief Retrieves the single instance of the FBAProcessor class.\n");
				sb.append("     * \n");
				sb.append("     * This method ensures that the FBAProcessor class is instantiated only once\n");
				sb.append("     * and the same instance is returned with every call. The instance is guaranteed\n");
				sb.append("     * to be destroyed only when the application exits, ensuring controlled and predictable\n");
				sb.append("     * lifecycle management of the singleton resource.\n");
				sb.append("     * \n");
				sb.append("     * @return Reference to the singleton FBAProcessor instance.\n");
				sb.append("     */\n");
				sb.append("    static FBAProcessor& getInstance() {\n");
				sb.append("        static FBAProcessor instance; // Static variable ensures that the instance is created only once.\n");
				sb.append("        return instance;\n");
				sb.append("    }\n\n");

				sb.append("    /**\n");
				sb.append("     * @brief Deletes the copy constructor to prevent copying of the singleton instance.\n");
				sb.append("     */\n");
				sb.append("    FBAProcessor(FBAProcessor const&) = delete;\n");
				sb.append("    /**\n");
				sb.append("     * @brief Deletes the assignment operator to prevent assignment and copying of the singleton instance.\n");
				sb.append("     */\n");
				sb.append("    void operator=(FBAProcessor const&) = delete;\n\n");


				sb.append("\n\n");
				

				sb.append("    /**\n");
				sb.append("     * @brief Processes metabolic changes and calculates the rate of a specified transition.\n");
				sb.append("     * \n");
				sb.append("     * This method serves as the primary access point for external components to interact\n");
				sb.append("     * with the FBAProcessor. It ensures that the system is initialized before proceeding\n");
				sb.append("     * with updates and calculations. The method handles the following tasks:\n");
				sb.append("     * - Initializes the data structures if not already initialized.\n");
				sb.append("     * - Updates the bounds based on current metabolic values and solves the LP problems.\n");
				sb.append("     * - Computes the rate of the specified transition based on the new LP solution.\n");
				sb.append("     * \n");
				sb.append("     * @param Value Pointer to the array containing current metabolic values.\n");
				sb.append("     * @param vec_fluxb Reference to a vector of LPprob objects representing flux balance problems.\n");
				sb.append("     * @param NumTrans Mapping of transition names to their corresponding indices.\n");
				sb.append("     * @param NumPlaces Mapping of place names to their corresponding indices in the metabolic array.\n");
				sb.append("     * @param NameTrans Vector containing names of transitions, indexed by transition IDs.\n");
				sb.append("     * @param Trans Pointer to the structure containing transition information (not used directly in this method but may be used for extending functionality).\n");
				sb.append("     * @param T The index of the transition for which the rate is to be calculated.\n");
				sb.append("     * @param time The current simulation time, used to check if updates are needed based on time-driven changes.\n");
				sb.append("     * @return The rate of the specified transition after processing the metabolic changes.\n");
				sb.append("     * This rate could be used to determine the speed or likelihood of the transition occurring.\n");
				sb.append("     */\n");
				sb.append("    double process(double *Value, vector<class FBGLPK::LPprob>& vec_fluxb, map<string,int>& NumTrans, map<string,int>& NumPlaces, const vector<string>& NameTrans, const struct InfTr* Trans, int T, const double& time) {\n");
				sb.append("        if (!init) {\n");
				sb.append("            init_data_structures_class(NameTrans, vec_fluxb, Value, NumPlaces); // Ensure it is initialized before proceeding\n");
				sb.append("            mapReactionsFromProblems(vec_fluxb);\n");
				sb.append("            loadAndApplyFBAReactionUpperBounds(vec_fluxb, \"EX_upper_bounds_FBA.csv\");\n");
				sb.append("            initializeNonFBAReactionsBaseUB(vec_fluxb);\n");
				sb.append("            updateNonFBAReactionUpperBoundsFromFile(vec_fluxb, \"EX_upper_bounds_nonFBA.csv\");\n");
				sb.append("            firstTransitionName = NameTrans[T];\n");
				sb.append("            init = true;\n");
				sb.append("        }\n");
				sb.append("        if (firstTransitionName == NameTrans[T]) {\n");
				sb.append("            for (auto& problem : FBAproblems) {\n");
				sb.append("                if (hasMultiSpecies && floor(Value[NumPlaces.at(problemBacteriaPlace.at(problem.second))]) < 1) {\n");
				sb.append("                    deadBacterialSpecies[problem.second] = true;\n");
				sb.append("                }\n");
				sb.append("            }\n");
				sb.append("            if (hasBioMASS) {\n");
				sb.append("            		updateAllBiomassReactionsUpperBounds(Value, NumPlaces, vec_fluxb); // Update biomass upper limits\n");
				sb.append("            }\n");
				sb.append("            updateFluxBoundsAndSolve(Value, vec_fluxb, NumPlaces, time); // Update fluxes only on first transition\n");
				sb.append("        }\n");
				sb.append("        double rate = 0;\n");
				sb.append("        size_t problemIndex = FBAproblems[NameTrans[T]];\n");
				sb.append("        // Return zero rate if the species for this transition is dead\n");
				sb.append("        if (deadBacterialSpecies.find(problemIndex) != deadBacterialSpecies.end()) {\n");
				sb.append("            return 0; // Return zero rate for dead species\n");
				sb.append("        }\n");
				sb.append("\n");
				sb.append("        // Check for transitions without associated file or specific biomass transitions.\n");
				sb.append("        if (transitionsWithoutFile.find(NameTrans[T]) != transitionsWithoutFile.end()) {\n");
				sb.append("            return rate; // Skip further processing for these cases\n");
				sb.append("        }\n");
				sb.append("\n");
				sb.append("        // Default computation if none of the above conditions met.\n");
				sb.append("        rate = computeRate(vec_fluxb, NumPlaces, NameTrans, Value, T, decimalTrunc, time); // Compute and return the rate for the given transition\n");
				sb.append("        return rate; // Return the calculated rate\n");
				sb.append("    }\n\n");

				
				sb.append("\n\n");


				sb.append("		private:\n");
				

				sb.append("    struct PairHash {\n");
				sb.append("        size_t operator()(const std::pair<std::string, size_t>& p) const {\n");
				sb.append("            // Combino gli hash\n");
				sb.append("            // (puoi usare qualunque formula di combinazione)\n");
				sb.append("            auto h1 = std::hash<std::string>()(p.first);\n");
				sb.append("            auto h2 = std::hash<size_t>()(p.second);\n");
				sb.append("            // Semplice combiner stile boost::hash_combine\n");
				sb.append("            h1 ^= (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));\n");
				sb.append("            return h1;\n");
				sb.append("        }\n");
				sb.append("    };\n");

				sb.append("    // Se vuoi un comparator personalizzato (ma di solito la pair ha già operator==)\n");
				sb.append("    struct PairEq {\n");
				sb.append("        bool operator()(const std::pair<std::string, size_t>& a,\n");
				sb.append("                        const std::pair<std::string, size_t>& b) const {\n");
				sb.append("            return (a.first == b.first && a.second == b.second);\n");
				sb.append("        }\n");
				sb.append("    };\n");

				sb.append("     set<string> reactionsToUpdate;\n");
				sb.append("     string firstTransitionName = \"\"; // Tracks problems associated with dead bacterial species\n");
				
				sb.append("     unordered_map<size_t, bool> deadBacterialSpecies; // Tracks problems associated with dead bacterial species\n");
				
				sb.append("    /**\n");
				sb.append("     * comment.\n");
				sb.append("     */\n");				
				sb.append("     unordered_map<size_t, bool> problemsWithLowBiomass; // Tracks problems with biomass below the minimum threshold\n");
				
				sb.append("			const double Lcutoff = 1e-6; // Define a small cutoff value for limit the biomass upper bound\n");

				sb.append("    /**\n");
				sb.append("     * Unordered set to store transitions identified as related to biomass.\n");
				sb.append("     */\n");
				sb.append("     unordered_set<string> biomassTransitions;\n");
				
				sb.append("			unordered_set<string> transitionsWithoutFile;\n");
				
				sb.append("    std::unordered_map<\n");
				sb.append("        std::pair<std::string, size_t>,\n");
				sb.append("        double,\n");
				sb.append("        PairHash,\n");
				sb.append("        PairEq\n");
				sb.append("    > NonFBAReactionBaseUB;\n");
				
				sb.append("    unordered_map<string, unordered_set<size_t>> reactionToFileMap; // Maps each reaction to a set of files where it appears\n\n");

				sb.append("    unordered_map<int, double> bacteriaToBioMin; // Maps each reaction to a set of files where it appears\n\n");

				sb.append("    unordered_map<int, double> bacteriaToBioMax; // Maps each reaction to a set of files where it appears\n\n");

				sb.append("    unordered_map<int,double> bacteriaToBioMean; // Maps each reaction to a set of files where it appears\n\n");
							
				sb.append("    /**\n");
				sb.append("     * Multiplicative constant for the reaction.\n");
				sb.append("     */\n");
				sb.append("    double multiplicativeConstant = 1;\n\n");
				
				sb.append("    /**\n");
				sb.append("     * Comment.\n");
				sb.append("     */\n");
				sb.append("    bool hasMultiSpecies = false;\n\n");
				
				sb.append("    /**\n");
				sb.append("     * Comment.\n");
				sb.append("     */\n");
				sb.append("    double hasBioMASS = false;\n\n");

				sb.append("    /**\n");
				sb.append("     * Maps each transition to its corresponding reaction. Used for linking network transitions to specific biochemical reactions.\n");
				sb.append("     */\n");
				sb.append("    unordered_map<string, string> FBAreact;\n");

				sb.append("    /**\n");
				sb.append("     * Stores mappings from reactions to their places. This helps in maintaining the state of metabolic concentrations involved in reactions.\n");
				sb.append("     */\n");
				sb.append("    unordered_map<string, string> FBAplace;\n");

				sb.append("    /**\n");
				sb.append("     * Maps each reaction to an index of the corresponding LP problem in a vector. This facilitates quick access to the problem related to a particular reaction.\n");
				sb.append("     */\n");
				sb.append("    unordered_map<string, size_t> FBAproblems;\n");
				
				sb.append("    /**\n");
				sb.append("     * Maps each LP problem index to a set of reactions associated with that problem.\n");
				sb.append("     * This helps in managing and optimizing the parallel processing of flux balance analysis problems,\n");
				sb.append("     * ensuring that all reactions associated with a particular problem are processed together.\n");
				sb.append("     */\n");
				sb.append("    unordered_map<size_t, set<string>> problemsToReactions;\n");

				sb.append("    /**\n");
				sb.append("     * A set of all reactions that are part of the FBA model. Helps in ensuring that each reaction is processed uniquely.\n");
				sb.append("     */\n");
				sb.append("    set<string> FBAreactions;\n");
				
				sb.append("    /**\n");
				sb.append("     * A set of all problem index that are part of the FBA model.\n");
				sb.append("     */\n");
				sb.append("    set<size_t> problems;\n");

				sb.append("    /**\n");
				sb.append("     * Initialization status flag. Used to ensure that the FBAProcessor is set up before any operations are performed.\n");
				sb.append("     */\n");
				sb.append("    bool init;\n");

				sb.append("    /**\n");
				sb.append("     * Precision setting for numerical operations, specifically truncating numbers to a fixed number of decimal places.\n");
				sb.append("     */\n");
				sb.append("    const double decimalTrunc = 16;\n");

				sb.append("    /**\n");
				sb.append("     * Stores the last time the FBA model was updated, used to prevent unnecessary recalculations within the same time frame.\n");
				sb.append("     */\n");
				sb.append("     double FBAtime = -1;\n");
				
				
				
				sb.append("    double minDeltaThreshold = 1e-16;\n");		
				
				sb.append("    unordered_map<string, double> ReactionMultiplicity;\n");

				sb.append("    /**\n");
				sb.append("     * Pointer to an array of pointers, each pointing to a set of variables representing the results of the linear programming problems.\n");
				sb.append("     */\n");
				sb.append("    double** Vars;\n");

				sb.append("    /**\n");
				sb.append("     * Tracks the previous concentrations for all places, allowing for comparison against new values to detect significant changes.\n");
				sb.append("     */\n");
				sb.append("    unordered_map<string, double> previousConcentrations;\n");
				
				sb.append("    /**\n");
				sb.append("     * Comment.\n");
				sb.append("     */\n");
				sb.append("    unordered_map<size_t, string> problemBacteriaPlace;\n");
				
				sb.append("    /**\n");
				sb.append("     * Comment.\n");
				sb.append("     */\n");
				sb.append("    unordered_map<size_t, string> problemBiomassPlace;\n");
				
				sb.append("    /**\n");
				sb.append("     * Comment.\n");
				sb.append("     */\n");
				sb.append("    unordered_map<string, string> reactionToBacteria;\n");
				
				
				sb.append("    /**\n");
				sb.append("     * Comment.\n");
				sb.append("     */\n");
				sb.append("    unordered_map<string, string> reactionToBacteriaBIOMASS;\n");
				
				

				sb.append("    /**\n");
				sb.append("     * Threshold for determining significant changes in metabolic concentrations, expressed as a percentage.\n");
				sb.append("     */\n");
				sb.append("    double epsilon = 0; // Initially set to 0%, can be configured.\n");
				
				sb.append("    double scalingFactor = 1e-12; // Initially set to 0%, can be configured.\n");

				sb.append("    /**\n");
				sb.append("     * Counter to track how many times the network state has undergone significant changes.\n");
				sb.append("     */\n");
				sb.append("    double count = 0;\n");

				sb.append("    /**\n");
				sb.append("     * Enumeration to distinguish between input and output transitions within the metabolic network.\n");
				sb.append("     */\n");
				sb.append("    enum TransitionType { Input, Output };\n");

				sb.append("    /**\n");
				sb.append("     * Private constructor to prevent instantiation outside of the getInstance() method, ensuring singleton behavior.\n");
				sb.append("     */\n");

				sb.append("    FBAProcessor() : init(false) {\n");
				sb.append("    }\n\n");
							
				// Definizione della mappa metaboliteToProblems
				sb.append("    /**\n");
				sb.append("     * Maps each metabolite to a set of problem indices that are affected by changes in this metabolite.\n");
				sb.append("     * This helps optimize the processing by updating only relevant LP problems when specific metabolite concentrations change.\n");
				sb.append("     */\n");
				sb.append("    unordered_map<string, set<size_t>> metaboliteToProblems;\n\n");
				
		
				sb.append("\n\n");
				

				sb.append("    /**\n");
				sb.append("     * Destructor to clean up memory allocated for the Vars pointer of pointers, ensuring no memory leaks.\n");
				sb.append("     */\n");
				sb.append("    ~FBAProcessor() {\n");
				sb.append("        // Clean up memory for the array of pointers\n");
				sb.append("        delete[] Vars;  // Only deallocates the array of pointers\n");
				sb.append("        Vars = nullptr;  // Sets the pointer to nullptr to prevent invalid memory access\n");
				sb.append("        FBGLPK::freeGLPKEnvironment();  // Sets the pointer to nullptr to prevent invalid memory access\n");
				sb.append("        cout << \"FBAProcessor destroyed and GLPK environment freed.\" << endl;\n");
				sb.append("    }\n");
				
				sb.append("\n\n");
					
				
				sb.append("    /**\n");
				sb.append("     * Recalculates the multiplicative constant for the specified transition based on the accessibility of each species to a metabolite.\n");
				sb.append("     * The constant is computed based on the sum of the products of biomass mean and bacterial count across all relevant species.\n");
				sb.append("     * @param transition The transition for which the constant is being recalculated.\n");
				sb.append("     * @param reactionToFileMap Mapping of reactions to the problems that contain these reactions.\n");
				sb.append("     * @param bacteriaToBioMean Mapping from problem index to mean biomass for that problem.\n");
				sb.append("     * @param problemBacteriaPlace Mapping from problem index to the bacteria count place.\n");
				sb.append("     * @param problemBiomassPlace Mapping from problem index to the biomass place.\n");
				sb.append("     * @param NumPlaces Mapping of place names to their corresponding indices in the metabolic array.\n");
				sb.append("     * @param Value Pointer to the array containing current metabolic values.\n");
				sb.append("     */\n");
				sb.append("    void calculateMultiplicativeConstant(double* Value, const map<string, int>& NumPlaces, size_t problemIndex, string reaction) {\n");
				sb.append("        double denominator = 0;\n");
				sb.append("        double numerator = 0;\n");
				sb.append("\n");
				sb.append("        if (reactionToFileMap.find(reaction) != reactionToFileMap.end()) {\n");
				sb.append("            const auto& problems = reactionToFileMap.at(reaction);\n");
				sb.append("            for (const auto& problem : problems) {\n");
				sb.append("                double numIndividuals = floor(Value[NumPlaces.at(problemBacteriaPlace.at(problem))]);\n");
				sb.append("                denominator += numIndividuals;  // Accumulate total number of individuals for active problems\n");
				sb.append("            }\n");
				sb.append("\n");
				sb.append("            double transitionCurrentBiomass = Value[NumPlaces.at(problemBiomassPlace.at(problemIndex))];\n");
				sb.append("            double transitionBioMean = bacteriaToBioMean.at(problemIndex);\n");
				sb.append("\n");
				sb.append("            denominator *= transitionBioMean;  // Adjust denominator by the biomass mean of the transition problem\n");
				sb.append("            numerator = transitionCurrentBiomass;  // Use current biomass of the transition problem as numerator\n");
				sb.append("        }\n");
				sb.append("\n");
				sb.append("        if (denominator > 0) {\n");
				sb.append("            multiplicativeConstant = trunc(numerator * floor(Value[NumPlaces.at(problemBacteriaPlace.at(problemIndex))]) / denominator, decimalTrunc);  // Calculate multiplicative constant\n");
				sb.append("        } else {\n");
				sb.append("            multiplicativeConstant = 0;  // Handle case where denominator is zero\n");
				sb.append("        }\n");
				sb.append("    }\n");
				

				sb.append("\n\n");
				

				sb.append("    /**\n");
				sb.append("     * Trims whitespace from both ends of a given string.\n");
				sb.append("     * @param str The string to trim.\n");
				sb.append("     * @return A trimmed string with no leading or trailing whitespace.\n");
				sb.append("     */\n");
				sb.append("    string trim(const string& str) {\n");
				sb.append("        size_t first = str.find_first_not_of(' ');\n");
				sb.append("        if (first == string::npos) return \"\"; // Return empty if string is made of spaces\n");
				sb.append("        size_t last = str.find_last_not_of(' ');\n");
				sb.append("        return str.substr(first, last - first + 1);\n");
				sb.append("    }\n\n");


				sb.append("\n\n");
				

				sb.append("    /**\n");
				sb.append("     * Splits a string by a given delimiter and trims each resulting token.\n");
				sb.append("     * @param str The string to split.\n");
				sb.append("     * @param delim The delimiter character.\n");
				sb.append("     * @return A vector of trimmed strings.\n");
				sb.append("     */\n");
				sb.append("    vector<string> splitAndTrim(const string& str, char delimiter) {\n");
				sb.append("        vector<string> tokens;\n");
				sb.append("        stringstream ss(str);\n");
				sb.append("        string item;\n");
				sb.append("        while (getline(ss, item, delimiter)) {\n");
				sb.append("            item.erase(remove(item.begin(), item.end(), ' '), item.end()); // Remove spaces\n");
				sb.append("            if (!item.empty()) {\n");
				sb.append("                tokens.push_back(item);\n");
				sb.append("            }\n");
				sb.append("        }\n");
				sb.append("        return tokens;\n");
				sb.append("    }\n");

				
				sb.append("\n\n");


				sb.append("    /**\n");
				sb.append("     * Finds the index of a linear programming problem associated with a given filename within a vector of LP problems.\n");
				sb.append("     * @param vec_fluxb A vector containing LP problems.\n");
				sb.append("     * @param lpFile The filename associated with the LP problem.\n");
				sb.append("     * @return The index of the LP problem in the vector or max size if not found.\n");
				sb.append("     */\n");
				sb.append("    size_t findLPIndex(const vector<class FBGLPK::LPprob>& vec_fluxb, const string& lpFile) {\n");
				sb.append("        for (size_t i = 0; i < vec_fluxb.size(); ++i) {\n");
				sb.append("            if (vec_fluxb[i].getFilename() == lpFile) {\n");
				sb.append("                return i;\n");
				sb.append("            }\n");
				sb.append("        }\n");
				sb.append("        return numeric_limits<size_t>::max();\n");
				sb.append("    }\n");
				
				
				sb.append("\n\n");
				
				sb.append("/**\n");
				sb.append(" * @brief Ritorna il nome reazione finale, con logica speciale per Biomass.\n");
				sb.append(" *\n");
				sb.append(" * Se isBiomass == true:\n");
				sb.append(" *   - se ha sia input che output => '_f'\n");
				sb.append(" *   - se ha solo input => '_r'\n");
				sb.append(" * Altrimenti, usa la logica di splitted e irreversibile.\n");
				sb.append(" */\n");
				sb.append("static std::string standardizeReactionName(\n");
				sb.append("    const std::string& transitionName,\n");
				sb.append("    const std::string& reactionBase,\n");
				sb.append("    const std::vector<std::string>& inputs,\n");
				sb.append("    const std::vector<std::string>& outputs,\n");
				sb.append("    const std::map<std::string, unsigned int>& forwardMap,\n");
				sb.append("    const std::map<std::string, unsigned int>& reverseMap,\n");
				sb.append("    const std::map<std::string, unsigned int>& irreversMap,\n");
				sb.append("    bool isBiomass // <--- nuovo parametro per la logica speciale\n");
				sb.append(")\n");
				sb.append("{\n");
				sb.append("    //std::cerr << \"\\n[DEBUG standardizeReactionName] \"\n");
				sb.append("      //        << \"transition='\" << transitionName << \"', \"\n");
				sb.append("     //         << \"reactionBase='\" << reactionBase << \"', \"\n");
				sb.append("     //         << \"isBiomass=\" << (isBiomass ? \"true\" : \"false\") << \"\\n\";\n");
				sb.append("\n");
				sb.append("    // 1) Se la reazione base è nella mappa irreversibile, usiamola invariata\n");
				sb.append("    if (irreversMap.find(reactionBase) != irreversMap.end()) {\n");
				sb.append("        //std::cerr << \"  [DEBUG] '\" << reactionBase\n");
				sb.append("          //        << \"' è irreversibile => la teniamo invariata.\\n\";\n");
				sb.append("        return reactionBase;\n");
				sb.append("    }\n");
				sb.append("\n");
				sb.append("    // 2) Controlla se esistono forward e/o reverse (splitted)\n");
				sb.append("    bool splittedF = (forwardMap.find(reactionBase + \"_f\") != forwardMap.end());\n");
				sb.append("    bool splittedR = (reverseMap.find(reactionBase + \"_r\") != reverseMap.end());\n");
				sb.append("\n");
				sb.append("    // 3) Se non è irreversibile e non è splitted, restituisco invariato\n");
				sb.append("    if (!splittedF && !splittedR) {\n");
				sb.append("        //std::cerr << \"  [WARN] Reaction '\" << reactionBase\n");
				sb.append("         //         << \"' non trovata in irreversMap, forwardMap, né reverseMap. La lascio invariata.\\n\";\n");
				sb.append("        return reactionBase;\n");
				sb.append("    }\n");
				sb.append("\n");
				sb.append("    // 4) Gestione speciale Biomass:\n");
				sb.append("    if (isBiomass) {\n");
				sb.append("        // Caso 4a: se ha sia input che output => '_f'\n");
				sb.append("        if (!inputs.empty() && !outputs.empty() && splittedF) {\n");
				sb.append("           // std::cerr << \"  [DEBUG Biomass] '\" << reactionBase\n");
				sb.append("             //         << \"' => '_f' (ha sia input che output)\\n\";\n");
				sb.append("            return reactionBase + \"_f\";\n");
				sb.append("        }\n");
				sb.append("        // Caso 4b: se ha solo input => '_r' (se splittedR)\n");
				sb.append("        else if (!inputs.empty() && outputs.empty() && splittedR) {\n");
				sb.append("            //std::cerr << \"  [DEBUG Biomass] '\" << reactionBase\n");
				sb.append("            //          << \"' => '_r' (ha solo input)\\n\";\n");
				sb.append("            return reactionBase + \"_r\";\n");
				sb.append("        }\n");
				sb.append("        // Altrimenti fallback biomass => invariato (oppure potresti mettere un fallback a _r)\n");
				sb.append("        else {\n");
				sb.append("           // std::cerr << \"  [WARN Biomass] '\" << reactionBase\n");
				sb.append("            //          << \"' mismatch => la lascio invariata.\\n\";\n");
				sb.append("            return reactionBase;\n");
				sb.append("        }\n");
				sb.append("    }\n");
				sb.append("\n");
				sb.append("    // 5) Non-biomass logic (classica):\n");
				sb.append("    //    - se ci sono input => '_f'\n");
				sb.append("    //    - se ci sono output => '_r'\n");
				sb.append("    if (!outputs.empty() && splittedR) {\n");
				sb.append("       // std::cerr << \"  [DEBUG splitted] '\" << reactionBase\n");
				sb.append("         //         << \"' => '_r' (ha inputPlaces)\\n\";\n");
				sb.append("        return reactionBase + \"_r\";\n");
				sb.append("    } else if (!inputs.empty() && splittedF) {\n");
				sb.append("        //std::cerr << \"  [DEBUG splitted] '\" << reactionBase\n");
				sb.append("         //         << \"' => '_f' (ha outputPlaces)\\n\";\n");
				sb.append("        return reactionBase + \"_f\";\n");
				sb.append("    }\n");
				sb.append("\n");
				sb.append("    // 6) Fallback\n");
				sb.append("   // std::cerr << \"  [WARN splitted] '\" << reactionBase\n");
				sb.append("      //        << \"' mismatch => la lascio invariata.\\n\";\n");
				sb.append("    return reactionBase;\n");
				sb.append("}\n");
				sb.append("\n");
				
				sb.append("    /**\n");
				sb.append("     * Reads and processes the FBA information from a specified file path, initializing data structures necessary for simulation.\n");
				sb.append("     * This includes mapping each transition to its corresponding reaction, and associating reactions to LP problem indices.\n");
				sb.append("     * Also builds a mapping from LP problem indices to sets of reactions for optimized problem solving.\n");
				sb.append("     * @param filePath The path to the file containing FBA information.\n");
				sb.append("     * @param vec_fluxb A vector of LP problems to be populated based on the file.\n");
				sb.append("     * @return true if the file was read successfully, false otherwise.\n");
				sb.append("     */\n");
				sb.append("    bool readFBAInfo(const string& filePath, vector<class FBGLPK::LPprob>& vec_fluxb) {\n");
				sb.append("        ifstream file(filePath);\n");
				sb.append("        if (!file.is_open()) {\n");
				sb.append("            cerr << \"Failed to open file: \" << filePath << endl;\n");
				sb.append("            return false;\n");
				sb.append("        }\n");
				sb.append("        stringstream buffer;\n");
				sb.append("        buffer << file.rdbuf();\n");
				sb.append("        string content = buffer.str();\n");
				sb.append("        file.close();\n");
				sb.append("\n");
				sb.append("        size_t pos = 0;\n");
				sb.append("        while ((pos = content.find('{', pos)) != string::npos) {\n");
				sb.append("            size_t endPos = content.find('}', pos);\n");
				sb.append("            if (endPos == string::npos) break;\n");
				sb.append("\n");
				sb.append("            string jsonObject = content.substr(pos + 1, endPos - pos - 1);\n");
				sb.append("            pos = endPos + 1;\n");
				sb.append("\n");
				sb.append("            string transition, lpFile, reaction, multiplicityStr, bacteriaCountPlace, bacteriaBiomassPlace, inputPlacesStr, outputPlacesStr, isBiomassStr;\n");
				sb.append("            int multiplicity = 1;\n");
				sb.append("            bool isBiomass = false;\n");
				sb.append("\n");
				sb.append("            size_t keyPos = 0, valuePos = 0;\n");
				sb.append("            while ((keyPos = jsonObject.find('\"', keyPos)) != string::npos) {\n");
				sb.append("                size_t keyEnd = jsonObject.find('\"', keyPos + 1);\n");
				sb.append("                string key = jsonObject.substr(keyPos + 1, keyEnd - keyPos - 1);\n");
				sb.append("\n");
				sb.append("                valuePos = jsonObject.find(':', keyEnd) + 1;\n");
				sb.append("                size_t valueEnd = (jsonObject[valuePos] == '\"') ? jsonObject.find('\"', valuePos + 1) : jsonObject.find(',', valuePos);\n");
				sb.append("                if (valueEnd == string::npos) valueEnd = jsonObject.length();\n");
				sb.append("                string value = jsonObject.substr(valuePos, valueEnd - valuePos);\n");
				sb.append("                value.erase(remove(value.begin(), value.end(), '\"'), value.end());\n");
				sb.append("                value.erase(remove(value.begin(), value.end(), ' '), value.end());\n");
				sb.append("\n");
				sb.append("                if (key == \"Transition\") transition = value;\n");
				sb.append("                else if (key == \"LPfile\") lpFile = value;\n");
				sb.append("                else if (key == \"Reaction\") reaction = value;\n");
				sb.append("                else if (key == \"Multiplicity\") multiplicityStr = value;\n");
				sb.append("                else if (key == \"BacteriaCountPlace\") bacteriaCountPlace = value;\n");
				sb.append("                else if (key == \"BacteriaBiomassPlace\") bacteriaBiomassPlace = value;\n");
				sb.append("                else if (key == \"InputPlaces\") inputPlacesStr = value;\n");
				sb.append("                else if (key == \"OutputPlaces\") outputPlacesStr = value;\n");
				sb.append("                else if (key == \"IsBiomass\") isBiomass = (value == \"true\\n\");\n");
				sb.append("\n");
				sb.append("                keyPos = valueEnd + 1;\n");
				sb.append("            }\n");
				sb.append("\n");
				sb.append("            try {\n");
				sb.append("                multiplicity = stoi(multiplicityStr);\n");
				sb.append("            } catch (const std::invalid_argument& e) {\n");
				sb.append("                cerr << \"Error converting multiplicity to integer: \" << multiplicityStr << endl;\n");
				sb.append("                cerr << \"Exception: \" << e.what() << endl;\n");
				sb.append("                return false;\n");
				sb.append("            } catch (const std::out_of_range& e) {\n");
				sb.append("                cerr << \"Multiplicity value out of range: \" << multiplicityStr << endl;\n");
				sb.append("                cerr << \"Exception: \" << e.what() << endl;\n");
				sb.append("                return false;\n");
				sb.append("            }\n");
				sb.append("\n");
				sb.append("            // Remove square brackets and split the strings\n");
				sb.append("            inputPlacesStr.erase(remove(inputPlacesStr.begin(), inputPlacesStr.end(), '['), inputPlacesStr.end());\n");
				sb.append("            inputPlacesStr.erase(remove(inputPlacesStr.begin(), inputPlacesStr.end(), ']'), inputPlacesStr.end());\n");
				sb.append("            outputPlacesStr.erase(remove(outputPlacesStr.begin(), outputPlacesStr.end(), '['), outputPlacesStr.end());\n");
				sb.append("            outputPlacesStr.erase(remove(outputPlacesStr.begin(), outputPlacesStr.end(), ']'), outputPlacesStr.end());\n");
				sb.append("\n");
				sb.append("            auto removeUnwantedChars = [](string& str) {\n");
				sb.append("                str.erase(remove_if(str.begin(), str.end(), [](char c) { return isspace(c) || iscntrl(c); }), str.end());\n");
				sb.append("            };\n");
				sb.append("\n");
				sb.append("            removeUnwantedChars(inputPlacesStr);\n");
				sb.append("            removeUnwantedChars(outputPlacesStr);\n");
				sb.append("\n");
				sb.append("            vector<string> inputs;\n");
				sb.append("            if (!inputPlacesStr.empty() && !(inputPlacesStr == \" \") && !(inputPlacesStr == \"\")) inputs = splitAndTrim(inputPlacesStr, ',');\n");
				sb.append("            vector<string> outputs;\n");
				sb.append("            if (!outputPlacesStr.empty() && !(outputPlacesStr == \" \") && !(outputPlacesStr == \"\")) outputs = splitAndTrim(outputPlacesStr, ',');\n");
				sb.append("\n");
				sb.append("            if (isBiomass) {\n");
				sb.append("                if (!inputs.empty() && !outputs.empty()) {\n");
				sb.append("                    biomassTransitions.insert(transition);\n");
				sb.append("                    //std::cerr << \"[DEBUG] => Inserito '\" << transition << \"' in biomassTransitions (entrambe input e output presenti).\\n\";\n");
				sb.append("                } else {\n");
				sb.append("                   // std::cerr << \"[DEBUG] => '\" << transition << \"' è IsBiomass ma NON ha entrambi input e output => NON inserita in biomassTransitions.\\n\";\n");
				sb.append("                }\n");
				sb.append("            }\n");
				sb.append("\n");
				sb.append("						string finalReaction;\n");
				sb.append("            if (!lpFile.empty() && !reaction.empty()) {\n");
				sb.append("								 size_t index = findLPIndex(vec_fluxb, lpFile);\n");
				sb.append("								 if (index == numeric_limits<size_t>::max()) {\n");
				sb.append("    						 		// Non trovata => transitionsWithoutFile\n");
				sb.append("    								transitionsWithoutFile.insert(transition);\n");
				sb.append("								 } else {\n");
				sb.append("    								// A) Prendiamo le mappe splitted/irrevers dal LPprob corrispondente\n");
				sb.append("    								const auto& forwardMap  = vec_fluxb[index].getForwardReactions();\n");
				sb.append("    								const auto& reverseMap  = vec_fluxb[index].getReverseReactions();\n");
				sb.append("    								const auto& irreversMap = vec_fluxb[index].getIrreversibileReactions();\n");
				sb.append("\n");
				sb.append("										//cerr << \"[DEBUG TRANSIZIONE] '\" << transition << endl;\n");
				sb.append("    								// B) Richiamiamo la nostra funzione di standardizzazione\n");
				sb.append("										  finalReaction = standardizeReactionName(\n");
				sb.append("    									transition,      // Nome della transizione (debug)\n");
				sb.append("    									reaction,        // Reazione base\n");
				sb.append("    									inputs,\n");
				sb.append("    									outputs,\n");
				sb.append("    									forwardMap,\n");
				sb.append("    									reverseMap,\n");
				sb.append("    									irreversMap,\n");
				sb.append("   	 								isBiomass\n");
			  sb.append("										);\n");
				sb.append("    								//cerr << \"[DEBUG readFBAInfo] reactionBase='\" << reaction\n");
				sb.append("         				//	<< \"' => finalReaction='\" << finalReaction << \"'\\n\";\n");
				sb.append("\n");
				sb.append("    								// C) Ora salviamo finalReaction in FBAreact\n");
				sb.append("    								FBAreact[transition] = finalReaction;\n");
				sb.append("\n");
				sb.append("    							 // D) Resto invariato (bacteriaCountPlace, ecc.)\n");
				sb.append("    							if (!bacteriaCountPlace.empty() && bacteriaCountPlace != \"N/A\") {\n");
				sb.append("        							problemBacteriaPlace[index] = bacteriaCountPlace;\n");
				sb.append("    							}\n");
				sb.append("    							if (!bacteriaBiomassPlace.empty() && bacteriaBiomassPlace != \"N/A\") {\n");
				sb.append("        							problemBiomassPlace[index] = bacteriaBiomassPlace;\n");
				sb.append("    							}\n");
				sb.append("\n");
				sb.append("    							problems.insert(index);\n");
				sb.append("    							bacteriaToBioMax[index]  = vec_fluxb[index].getBioMax();\n");
				sb.append("    							bacteriaToBioMean[index] = vec_fluxb[index].getBioMean();\n");
				sb.append("    							bacteriaToBioMin[index]  = vec_fluxb[index].getBioMin();\n");
				sb.append("\n");
				sb.append("    							FBAproblems[transition] = index;\n");
				sb.append("    							problemsToReactions[index].insert(transition);\n");
				sb.append("								 }\n");
				sb.append("            		 if (isBiomass) {\n");
				sb.append("                		FBAreactions.insert(reaction + \"_f\");\n");
				sb.append("                		FBAreactions.insert(reaction + \"_r\");\n");
				sb.append("                }else{\n");
				sb.append("                		FBAreactions.insert(finalReaction);\n");				
				sb.append("                }\n");
				sb.append("                if (FBAreact.find(transition) != FBAreact.end()) {\n");
				sb.append("                    string reactionName = FBAreact[transition];\n");
				sb.append("                    if ((!inputs.empty() || !outputs.empty())) {\n");
				sb.append("                        string combinedPlaces;\n");
				sb.append("                        for (const auto& place : inputs) {\n");
				sb.append("                            if (!place.empty() && combinedPlaces.find(place + \",\") == string::npos) {\n");
				sb.append("                                combinedPlaces += place + \",\";\n");
				sb.append("                            }\n");
				sb.append("                        }\n");
				sb.append("                        for (const auto& place : outputs) {\n");
				sb.append("                            if (!place.empty() && combinedPlaces.find(place + \",\") == string::npos) {\n");
				sb.append("                                combinedPlaces += place + \",\";\n");
				sb.append("                            }\n");
				sb.append("                        }\n");
				sb.append("                        if (!combinedPlaces.empty()) {\n");
				sb.append("                            combinedPlaces.pop_back(); // Remove the last comma\n");
				sb.append("                            FBAplace[reactionName] = combinedPlaces;\n");
				sb.append("                        }\n");
				sb.append("                    }\n");
				sb.append("                }\n");
				sb.append("            }\n");
				sb.append("            ReactionMultiplicity[FBAreact[transition]] = multiplicity;\n");
				sb.append("\n");
				sb.append("            // Populate the reactionToBacteria map if bacteriaCountPlace is specified\n");
				sb.append("            if (!bacteriaCountPlace.empty() && bacteriaCountPlace != \"N/A\") {\n");
				sb.append("                reactionToBacteria[transition] = bacteriaCountPlace;\n");
				sb.append("                hasMultiSpecies = true;\n");
				sb.append("            }\n");
				sb.append("            if (!bacteriaBiomassPlace.empty() && bacteriaBiomassPlace != \"N/A\") {\n");
				sb.append("                reactionToBacteriaBIOMASS[transition] = bacteriaBiomassPlace;\n");
				sb.append("                hasBioMASS = true;\n");
				sb.append("            }\n");
				sb.append("        }\n");
				sb.append("\n");
				sb.append("        return true;\n");
				sb.append("    }\n");
				

				sb.append("\n\n");				
				
				
				sb.append("		double dynamicThreshold(double previousConcentration) {\n");
				sb.append("    		double baseThreshold = 1e-3;\n");
				sb.append("    		return std::max(minDeltaThreshold, baseThreshold * fabs(previousConcentration));\n");
				sb.append("    		// return minDeltaThreshold;\n");
				sb.append("		}\n");


				sb.append("\n\n");				
								
				
				sb.append("    /**\n");
				sb.append("     * Checks for significant changes in metabolite concentrations and returns a set of changed metabolites.\n");
				sb.append("     * This set is used to determine which specific LP problems need to be updated based on the changed metabolites.\n");
				sb.append("     *\n");
				sb.append("     * @param Value Pointer to the array containing current metabolic values.\n");
				sb.append("     * @param NumPlaces Mapping of place names to their corresponding indices in the metabolic array.\n");
				sb.append("     * @return set<string> Set of metabolites that have undergone significant changes.\n");
				sb.append("     */\n");
				sb.append("    set<string> checkSignificantChange(double* Value, map<string, int>& NumPlaces, double time) {\n");
				sb.append("        reactionsToUpdate.clear();\n");
				sb.append("        set<string> changedMetabolites;\n");
				sb.append("\n");
				sb.append("        for (const auto& place : FBAplace) {\n");
				sb.append("            string metabolite = place.second;\n");
				sb.append("            double currentConcentration = trunc(Value[NumPlaces[metabolite]], decimalTrunc);\n");
				sb.append("            double previousConcentration = previousConcentrations[metabolite];\n");
				sb.append("            double change = fabs(currentConcentration - previousConcentration);\n");
				sb.append("           	//double threshold = minDeltaThreshold;\n");
				sb.append("            double percentChange = (previousConcentration != 0) ? (change / fabs(previousConcentration)) * 100.0 : 100.0;\n");
				sb.append("\n");
				sb.append("            bool condition1 = percentChange > epsilon;\n");
				sb.append("            // && change > threshold;\n");
				sb.append("            bool condition2 = previousConcentration == 0 && currentConcentration != 0;\n");
				sb.append("\n");
				sb.append("            if (condition1 || condition2) {\n");
				sb.append("                changedMetabolites.insert(metabolite);\n");
				sb.append("            }\n");
				sb.append("        }\n");
				sb.append("\n");
				sb.append("        return changedMetabolites;\n");
				sb.append("    }\n");
				

				sb.append("\n\n");				
						

				sb.append("    /**\n");
				sb.append("     * Truncates a double value to a specified number of decimal places.\n");
				sb.append("     * This method uses mathematical manipulation to achieve truncation without rounding.\n");
				sb.append("     * The value is scaled up by a power of ten corresponding to the desired number of decimal places,\n");
				sb.append("     * then truncated using floor to drop any fractional part, and finally scaled back down.\n");
				sb.append("     * This method ensures that the truncation is always downward, similar to how floor operates on positive numbers.\n");
				sb.append("     *\n");
				sb.append("     * @param value The double value to truncate.\n");
				sb.append("     * @param decimal The number of decimal places to retain in the truncated result.\n");
				sb.append("     * @return double The truncated value, preserving only the specified number of decimals.\n");
				sb.append("     * \n");
				sb.append("     * Example:\n");
				sb.append("     *    If value = 123.456789 and decimal = 3,\n");
				sb.append("     *    the result would be 123.456.\n");
				sb.append("     */\n");
				sb.append("    double trunc(double value, double decimal) {\n");
				sb.append("        const double multiplier = pow(10.0, decimal); // Calculate the scaling factor based on the number of decimals\n");
				sb.append("        return floor(value * multiplier) / multiplier; // Scale up, truncate, scale down\n");
				sb.append("        // Alternative implementation using type casting:\n");
				sb.append("        // return ((unsigned long int)(value * multiplier)) / multiplier; // Cast to integer to truncate\n");
				sb.append("    }\n");
				
				
				sb.append("\n\n");				
								
				sb.append("    void updateFluxBounds(\n");
				sb.append("        double *Value, \n");
				sb.append("        vector<class FBGLPK::LPprob>& vec_fluxb, \n");
				sb.append("        map<string, int>& NumPlaces, \n");
				sb.append("        set<string>& changedMetabolites,\n");
				sb.append("        double time\n");
				sb.append("    ) {\n");
				sb.append("        set<size_t> problemsToUpdate;\n");
				sb.append("\n");
				sb.append("        // (1) Costruiamo l'insieme dei problemIndex da aggiornare\n");
				sb.append("        for (const string& metabolite : changedMetabolites) {\n");
				sb.append("            if (metaboliteToProblems.find(metabolite) != metaboliteToProblems.end()) {\n");
				sb.append("                for (auto prob : metaboliteToProblems[metabolite]) {\n");
				sb.append("                    if (deadBacterialSpecies.find(prob) == deadBacterialSpecies.end() \n");
				sb.append("                        || !deadBacterialSpecies[prob]) {\n");
				sb.append("                        problemsToUpdate.insert(prob);\n");
				sb.append("                    }\n");
				sb.append("                }\n");
				sb.append("            }\n");
				sb.append("        }\n");
				sb.append("\n");
				sb.append("        // (2) Per ciascun problemIndex da aggiornare\n");
				sb.append("        for (auto problemIndex : problemsToUpdate) {\n");
				sb.append("\n");
				sb.append("            for (const string& reaction : reactionsToUpdate) {\n");
				sb.append("                int colIdx = vec_fluxb[problemIndex].fromNametoid(reaction);\n");
				sb.append("                if (colIdx == -1) {\n");
				sb.append("                    continue;\n");
				sb.append("                }\n");
				sb.append("\n");
				sb.append("                bool isReverse = false;\n");
				sb.append("                if (reaction.size() >= 2 && reaction.substr(reaction.size() - 2) == \"_f\") {\n");
				sb.append("                    isReverse = true;\n");
				sb.append("                }\n");
				sb.append("                if (!isReverse) {\n");
				sb.append("                    continue;\n");
				sb.append("                }\n");
				sb.append("\n");
				sb.append("                double conc = trunc(Value[NumPlaces.at(FBAplace[reaction])], decimalTrunc);\n");
				sb.append("\n");
				sb.append("        				 double fluxMaxCandidate = 1e9;\n");
				sb.append("        				 if (hasBioMASS && hasMultiSpecies) {\n");
				sb.append("            					calculateMultiplicativeConstant(Value, NumPlaces, problemIndex, reaction);\n");
				sb.append("            					double lambda = multiplicativeConstant;\n");
				sb.append("\n");
				sb.append("            					if (lambda == 0.0 || conc <= 0.0) {\n");
				sb.append("               				  fluxMaxCandidate = 0; // No flux if lambda or concentration is zero\n");
				sb.append("            					} else {\n");
				sb.append("                					fluxMaxCandidate = conc * lambda; // Dynamically scale with concentration and lambda\n");
				sb.append("            					}\n");
				sb.append("        					} else {\n");
				sb.append("            					if (conc > 0.0) {\n");
				sb.append("                					fluxMaxCandidate = conc; // Scale only with concentration\n");
				sb.append("            					} else {\n");
				sb.append("                					fluxMaxCandidate = 0; // No flux if concentration is zero\n");
				sb.append("            					}\n");
				sb.append("        					}\n");
				sb.append("\n");
				sb.append("\n");
				sb.append("        					double oldUb = vec_fluxb[problemIndex].getUpBounds(colIdx);\n");
				sb.append("        					if (conc > 0.0) {\n");
				sb.append("           				 if (oldUb == 0) {\n");
				sb.append("               				 oldUb = fluxMaxCandidate;\n");
				sb.append("            					} else {\n");
				sb.append("               				 oldUb = std::max(oldUb, fluxMaxCandidate);\n");
				sb.append("           				  }\n");
				sb.append("        					} else {\n");
				sb.append("           				 oldUb = 0;\n");
				sb.append("       					}\n");
				sb.append("\n");
				sb.append("\n");
				sb.append("        					double newUb = fluxMaxCandidate;\n");
				sb.append("\n");
				sb.append("        					double Lb = 0.0;\n");
				sb.append("        					if (newUb <= Lb) {\n");
				sb.append("            					vec_fluxb[problemIndex].update_bound(colIdx, \"GLP_FX\", Lb, Lb);\n");
				sb.append("        					} else {\n");
				sb.append("            					vec_fluxb[problemIndex].update_bound(colIdx, \"GLP_DB\", Lb, newUb);\n");
				sb.append("        					}\n");
				sb.append("            }\n");
				sb.append("        }\n");
				sb.append("    }\n");


				sb.append("\n\n");				

				sb.append("    /**\n");
				sb.append("     * Solves the FBA linear programming problems that are affected by significant changes in metabolite concentrations.\n");
				sb.append("     * This method iterates through a subset of LP problems identified by changed metabolites, solving them to find the optimal flux\n");
				sb.append("     * distributions under current constraints. It updates an array of variable pointers, storing the solution\n");
				sb.append("     * variables for each problem. This is crucial for subsequent steps in metabolic analysis where these\n");
				sb.append("     * solutions are used to compute metabolic flux rates.\n");
				sb.append("     * \n");
				sb.append("     * This method improves efficiency by focusing on problems affected by recent changes, ensuring that\n");
				sb.append("     * results are immediately accessible and the model is adjusted only as needed in response to\n");
				sb.append("     * changing conditions within the biological system.\n");
				sb.append("     * \n");
				sb.append("     * @param vec_fluxb Reference to a vector containing FBGLPK::LPprob objects.\n");
				sb.append("     * @param changedMetabolites Set of metabolites that have changed, used to identify relevant LP problems to solve.\n");
				sb.append("     */\n");
				sb.append("    void solveFBAProblems(vector<class FBGLPK::LPprob>& vec_fluxb, set<string>& changedMetabolites) {\n");
				sb.append("        set<size_t> toSolve;\n");
				sb.append("        for (const string& metabolite : changedMetabolites) {\n");
				sb.append("            if (metaboliteToProblems.find(metabolite) != metaboliteToProblems.end()) {\n");
				sb.append("                for (auto problemIndex : metaboliteToProblems[metabolite]) {\n");
				sb.append("                    if ((deadBacterialSpecies.find(problemIndex) == deadBacterialSpecies.end() || !deadBacterialSpecies[problemIndex])) {\n");
				sb.append("                        toSolve.insert(problemIndex);\n");
				sb.append("                    }\n");
				sb.append("                }\n");
				sb.append("            }\n");
				sb.append("        }\n");
				sb.append("\n");
				sb.append("        for (auto index : toSolve) {\n");
				sb.append("           // cout << \"Problema da risolvere ---> \" << index <<  \"   Name: \" << vec_fluxb[index].getFilename() << endl;\n");                
				sb.append("            vec_fluxb[index].solve();\n");
				sb.append("            // PARSIMONIUS FLAGS:\n");
				sb.append("            if(vec_fluxb[index].getPFbaFlag() != -1) performPFBA(vec_fluxb, index);\n");
				sb.append("            Vars[index] = vec_fluxb[index].getVariables(); // Update the variable pointers\n");
				sb.append("        }\n");
				sb.append("    }\n");
				

				sb.append("\n\n");								
				
				
				sb.append("    /**\n");
				sb.append("     * Updates the stored concentrations of metabolites based on the latest computational results.\n");
				sb.append("     * This method iterates over a map linking metabolite names to their indices in an array of current\n");
				sb.append("     * metabolic concentrations. It updates a map of previous concentrations with the current values,\n");
				sb.append("     * which is essential for tracking changes in metabolite levels over time and responding to dynamic\n");
				sb.append("     * metabolic conditions. This tracking supports the system's ability to determine significant changes\n");
				sb.append("     * in metabolic states that may require further adjustments in the model.\n");
				sb.append("     * \n");
				sb.append("     * By maintaining an up-to-date record of metabolite concentrations, this method ensures that\n");
				sb.append("     * the metabolic simulation reflects the most current state of the system, enabling accurate\n");
				sb.append("     * and timely decision-making in metabolic engineering and research.\n");
				sb.append("     * \n");
				sb.append("     * @param Value Pointer to an array containing the current concentrations of metabolites, indexed numerically.\n");
				sb.append("     * @param NumPlaces Map linking metabolite names to their respective indices in the Value array, used for direct access.\n");
				sb.append("     * @param changedMetabolites Set of metabolite names whose concentrations have changed significantly.\n");
				sb.append("     */\n");
				sb.append("    void updateConcentrations(double* Value, map<string, int>& NumPlaces, set<string>& changedMetabolites) {\n");
				sb.append("        for (const string& metabolite : changedMetabolites) {\n");
				sb.append("            int index = NumPlaces.at(metabolite);\n");
				sb.append("            double newConcentration = trunc(Value[index], decimalTrunc);\n");
				sb.append("            previousConcentrations[metabolite] = newConcentration; // Update the concentration record even if negative\n");
				sb.append("        }\n");
				sb.append("    }\n\n");


				sb.append("\n\n");								
							
				
				sb.append("    /**\n");
				sb.append("     * Updates the flux bounds and solves the FBA problems only for those metabolites that have undergone significant changes.\n");
				sb.append("     * This method first checks for significant changes in metabolite concentrations. If changes are detected,\n");
				sb.append("     * it updates the flux bounds for those metabolites, solves the corresponding FBA problems,\n");
				sb.append("     * and then updates the metabolite concentrations in the model.\n");
				sb.append("     *\n");
				sb.append("     * @param Value Pointer to an array containing current metabolic values.\n");
				sb.append("     * @param vec_fluxb Reference to a vector of FBGLPK::LPprob objects, each representing a distinct FBA problem.\n");
				sb.append("     * @param NumPlaces Mapping of place names to their indices in the metabolic array.\n");
				sb.append("     * @param time Current time to manage updates efficiently.\n");
				sb.append("     */\n");
				sb.append("    void updateFluxBoundsAndSolve(double* Value, vector<class FBGLPK::LPprob>& vec_fluxb, map<string, int>& NumPlaces, double time) {\n");
				sb.append("        set<string> changedMetabolites = checkSignificantChange(Value, NumPlaces, time);\n");
				sb.append("        if (!changedMetabolites.empty()) {\n");
				sb.append("            for (const auto& place : FBAplace) {\n");
				sb.append("                reactionsToUpdate.insert(place.first);\n");
				sb.append("            }\n");
				sb.append("\n");
				sb.append("            updateFluxBounds(Value, vec_fluxb, NumPlaces, changedMetabolites, time);\n");
				sb.append("            updateNonFBAReactionsUB(vec_fluxb, NumPlaces, Value);\n");
				sb.append("            solveFBAProblems(vec_fluxb, changedMetabolites);\n");
				sb.append("            updateConcentrations(Value, NumPlaces, changedMetabolites);\n");
				sb.append("            count += 1;\n");
				sb.append("        }\n");
				sb.append("    }\n");
				
				sb.append("\n\n");								

				sb.append("    /**\n");
				sb.append("     * Computes the metabolic rate for a given transition based on its position and the current metabolic state.\n");
				sb.append("     * The rate is calculated from the solution to the FBA LP problem associated with the transition.\n");
				sb.append("     * The rate is then adjusted based on whether the transition is an input or an output.\n");
				sb.append("     * \n");
				sb.append("     * @param vec_fluxb A vector containing all the FBA problems.\n");
				sb.append("     * @param NumPlaces A map linking metabolite names to their indices in the solution vector.\n");
				sb.append("     * @param NameTrans A vector containing the names of all transitions.\n");
				sb.append("     * @param Value The current state values of metabolites.\n");
				sb.append("     * @param T The index of the transition for which the rate is being computed.\n");
				sb.append("     * @param decimalTrunc The precision to which values are truncated.\n");
				sb.append("     * @param time The current simulation time.\n");
				sb.append("     * @return double The computed and adjusted metabolic rate for the transition.\n");
				sb.append("     */\n");
				sb.append("    double computeRate(vector<class FBGLPK::LPprob>& vec_fluxb, map<string, int>& NumPlaces, const vector<string>& NameTrans, const double* Value, const int T, const double decimalTrunc, const double& time) {\n");
				sb.append("        string transitionName = NameTrans[T];\n");
				sb.append("\n");
				sb.append("        int problemIndex = FBAproblems[transitionName];\n");
				sb.append("\n");
				sb.append("        int varIndex = vec_fluxb[problemIndex].fromNametoid(FBAreact[transitionName]);\n");
				sb.append("\n");
				sb.append("        double rate = Vars[problemIndex][varIndex];\n");
				sb.append("\n");
				sb.append("        double constant = 1.0;\n");
				sb.append("        double currentBiomass = 1.0;\n");
				sb.append("\n");
				sb.append("        if (hasBioMASS) {\n");
				sb.append("            currentBiomass = Value[NumPlaces.at(problemBiomassPlace.at(problemIndex))];\n");
				sb.append("        }\n");
				sb.append("\n");
				sb.append("        if (hasBioMASS && biomassTransitions.find(transitionName) != biomassTransitions.end()) {\n");
				sb.append("            //std::cout << \"[computeRate] Biomass Transition Detected for Transition: \" << transitionName << std::endl;\n");
				sb.append("            constant = floor(Value[NumPlaces.at(problemBacteriaPlace.at(problemIndex))]);\n");
				sb.append("            return rate;\n");
				sb.append("        } else if (hasMultiSpecies && hasBioMASS) {\n");
				sb.append("            constant = floor(Value[NumPlaces.at(problemBacteriaPlace.at(problemIndex))]);\n");
				sb.append("        }\n");
				sb.append("\n");
				sb.append("\n");
				sb.append("        if (hasBioMASS) {\n");
				sb.append("            rate = rate * (constant * currentBiomass * scalingFactor);\n");
				sb.append("        }\n");
				sb.append("\n");
				sb.append("        //cout << \"Ritorno rateo: \" << rate << endl;\n");
				sb.append("        return rate;\n");
				sb.append("    }\n");


				sb.append("\n\n");								


				sb.append("    /**\n");
				sb.append("     * Initializes the metabolic concentrations from provided values.\n");
				sb.append("     * This method initializes the concentrations for each metabolite based on the indices provided in a map.\n");
				sb.append("     * Each concentration is truncated to a specified number of decimal places before being set.\n");
				sb.append("     *\n");
				sb.append("     * @param Value Array of initial concentration values.\n");
				sb.append("     * @param NumPlaces Mapping from metabolite names to their indices in the Value array.\n");
				sb.append("     * @return void\n");
				sb.append("     */\n");
				sb.append("    void initializeConcentrations(double* Value, map<string, int>& NumPlaces) {\n");
				sb.append("        for (const auto& pair : FBAplace) {\n");
				sb.append("            string placeName = pair.second;\n");
				sb.append("            previousConcentrations[pair.second] = -1;\n");
				sb.append("        }\n");
				sb.append("    }\n");
				

				sb.append("\n\n");										
			
			
				sb.append("    /**\n");
				sb.append("     * Maps each metabolite to the set of LP problems that it affects.\n");
				sb.append("     * This method initializes the mapping from metabolites to their related LP problems,\n");
				sb.append("     * which is used to optimize updates when only certain metabolites change.\n");
				sb.append("     */\n");
				sb.append("    void mapMetabolitesToProblems() {\n");
				sb.append("        for (const auto& problem : FBAproblems) {\n");
				sb.append("            const string& reaction = FBAreact[problem.first];\n");
				sb.append("            if (FBAplace.find(reaction) != FBAplace.end()) {\n"); 
				sb.append("                size_t problemIndex = problem.second;\n");
				sb.append("                string places = FBAplace[reaction];\n");
				sb.append("                vector<string> metabolites = splitAndTrim(places, ',');\n");
				sb.append("                for (const string& metabolite : metabolites) {\n");
				sb.append("                    metaboliteToProblems[metabolite].insert(problemIndex);\n");
				sb.append("                }\n");
				sb.append("            }\n");
				sb.append("        }\n");
				sb.append("    }\n");


				sb.append("\n\n");								


				sb.append("    /**\n");
				sb.append("     * Initializes the data structures necessary for the FBA simulation process.\n");
				sb.append("     * This method is invoked only once per instance, conforming to the singleton design pattern of this class.\n");
				sb.append("     * It loads necessary data from a specified .fbainfo file into various structures for managing reactions,\n");
				sb.append("     * places, and problem indices, and initializes all metabolic concentrations to a default negative value.\n");
				sb.append("     * This ensures that any uninitialized metabolic state is clearly indicated and handled appropriately.\n");
				sb.append("     *\n");
				sb.append("     * @param NameTrans The names of the transitions involved in the network, used for initialization checks.\n");
				sb.append("     * @param vec_fluxb A reference to a vector of LP problems representing the metabolic fluxes.\n");
				sb.append("     * @param Value Pointer to an array storing the metabolic concentrations.\n");
				sb.append("     * @param NumPlaces A map linking place names to their corresponding indices.\n");
				sb.append("     * @return void\n");
				sb.append("     * @throws If the file cannot be read, it prints an error message and terminates initialization.\n");
				sb.append("     */\n");
				sb.append("    void init_data_structures_class(const vector<string>& NameTrans, vector<class FBGLPK::LPprob>& vec_fluxb, double* Value, map<string, int>& NumPlaces) {\n");
				sb.append("        if (!readFBAInfo(FBA_INFO_FILE, vec_fluxb)) { // Attempt to read initialization data from the .fbainfo file\n");
				sb.append("            cerr << \"Failed to read places from .fbainfo file\" << endl;\n");
				sb.append("            return; // Early return if file reading fails\n");
				sb.append("        }\n");
				sb.append("\n");
				sb.append("        Vars = new double*[vec_fluxb.size()];\n");
				sb.append("        initializeConcentrations(Value, NumPlaces);\n");
				sb.append("        mapMetabolitesToProblems();\n");
				sb.append("    }\n");


				sb.append("\n\n");								

				
				sb.append("		void mapReactionsFromProblems(const vector<class FBGLPK::LPprob>& vec_fluxb) {\n");
				sb.append("    		reactionToFileMap.clear(); // Clear the existing map\n");
				sb.append("    		for (const auto& index : problems) {\n");
				sb.append("        		const auto& lp = vec_fluxb[index];\n");
				sb.append("        		vector<string> reactions = lp.getReactions();\n");
				sb.append("        		for (const auto& reaction : reactions) {\n");
				sb.append("            		if (reaction.substr(0, 3) == \"EX_\") { // Check if reaction starts with 'EX_'\n");
				sb.append("                		reactionToFileMap[reaction].insert(index);\n");
				sb.append("            		}\n");
				sb.append("        		}\n");
				sb.append("    		}\n");
				sb.append("		}\n\n");
				
				sb.append("\n\n");								
									
				sb.append("    void updateNonFBAReactionUpperBoundsFromFile(\n");
				sb.append("        vector<FBGLPK::LPprob>& vec_fluxb,\n");
				sb.append("        const string& filename = \"EX_upper_bounds_nonFBA.csv\"\n");
				sb.append("    ) {\n");
				sb.append("        ifstream file(filename);\n");
				sb.append("        if (!file.is_open()) {\n");
				sb.append("            cerr << \"Warning: \" << filename << \" not found. Skipping updating NON-FBA reaction upper bounds.\" << endl;\n");
				sb.append("            return;\n");
				sb.append("        }\n");
				sb.append("\n");
				sb.append("        string line;\n");
				sb.append("        while (getline(file, line)) {\n");
				sb.append("            istringstream ss(line);\n");
				sb.append("            string reactionName;\n");
				sb.append("            vector<double> bounds;\n");
				sb.append("            string boundStr;\n");
				sb.append("\n");
				sb.append("            // 1) Extract the reaction name\n");
				sb.append("            getline(ss, reactionName, ',');\n");
				sb.append("\n");
				sb.append("            // 2) Extract the numeric fields\n");
				sb.append("            while (getline(ss, boundStr, ',')) {\n");
				sb.append("                bounds.push_back(stod(boundStr));\n");
				sb.append("            }\n");
				sb.append("\n");
				sb.append("            // 3) Check if the reaction exists in reactionToFileMap\n");
				sb.append("            auto it = reactionToFileMap.find(reactionName);\n");
				sb.append("            if (it != reactionToFileMap.end()) {\n");
				sb.append("                const auto& problemIndices = it->second;\n");
				sb.append("\n");
				sb.append("                // 4) Update the bounds\n");
				sb.append("                size_t i = 0;\n");
				sb.append("                for (auto indexIter = problemIndices.begin(); indexIter != problemIndices.end(); ++indexIter, ++i) {\n");
				sb.append("                    size_t problemIndex = *indexIter;\n");
				sb.append("                    int reactionId = vec_fluxb[problemIndex].fromNametoid(reactionName);\n");
				sb.append("                    if (reactionId == -1) {\n");
				sb.append("                        continue;\n");
				sb.append("                    }\n");
				sb.append("\n");
				sb.append("                    if (problemIndex >= bounds.size()) {\n");
				sb.append("                        continue;\n");
				sb.append("                    }\n");
				sb.append("\n");
				sb.append("                    // Use the correct value from bounds\n");
				sb.append("                    double newUb = bounds[problemIndex];\n");
				sb.append("\n");
				sb.append("                    // Retrieve the current lower bound\n");
				sb.append("                    double currentLb = vec_fluxb[problemIndex].getLwBounds(reactionId);\n");
				sb.append("\n");
				sb.append("                    // Update bounds\n");
				sb.append("                    vec_fluxb[problemIndex].update_bound(\n");
				sb.append("                        reactionId,\n");
				sb.append("                        \"GLP_DB\",\n");
				sb.append("                        trunc(currentLb, decimalTrunc),\n");
				sb.append("                        trunc(newUb, decimalTrunc)\n");
				sb.append("                    );\n");
				sb.append("                }\n");
				sb.append("            }\n");
				sb.append("        }\n");
				sb.append("\n");
				sb.append("        file.close();\n");
				sb.append("    }\n");


				
				sb.append("\n\n");	
				
				sb.append("    void loadAndApplyFBAReactionUpperBounds(\n");
				sb.append("        vector<FBGLPK::LPprob>& vec_fluxb,\n");
				sb.append("        const string& filename = \"EX_upper_bounds_FBA.csv\"\n");
				sb.append("    ) {\n");
				sb.append("        ifstream file(filename);\n");
				sb.append("        if (!file.is_open()) {\n");
				sb.append("            cerr << \"Warning: \" << filename << \" not found. Skipping updating FBA reaction upper bounds.\" << endl;\n");
				sb.append("            return;\n");
				sb.append("        }\n");
				sb.append("\n");
				sb.append("        string line;\n");
				sb.append("        while (getline(file, line)) {\n");
				sb.append("            istringstream ss(line);\n");
				sb.append("            string reaction;\n");
				sb.append("            vector<double> bounds;\n");
				sb.append("            string boundStr;\n");
				sb.append("\n");
				sb.append("            // 1) Extract the reaction name\n");
				sb.append("            getline(ss, reaction, ',');\n");
				sb.append("\n");
				sb.append("            // 2) Extract the numeric fields\n");
				sb.append("            while (getline(ss, boundStr, ',')) {\n");
				sb.append("                bounds.push_back(stod(boundStr));\n");
				sb.append("            }\n");
				sb.append("\n");
				sb.append("            // 3) Check if the reaction exists in reactionToFileMap\n");
				sb.append("            auto it = reactionToFileMap.find(reaction);\n");
				sb.append("            if (it != reactionToFileMap.end()) {\n");
				sb.append("                const auto& problemIndices = it->second;\n");
				sb.append("\n");
				sb.append("                // 4) Update the bounds\n");
				sb.append("                size_t i = 0;\n");
				sb.append("                for (auto idxIter = problemIndices.begin(); idxIter != problemIndices.end(); ++idxIter, ++i) {\n");
				sb.append("                    size_t problemIndex = *idxIter;\n");
				sb.append("                    int reactionId = vec_fluxb[problemIndex].fromNametoid(reaction);\n");
				sb.append("                    if (reactionId == -1) {\n");
				sb.append("                        continue;\n");
				sb.append("                    }\n");
				sb.append("\n");
				sb.append("                    if (problemIndex >= bounds.size()) {\n");
				sb.append("                        continue;\n");
				sb.append("                    }\n");
				sb.append("\n");
				sb.append("                    // Use the correct value from bounds\n");
				sb.append("                    double newUb = bounds[problemIndex];\n");
				sb.append("\n");
				sb.append("                    // Retrieve the current lower bound\n");
				sb.append("                    double currentLb = vec_fluxb[problemIndex].getLwBounds(reactionId);\n");
				sb.append("\n");
				sb.append("                    // Update bounds\n");
				sb.append("                    vec_fluxb[problemIndex].update_bound(\n");
				sb.append("                        reactionId,\n");
				sb.append("                        \"GLP_DB\",\n");
				sb.append("                        trunc(currentLb, decimalTrunc),\n");
				sb.append("                        trunc(newUb, decimalTrunc)\n");
				sb.append("                    );\n");
				sb.append("                }\n");
				sb.append("            }\n");
				sb.append("        }\n");
				sb.append("\n");
				sb.append("        file.close();\n");
				sb.append("    }\n");


				sb.append("\n\n");	
				
				sb.append("    void initializeNonFBAReactionsBaseUB(\n");
				sb.append("        vector<FBGLPK::LPprob>& vec_fluxb\n");
				sb.append("    ) {\n");
				sb.append("        // Per ciascuna reazione nel reactionToFileMap\n");
				sb.append("        for (const auto& reactionEntry : reactionToFileMap) {\n");
				sb.append("            const string& reaction = reactionEntry.first;\n");
				sb.append("\n");
				sb.append("            // Se è reazione FBA, salta\n");
				sb.append("            if (FBAreactions.find(reaction) != FBAreactions.end()) {\n");
				sb.append("                continue;\n");
				sb.append("            }\n");
				sb.append("\n");
				sb.append("            // Altrimenti, itera i vari problemIndex (specie) in cui questa reazione appare\n");
				sb.append("            const auto& problemIndices = reactionEntry.second;\n");
				sb.append("            for (size_t index : problemIndices) {\n");
				sb.append("                int reactionId = vec_fluxb[index].fromNametoid(reaction);\n");
				sb.append("                if (reactionId != -1) {\n");
				sb.append("                    double ub = vec_fluxb[index].getUpBounds(reactionId);\n");
				sb.append("\n");
				sb.append("                    NonFBAReactionBaseUB[ std::make_pair(reaction, index) ] = ub;\n");
				sb.append("\n");
				sb.append("                }\n");
				sb.append("            }\n");
				sb.append("        }\n");
				sb.append("    }\n");


				sb.append("\n\n");				
												
				
				sb.append("    void updateNonFBAReactionsUB(\n");
				sb.append("        vector<FBGLPK::LPprob>& vec_fluxb,\n");
				sb.append("        map<string, int>& NumPlaces,\n");
				sb.append("        double* Value\n");
				sb.append("    ) {\n");
				sb.append("        for (const auto& reactionEntry : reactionToFileMap) {\n");
				sb.append("            const string& reaction = reactionEntry.first;\n");
				sb.append("\n");
				sb.append("            // Ignora se reazione FBA\n");
				sb.append("            if (FBAreactions.find(reaction) != FBAreactions.end()) {\n");
				sb.append("                continue;\n");
				sb.append("            }\n");
				sb.append("\n");
				sb.append("            //cout << \"Debug aggiorno reazione: \" << reaction << endl;\n");
				sb.append("            const auto& problemIndices = reactionEntry.second;\n");
				sb.append("            for (size_t index : problemIndices) {\n");
				sb.append("                int reactionId = vec_fluxb[index].fromNametoid(reaction);\n");
				sb.append("                if (reactionId == -1) {\n");
				sb.append("                    continue;\n");
				sb.append("                }\n");
				sb.append("\n");
				sb.append("                string placeName = problemBacteriaPlace[index];\n");
				sb.append("                double pop = floor(Value[NumPlaces.at(placeName)]);\n");
				sb.append("\n");
				sb.append("                if (pop <= 0.0) {\n");
				sb.append("                    vec_fluxb[index].update_bound(\n");
				sb.append("                        reactionId, \"GLP_FX\",\n");
				sb.append("                        0.0, 0.0\n");
				sb.append("                    );\n");
				sb.append("                    continue;\n");
				sb.append("                }\n");
				sb.append("\n");
				sb.append("                double baseUb = NonFBAReactionBaseUB[std::make_pair(reaction, index)];\n");
				sb.append("\n");
				sb.append("                double newUb = baseUb / pop;\n");
				sb.append("\n");
				sb.append("                vec_fluxb[index].update_bound(\n");
				sb.append("                    reactionId, \"GLP_DB\",\n");
				sb.append("                    0.0, newUb\n");
				sb.append("                );\n");
				sb.append("            }\n");
				sb.append("        }\n");
				sb.append("    }\n");


				sb.append("\n\n");												
				
				sb.append("    void updateAllBiomassReactionsUpperBounds(double* Value, const map<string, int>& NumPlaces, vector<class FBGLPK::LPprob>& vec_fluxb) {\n");
				sb.append("        problemsWithLowBiomass.clear(); // Clear the low biomass problems map at the start of each update cycle\n");
				sb.append("        for (const auto& transition : biomassTransitions) {\n");
				sb.append("            size_t problemIndex = FBAproblems[transition];  // Get the problem index associated with the transition\n");
				sb.append("\n");
				sb.append("            string reaction = FBAreact[transition];\n");
				sb.append("\n");
				sb.append("            double BioMax = vec_fluxb[problemIndex].getBioMax();\n");
				sb.append("            double CurrentBiomass = trunc(Value[NumPlaces.at(problemBiomassPlace.at(problemIndex))], decimalTrunc);\n");
				sb.append("            double BioMin = vec_fluxb[problemIndex].getBioMin();\n");
				sb.append("\n");
				sb.append("            double Gamma = BioMax - CurrentBiomass;\n");
				sb.append("            //cout << \"Valore Gamma \" << Gamma << \" per il problema \" << problemIndex << endl;\n");
				sb.append("\n");
				sb.append("            double newUpperBound;\n");
				sb.append("            if (CurrentBiomass < BioMin) {\n");
				sb.append("                newUpperBound = Gamma; // Positive Gamma, set upper bound to Gamma\n");
				sb.append("                problemsWithLowBiomass[problemIndex] = true; // Mark this problem as having low biomass\n");
				sb.append("            } else if (Gamma > 0) {\n");
				sb.append("                newUpperBound = Gamma; // Positive Gamma, set upper bound to Gamma\n");
				sb.append("            } else {\n");
				sb.append("                newUpperBound = 1e-6; // Gamma not positive, set upper bound to small cutoff\n");
				sb.append("            }\n");
				sb.append("\n");
				sb.append("        			int index = vec_fluxb[problemIndex].getPFBA_index();\n");
				sb.append("            double currentLowerBound = vec_fluxb[problemIndex].getLwBounds(index);\n");
				sb.append("\n");
				sb.append("            vec_fluxb[problemIndex].update_bound(index, \"GLP_DB\", currentLowerBound, trunc(newUpperBound, decimalTrunc));\n");
				sb.append("        }\n");
				sb.append("    }\n");
				
/*			
				sb.append("void updateAllBiomassReactionsUpperBounds(double* Value, const std::map<std::string, int>& NumPlaces, std::vector<class FBGLPK::LPprob>& vec_fluxb) {\n");
				sb.append("    problemsWithLowBiomass.clear();\n");
				sb.append("    std::cout << \"[updateAllBiomassReactionsUpperBounds] Starting update of biomass reactions upper bounds.\" << std::endl;\n");
				sb.append("    for (const auto& transition : biomassTransitions) {\n");
				sb.append("        size_t problemIndex = FBAproblems[transition];\n");
				sb.append("        std::cout << \"[updateAllBiomassReactionsUpperBounds] Processing transition: \" << transition << \", problemIndex: \" << problemIndex << std::endl;\n");
				sb.append("        std::string reaction = FBAreact[transition];\n");
				sb.append("        std::cout << \"[updateAllBiomassReactionsUpperBounds] Reaction: \" << reaction << std::endl;\n");
				sb.append("        double BioMax_pgPerCell = vec_fluxb[problemIndex].getBioMax();\n");
				sb.append("        double BioMin_pgPerCell = vec_fluxb[problemIndex].getBioMin();\n");
				sb.append("        double currentBio_pgPerCell = trunc(Value[NumPlaces.at(problemBiomassPlace.at(problemIndex))], decimalTrunc);\n");
				sb.append("        std::cout << \"[updateAllBiomassReactionsUpperBounds] BioMax: \" << BioMax_pgPerCell << \", CurrentBiomass: \" << currentBio_pgPerCell << \", BioMin: \" << BioMin_pgPerCell << std::endl;\n");
				sb.append("        double Gamma_pgPerCell = BioMax_pgPerCell - currentBio_pgPerCell;\n");
				sb.append("        std::cout << \"[updateAllBiomassReactionsUpperBounds] Gamma (BioMax - CurrentBiomass): \" << Gamma_pgPerCell << \" (pg/cell)\" << std::endl;\n");
				sb.append("        double newUpperBound_pgPerCell;\n");
				sb.append("        if (currentBio_pgPerCell < BioMin_pgPerCell) {\n");
				sb.append("            problemsWithLowBiomass[problemIndex] = true;\n");
				sb.append("            newUpperBound_pgPerCell = Gamma_pgPerCell;\n");
				sb.append("            std::cout << \"[updateAllBiomassReactionsUpperBounds] CurrentBiomass < BioMin, newUpperBound_pgPerCell: \" << newUpperBound_pgPerCell << std::endl;\n");
				sb.append("        } else if (Gamma_pgPerCell > 0) {\n");
				sb.append("            newUpperBound_pgPerCell = Gamma_pgPerCell;\n");
				sb.append("            std::cout << \"[updateAllBiomassReactionsUpperBounds] Gamma > 0, newUpperBound_pgPerCell: \" << newUpperBound_pgPerCell << std::endl;\n");
				sb.append("        } else {\n");
				sb.append("            newUpperBound_pgPerCell = 1e-6;\n");
				sb.append("            std::cout << \"[updateAllBiomassReactionsUpperBounds] Gamma <= 0, newUpperBound_pgPerCell set to 1e-6.\" << std::endl;\n");
				sb.append("        }\n");
				sb.append("        double nBac = floor(Value[NumPlaces.at(problemBacteriaPlace.at(problemIndex))]);\n");
				sb.append("        std::cout << \"[DEBUG] nBac (number of bacteria): \" << nBac << std::endl;\n");
				sb.append("        double gamma_g = newUpperBound_pgPerCell * nBac * 1e-12;\n");
				sb.append("        std::cout << \"[DEBUG] gamma_g (total dry weight in grams): \" << gamma_g << std::endl;\n");
				sb.append("        double currentBio_g = currentBio_pgPerCell * nBac * 1e-12;\n");
				sb.append("        std::cout << \"[DEBUG] currentBio_g (current dry weight in grams): \" << currentBio_g << std::endl;\n");
				sb.append("        double gamma_mmol = gamma_g;\n");
				sb.append("        std::cout << \"[DEBUG] gamma_mmol (total mmol): \" << gamma_mmol << std::endl;\n");
				sb.append("        double currentBio_mmol = currentBio_g;\n");
				sb.append("        std::cout << \"[DEBUG] currentBio_mmol (current mmol): \" << currentBio_mmol << std::endl;\n");
				sb.append("        double newUpperBound_FBA = 1e-6;\n");
				sb.append("        if (gamma_mmol > 0.0 && currentBio_mmol > 1e-16) {\n");
				sb.append("            newUpperBound_FBA = gamma_mmol / currentBio_mmol;\n");
				sb.append("        }\n");
				sb.append("        std::cout << \"[DEBUG] newUpperBound_FBA (mmol/gDW/h): \" << newUpperBound_FBA << std::endl;\n");

				sb.append("        int index = vec_fluxb[problemIndex].getPFBA_index();\n");
				sb.append("        double currentLowerBound = vec_fluxb[problemIndex].getLwBounds(index);\n");
				sb.append("        std::cout << \"[updateAllBiomassReactionsUpperBounds] Reaction index: \" << index << \", currentLowerBound: \" << currentLowerBound << \", newUpperBound_FBA (mmol/gDW/h): \" << newUpperBound_FBA << std::endl;\n");
				sb.append("        double finalUB = trunc(newUpperBound_FBA, decimalTrunc);\n");
				sb.append("        vec_fluxb[problemIndex].update_bound(index, \"GLP_DB\", currentLowerBound, finalUB);\n");
				sb.append("        std::cout << \"[updateAllBiomassReactionsUpperBounds] Updated bounds for reaction: \" << reaction << \", newUpperBound (mmol/gDW/h): \" << finalUB << std::endl;\n");
				sb.append("    }\n");
				sb.append("    std::cout << \"[updateAllBiomassReactionsUpperBounds] Finished updating biomass reactions upper bounds.\" << std::endl;\n");
				sb.append("}\n");
*/
			/*
				sb.append("    void updateAllBiomassReactionsUpperBounds(double* Value, \n");
				sb.append("                                             const std::map<std::string, int>& NumPlaces, \n");
				sb.append("                                             std::vector<class FBGLPK::LPprob>& vec_fluxb) \n");
				sb.append("    {\n");
				sb.append("        problemsWithLowBiomass.clear(); \n");
				sb.append("        std::cout << \"[updateAllBiomassReactionsUpperBounds] Starting update of biomass reactions upper bounds.\" << std::endl;\n");
				sb.append("\n");
				sb.append("        for (const auto& transition : biomassTransitions) \n");
				sb.append("        {\n");
				sb.append("            size_t problemIndex = FBAproblems[transition];  \n");
				sb.append("            std::cout << \"[updateAllBiomassReactionsUpperBounds] Processing transition: \" \n");
				sb.append("                      << transition << \", problemIndex: \" << problemIndex << std::endl;\n");
				sb.append("\n");
				sb.append("            std::string reaction = FBAreact[transition];\n");
				sb.append("            std::cout << \"[updateAllBiomassReactionsUpperBounds] Reaction: \" << reaction << std::endl;\n");
				sb.append("\n");
				sb.append("            // Ottenere BioMax e BioMin in pg/cell\n");
				sb.append("            double BioMax_pgPerCell = vec_fluxb[problemIndex].getBioMax();  \n");
				sb.append("            double BioMin_pgPerCell = vec_fluxb[problemIndex].getBioMin();  \n");
				sb.append("\n");
				sb.append("            // Biomassa corrente in pg/cell\n");
				sb.append("            double currentBio_pgPerCell = trunc(Value[NumPlaces.at(problemBiomassPlace.at(problemIndex))], decimalTrunc);\n");
				sb.append("            std::cout << \"[updateAllBiomassReactionsUpperBounds] BioMax: \" << BioMax_pgPerCell \n");
				sb.append("                      << \", CurrentBiomass: \" << currentBio_pgPerCell \n");
				sb.append("                      << \", BioMin: \" << BioMin_pgPerCell << std::endl;\n");
				sb.append("\n");
				sb.append("            // Calcolo di Gamma in pg/cell\n");
				sb.append("            double Gamma_pgPerCell = BioMax_pgPerCell - currentBio_pgPerCell;\n");
				sb.append("            std::cout << \"[updateAllBiomassReactionsUpperBounds] Gamma (BioMax - CurrentBiomass): \" \n");
				sb.append("                      << Gamma_pgPerCell << \" (pg/cell)\" << std::endl;\n");
				sb.append("\n");
				sb.append("            double newUpperBound_FBA;\n");
				sb.append("        double nBac = floor(Value[NumPlaces.at(problemBacteriaPlace.at(problemIndex))]);\n");
				sb.append("\n");
				sb.append("            if (currentBio_pgPerCell < BioMin_pgPerCell) {\n");
				sb.append("                problemsWithLowBiomass[problemIndex] = true;\n");
				sb.append("                newUpperBound_FBA = 1e9; // Permette crescita massima\n");
				sb.append("                std::cout << \"[updateAllBiomassReactionsUpperBounds] CurrentBiomass < BioMin, setting newUpperBound_FBA to 1e9.\" \n");
				sb.append("                          << std::endl;\n");
				sb.append("            } \n");
				sb.append("            else if (Gamma_pgPerCell > 0) {\n");
				sb.append("                // Calcolo della biomassa residua totale in mmol\n");
				sb.append("                double remaining_biom_mmol = Gamma_pgPerCell * nBac * 1e-12; // pg -> mmol\n");
				sb.append("\n");
				sb.append("                // Biomassa corrente totale in mmol\n");
				sb.append("                double currentBiom_mmol = currentBio_pgPerCell * nBac * 1e-12; // pg -> mmol\n");
				sb.append("\n");
				sb.append("                if (currentBiom_mmol > 0) {\n");
				sb.append("                    newUpperBound_FBA = remaining_biom_mmol / (currentBiom_mmol); // mmol/gDW/h\n");
				sb.append("                }\n");
				sb.append("                else {\n");
				sb.append("                    newUpperBound_FBA = 1e9; // Se currentBiom_mmol ==0, setto mu_max\n");
				sb.append("                }\n");
				sb.append("\n");
				sb.append("                std::cout << \"[updateAllBiomassReactionsUpperBounds] Gamma > 0, setting newUpperBound_FBA to \" \n");
				sb.append("                          << newUpperBound_FBA << \" (Gamma_mmol / currentBiom_mmol).\" << std::endl;\n");
				sb.append("            } \n");
				sb.append("            else {\n");
				sb.append("                newUpperBound_FBA = 1e-6; // Disattiva crescita\n");
				sb.append("                std::cout << \"[updateAllBiomassReactionsUpperBounds] Gamma <= 0, setting newUpperBound_FBA to 1e-6.\" \n");
				sb.append("                          << std::endl;\n");
				sb.append("            }\n");
				sb.append("\n");
				sb.append("            // Recupera l'indice reazione ed il bound inferiore\n");
				sb.append("            int index = vec_fluxb[problemIndex].getPFBA_index(); \n");
				sb.append("            double currentLowerBound = vec_fluxb[problemIndex].getLwBounds(index);\n");
				sb.append("\n");
				sb.append("            // Aggiorna i bounds nel solver FBA \n");
				sb.append("            vec_fluxb[problemIndex].update_bound(index, \"GLP_DB\", currentLowerBound, newUpperBound_FBA);\n");
				sb.append("\n");
				sb.append("            std::cout << \"[updateAllBiomassReactionsUpperBounds] Updated bounds for reaction: \" \n");
				sb.append("                      << reaction << \", newUpperBound (mmol/gDW/h): \" << newUpperBound_FBA << std::endl;\n");
				sb.append("        }\n");
				sb.append("\n");
				sb.append("        std::cout << \"[updateAllBiomassReactionsUpperBounds] Finished updating biomass reactions upper bounds.\" << std::endl;\n");
				sb.append("    }\n");
				*/
				sb.append("\n\n");												
				
				sb.append("    void performPFBA(vector<class FBGLPK::LPprob>& vec_fluxb, int problemIndex) {\n");
				sb.append("        double optimalBiomass = vec_fluxb[problemIndex].getOBJ();\n");
				sb.append("        \n");
				sb.append("        int biomassIndex = vec_fluxb[problemIndex].getPFBA_index();\n");
				sb.append("        double originalLb = vec_fluxb[problemIndex].getLwBounds(biomassIndex);\n");
				sb.append("        double originalUb = vec_fluxb[problemIndex].getUpBounds(biomassIndex);\n");
				sb.append("        int originalType = vec_fluxb[problemIndex].get_bound_type(biomassIndex);\n");
				sb.append("        \n");
				sb.append("        // Prove debug metto un piccolo range\n");
				sb.append("        double eps = 1e-6;\n");
				sb.append("        double lb = optimalBiomass - eps;\n");
				sb.append("        double ub = optimalBiomass + eps;\n");				
				sb.append("        vec_fluxb[problemIndex].update_bound(biomassIndex, \"GLP_DB\", lb, ub);\n");
				sb.append("        \n");
				sb.append("        vec_fluxb[problemIndex].setMinimizeFluxObjective(biomassIndex);\n");
				sb.append("        \n");
				sb.append("        // Rieseguo il solver con la pFBA\n");
				sb.append("        //cout << \"Ora risolvo pFBA\" << endl;\n");
				sb.append("        vec_fluxb[problemIndex].solve();\n");
				sb.append("        //cout << \"RISOLTO pFBA\" << endl;\n");
				sb.append("        \n");
				sb.append("        // -- DEBUG 3: Stampo la situazione pFBA\n");
				sb.append("        //vec_fluxb[problemIndex].debugPFBA();\n");
				sb.append("        \n");
				sb.append("        // Step 3: Ripristino i bound originali\n");
				sb.append("        vec_fluxb[problemIndex].update_bound(biomassIndex, originalType, originalLb, originalUb);\n");
				sb.append("        \n");
				sb.append("        // Step 3.1: Torno a massimizzare la biomassa\n");
				sb.append("        vec_fluxb[problemIndex].resetMaximizationObjectiveForBiomass(biomassIndex);\n");
				sb.append("    }\n");

				sb.append("};\n");
				return sb.toString();
		}


}
