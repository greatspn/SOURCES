
#define FBA_INFO_FILE "${FBA_INFO_FILE}"

/**FBAProcessor Class Definition**/


/**
 * @class FBAProcessor
 * @brief A singleton class that manages flux balance analysis processes.
 *
 * The FBAProcessor class is designed using the Singleton design pattern to ensure
 * that only one instance of this class is created throughout the lifetime of the application.
 * This class handles the initialization, management, and computation of flux balance analysis
 * based on metabolic reaction data.
 */
class FBAProcessor {
		public:
    /**
     * @brief Retrieves the single instance of the FBAProcessor class.
     * 
     * This method ensures that the FBAProcessor class is instantiated only once
     * and the same instance is returned with every call. The instance is guaranteed
     * to be destroyed only when the application exits, ensuring controlled and predictable
     * lifecycle management of the singleton resource.
     * 
     * @return Reference to the singleton FBAProcessor instance.
     */
    static FBAProcessor& getInstance() {
        static FBAProcessor instance; // Static variable ensures that the instance is created only once.
        return instance;
    }

    /**
     * @brief Deletes the copy constructor to prevent copying of the singleton instance.
     */
    FBAProcessor(FBAProcessor const&) = delete;
    /**
     * @brief Deletes the assignment operator to prevent assignment and copying of the singleton instance.
     */
    void operator=(FBAProcessor const&) = delete;



    /**
     * @brief Processes metabolic changes and calculates the rate of a specified transition.
     * 
     * This method serves as the primary access point for external components to interact
     * with the FBAProcessor. It ensures that the system is initialized before proceeding
     * with updates and calculations. The method handles the following tasks:
     * - Initializes the data structures if not already initialized.
     * - Updates the bounds based on current metabolic values and solves the LP problems.
     * - Computes the rate of the specified transition based on the new LP solution.
     * 
     * @param Value Pointer to the array containing current metabolic values.
     * @param vec_fluxb Reference to a vector of LPprob objects representing flux balance problems.
     * @param NumTrans Mapping of transition names to their corresponding indices.
     * @param NumPlaces Mapping of place names to their corresponding indices in the metabolic array.
     * @param NameTrans Vector containing names of transitions, indexed by transition IDs.
     * @param Trans Pointer to the structure containing transition information (not used directly in this method but may be used for extending functionality).
     * @param T The index of the transition for which the rate is to be calculated.
     * @param time The current simulation time, used to check if updates are needed based on time-driven changes.
     * @return The rate of the specified transition after processing the metabolic changes.
     * This rate could be used to determine the speed or likelihood of the transition occurring.
     */
    double process(double *Value, vector<class FBGLPK::LPprob>& vec_fluxb, map<string,int>& NumTrans, map<string,int>& NumPlaces, const vector<string>& NameTrans, const struct InfTr* Trans, int T, const double& time) {
        if (!init) {
            init_data_structures_class(NameTrans, vec_fluxb, Value, NumPlaces); // Ensure it is initialized before proceeding
            mapReactionsFromProblems(vec_fluxb);
            loadAndApplyFBAReactionUpperBounds(vec_fluxb, "EX_upper_bounds_FBA.csv");
            updateNonFBAReactionUpperBoundsFromFile(vec_fluxb, "EX_upper_bounds_nonFBA.csv");
            loadGeneRules(vec_fluxb, "GeneRules.txt");
            debugPrintGeneRules();
            firstTransitionName = NameTrans[T];
            init = true;
        }
        if (firstTransitionName == NameTrans[T]) {
            for (auto& problem : FBAproblems) {
                if (hasMultiSpecies && floor(Value[NumPlaces.at(problemBacteriaPlace.at(problem.second))]) < 1) {
                    deadBacterialSpecies[problem.second] = true;
                }
            }
            if(hasBioMASS){
            		updateAllBiomassReactionsUpperBounds(Value, NumPlaces, vec_fluxb); // Update biomass upper limits
            }
            updateFluxBoundsAndSolve(Value, vec_fluxb, NumPlaces, time); // Update fluxes only on first transition
        }
        double rate = 0;
        size_t problemIndex = FBAproblems[NameTrans[T]];
        // Return zero rate if the species for this transition is dead
        if (deadBacterialSpecies.find(problemIndex) != deadBacterialSpecies.end()) {
            return 0; // Return zero rate for dead species
        }

        // Check for transitions without associated file or specific biomass transitions.
        if (transitionsWithoutFile.find(NameTrans[T]) != transitionsWithoutFile.end()) {
            return rate; // Skip further processing for these cases
        }

        // Default computation if none of the above conditions met.
        rate = computeRate(vec_fluxb, NumPlaces, NameTrans, Value, T, decimalTrunc, time); // Compute and return the rate for the given transition
        return rate; // Return the calculated rate
    }



		private:
    struct GeneRule {
        bool timeSpecified;       // true if a time condition is specified
        double time;              // simulation time condition (only valid if timeSpecified is true)

        bool placeSpecified;      // true if a metabolite condition is specified
        std::string place;        // name of the metabolite (e.g., "glc_D_e")

        bool thresholdSpecified;  // true if a threshold condition is specified
        double threshold;         // concentration threshold (only valid if thresholdSpecified is true)

        std::string compType;     // comparison type: "min", "max", or "NA"

        std::string reactionID;   // reaction identifier (e.g., "LACZ")
        double newLB;             // new lower bound for the reaction
        double newUB;             // new upper bound for the reaction
        std::string bacterium;    // bacterium name for which the rule is associated
        size_t lpIndex;           // index in the vector of LP problems (default: numeric_limits<size_t>::max())
        bool applied;             // flag to track if the rule has been applied
    };

    struct PairHash {
        size_t operator()(const std::pair<std::string, size_t>& p) const {
            // Combino gli hash
            // (puoi usare qualunque formula di combinazione)
            auto h1 = std::hash<std::string>()(p.first);
            auto h2 = std::hash<size_t>()(p.second);
            // Semplice combiner stile boost::hash_combine
            h1 ^= (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
            return h1;
        }
    };
    // Se vuoi un comparator personalizzato (ma di solito la pair ha già operator==)
    struct PairEq {
        bool operator()(const std::pair<std::string, size_t>& a,
                        const std::pair<std::string, size_t>& b) const {
            return (a.first == b.first && a.second == b.second);
        }
    };
     set<string> reactionsToUpdate;
     string firstTransitionName = ""; // Tracks problems associated with dead bacterial species
     unordered_map<size_t, bool> deadBacterialSpecies; // Tracks problems associated with dead bacterial species
    /**
     * comment.
     */
     unordered_map<size_t, bool> problemsWithLowBiomass; // Tracks problems with biomass below the minimum threshold
			const double Lcutoff = 1e-6; // Define a small cutoff value for limit the biomass upper bound
    /**
     * Unordered set to store transitions identified as related to biomass.
     */
     unordered_set<string> biomassTransitions;
			unordered_set<string> transitionsWithoutFile;
    std::unordered_map<
        std::pair<std::string, size_t>,
        double,
        PairHash,
        PairEq
    > NonFBAReactionBaseUB;
    unordered_map<string, unordered_set<size_t>> reactionToFileMap; // Maps each reaction to a set of files where it appears

    unordered_map<int, double> bacteriaToBioMin; // Maps each reaction to a set of files where it appears

    unordered_map<int, double> bacteriaToBioMax; // Maps each reaction to a set of files where it appears

    unordered_map<int,double> bacteriaToBioMean; // Maps each reaction to a set of files where it appears

    /**
     * Multiplicative constant for the reaction.
     */
    double multiplicativeConstant = 1;

    /**
     * Comment.
     */
    bool hasMultiSpecies = false;

