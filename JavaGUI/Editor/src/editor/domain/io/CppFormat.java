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
				sb.append("            updateNonFBAReactionUpperBoundsFromFile(vec_fluxb, \"EX_upper_bounds_nonFBA.csv\");\n");
				sb.append("            loadGeneRules(vec_fluxb, \"GeneRules.txt\");\n");				
				sb.append("            debugPrintGeneRules();\n");	
				sb.append("            firstTransitionName = NameTrans[T];\n");
				sb.append("            init = true;\n");
				sb.append("        }\n");
				sb.append("        if (firstTransitionName == NameTrans[T]) {\n");
				sb.append("            for (auto& problem : FBAproblems) {\n");
				sb.append("                if (hasMultiSpecies && floor(Value[NumPlaces.at(problemBacteriaPlace.at(problem.second))]) < 1) {\n");
				sb.append("                    deadBacterialSpecies[problem.second] = true;\n");
				sb.append("                }\n");
				sb.append("            }\n");
				sb.append("            if(hasBioMASS){\n");
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
				
				// ---------------------- Gene Regulation Rules ----------------------
				/**
				 * @brief Structure to hold gene regulation rule data.
				 *
				 * Each rule can have a time condition, a concentration condition (based on a metabolite),
				 * or both. Use "NA" in the input file to indicate that a field is not specified.
				 */
				sb.append("    struct GeneRule {\n");
				sb.append("        bool timeSpecified;       // true if a time condition is specified\n");
				sb.append("        double time;              // simulation time condition (only valid if timeSpecified is true)\n\n");
				sb.append("        bool placeSpecified;      // true if a metabolite condition is specified\n");
				sb.append("        std::string place;        // name of the metabolite (e.g., \"glc_D_e\")\n\n");
				sb.append("        bool thresholdSpecified;  // true if a threshold condition is specified\n");
				sb.append("        double threshold;         // concentration threshold (only valid if thresholdSpecified is true)\n\n");
				sb.append("        std::string compType;     // comparison type: \"min\", \"max\", or \"NA\"\n\n");
				sb.append("        std::string reactionID;   // reaction identifier (e.g., \"LACZ\")\n");
				sb.append("        double newLB;             // new lower bound for the reaction\n");
				sb.append("        double newUB;             // new upper bound for the reaction\n");
				sb.append("        std::string bacterium;    // bacterium name for which the rule is associated\n");
				sb.append("        size_t lpIndex;           // index in the vector of LP problems (default: numeric_limits<size_t>::max())\n");
				sb.append("        bool applied;             // flag to track if the rule has been applied\n");
				sb.append("    };\n\n");
				// Structure to maintain the correlation between gene and nonGene associated reactions

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
				
				sb.append("    /**\n");
				sb.append("     * Molecular Weight scaling factor.\n");
				sb.append("     */\n");				
				sb.append("    double Mw = 1;\n");				
				
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
				sb.append("     * Threshold for determining significant changes in metabolic concentrations, expressed as a absolute value.\n");
				sb.append("     */\n");
				sb.append("    double absEpsilon = g_absEpsilon; // Initially set to 0%, can be configured.\n");
				

				sb.append("    /**\n");
				sb.append("     * Threshold for determining significant changes in metabolic concentrations, expressed as a percentage.\n");
				sb.append("     */\n");				
				sb.append("    double relEpsilon = g_relEpsilon; // Initially set to 0%, can be configured.\n");
				
				
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
							
				sb.append("    /**\n");
				sb.append("     * Maps each metabolite to a set of problem indices that are affected by changes in this metabolite.\n");
				sb.append("     * This helps optimize the processing by updating only relevant LP problems when specific metabolite concentrations change.\n");
				sb.append("     */\n");
				sb.append("    unordered_map<string, set<size_t>> metaboliteToProblems;\n\n");
				
				// Gene Rules related structure:

				sb.append("    std::vector<GeneRule> geneRules;\n");
				sb.append("    bool geneRulesLoaded;\n\n");
				
				
				sb.append("    /**\n");
				sb.append("     * @brief Loads gene regulation rules from a file with a default name.\n");
				sb.append("     *        If the file is not present, the simulation continues without gene rules.\n");
				sb.append("     * @param vec_fluxb The vector of LP problems (used to map bacterium names to LP indices).\n");
				sb.append("     * @param filename The file name to load; default is \"GeneRules.txt\".\n");
				sb.append("     */\n");
				sb.append("    void loadGeneRules(const vector<class FBGLPK::LPprob>& vec_fluxb, const std::string &filename = \"GeneRules.txt\") {\n");
				sb.append("        std::ifstream infile(filename.c_str());\n");
				sb.append("        if (!infile) {\n");
				sb.append("            std::cout << \"Gene rules file '\" << filename \n");
				sb.append("                      << \"' not found. Continuing simulation without gene rules.\" << std::endl;\n");
				sb.append("            geneRulesLoaded = false;\n");
				sb.append("            return;\n");
				sb.append("        }\n");
				sb.append("        geneRules.clear();\n");
				sb.append("        std::string line;\n");
				sb.append("        while (std::getline(infile, line)) {\n");
				sb.append("            if (line.empty() || line[0] == '#') continue;\n");
				sb.append("            size_t commentPos = line.find('#');\n");
				sb.append("            if (commentPos != std::string::npos) line = line.substr(0, commentPos);\n");
				sb.append("            std::istringstream iss(line);\n");
				sb.append("            std::vector<std::string> tokens;\n");
				sb.append("            std::string token;\n");
				sb.append("            while (std::getline(iss, token, ',')) {\n");
				sb.append("                token.erase(0, token.find_first_not_of(\" \\t\\r\\n\"));\n");
				sb.append("                token.erase(token.find_last_not_of(\" \\t\\r\\n\") + 1);\n");
				sb.append("                if (!token.empty()) tokens.push_back(token);\n");
				sb.append("            }\n");
				sb.append("            if (tokens.size() < 7) {\n");
				sb.append("                std::cerr << \"Skipping invalid gene rule: \" << line << std::endl;\n");
				sb.append("                continue;\n");
				sb.append("            }\n");
				sb.append("            GeneRule rule;\n");
				sb.append("            if (tokens[0] == \"NA\") { rule.timeSpecified = false; } else { rule.timeSpecified = true; rule.time = std::stod(tokens[0]); }\n");
				sb.append("            if (tokens[1] == \"NA\") { rule.placeSpecified = false; } else { rule.placeSpecified = true; rule.place = tokens[1]; }\n");
				sb.append("            if (tokens[2] == \"NA\") { rule.thresholdSpecified = false; } else { rule.thresholdSpecified = true; rule.threshold = std::stod(tokens[2]); }\n");
				sb.append("            // tokens[3] is reactionID\n");
				sb.append("            rule.reactionID = tokens[3];\n");
				sb.append("            // tokens[4] is newLB, tokens[5] is newUB\n");
				sb.append("            rule.newLB = std::stod(tokens[4]);\n");
				sb.append("            rule.newUB = std::stod(tokens[5]);\n");
				sb.append("            // tokens[6] is bacterium name\n");
				sb.append("            rule.bacterium = tokens[6];\n");
				sb.append("            // If a comparison type is provided as an optional 7th token, use it; otherwise, set to \"NA\"\n");
				sb.append("            if (tokens.size() >= 8) {\n");
				sb.append("                rule.compType = tokens[7];\n");
				sb.append("            } else {\n");
				sb.append("                rule.compType = \"NA\";\n");
				sb.append("            }\n");
				sb.append("            // Determine the LP index for the bacterium; assume findLPIndex is defined\n");
				sb.append("            rule.lpIndex = findLPIndex(vec_fluxb, rule.bacterium);\n");
				sb.append("            if (rule.lpIndex == std::numeric_limits<size_t>::max()) {\n");
				sb.append("                std::cerr << \"Error: Bacterium '\" << rule.bacterium << \"' not found among LP problems.\" << std::endl;\n");
				sb.append("            }\n");
				sb.append("            // Set the applied flag to false initially\n");
				sb.append("            rule.applied = false;\n");
				sb.append("            geneRules.push_back(rule);\n");
				sb.append("        }\n");
				sb.append("        infile.close();\n");
				sb.append("        geneRulesLoaded = true;\n");
				sb.append("        std::cout << \"Loaded \" << geneRules.size() << \" gene rule(s) from \" << filename << std::endl;\n");
				sb.append("    }\n");
				// ---------------------- End Gene Regulation Rules ----------------------
				
				// ----------------------- Debug Gene Rules Function ----------------------
				sb.append("    void debugPrintGeneRules() {\n");
				sb.append("        std::cout << \"DEBUG: Loaded \" << geneRules.size() << \" gene rule(s):\" << std::endl;\n");
				sb.append("        for (size_t i = 0; i < geneRules.size(); ++i) {\n");
				sb.append("            const GeneRule &rule = geneRules[i];\n");
				sb.append("            std::cout << \"GeneRule[\" << i << \"]: \";\n");
				sb.append("            std::cout << \"Time = \";\n");
				sb.append("            if (rule.timeSpecified)\n");
				sb.append("                std::cout << rule.time;\n");
				sb.append("            else\n");
				sb.append("                std::cout << \"NA\";\n");
				sb.append("            std::cout << \", Place = \";\n");
				sb.append("            if (rule.placeSpecified)\n");
				sb.append("                std::cout << rule.place;\n");
				sb.append("            else\n");
				sb.append("                std::cout << \"NA\";\n");
				sb.append("            std::cout << \", Threshold = \";\n");
				sb.append("            if (rule.thresholdSpecified)\n");
				sb.append("                std::cout << rule.threshold;\n");
				sb.append("            else\n");
				sb.append("                std::cout << \"NA\";\n");
				sb.append("            std::cout << \", ReactionID = \" << rule.reactionID;\n");
				sb.append("            std::cout << \", newLB = \" << rule.newLB;\n");
				sb.append("            std::cout << \", newUB = \" << rule.newUB;\n");
				sb.append("            std::cout << \", Bacterium = \" << rule.bacterium;\n");
				sb.append("            std::cout << \", lpIndex = \" << rule.lpIndex;\n");
				sb.append("            std::cout << \", compType = \" << rule.compType;\n");
				sb.append("            std::cout << \", applied = \" << (rule.applied ? \"YES\" : \"NO\") << std::endl;\n");
				sb.append("        }\n");
				sb.append("    }\n");
				// ------------------- End Debug Gene Rules Function ----------------------


				// ---------------------- Gene Regulation Rule Application (Debug Version) ----------------------
				sb.append("    void applyGeneRegulationRules(double *Value, vector<class FBGLPK::LPprob>& vec_fluxb, map<string,int>& NumPlaces, double time) {\n");
				sb.append("        //std::cout << \"[DEBUG] Applying gene regulation rules at time \" << time << std::endl;\n");
				sb.append("        for (size_t i = 0; i < geneRules.size(); ++i) {\n");
				sb.append("            GeneRule &rule = geneRules[i];\n");
				sb.append("            // Skip rule if already applied\n");
				sb.append("            if (rule.applied) {\n");
				sb.append("                //std::cout << \"[DEBUG] GeneRule[\" << i << \"] already applied, skipping.\" << std::endl;\n");
				sb.append("                continue;\n");
				sb.append("            }\n");
				sb.append("           // std::cout << \"[DEBUG] Evaluating GeneRule[\" << i << \"]: Reaction = \" << rule.reactionID;\n");
				sb.append("            if (rule.timeSpecified) {\n");
				sb.append("                //std::cout << \", Time condition: current time (\" << time << \") >= \" << rule.time;\n");
				sb.append("            } else {\n");
				sb.append("                //std::cout << \", Time condition: NA\";\n");
				sb.append("            }\n");
				sb.append("            if (rule.placeSpecified && rule.thresholdSpecified) {\n");
				sb.append("                //std::cout << \", Place condition: \" << rule.place;\n");
				sb.append("                if (NumPlaces.find(rule.place) != NumPlaces.end()) {\n");
				sb.append("                    //int idx = NumPlaces[rule.place];\n");
				sb.append("                    //double conc = Value[idx];\n");
				sb.append("                    //std::cout << \" (current conc = \" << conc << \")\";\n");
				sb.append("                } else {\n");
				sb.append("                    //std::cout << \" (metabolite not found)\";\n");
				sb.append("                }\n");
				sb.append("            } else {\n");
				sb.append("               // std::cout << \", Place condition: NA\";\n");
				sb.append("            }\n");
				sb.append("            //std::cout << \", compType = \" << rule.compType << std::endl;\n");
				sb.append("\n");
				sb.append("            bool triggered = true;\n");
				sb.append("            if (rule.timeSpecified) {\n");
				sb.append("                triggered = triggered && (time >= rule.time);\n");
				sb.append("            }\n");
				sb.append("            if (rule.placeSpecified && rule.thresholdSpecified) {\n");
				sb.append("                if (NumPlaces.find(rule.place) != NumPlaces.end()) {\n");
				sb.append("                    int idx = NumPlaces[rule.place];\n");
				sb.append("                    double conc = Value[idx];\n");
				sb.append("                    if (rule.compType == \"min\") {\n");
				sb.append("                        triggered = triggered && (conc <= rule.threshold);\n");
				sb.append("                    } else if (rule.compType == \"max\") {\n");
				sb.append("                        triggered = triggered && (conc >= rule.threshold);\n");
				sb.append("                    } else {\n");
				sb.append("                        // Default behavior: use 'max' condition\n");
				sb.append("                        triggered = triggered && (conc >= rule.threshold);\n");
				sb.append("                    }\n");
				sb.append("                } else {\n");
				sb.append("                    triggered = false;\n");
				sb.append("                }\n");
				sb.append("            }\n");
				sb.append("            //std::cout << \"[DEBUG] GeneRule[\" << i << \"] triggered: \" << (triggered ? \"YES\" : \"NO\") << std::endl;\n");
				sb.append("            if (triggered) {\n");
				sb.append("                for (size_t j = 0; j < vec_fluxb.size(); ++j) {\n");
				sb.append("                    int colIdx = vec_fluxb[j].fromNametoid(rule.reactionID);\n");
				sb.append("                    if (colIdx != -1) {\n");
				sb.append("                        if (rule.newLB == rule.newUB) {\n");
				sb.append("                            vec_fluxb[j].update_bound(colIdx, \"GLP_FX\", rule.newLB, rule.newUB);\n");
				sb.append("                        } else {\n");
				sb.append("                            vec_fluxb[j].update_bound(colIdx, \"GLP_DB\", rule.newLB, rule.newUB);\n");
				sb.append("                        }\n");
				sb.append("                        std::cout << \"[DEBUG] Applied gene rule for reaction \" << rule.reactionID \n");
				sb.append("                                  << \" in LP problem \" << j << \" at time \" << time \n");
				sb.append("                                  << \" (new bounds: [\" << rule.newLB << \", \" << rule.newUB << \"])\" << std::endl;\n");
				sb.append("                    } else {\n");
				sb.append("                        std::cout << \"[DEBUG] Reaction \" << rule.reactionID \n");
				sb.append("                                  << \" not found in LP problem \" << j << std::endl;\n");
				sb.append("                    }\n");
				sb.append("                }\n");
				sb.append("                rule.applied = true;\n");
				sb.append("            }\n");
				sb.append("        }\n");
				sb.append("    }\n");
				// ---------------------- End Gene Regulation Rule Application (Debug Version) ----------------------


		
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
				sb.append("    //    - se ci sono input => '_r'\n");
				sb.append("    //    - se ci sono output => '_f'\n");
				sb.append("    if (!outputs.empty() && splittedF) {\n");
				sb.append("        //std::cerr << \"  [DEBUG splitted] '\" << reactionBase\n");
				sb.append("        //          << \"' => '_r' (ha outputPlaces)\\n\";\n");
				sb.append("        return reactionBase + \"_f\";\n");
				sb.append("    } else if (!inputs.empty() && splittedR) {\n");
				sb.append("        //std::cerr << \"  [DEBUG splitted] '\" << reactionBase\n");
				sb.append("        //          << \"' => '_r' (ha inputPlaces)\\n\";\n");
				sb.append("        return reactionBase + \"_r\";\n");
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
				sb.append("                    //std::cerr << \"[DEBUG] => '\" << transition << \"' è IsBiomass ma NON ha entrambi input e output => NON inserita in biomassTransitions.\\n\";\n");
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
				sb.append("    								const auto& forwardMap  = vec_fluxb[index].getForwardReactions();\n");
				sb.append("    								const auto& reverseMap  = vec_fluxb[index].getReverseReactions();\n");
				sb.append("    								const auto& irreversMap = vec_fluxb[index].getIrreversibileReactions();\n");
				sb.append("\n");
				sb.append("										//cerr << \"[DEBUG TRANSIZIONE] '\" << transition << endl;\n");
				sb.append("										  finalReaction = standardizeReactionName(\n");
				sb.append("    									transition,      \n");
				sb.append("    									reaction,        \n");
				sb.append("    									inputs,\n");
				sb.append("    									outputs,\n");
				sb.append("    									forwardMap,\n");
				sb.append("    									reverseMap,\n");
				sb.append("    									irreversMap,\n");
				sb.append("   	 								isBiomass\n");
			  sb.append("										);\n");
				sb.append("    								//cout << \"[DEBUG readFBAInfo] reactionBase='\" << reaction\n");
				sb.append("         				  //<< \"transition: \" << transition	<< \"' => finalReaction='\" << finalReaction << \"'\\n\";\n");
				sb.append("\n");
				sb.append("    								FBAreact[transition] = finalReaction;\n");
				sb.append("                   int checkReacId = vec_fluxb[index].fromNametoid(finalReaction);\n");
				sb.append("                   if (checkReacId == -1) {\n");
				sb.append("                   		cerr << \"[WARNING readFBAInfo] Reaction '\" << finalReaction\n");
				sb.append("                            << \"' was NOT found in LP index=\" << index << \" (file='\" << lpFile << \"').\\n\";\n");
				sb.append("        								return false;\n");
				sb.append("                   }\n");
				sb.append("\n");
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
				sb.append("            ReactionMultiplicity[transition] = multiplicity;\n");
				// Debug multiplicity
				sb.append("            //cout << \"For transition: \" << transition << \" multiplicity: \" << ReactionMultiplicity[transition] << endl; \n");
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
								
				sb.append("    set<string> checkSignificantChange(double* Value, map<string, int>& NumPlaces, double time) {\n");
				sb.append("        reactionsToUpdate.clear();\n");
				sb.append("        set<string> changedMetabolites;\n");
				sb.append("        double relChange = (relEpsilon > 100.0) ? 100.0 : ((relEpsilon < 0.0 && absEpsilon == -1) ? 0 : ((relEpsilon < 0.0 && absEpsilon != -1) ? -1 : relEpsilon));\n");
				sb.append("       //cout << \"[DEBUG] relChange: \" << relChange << endl;\n");
				sb.append("       // cout << \"[DEBUG] absChange: \" << absEpsilon << endl;\n");
				sb.append("        bool useAbsolute = (absEpsilon != -1);\n");
				sb.append("        bool useRelative = (relChange != -1);\n");
				sb.append("        for (const auto& place : FBAplace) {\n");
				sb.append("            string metabolite = place.second;\n");
				sb.append("            double currentConcentration = trunc(Value[NumPlaces[metabolite]], decimalTrunc);\n");
				sb.append("            double previousConcentration = previousConcentrations[metabolite];\n");
				sb.append("            double absoluteDiff = fabs(currentConcentration - previousConcentration);\n");
				sb.append("            double percentChange = (previousConcentration != 0)\n");
				sb.append("                ? (absoluteDiff / fabs(previousConcentration)) * 100.0\n");
				sb.append("                : 100.0;\n");
				sb.append("            bool condAbsolute = useAbsolute && (absoluteDiff > absEpsilon);\n");
				sb.append("            bool condRelative = useRelative && (percentChange > relChange);\n");
				sb.append("            bool condFromZero = useRelative && (previousConcentration == 0 && currentConcentration != 0);\n");
				sb.append("            bool condFirstSol = (previousConcentration == -1);\n");
				sb.append("            if (condAbsolute || condRelative || condFromZero || condFirstSol) {\n");
				sb.append("                /*cout << \"[DEBUG] Metabolite '\" << metabolite\n");
				sb.append("                     << \"' changed. Old: \" << previousConcentration\n");
				sb.append("                     << \", New: \" << currentConcentration\n");
				sb.append("                     << \", Diff: \" << (currentConcentration - previousConcentration)\n");
				sb.append("                     << \", AbsDiff: \" << absoluteDiff\n");
				sb.append("                     << \", PercentChange: \" << percentChange << \"%\"\n");
				sb.append("                     << \", condAbsolute: \" << condAbsolute\n");
				sb.append("                     << \", condRelative: \" << condRelative\n");
				sb.append("                     << \", condFromZero: \" << condFromZero << endl;*/\n");
				sb.append("                changedMetabolites.insert(metabolite);\n");
				sb.append("            }\n");
				sb.append("        }\n");
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
				sb.append("                if (reaction == \"EX_biomass_e_f\" || reaction == \"EX_biomass_e_r\") {\n");
				sb.append("                    continue; // do NOT update dynamic bounds for biomass reaction\n");
				sb.append("                }\n");
				sb.append("                bool isReverse = false;\n");
				sb.append("                if (reaction.size() >= 2 && reaction.substr(reaction.size() - 2) == \"_r\") {\n");
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
				sb.append("            //cout << \"Problema da risolvere ---> \" << index <<  \"   Name: \" << vec_fluxb[index].getFilename() << endl;\n");                
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
				sb.append("            applyGeneRegulationRules(Value, vec_fluxb, NumPlaces, time);\n");
				sb.append("            updateFluxBounds(Value, vec_fluxb, NumPlaces, changedMetabolites, time);\n");
				sb.append("            updateNonFBAReactionsUB(vec_fluxb, NumPlaces, Value);\n");
			//	sb.append("            cout << \"Mi preparo a risolvere per la: \" << count << endl;\n");
				sb.append("            solveFBAProblems(vec_fluxb, changedMetabolites);\n");
								//sb.append("            cout << \"Ho risolto per la: \" << count << endl;\n");
				sb.append("            updateConcentrations(Value, NumPlaces, changedMetabolites);\n");
				sb.append("            count += 1;\n");
				sb.append("            //cout << \"risolvo per la: \" << count << endl;\n");
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
				sb.append("       // int tmpProva = vec_fluxb[problemIndex].fromNametoid(\"biomass525\");\n");
				sb.append("        //double rateTmp = Vars[problemIndex][tmpProva];\n");
				sb.append("        //std::cout << \"[tmpProva] biomass525 rate : \" << rateTmp <<  std::endl;\n");
				sb.append("\n");
				sb.append("        double rate = Vars[problemIndex][varIndex];\n");
				sb.append("\n");
				sb.append("        double constant = 1.0;\n");
				sb.append("        double currentBiomass = 1.0;\n");
				sb.append("\n");
				sb.append("        double scalingMeasure = ReactionMultiplicity[transitionName];\n");
				sb.append("        //cout << \" [Debug Transition Rate]: \" << transitionName << \"Scaling Factor for measure unit: \" <<  scalingMeasure << endl;\n");
				sb.append("\n");
				sb.append("        if(hasBioMASS){\n");
				sb.append("            currentBiomass = Value[NumPlaces.at(problemBiomassPlace.at(problemIndex))];\n");
				sb.append("        }\n");
				sb.append("\n");
				sb.append("        if (hasBioMASS && biomassTransitions.find(transitionName) != biomassTransitions.end()) {\n");
				sb.append("           // std::cout << \"[computeRate] Biomass Transition Detected for Transition : \" << transitionName << \" rateo: \" << rate << std::endl;\n");
				sb.append("            constant = floor(Value[NumPlaces.at(problemBacteriaPlace.at(problemIndex))]);\n");
				sb.append("            return rate * scalingMeasure;\n");
				sb.append("        } else if (hasMultiSpecies && hasBioMASS) {\n");
				sb.append("            constant = floor(Value[NumPlaces.at(problemBacteriaPlace.at(problemIndex))]);\n");
				sb.append("        }\n");
				sb.append("\n");
				sb.append("\n");
				sb.append("        if(hasBioMASS) {\n");
				sb.append("           // std::cout << \"[computeRate] Other rate Transition Detected for Transition : \" << transitionName << \" rateo: \" << rate << std::endl;\n");
				sb.append("            rate = rate * (constant * currentBiomass * scalingFactor) * scalingMeasure;\n");
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
				sb.append("            exit(1); // Early return if file reading fails\n");
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
				sb.append("            cerr << \"Warning: \" << filename << \" not found. \"\n");
				sb.append("                 << \"Using original model's bounds as default base UB.\" << endl;\n");
				sb.append("\n");
				sb.append("            // Se il file non viene trovato, popola NonFBAReactionBaseUB con i bound attuali del modello per le reazioni non-FBA.\n");
				sb.append("            for (const auto& rxnEntry : reactionToFileMap) {\n");
				sb.append("                const string& rxn = rxnEntry.first;\n");
				sb.append("                if (FBAreactions.find(rxn) != FBAreactions.end()) {\n");
				sb.append("                    continue;\n");
				sb.append("                }\n");
				sb.append("                const auto& problemIndices = rxnEntry.second;\n");
				sb.append("                for (size_t idx : problemIndices) {\n");
				sb.append("                    int reactionId = vec_fluxb[idx].fromNametoid(rxn);\n");
				sb.append("                    if (reactionId == -1) {\n");
				sb.append("                        continue; // Reaction not found in this LP\n");
				sb.append("                    }\n");
				sb.append("                    double originalUb = vec_fluxb[idx].getUpBounds(reactionId);\n");
				sb.append("                    cout << \"reaction: \" << rxn << \" original bound: \" << originalUb << endl;\n");
				sb.append("                    NonFBAReactionBaseUB[std::make_pair(rxn, idx)] = originalUb;\n");
				sb.append("                }\n");
				sb.append("            }\n");
				sb.append("            return;\n");
				sb.append("        }\n");
				sb.append("\n");
				sb.append("        vector<string> lines;\n");
				sb.append("        {\n");
				sb.append("            string line;\n");
				sb.append("            while (getline(file, line)) {\n");
				sb.append("                lines.push_back(line);\n");
				sb.append("            }\n");
				sb.append("        }\n");
				sb.append("        file.close();\n");
				sb.append("\n");
				sb.append("        if (lines.empty()) {\n");
				sb.append("            return;\n");
				sb.append("        }\n");
				sb.append("\n");
				sb.append("        double baseUB = 0.0;\n");
				sb.append("        {\n");
				sb.append("            istringstream ss(lines[0]);\n");
				sb.append("            string header;\n");
				sb.append("            getline(ss, header, ','); // legge \"base_upper_bounds\"\n");
				sb.append("            string valueStr;\n");
				sb.append("            getline(ss, valueStr, ',');\n");
				sb.append("            baseUB = stod(valueStr);\n");
				sb.append("        }\n");
				sb.append("\n");
				sb.append("        for (size_t lineIdx = 1; lineIdx < lines.size(); ++lineIdx) {\n");
				sb.append("            istringstream ss(lines[lineIdx]);\n");
				sb.append("            string reactionName;\n");
				sb.append("            getline(ss, reactionName, ',');\n");
				sb.append("\n");
				sb.append("            if (FBAreactions.find(reactionName) != FBAreactions.end()) {\n");
				sb.append("                continue;\n");
				sb.append("            }\n");
				sb.append("\n");
				sb.append("            string newUbStr;\n");
				sb.append("            getline(ss, newUbStr, ',');\n");
				sb.append("            double newUb = stod(newUbStr);\n");
				sb.append("\n");
				sb.append("            auto it = reactionToFileMap.find(reactionName);\n");
				sb.append("            if (it != reactionToFileMap.end()) {\n");
				sb.append("                const auto& problemIndices = it->second;\n");
				sb.append("                for (size_t idx : problemIndices) {\n");
				sb.append("                    int reactionId = vec_fluxb[idx].fromNametoid(reactionName);\n");
				sb.append("                    if (reactionId == -1) {\n");
				sb.append("                        continue;");
				sb.append("                    }\n");
				sb.append("\n");
				sb.append("                    NonFBAReactionBaseUB[std::make_pair(reactionName, idx)] = baseUB;\n");
				sb.append("\n");
				sb.append("                    double currentLb = vec_fluxb[idx].getLwBounds(reactionId);\n");
				sb.append("                    vec_fluxb[idx].update_bound(\n");
				sb.append("                        reactionId,\n");
				sb.append("                        \"GLP_DB\",\n");
				sb.append("                        trunc(currentLb, decimalTrunc),\n");
				sb.append("                        trunc(newUb, decimalTrunc)\n");
				sb.append("                    );\n");
				sb.append("\n");
				sb.append("                   // debug cout << \"Updating reaction: \" << reactionName << \" with new bound: \" << newUb << endl;\n");
				sb.append("                }\n");
				sb.append("            }\n");
				sb.append("        }\n");
				sb.append("    }\n");





				sb.append("\n\n");	

				
				sb.append("    void loadAndApplyFBAReactionUpperBounds(\n");
				sb.append("        vector<FBGLPK::LPprob>& vec_fluxb,\n");
				sb.append("        const string& filename = \"EX_upper_bounds_FBA.csv\"\n");
				sb.append("    ) {\n");
				sb.append("        ifstream file(filename);\n");
				sb.append("        if (!file.is_open()) {\n");
				sb.append("            cerr << \"[Warning] : \" << filename << \" not found. Skipping updating FBA reaction upper bounds.\" << endl;\n");
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


    sb.append("    void updateNonFBAReactionsUB(\n");
    sb.append("        vector<FBGLPK::LPprob>& vec_fluxb,\n");
    sb.append("        map<string, int>& NumPlaces,\n");
    sb.append("        double* Value\n");
    sb.append("    ) {\n");
    sb.append("        for (const auto& reactionEntry : reactionToFileMap) {\n");
    sb.append("            const string& reaction = reactionEntry.first;\n");
    sb.append("\n");
    sb.append("            // Skip if it's an FBA reaction\n");
    sb.append("            if (FBAreactions.find(reaction) != FBAreactions.end()) {\n");
    sb.append("                continue;\n");
    sb.append("            }\n");
    sb.append("\n");
    sb.append("            // All non-FBA reactions\n");
    sb.append("            const auto& problemIndices = reactionEntry.second;\n");
    sb.append("            for (size_t index : problemIndices) {\n");
    sb.append("                int reactionId = vec_fluxb[index].fromNametoid(reaction);\n");
    sb.append("                if (reactionId == -1) {\n");
    sb.append("                    continue; // reaction not found in this LP\n");
    sb.append("                }\n");
    sb.append("\n");
    sb.append("                // Population, biomass, etc.\n");
    sb.append("                string placeName = problemBacteriaPlace[index];\n");
    sb.append("                double population = floor(Value[NumPlaces.at(placeName)]);\n");
    sb.append("                double biomass    = trunc(Value[NumPlaces.at(problemBiomassPlace.at(index))], decimalTrunc);\n");
    sb.append("\n");
    sb.append("                // If no population => set reaction to 0.\n");
    sb.append("                if (population <= 0.0) {\n");
    sb.append("                    vec_fluxb[index].update_bound(\n");
    sb.append("                        reactionId, \"GLP_FX\", 0.0, 0.0\n");
    sb.append("                    );\n");
    sb.append("                    continue;\n");
    sb.append("                }\n");
    sb.append("\n");
    sb.append("                // Attempt to find a baseUb in NonFBAReactionBaseUB.\n");
    sb.append("                auto mapKey = std::make_pair(reaction, index);\n");
    sb.append("                auto it = NonFBAReactionBaseUB.find(mapKey);\n");
    sb.append("                if (it == NonFBAReactionBaseUB.end()) {\n");
    sb.append("                    // Key not in map => do NOT update bounds, keep the model's original.\n");
    sb.append("                    continue;\n");
    sb.append("                }\n");
    sb.append("\n");
    sb.append("                // Otherwise, we have a baseUb from the map.\n");
    sb.append("                double baseUb = it->second;\n");
    sb.append("                double newUb  = baseUb / ((population * biomass) / Mw);\n");
    sb.append("\n");
    // Debug: Stampa il nome della reazione e il nuovo upper bound calcolato
  //sb.append("               // cout << \"baseUb: \" << baseUb << \" population: \" << population << \"biomass: \" << biomass << endl;\n");
  //sb.append("               // cout << \"Updating reaction: \" << reaction << \" with new computed UB: \" << newUb << endl;\n");
    sb.append("\n");
    sb.append("                // Update the GLPK problem with newUb\n");
    sb.append("                vec_fluxb[index].update_bound(\n");
    sb.append("                    reactionId, \"GLP_DB\", 0.0, newUb\n");
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