    /**
     * Comment.
     */
    double hasBioMASS = false;

    /**
     * Maps each transition to its corresponding reaction. Used for linking network transitions to specific biochemical reactions.
     */
    unordered_map<string, string> FBAreact;
    /**
     * Stores mappings from reactions to their places. This helps in maintaining the state of metabolic concentrations involved in reactions.
     */
    unordered_map<string, string> FBAplace;
    /**
     * Maps each reaction to an index of the corresponding LP problem in a vector. This facilitates quick access to the problem related to a particular reaction.
     */
    unordered_map<string, size_t> FBAproblems;
    /**
     * Maps each LP problem index to a set of reactions associated with that problem.
     * This helps in managing and optimizing the parallel processing of flux balance analysis problems,
     * ensuring that all reactions associated with a particular problem are processed together.
     */
    unordered_map<size_t, set<string>> problemsToReactions;
    /**
     * A set of all reactions that are part of the FBA model. Helps in ensuring that each reaction is processed uniquely.
     */
    set<string> FBAreactions;
    /**
     * A set of all problem index that are part of the FBA model.
     */
    set<size_t> problems;
    /**
     * Initialization status flag. Used to ensure that the FBAProcessor is set up before any operations are performed.
     */
    bool init;
    /**
     * Precision setting for numerical operations, specifically truncating numbers to a fixed number of decimal places.
     */
    const double decimalTrunc = 16;
    /**
     * Stores the last time the FBA model was updated, used to prevent unnecessary recalculations within the same time frame.
     */
     double FBAtime = -1;
    /**
     * Molecular Weight scaling factor.
     */
    double Mw = 1;
    double minDeltaThreshold = 1e-16;
    unordered_map<string, double> ReactionMultiplicity;
    /**
     * Pointer to an array of pointers, each pointing to a set of variables representing the results of the linear programming problems.
     */
    double** Vars;
    /**
     * Tracks the previous concentrations for all places, allowing for comparison against new values to detect significant changes.
     */
    unordered_map<string, double> previousConcentrations;
    /**
     * Comment.
     */
    unordered_map<size_t, string> problemBacteriaPlace;
    /**
     * Comment.
     */
    unordered_map<size_t, string> problemBiomassPlace;
    /**
     * Comment.
     */
    unordered_map<string, string> reactionToBacteria;
    /**
     * Comment.
     */
    unordered_map<string, string> reactionToBacteriaBIOMASS;
    /**
     * Threshold for determining significant changes in metabolic concentrations, expressed as a absolute value.
     */
    double absEpsilon = g_absEpsilon; // Initially set to 0%, can be configured.
    /**
     * Threshold for determining significant changes in metabolic concentrations, expressed as a percentage.
     */
    double relEpsilon = g_relEpsilon; // Initially set to 0%, can be configured.
    double scalingFactor = 1e-12; // Initially set to 0%, can be configured.
    /**
     * Counter to track how many times the network state has undergone significant changes.
     */
    double count = 0;
    /**
     * Enumeration to distinguish between input and output transitions within the metabolic network.
     */
    enum TransitionType { Input, Output };
    /**
     * Private constructor to prevent instantiation outside of the getInstance() method, ensuring singleton behavior.
     */
    FBAProcessor() : init(false) {
    }

    /**
     * Maps each metabolite to a set of problem indices that are affected by changes in this metabolite.
     * This helps optimize the processing by updating only relevant LP problems when specific metabolite concentrations change.
     */
    unordered_map<string, set<size_t>> metaboliteToProblems;

    std::vector<GeneRule> geneRules;
    bool geneRulesLoaded;

    /**
     * @brief Loads gene regulation rules from a file with a default name.
     *        If the file is not present, the simulation continues without gene rules.
     * @param vec_fluxb The vector of LP problems (used to map bacterium names to LP indices).
     * @param filename The file name to load; default is "GeneRules.txt".
     */
    void loadGeneRules(const vector<class FBGLPK::LPprob>& vec_fluxb, const std::string &filename = "GeneRules.txt") {
        std::ifstream infile(filename.c_str());
        if (!infile) {
            std::cout << "Gene rules file '" << filename 
                      << "' not found. Continuing simulation without gene rules." << std::endl;
            geneRulesLoaded = false;
            return;
        }
        geneRules.clear();
        std::string line;
        while (std::getline(infile, line)) {
            if (line.empty() || line[0] == '#') continue;
            size_t commentPos = line.find('#');
            if (commentPos != std::string::npos) line = line.substr(0, commentPos);
            std::istringstream iss(line);
            std::vector<std::string> tokens;
            std::string token;
            while (std::getline(iss, token, ',')) {
                token.erase(0, token.find_first_not_of(" \t\r\n"));
                token.erase(token.find_last_not_of(" \t\r\n") + 1);
                if (!token.empty()) tokens.push_back(token);
            }
            if (tokens.size() < 7) {
                std::cerr << "Skipping invalid gene rule: " << line << std::endl;
                continue;
            }
            GeneRule rule;
            if (tokens[0] == "NA") { rule.timeSpecified = false; } else { rule.timeSpecified = true; rule.time = std::stod(tokens[0]); }
            if (tokens[1] == "NA") { rule.placeSpecified = false; } else { rule.placeSpecified = true; rule.place = tokens[1]; }
            if (tokens[2] == "NA") { rule.thresholdSpecified = false; } else { rule.thresholdSpecified = true; rule.threshold = std::stod(tokens[2]); }
            // tokens[3] is reactionID
            rule.reactionID = tokens[3];
            // tokens[4] is newLB, tokens[5] is newUB
            rule.newLB = std::stod(tokens[4]);
            rule.newUB = std::stod(tokens[5]);
            // tokens[6] is bacterium name
            rule.bacterium = tokens[6];
            // If a comparison type is provided as an optional 7th token, use it; otherwise, set to "NA"
            if (tokens.size() >= 8) {
                rule.compType = tokens[7];
            } else {
                rule.compType = "NA";
            }
            // Determine the LP index for the bacterium; assume findLPIndex is defined
            rule.lpIndex = findLPIndex(vec_fluxb, rule.bacterium);
            if (rule.lpIndex == std::numeric_limits<size_t>::max()) {
                std::cerr << "Error: Bacterium '" << rule.bacterium << "' not found among LP problems." << std::endl;
            }
            // Set the applied flag to false initially
            rule.applied = false;
            geneRules.push_back(rule);
        }
        infile.close();
        geneRulesLoaded = true;
        std::cout << "Loaded " << geneRules.size() << " gene rule(s) from " << filename << std::endl;
    }
    void debugPrintGeneRules() {
        std::cout << "DEBUG: Loaded " << geneRules.size() << " gene rule(s):" << std::endl;
        for (size_t i = 0; i < geneRules.size(); ++i) {
            const GeneRule &rule = geneRules[i];
            std::cout << "GeneRule[" << i << "]: ";
            std::cout << "Time = ";
            if (rule.timeSpecified)
                std::cout << rule.time;
            else
                std::cout << "NA";
            std::cout << ", Place = ";
            if (rule.placeSpecified)
                std::cout << rule.place;
            else
                std::cout << "NA";
            std::cout << ", Threshold = ";
            if (rule.thresholdSpecified)
                std::cout << rule.threshold;
            else
                std::cout << "NA";
            std::cout << ", ReactionID = " << rule.reactionID;
            std::cout << ", newLB = " << rule.newLB;
            std::cout << ", newUB = " << rule.newUB;
            std::cout << ", Bacterium = " << rule.bacterium;
            std::cout << ", lpIndex = " << rule.lpIndex;
            std::cout << ", compType = " << rule.compType;
            std::cout << ", applied = " << (rule.applied ? "YES" : "NO") << std::endl;
        }
    }
    void applyGeneRegulationRules(double *Value, vector<class FBGLPK::LPprob>& vec_fluxb, map<string,int>& NumPlaces, double time) {
        //std::cout << "[DEBUG] Applying gene regulation rules at time " << time << std::endl;
        for (size_t i = 0; i < geneRules.size(); ++i) {
            GeneRule &rule = geneRules[i];
            // Skip rule if already applied
            if (rule.applied) {
                //std::cout << "[DEBUG] GeneRule[" << i << "] already applied, skipping." << std::endl;
                continue;
            }
           // std::cout << "[DEBUG] Evaluating GeneRule[" << i << "]: Reaction = " << rule.reactionID;
            if (rule.timeSpecified) {
                //std::cout << ", Time condition: current time (" << time << ") >= " << rule.time;
            } else {
                //std::cout << ", Time condition: NA";
            }
            if (rule.placeSpecified && rule.thresholdSpecified) {
                //std::cout << ", Place condition: " << rule.place;
                if (NumPlaces.find(rule.place) != NumPlaces.end()) {
                    //int idx = NumPlaces[rule.place];
                    //double conc = Value[idx];
                    //std::cout << " (current conc = " << conc << ")";
                } else {
                    //std::cout << " (metabolite not found)";
                }
            } else {
               // std::cout << ", Place condition: NA";
            }
            //std::cout << ", compType = " << rule.compType << std::endl;

            bool triggered = true;
            if (rule.timeSpecified) {
                triggered = triggered && (time >= rule.time);
            }
            if (rule.placeSpecified && rule.thresholdSpecified) {
                if (NumPlaces.find(rule.place) != NumPlaces.end()) {
                    int idx = NumPlaces[rule.place];
                    double conc = Value[idx];
                    if (rule.compType == "min") {
                        triggered = triggered && (conc <= rule.threshold);
                    } else if (rule.compType == "max") {
                        triggered = triggered && (conc >= rule.threshold);
                    } else {
                        // Default behavior: use 'max' condition
                        triggered = triggered && (conc >= rule.threshold);
                    }
                } else {
                    triggered = false;
                }
            }
            //std::cout << "[DEBUG] GeneRule[" << i << "] triggered: " << (triggered ? "YES" : "NO") << std::endl;
            if (triggered) {
                for (size_t j = 0; j < vec_fluxb.size(); ++j) {
                    int colIdx = vec_fluxb[j].fromNametoid(rule.reactionID);
                    if (colIdx != -1) {
                        if (rule.newLB == rule.newUB) {
                            vec_fluxb[j].update_bound(colIdx, "GLP_FX", rule.newLB, rule.newUB);
                        } else {
                            vec_fluxb[j].update_bound(colIdx, "GLP_DB", rule.newLB, rule.newUB);
                        }
                        std::cout << "[DEBUG] Applied gene rule for reaction " << rule.reactionID 
                                  << " in LP problem " << j << " at time " << time 
                                  << " (new bounds: [" << rule.newLB << ", " << rule.newUB << "])" << std::endl;
                    } else {
                        std::cout << "[DEBUG] Reaction " << rule.reactionID 
                                  << " not found in LP problem " << j << std::endl;
                    }
                }
                rule.applied = true;
            }
        }
    }


    /**
     * Destructor to clean up memory allocated for the Vars pointer of pointers, ensuring no memory leaks.
     */
    ~FBAProcessor() {
        // Clean up memory for the array of pointers
        delete[] Vars;  // Only deallocates the array of pointers
        Vars = nullptr;  // Sets the pointer to nullptr to prevent invalid memory access
        FBGLPK::freeGLPKEnvironment();  // Sets the pointer to nullptr to prevent invalid memory access
        cout << "FBAProcessor destroyed and GLPK environment freed." << endl;
    }


    /**
     * Recalculates the multiplicative constant for the specified transition based on the accessibility of each species to a metabolite.
     * The constant is computed based on the sum of the products of biomass mean and bacterial count across all relevant species.
     * @param transition The transition for which the constant is being recalculated.
     * @param reactionToFileMap Mapping of reactions to the problems that contain these reactions.
     * @param bacteriaToBioMean Mapping from problem index to mean biomass for that problem.
     * @param problemBacteriaPlace Mapping from problem index to the bacteria count place.
     * @param problemBiomassPlace Mapping from problem index to the biomass place.
     * @param NumPlaces Mapping of place names to their corresponding indices in the metabolic array.
     * @param Value Pointer to the array containing current metabolic values.
     */
    void calculateMultiplicativeConstant(double* Value, const map<string, int>& NumPlaces, size_t problemIndex, string reaction) {
        double denominator = 0;
        double numerator = 0;

        if (reactionToFileMap.find(reaction) != reactionToFileMap.end()) {
            const auto& problems = reactionToFileMap.at(reaction);
            for (const auto& problem : problems) {
                double numIndividuals = floor(Value[NumPlaces.at(problemBacteriaPlace.at(problem))]);
                denominator += numIndividuals;  // Accumulate total number of individuals for active problems
            }

            double transitionCurrentBiomass = Value[NumPlaces.at(problemBiomassPlace.at(problemIndex))];
            double transitionBioMean = bacteriaToBioMean.at(problemIndex);

            denominator *= transitionBioMean;  // Adjust denominator by the biomass mean of the transition problem
            numerator = transitionCurrentBiomass;  // Use current biomass of the transition problem as numerator
        }

        if (denominator > 0) {
            multiplicativeConstant = trunc(numerator * floor(Value[NumPlaces.at(problemBacteriaPlace.at(problemIndex))]) / denominator, decimalTrunc);  // Calculate multiplicative constant
        } else {
            multiplicativeConstant = 0;  // Handle case where denominator is zero
        }
    }


    /**
     * Trims whitespace from both ends of a given string.
     * @param str The string to trim.
     * @return A trimmed string with no leading or trailing whitespace.
     */
    string trim(const string& str) {
        size_t first = str.find_first_not_of(' ');
        if (first == string::npos) return ""; // Return empty if string is made of spaces
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, last - first + 1);
    }



    /**
     * Splits a string by a given delimiter and trims each resulting token.
     * @param str The string to split.
     * @param delim The delimiter character.
     * @return A vector of trimmed strings.
     */
    vector<string> splitAndTrim(const string& str, char delimiter) {
        vector<string> tokens;
        stringstream ss(str);
        string item;
        while (getline(ss, item, delimiter)) {
            item.erase(remove(item.begin(), item.end(), ' '), item.end()); // Remove spaces
            if (!item.empty()) {
                tokens.push_back(item);
            }
        }
        return tokens;
    }


    /**
     * Finds the index of a linear programming problem associated with a given filename within a vector of LP problems.
     * @param vec_fluxb A vector containing LP problems.
     * @param lpFile The filename associated with the LP problem.
     * @return The index of the LP problem in the vector or max size if not found.
     */
    size_t findLPIndex(const vector<class FBGLPK::LPprob>& vec_fluxb, const string& lpFile) {
        for (size_t i = 0; i < vec_fluxb.size(); ++i) {
            if (vec_fluxb[i].getFilename() == lpFile) {
                return i;
            }
        }
        return numeric_limits<size_t>::max();
    }


/**
 * @brief Ritorna il nome reazione finale, con logica speciale per Biomass.
 *
 * Se isBiomass == true:
 *   - se ha sia input che output => '_f'
 *   - se ha solo input => '_r'
 * Altrimenti, usa la logica di splitted e irreversibile.
 */
static std::string standardizeReactionName(
    const std::string& transitionName,
    const std::string& reactionBase,
    const std::vector<std::string>& inputs,
    const std::vector<std::string>& outputs,
    const std::map<std::string, unsigned int>& forwardMap,
    const std::map<std::string, unsigned int>& reverseMap,
    const std::map<std::string, unsigned int>& irreversMap,
    bool isBiomass // <--- nuovo parametro per la logica speciale
)
{
    //std::cerr << "\n[DEBUG standardizeReactionName] "
      //        << "transition='" << transitionName << "', "
     //         << "reactionBase='" << reactionBase << "', "
     //         << "isBiomass=" << (isBiomass ? "true" : "false") << "\n";

    // 1) Se la reazione base è nella mappa irreversibile, usiamola invariata
    if (irreversMap.find(reactionBase) != irreversMap.end()) {
        //std::cerr << "  [DEBUG] '" << reactionBase
          //        << "' è irreversibile => la teniamo invariata.\n";
        return reactionBase;
    }

    // 2) Controlla se esistono forward e/o reverse (splitted)
    bool splittedF = (forwardMap.find(reactionBase + "_f") != forwardMap.end());
    bool splittedR = (reverseMap.find(reactionBase + "_r") != reverseMap.end());

    // 3) Se non è irreversibile e non è splitted, restituisco invariato
    if (!splittedF && !splittedR) {
        //std::cerr << "  [WARN] Reaction '" << reactionBase
         //         << "' non trovata in irreversMap, forwardMap, né reverseMap. La lascio invariata.\n";
        return reactionBase;
    }

    // 4) Gestione speciale Biomass:
    if (isBiomass) {
        // Caso 4a: se ha sia input che output => '_f'
        if (!inputs.empty() && !outputs.empty() && splittedF) {
           // std::cerr << "  [DEBUG Biomass] '" << reactionBase
             //         << "' => '_f' (ha sia input che output)\n";
            return reactionBase + "_f";
        }
        // Caso 4b: se ha solo input => '_r' (se splittedR)
        else if (!inputs.empty() && outputs.empty() && splittedR) {
            //std::cerr << "  [DEBUG Biomass] '" << reactionBase
            //          << "' => '_r' (ha solo input)\n";
            return reactionBase + "_r";
        }
        // Altrimenti fallback biomass => invariato (oppure potresti mettere un fallback a _r)
        else {
           // std::cerr << "  [WARN Biomass] '" << reactionBase
            //          << "' mismatch => la lascio invariata.\n";
            return reactionBase;
        }
    }

    // 5) Non-biomass logic (classica):
    //    - se ci sono input => '_r'
    //    - se ci sono output => '_f'
    if (!outputs.empty() && splittedF) {
        //std::cerr << "  [DEBUG splitted] '" << reactionBase
        //          << "' => '_r' (ha outputPlaces)\n";
        return reactionBase + "_f";
    } else if (!inputs.empty() && splittedR) {
        //std::cerr << "  [DEBUG splitted] '" << reactionBase
        //          << "' => '_r' (ha inputPlaces)\n";
        return reactionBase + "_r";
    }

    // 6) Fallback
   // std::cerr << "  [WARN splitted] '" << reactionBase
      //        << "' mismatch => la lascio invariata.\n";
    return reactionBase;
}

    /**
     * Reads and processes the FBA information from a specified file path, initializing data structures necessary for simulation.
     * This includes mapping each transition to its corresponding reaction, and associating reactions to LP problem indices.
     * Also builds a mapping from LP problem indices to sets of reactions for optimized problem solving.
     * @param filePath The path to the file containing FBA information.
     * @param vec_fluxb A vector of LP problems to be populated based on the file.
     * @return true if the file was read successfully, false otherwise.
     */
    bool readFBAInfo(const string& filePath, vector<class FBGLPK::LPprob>& vec_fluxb) {
        ifstream file(filePath);
        if (!file.is_open()) {
            cerr << "Failed to open file: " << filePath << endl;
            return false;
        }
        stringstream buffer;
        buffer << file.rdbuf();
        string content = buffer.str();
        file.close();

        size_t pos = 0;
        while ((pos = content.find('{', pos)) != string::npos) {
            size_t endPos = content.find('}', pos);
            if (endPos == string::npos) break;

            string jsonObject = content.substr(pos + 1, endPos - pos - 1);
            pos = endPos + 1;

            string transition, lpFile, reaction, multiplicityStr, bacteriaCountPlace, bacteriaBiomassPlace, inputPlacesStr, outputPlacesStr, isBiomassStr;
            int multiplicity = 1;
            bool isBiomass = false;

            size_t keyPos = 0, valuePos = 0;
            while ((keyPos = jsonObject.find('"', keyPos)) != string::npos) {
                size_t keyEnd = jsonObject.find('"', keyPos + 1);
                string key = jsonObject.substr(keyPos + 1, keyEnd - keyPos - 1);

                valuePos = jsonObject.find(':', keyEnd) + 1;
                size_t valueEnd = (jsonObject[valuePos] == '"') ? jsonObject.find('"', valuePos + 1) : jsonObject.find(',', valuePos);
                if (valueEnd == string::npos) valueEnd = jsonObject.length();
                string value = jsonObject.substr(valuePos, valueEnd - valuePos);
                value.erase(remove(value.begin(), value.end(), '"'), value.end());
                value.erase(remove(value.begin(), value.end(), ' '), value.end());

                if (key == "Transition") transition = value;
                else if (key == "LPfile") lpFile = value;
                else if (key == "Reaction") reaction = value;
                else if (key == "Multiplicity") multiplicityStr = value;
                else if (key == "BacteriaCountPlace") bacteriaCountPlace = value;
                else if (key == "BacteriaBiomassPlace") bacteriaBiomassPlace = value;
                else if (key == "InputPlaces") inputPlacesStr = value;
                else if (key == "OutputPlaces") outputPlacesStr = value;
                else if (key == "IsBiomass") isBiomass = (value == "true\n");

                keyPos = valueEnd + 1;
            }

            try {
                multiplicity = stoi(multiplicityStr);
            } catch (const std::invalid_argument& e) {
                cerr << "Error converting multiplicity to integer: " << multiplicityStr << endl;
                cerr << "Exception: " << e.what() << endl;
                return false;
            } catch (const std::out_of_range& e) {
                cerr << "Multiplicity value out of range: " << multiplicityStr << endl;
                cerr << "Exception: " << e.what() << endl;
                return false;
            }

            // Remove square brackets and split the strings
            inputPlacesStr.erase(remove(inputPlacesStr.begin(), inputPlacesStr.end(), '['), inputPlacesStr.end());
            inputPlacesStr.erase(remove(inputPlacesStr.begin(), inputPlacesStr.end(), ']'), inputPlacesStr.end());
            outputPlacesStr.erase(remove(outputPlacesStr.begin(), outputPlacesStr.end(), '['), outputPlacesStr.end());
            outputPlacesStr.erase(remove(outputPlacesStr.begin(), outputPlacesStr.end(), ']'), outputPlacesStr.end());

            auto removeUnwantedChars = [](string& str) {
                str.erase(remove_if(str.begin(), str.end(), [](char c) { return isspace(c) || iscntrl(c); }), str.end());
            };

            removeUnwantedChars(inputPlacesStr);
            removeUnwantedChars(outputPlacesStr);

            vector<string> inputs;
            if (!inputPlacesStr.empty() && !(inputPlacesStr == " ") && !(inputPlacesStr == "")) inputs = splitAndTrim(inputPlacesStr, ',');
            vector<string> outputs;
            if (!outputPlacesStr.empty() && !(outputPlacesStr == " ") && !(outputPlacesStr == "")) outputs = splitAndTrim(outputPlacesStr, ',');

            if (isBiomass) {
                if (!inputs.empty() && !outputs.empty()) {
                    biomassTransitions.insert(transition);
                    //std::cerr << "[DEBUG] => Inserito '" << transition << "' in biomassTransitions (entrambe input e output presenti).\n";
                } else {
                    //std::cerr << "[DEBUG] => '" << transition << "' è IsBiomass ma NON ha entrambi input e output => NON inserita in biomassTransitions.\n";
                }
            }

						string finalReaction;
            if (!lpFile.empty() && !reaction.empty()) {
								 size_t index = findLPIndex(vec_fluxb, lpFile);
								 if (index == numeric_limits<size_t>::max()) {
    						 		// Non trovata => transitionsWithoutFile
    								transitionsWithoutFile.insert(transition);
								 } else {
    								const auto& forwardMap  = vec_fluxb[index].getForwardReactions();
    								const auto& reverseMap  = vec_fluxb[index].getReverseReactions();
    								const auto& irreversMap = vec_fluxb[index].getIrreversibileReactions();

										//cerr << "[DEBUG TRANSIZIONE] '" << transition << endl;
										  finalReaction = standardizeReactionName(
    									transition,      
    									reaction,        
    									inputs,
    									outputs,
    									forwardMap,
    									reverseMap,
    									irreversMap,
   	 								isBiomass
										);
    								//cout << "[DEBUG readFBAInfo] reactionBase='" << reaction
         				  //<< "transition: " << transition	<< "' => finalReaction='" << finalReaction << "'\n";

    								FBAreact[transition] = finalReaction;
                   int checkReacId = vec_fluxb[index].fromNametoid(finalReaction);
                   if (checkReacId == -1) {
                   		cerr << "[WARNING readFBAInfo] Reaction '" << finalReaction
                            << "' was NOT found in LP index=" << index << " (file='" << lpFile << "').\n";
        								return false;
                   }

    							if (!bacteriaCountPlace.empty() && bacteriaCountPlace != "N/A") {
        							problemBacteriaPlace[index] = bacteriaCountPlace;
    							}
    							if (!bacteriaBiomassPlace.empty() && bacteriaBiomassPlace != "N/A") {
        							problemBiomassPlace[index] = bacteriaBiomassPlace;
    							}

    							problems.insert(index);
    							bacteriaToBioMax[index]  = vec_fluxb[index].getBioMax();
    							bacteriaToBioMean[index] = vec_fluxb[index].getBioMean();
    							bacteriaToBioMin[index]  = vec_fluxb[index].getBioMin();

    							FBAproblems[transition] = index;
    							problemsToReactions[index].insert(transition);
								 }
            		 if (isBiomass) {
                		FBAreactions.insert(reaction + "_f");
                		FBAreactions.insert(reaction + "_r");
                }else{
                		FBAreactions.insert(finalReaction);
                }
                if (FBAreact.find(transition) != FBAreact.end()) {
                    string reactionName = FBAreact[transition];
                    if ((!inputs.empty() || !outputs.empty())) {
                        string combinedPlaces;
                        for (const auto& place : inputs) {
                            if (!place.empty() && combinedPlaces.find(place + ",") == string::npos) {
                                combinedPlaces += place + ",";
                            }
                        }
                        for (const auto& place : outputs) {
                            if (!place.empty() && combinedPlaces.find(place + ",") == string::npos) {
                                combinedPlaces += place + ",";
                            }
                        }
                        if (!combinedPlaces.empty()) {
                            combinedPlaces.pop_back(); // Remove the last comma
                            FBAplace[reactionName] = combinedPlaces;
                        }
                    }
                }
            }
            ReactionMultiplicity[transition] = multiplicity;
            //cout << "For transition: " << transition << " multiplicity: " << ReactionMultiplicity[transition] << endl; 

            // Populate the reactionToBacteria map if bacteriaCountPlace is specified
            if (!bacteriaCountPlace.empty() && bacteriaCountPlace != "N/A") {
                reactionToBacteria[transition] = bacteriaCountPlace;
                hasMultiSpecies = true;
            }
            if (!bacteriaBiomassPlace.empty() && bacteriaBiomassPlace != "N/A") {
                reactionToBacteriaBIOMASS[transition] = bacteriaBiomassPlace;
                hasBioMASS = true;
            }
        }

        return true;
    }


		double dynamicThreshold(double previousConcentration) {
    		double baseThreshold = 1e-3;
    		return std::max(minDeltaThreshold, baseThreshold * fabs(previousConcentration));
    		// return minDeltaThreshold;
		}


    set<string> checkSignificantChange(double* Value, map<string, int>& NumPlaces, double time) {
        reactionsToUpdate.clear();
        set<string> changedMetabolites;
        double relChange = (relEpsilon > 100.0) ? 100.0 : ((relEpsilon < 0.0 && absEpsilon == -1) ? 0 : ((relEpsilon < 0.0 && absEpsilon != -1) ? -1 : relEpsilon));
       //cout << "[DEBUG] relChange: " << relChange << endl;
       // cout << "[DEBUG] absChange: " << absEpsilon << endl;
        bool useAbsolute = (absEpsilon != -1);
        bool useRelative = (relChange != -1);
        for (const auto& place : FBAplace) {
            string metabolite = place.second;
            double currentConcentration = trunc(Value[NumPlaces[metabolite]], decimalTrunc);
            double previousConcentration = previousConcentrations[metabolite];
            double absoluteDiff = fabs(currentConcentration - previousConcentration);
            double percentChange = (previousConcentration != 0)
                ? (absoluteDiff / fabs(previousConcentration)) * 100.0
                : 100.0;
            bool condAbsolute = useAbsolute && (absoluteDiff > absEpsilon);
            bool condRelative = useRelative && (percentChange > relChange);
            bool condFromZero = useRelative && (previousConcentration == 0 && currentConcentration != 0);
            bool condFirstSol = (previousConcentration == -1);
            if (condAbsolute || condRelative || condFromZero || condFirstSol) {
                /*cout << "[DEBUG] Metabolite '" << metabolite
                     << "' changed. Old: " << previousConcentration
                     << ", New: " << currentConcentration
                     << ", Diff: " << (currentConcentration - previousConcentration)
                     << ", AbsDiff: " << absoluteDiff
                     << ", PercentChange: " << percentChange << "%"
                     << ", condAbsolute: " << condAbsolute
                     << ", condRelative: " << condRelative
                     << ", condFromZero: " << condFromZero << endl;*/
                changedMetabolites.insert(metabolite);
            }
        }
        return changedMetabolites;
    }


    /**
     * Truncates a double value to a specified number of decimal places.
     * This method uses mathematical manipulation to achieve truncation without rounding.
     * The value is scaled up by a power of ten corresponding to the desired number of decimal places,
     * then truncated using floor to drop any fractional part, and finally scaled back down.
     * This method ensures that the truncation is always downward, similar to how floor operates on positive numbers.
     *
     * @param value The double value to truncate.
     * @param decimal The number of decimal places to retain in the truncated result.
     * @return double The truncated value, preserving only the specified number of decimals.
     * 
     * Example:
     *    If value = 123.456789 and decimal = 3,
     *    the result would be 123.456.
     */
    double trunc(double value, double decimal) {
        const double multiplier = pow(10.0, decimal); // Calculate the scaling factor based on the number of decimals
        return floor(value * multiplier) / multiplier; // Scale up, truncate, scale down
        // Alternative implementation using type casting:
        // return ((unsigned long int)(value * multiplier)) / multiplier; // Cast to integer to truncate
    }


    void updateFluxBounds(
        double *Value, 
        vector<class FBGLPK::LPprob>& vec_fluxb, 
        map<string, int>& NumPlaces, 
        set<string>& changedMetabolites,
        double time
    ) {
        set<size_t> problemsToUpdate;

        // (1) Costruiamo l'insieme dei problemIndex da aggiornare
        for (const string& metabolite : changedMetabolites) {
            if (metaboliteToProblems.find(metabolite) != metaboliteToProblems.end()) {
                for (auto prob : metaboliteToProblems[metabolite]) {
                    if (deadBacterialSpecies.find(prob) == deadBacterialSpecies.end() 
                        || !deadBacterialSpecies[prob]) {
                        problemsToUpdate.insert(prob);
                    }
                }
            }
        }

        // (2) Per ciascun problemIndex da aggiornare
        for (auto problemIndex : problemsToUpdate) {

            for (const string& reaction : reactionsToUpdate) {
                int colIdx = vec_fluxb[problemIndex].fromNametoid(reaction);
                if (colIdx == -1) {
                    continue;
                }

                if (reaction == "EX_biomass_e_f" || reaction == "EX_biomass_e_r") {
                    continue; // do NOT update dynamic bounds for biomass reaction
                }
                bool isReverse = false;
                if (reaction.size() >= 2 && reaction.substr(reaction.size() - 2) == "_r") {
                    isReverse = true;
                }
                if (!isReverse) {
                    continue;
                }

                double conc = trunc(Value[NumPlaces.at(FBAplace[reaction])], decimalTrunc);

        				 double fluxMaxCandidate = 1e9;
        				 if (hasBioMASS && hasMultiSpecies) {
            					calculateMultiplicativeConstant(Value, NumPlaces, problemIndex, reaction);
            					double lambda = multiplicativeConstant;

            					if (lambda == 0.0 || conc <= 0.0) {
               				  fluxMaxCandidate = 0; // No flux if lambda or concentration is zero
            					} else {
                					fluxMaxCandidate = conc * lambda; // Dynamically scale with concentration and lambda
            					}
        					} else {
            					if (conc > 0.0) {
                					fluxMaxCandidate = conc; // Scale only with concentration
            					} else {
                					fluxMaxCandidate = 0; // No flux if concentration is zero
            					}
        					}


        					double oldUb = vec_fluxb[problemIndex].getUpBounds(colIdx);
        					if (conc > 0.0) {
           				 if (oldUb == 0) {
               				 oldUb = fluxMaxCandidate;
            					} else {
               				 oldUb = std::max(oldUb, fluxMaxCandidate);
           				  }
        					} else {
           				 oldUb = 0;
       					}


        					double newUb = fluxMaxCandidate;

        					double Lb = 0.0;
        					if (newUb <= Lb) {
            					vec_fluxb[problemIndex].update_bound(colIdx, "GLP_FX", Lb, Lb);
        					} else {
            					vec_fluxb[problemIndex].update_bound(colIdx, "GLP_DB", Lb, newUb);
        					}
            }
        }
    }


    /**
     * Solves the FBA linear programming problems that are affected by significant changes in metabolite concentrations.
     * This method iterates through a subset of LP problems identified by changed metabolites, solving them to find the optimal flux
     * distributions under current constraints. It updates an array of variable pointers, storing the solution
     * variables for each problem. This is crucial for subsequent steps in metabolic analysis where these
     * solutions are used to compute metabolic flux rates.
     * 
     * This method improves efficiency by focusing on problems affected by recent changes, ensuring that
     * results are immediately accessible and the model is adjusted only as needed in response to
     * changing conditions within the biological system.
     * 
     * @param vec_fluxb Reference to a vector containing FBGLPK::LPprob objects.
     * @param changedMetabolites Set of metabolites that have changed, used to identify relevant LP problems to solve.
     */
    void solveFBAProblems(vector<class FBGLPK::LPprob>& vec_fluxb, set<string>& changedMetabolites) {
        set<size_t> toSolve;
        for (const string& metabolite : changedMetabolites) {
            if (metaboliteToProblems.find(metabolite) != metaboliteToProblems.end()) {
                for (auto problemIndex : metaboliteToProblems[metabolite]) {
                    if ((deadBacterialSpecies.find(problemIndex) == deadBacterialSpecies.end() || !deadBacterialSpecies[problemIndex])) {
                        toSolve.insert(problemIndex);
                    }
                }
            }
        }

        for (auto index : toSolve) {
            //cout << "Problema da risolvere ---> " << index <<  "   Name: " << vec_fluxb[index].getFilename() << endl;
            vec_fluxb[index].solve();
            // PARSIMONIUS FLAGS:
            if(vec_fluxb[index].getPFbaFlag() != -1) performPFBA(vec_fluxb, index);
            Vars[index] = vec_fluxb[index].getVariables(); // Update the variable pointers
        }
    }


    /**
     * Updates the stored concentrations of metabolites based on the latest computational results.
     * This method iterates over a map linking metabolite names to their indices in an array of current
     * metabolic concentrations. It updates a map of previous concentrations with the current values,
     * which is essential for tracking changes in metabolite levels over time and responding to dynamic
     * metabolic conditions. This tracking supports the system's ability to determine significant changes
     * in metabolic states that may require further adjustments in the model.
     * 
     * By maintaining an up-to-date record of metabolite concentrations, this method ensures that
     * the metabolic simulation reflects the most current state of the system, enabling accurate
     * and timely decision-making in metabolic engineering and research.
     * 
     * @param Value Pointer to an array containing the current concentrations of metabolites, indexed numerically.
     * @param NumPlaces Map linking metabolite names to their respective indices in the Value array, used for direct access.
     * @param changedMetabolites Set of metabolite names whose concentrations have changed significantly.
     */
    void updateConcentrations(double* Value, map<string, int>& NumPlaces, set<string>& changedMetabolites) {
        for (const string& metabolite : changedMetabolites) {
            int index = NumPlaces.at(metabolite);
            double newConcentration = trunc(Value[index], decimalTrunc);
            previousConcentrations[metabolite] = newConcentration; // Update the concentration record even if negative
        }
    }



    /**
     * Updates the flux bounds and solves the FBA problems only for those metabolites that have undergone significant changes.
     * This method first checks for significant changes in metabolite concentrations. If changes are detected,
     * it updates the flux bounds for those metabolites, solves the corresponding FBA problems,
     * and then updates the metabolite concentrations in the model.
     *
     * @param Value Pointer to an array containing current metabolic values.
     * @param vec_fluxb Reference to a vector of FBGLPK::LPprob objects, each representing a distinct FBA problem.
     * @param NumPlaces Mapping of place names to their indices in the metabolic array.
     * @param time Current time to manage updates efficiently.
     */
    void updateFluxBoundsAndSolve(double* Value, vector<class FBGLPK::LPprob>& vec_fluxb, map<string, int>& NumPlaces, double time) {
        set<string> changedMetabolites = checkSignificantChange(Value, NumPlaces, time);
        if (!changedMetabolites.empty()) {
            for (const auto& place : FBAplace) {
                reactionsToUpdate.insert(place.first);
            }

            applyGeneRegulationRules(Value, vec_fluxb, NumPlaces, time);
            updateFluxBounds(Value, vec_fluxb, NumPlaces, changedMetabolites, time);
            updateNonFBAReactionsUB(vec_fluxb, NumPlaces, Value);
            solveFBAProblems(vec_fluxb, changedMetabolites);
            updateConcentrations(Value, NumPlaces, changedMetabolites);
            count += 1;
            //cout << "risolvo per la: " << count << endl;
        }
    }


    /**
     * Computes the metabolic rate for a given transition based on its position and the current metabolic state.
     * The rate is calculated from the solution to the FBA LP problem associated with the transition.
     * The rate is then adjusted based on whether the transition is an input or an output.
     * 
     * @param vec_fluxb A vector containing all the FBA problems.
     * @param NumPlaces A map linking metabolite names to their indices in the solution vector.
     * @param NameTrans A vector containing the names of all transitions.
     * @param Value The current state values of metabolites.
     * @param T The index of the transition for which the rate is being computed.
     * @param decimalTrunc The precision to which values are truncated.
     * @param time The current simulation time.
     * @return double The computed and adjusted metabolic rate for the transition.
     */
    double computeRate(vector<class FBGLPK::LPprob>& vec_fluxb, map<string, int>& NumPlaces, const vector<string>& NameTrans, const double* Value, const int T, const double decimalTrunc, const double& time) {
        string transitionName = NameTrans[T];

        int problemIndex = FBAproblems[transitionName];

        int varIndex = vec_fluxb[problemIndex].fromNametoid(FBAreact[transitionName]);
       // int tmpProva = vec_fluxb[problemIndex].fromNametoid("biomass525");
        //double rateTmp = Vars[problemIndex][tmpProva];
        //std::cout << "[tmpProva] biomass525 rate : " << rateTmp <<  std::endl;

        double rate = Vars[problemIndex][varIndex];

        double constant = 1.0;
        double currentBiomass = 1.0;

        double scalingMeasure = ReactionMultiplicity[transitionName];
        //cout << " [Debug Transition Rate]: " << transitionName << "Scaling Factor for measure unit: " <<  scalingMeasure << endl;

        if(hasBioMASS){
            currentBiomass = Value[NumPlaces.at(problemBiomassPlace.at(problemIndex))];
        }

        if (hasBioMASS && biomassTransitions.find(transitionName) != biomassTransitions.end()) {
           // std::cout << "[computeRate] Biomass Transition Detected for Transition : " << transitionName << " rateo: " << rate << std::endl;
            constant = floor(Value[NumPlaces.at(problemBacteriaPlace.at(problemIndex))]);
            return rate * scalingMeasure;
        } else if (hasMultiSpecies && hasBioMASS) {
            constant = floor(Value[NumPlaces.at(problemBacteriaPlace.at(problemIndex))]);
        }


        if(hasBioMASS) {
           // std::cout << "[computeRate] Other rate Transition Detected for Transition : " << transitionName << " rateo: " << rate << std::endl;
            rate = rate * (constant * currentBiomass * scalingFactor) * scalingMeasure;
        }

        //cout << "Ritorno rateo: " << rate << endl;
        return rate;
    }


    /**
     * Initializes the metabolic concentrations from provided values.
     * This method initializes the concentrations for each metabolite based on the indices provided in a map.
     * Each concentration is truncated to a specified number of decimal places before being set.
     *
     * @param Value Array of initial concentration values.
     * @param NumPlaces Mapping from metabolite names to their indices in the Value array.
     * @return void
     */
    void initializeConcentrations(double* Value, map<string, int>& NumPlaces) {
        for (const auto& pair : FBAplace) {
            string placeName = pair.second;
            previousConcentrations[pair.second] = -1;
        }
    }


    /**
     * Maps each metabolite to the set of LP problems that it affects.
     * This method initializes the mapping from metabolites to their related LP problems,
     * which is used to optimize updates when only certain metabolites change.
     */
    void mapMetabolitesToProblems() {
        for (const auto& problem : FBAproblems) {
            const string& reaction = FBAreact[problem.first];
            if (FBAplace.find(reaction) != FBAplace.end()) {
                size_t problemIndex = problem.second;
                string places = FBAplace[reaction];
                vector<string> metabolites = splitAndTrim(places, ',');
                for (const string& metabolite : metabolites) {
                    metaboliteToProblems[metabolite].insert(problemIndex);
                }
            }
        }
    }


    /**
     * Initializes the data structures necessary for the FBA simulation process.
     * This method is invoked only once per instance, conforming to the singleton design pattern of this class.
     * It loads necessary data from a specified .fbainfo file into various structures for managing reactions,
     * places, and problem indices, and initializes all metabolic concentrations to a default negative value.
     * This ensures that any uninitialized metabolic state is clearly indicated and handled appropriately.
     *
     * @param NameTrans The names of the transitions involved in the network, used for initialization checks.
     * @param vec_fluxb A reference to a vector of LP problems representing the metabolic fluxes.
     * @param Value Pointer to an array storing the metabolic concentrations.
     * @param NumPlaces A map linking place names to their corresponding indices.
     * @return void
     * @throws If the file cannot be read, it prints an error message and terminates initialization.
     */
    void init_data_structures_class(const vector<string>& NameTrans, vector<class FBGLPK::LPprob>& vec_fluxb, double* Value, map<string, int>& NumPlaces) {
        if (!readFBAInfo(FBA_INFO_FILE, vec_fluxb)) { // Attempt to read initialization data from the .fbainfo file
            cerr << "Failed to read places from .fbainfo file" << endl;
            exit(1); // Early return if file reading fails
        }

        Vars = new double*[vec_fluxb.size()];
        initializeConcentrations(Value, NumPlaces);
        mapMetabolitesToProblems();
    }


		void mapReactionsFromProblems(const vector<class FBGLPK::LPprob>& vec_fluxb) {
    		reactionToFileMap.clear(); // Clear the existing map
    		for (const auto& index : problems) {
        		const auto& lp = vec_fluxb[index];
        		vector<string> reactions = lp.getReactions();
        		for (const auto& reaction : reactions) {
            		if (reaction.substr(0, 3) == "EX_") { // Check if reaction starts with 'EX_'
                		reactionToFileMap[reaction].insert(index);
            		}
        		}
    		}
		}



    void updateNonFBAReactionUpperBoundsFromFile(
        vector<FBGLPK::LPprob>& vec_fluxb,
        const string& filename = "EX_upper_bounds_nonFBA.csv"
    ) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Warning: " << filename << " not found. "
                 << "Using original model's bounds as default base UB." << endl;

            // Se il file non viene trovato, popola NonFBAReactionBaseUB con i bound attuali del modello per le reazioni non-FBA.
            for (const auto& rxnEntry : reactionToFileMap) {
                const string& rxn = rxnEntry.first;
                if (FBAreactions.find(rxn) != FBAreactions.end()) {
                    continue;
                }
                const auto& problemIndices = rxnEntry.second;
                for (size_t idx : problemIndices) {
                    int reactionId = vec_fluxb[idx].fromNametoid(rxn);
                    if (reactionId == -1) {
                        continue; // Reaction not found in this LP
                    }
                    double originalUb = vec_fluxb[idx].getUpBounds(reactionId);
                    cout << "reaction: " << rxn << " original bound: " << originalUb << endl;
                    NonFBAReactionBaseUB[std::make_pair(rxn, idx)] = originalUb;
                }
            }
            return;
        }

        vector<string> lines;
        {
            string line;
            while (getline(file, line)) {
                lines.push_back(line);
            }
        }
        file.close();

        if (lines.empty()) {
            return;
        }

        double baseUB = 0.0;
        {
            istringstream ss(lines[0]);
            string header;
            getline(ss, header, ','); // legge "base_upper_bounds"
            string valueStr;
            getline(ss, valueStr, ',');
            baseUB = stod(valueStr);
        }

        for (size_t lineIdx = 1; lineIdx < lines.size(); ++lineIdx) {
            istringstream ss(lines[lineIdx]);
            string reactionName;
            getline(ss, reactionName, ',');

            if (FBAreactions.find(reactionName) != FBAreactions.end()) {
                continue;
            }

            string newUbStr;
            getline(ss, newUbStr, ',');
            double newUb = stod(newUbStr);

            auto it = reactionToFileMap.find(reactionName);
            if (it != reactionToFileMap.end()) {
                const auto& problemIndices = it->second;
                for (size_t idx : problemIndices) {
                    int reactionId = vec_fluxb[idx].fromNametoid(reactionName);
                    if (reactionId == -1) {
                        continue;                    }

                    NonFBAReactionBaseUB[std::make_pair(reactionName, idx)] = baseUB;

                    double currentLb = vec_fluxb[idx].getLwBounds(reactionId);
                    vec_fluxb[idx].update_bound(
                        reactionId,
                        "GLP_DB",
                        trunc(currentLb, decimalTrunc),
                        trunc(newUb, decimalTrunc)
                    );

                   // debug cout << "Updating reaction: " << reactionName << " with new bound: " << newUb << endl;
                }
            }
        }
    }


    void loadAndApplyFBAReactionUpperBounds(
        vector<FBGLPK::LPprob>& vec_fluxb,
        const string& filename = "EX_upper_bounds_FBA.csv"
    ) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "[Warning] : " << filename << " not found. Skipping updating FBA reaction upper bounds." << endl;
            return;
        }

        string line;
        while (getline(file, line)) {
            istringstream ss(line);
            string reaction;
            vector<double> bounds;
            string boundStr;

            // 1) Extract the reaction name
            getline(ss, reaction, ',');

            // 2) Extract the numeric fields
            while (getline(ss, boundStr, ',')) {
                bounds.push_back(stod(boundStr));
            }

            // 3) Check if the reaction exists in reactionToFileMap
            auto it = reactionToFileMap.find(reaction);
            if (it != reactionToFileMap.end()) {
                const auto& problemIndices = it->second;

                // 4) Update the bounds
                size_t i = 0;
                for (auto idxIter = problemIndices.begin(); idxIter != problemIndices.end(); ++idxIter, ++i) {
                    size_t problemIndex = *idxIter;
                    int reactionId = vec_fluxb[problemIndex].fromNametoid(reaction);
                    if (reactionId == -1) {
                        continue;
                    }

                    if (problemIndex >= bounds.size()) {
                        continue;
                    }

                    // Use the correct value from bounds
                    double newUb = bounds[problemIndex];

                    // Retrieve the current lower bound
                    double currentLb = vec_fluxb[problemIndex].getLwBounds(reactionId);

                    // Update bounds
                    vec_fluxb[problemIndex].update_bound(
                        reactionId,
                        "GLP_DB",
                        trunc(currentLb, decimalTrunc),
                        trunc(newUb, decimalTrunc)
                    );
                }
            }
        }

        file.close();
    }


    void updateNonFBAReactionsUB(
        vector<FBGLPK::LPprob>& vec_fluxb,
        map<string, int>& NumPlaces,
        double* Value
    ) {
        for (const auto& reactionEntry : reactionToFileMap) {
            const string& reaction = reactionEntry.first;

            // Skip if it's an FBA reaction
            if (FBAreactions.find(reaction) != FBAreactions.end()) {
                continue;
            }

            // All non-FBA reactions
            const auto& problemIndices = reactionEntry.second;
            for (size_t index : problemIndices) {
                int reactionId = vec_fluxb[index].fromNametoid(reaction);
                if (reactionId == -1) {
                    continue; // reaction not found in this LP
                }

                // Population, biomass, etc.
                string placeName = problemBacteriaPlace[index];
                double population = floor(Value[NumPlaces.at(placeName)]);
                double biomass    = trunc(Value[NumPlaces.at(problemBiomassPlace.at(index))], decimalTrunc);

                // If no population => set reaction to 0.
                if (population <= 0.0) {
                    vec_fluxb[index].update_bound(
                        reactionId, "GLP_FX", 0.0, 0.0
                    );
                    continue;
                }

                // Attempt to find a baseUb in NonFBAReactionBaseUB.
                auto mapKey = std::make_pair(reaction, index);
                auto it = NonFBAReactionBaseUB.find(mapKey);
                if (it == NonFBAReactionBaseUB.end()) {
                    // Key not in map => do NOT update bounds, keep the model's original.
                    continue;
                }

                // Otherwise, we have a baseUb from the map.
                double baseUb = it->second;
                double newUb  = baseUb / ((population * biomass) / Mw);


                // Update the GLPK problem with newUb
                vec_fluxb[index].update_bound(
                    reactionId, "GLP_DB", 0.0, newUb
                );
            }
        }
    }


    void updateAllBiomassReactionsUpperBounds(double* Value, const map<string, int>& NumPlaces, vector<class FBGLPK::LPprob>& vec_fluxb) {
        problemsWithLowBiomass.clear(); // Clear the low biomass problems map at the start of each update cycle
        for (const auto& transition : biomassTransitions) {
            size_t problemIndex = FBAproblems[transition];  // Get the problem index associated with the transition

            string reaction = FBAreact[transition];

            double BioMax = vec_fluxb[problemIndex].getBioMax();
            double CurrentBiomass = trunc(Value[NumPlaces.at(problemBiomassPlace.at(problemIndex))], decimalTrunc);
            double BioMin = vec_fluxb[problemIndex].getBioMin();

            double Gamma = BioMax - CurrentBiomass;
            //cout << "Valore Gamma " << Gamma << " per il problema " << problemIndex << endl;

            double newUpperBound;
            if (CurrentBiomass < BioMin) {
                newUpperBound = Gamma; // Positive Gamma, set upper bound to Gamma
                problemsWithLowBiomass[problemIndex] = true; // Mark this problem as having low biomass
            } else if (Gamma > 0) {
                newUpperBound = Gamma; // Positive Gamma, set upper bound to Gamma
            } else {
                newUpperBound = 1e-6; // Gamma not positive, set upper bound to small cutoff
            }

        			int index = vec_fluxb[problemIndex].getPFBA_index();
            double currentLowerBound = vec_fluxb[problemIndex].getLwBounds(index);

            vec_fluxb[problemIndex].update_bound(index, "GLP_DB", currentLowerBound, trunc(newUpperBound, decimalTrunc));
        }
    }


    void performPFBA(vector<class FBGLPK::LPprob>& vec_fluxb, int problemIndex) {
        double optimalBiomass = vec_fluxb[problemIndex].getOBJ();
        
        int biomassIndex = vec_fluxb[problemIndex].getPFBA_index();
        double originalLb = vec_fluxb[problemIndex].getLwBounds(biomassIndex);
        double originalUb = vec_fluxb[problemIndex].getUpBounds(biomassIndex);
        int originalType = vec_fluxb[problemIndex].get_bound_type(biomassIndex);
        
        // Prove debug metto un piccolo range
        double eps = 1e-6;
        double lb = optimalBiomass - eps;
        double ub = optimalBiomass + eps;
        vec_fluxb[problemIndex].update_bound(biomassIndex, "GLP_DB", lb, ub);
        
        vec_fluxb[problemIndex].setMinimizeFluxObjective(biomassIndex);
        
        // Rieseguo il solver con la pFBA
        //cout << "Ora risolvo pFBA" << endl;
        vec_fluxb[problemIndex].solve();
        //cout << "RISOLTO pFBA" << endl;
        
        // -- DEBUG 3: Stampo la situazione pFBA
        //vec_fluxb[problemIndex].debugPFBA();
        
        // Step 3: Ripristino i bound originali
        vec_fluxb[problemIndex].update_bound(biomassIndex, originalType, originalLb, originalUb);
        
        // Step 3.1: Torno a massimizzare la biomassa
        vec_fluxb[problemIndex].resetMaximizationObjectiveForBiomass(biomassIndex);
    }
};
