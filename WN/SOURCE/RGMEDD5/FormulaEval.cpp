#include <stdio.h>
#include <cstdio>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string>
#include <sstream>
#include <fstream>
#include <optional>
#include <numeric>
#include <iterator>
#include <functional>
#include <unistd.h>
#include "CTL.h"
#include "rgmedd5.h"
#include "parallel.h"

using namespace std;
using namespace ctlmdd;

extern const char* MCC_TECHNIQUES;
// Are we parsing a boolean expression of HOA edges?
bool parsing_HOA_edge = false;
RSRG *g_rsrg;
// tempi di clock quando inizia la generazione dell'MDD della formula, 
// quando finisce e dopo aver controllato la presenza dello stato iniziale nella formula
// static clock_t startMDD, endMDD, endMDD2; 
// From the command line
extern bool print_CTL_counterexamples;
extern bool sort_CTL_queries;
extern bool CTL_atoms_use_potential_state_space;
// sequential/parallel model checking
int g_par_mc_max_time_round0 = -1;
int g_par_mc_num_parallel_procs = -1;
int g_par_mc_max_MB_statespace = -1;
bool g_par_mc_do_second_round = true;

static bool is_mc_parallel() { return g_par_mc_num_parallel_procs >= 0; }

const std::vector<size_t> *p_spot_ap_to_greatspn_ap_index = nullptr;
const std::vector<Formula*> *p_greatspn_atomic_propositions = nullptr;

//-----------------------------------------------------------------------------
// Encoding of an evaluated formula
//-----------------------------------------------------------------------------

struct ctl_query_t {
    std::string line;    // query in textual form
    std::string name;    // name (for named formulas)
    std::optional<result_t> expected;   // expected result (if available)
    Language lang;       // sublanguage
    int score;           // complexity score
    bool is_fairness_constraint; // is this query a new fairness constraint?
    bool always_use_SatELTL; // use only SatELTL for all CTL/LTL queries. Otherwise use CTL when possible
};

//-----------------------------------------------------------------------------

char ctl_result_buffer[32];
const char* format_result(const result_t& result, bool uppercase) {
    return boost::apply_visitor(overload{
        [](ssize_t i)  { 
            sprintf(ctl_result_buffer, "%d", i); 
            return (const char*)ctl_result_buffer;
        },
        [&uppercase](bool b) {
            return b ? (uppercase ? "TRUE" : "true") :
                       (uppercase ? "FALSE" : "false");

        },
        [](double d)   { return "??"; },
    }, result);
}

//-----------------------------------------------------------------------------

// Parse and evaluate a CTL formula
BaseFormula *parse_and_evaluate_formula(Context& ctx, const std::string& formula, result_t *out_result) {
    istringstream *p_buffer = new istringstream(formula);
    initialize_lexer(p_buffer);
    BaseFormula *parsedFrm = parse_formula();
    deinitialize_lexer();
    delete p_buffer;
    if(parsedFrm != NULL) {
        // parsedFrm->set_top_level(true);
        // parsedFrm->addOwner();
        // startMDD = clock();
        if (!running_for_MCC() && !CTL_quiet) {
			/*cout << "==================" << endl;*/
			/*cout << "isPathFormula: " << parsedFrm->isPathFormula() << endl;*/
			/*parsedFrm->print(cout, true);*/
			/*cout << "\n==================" << endl;*/
            cout << "Processing: " << *parsedFrm << "  ->  " 
                 << (parsedFrm->isIntFormula()?"int":"bool") << endl;
        }

        if (parsedFrm->isIntFormula()) { 
            IntLiteral* intFrm = dynamic_cast<IntLiteral*>(parsedFrm);
            *out_result = result_t{(ssize_t)intFrm->getConstant()};
        }
        else { // boolean formula
            Formula* boolFrm = dynamic_cast<Formula*>(parsedFrm);

            // Fix unquantified LTL formulas
            if (ctx.language == Language::LTL && boolFrm->isPathFormula()) { 
                boolFrm = ctlnew<QuantifiedFormula>(boolFrm, QOP_ALWAYS);
                parsedFrm = boolFrm; // important: change also parsedFrm!
	        }
            // Mark the top-level node
            boolFrm->set_top_level(true);

            // Get the Sat-set of the formula
            dd_edge dd = boolFrm->getMDD(ctx);
            assert(dd.getForest() != nullptr);
            // endMDD = clock();

            // Evaluate the CTL expression:  s0 |= formula
            dd_edge r(ctx.get_MDD_forest());

            /*cout << "Markings that satisfy the formula: \n";
            enumerator i(*dd);
            int nvar = rsrg->getDomain()->getNumVariables();
            while(i != 0) { // for each marking in the sat set
                int j;
                for(j=1; j <= nvar; j++) { // for each place
                    int val = *(i.getAssignments() + j);
                    const std::string& s = rsrg->getPL(j - 1);
                    if(val!=0) 
                        cout << s << "(" << val << ")";
                }
                ++i;
                cout << endl;
            }
            cout << endl;*/

            apply(INTERSECTION, g_rsrg->getInitMark(), dd, r);
            *out_result = result_t{!ctx.is_false(r)};
        }
        // endMDD2=clock();
    }
    return parsedFrm;
}

//-----------------------------------------------------------------------------

// Parse spot edge labels (boolean expressions over atomic propositions)
// An edge label generated by SPOT has a form like:  0 & !2 | !1
//  where the numbers are the atomic propositions indices
Formula* 
parse_spot_formula(const std::string& formula, 
                   const std::vector<Formula*>& atomic_propositions,
                   const std::vector<size_t>& spot_ap_to_greatspn_ap_index)
{
    assert(p_greatspn_atomic_propositions == nullptr);

    p_greatspn_atomic_propositions = &atomic_propositions;
    p_spot_ap_to_greatspn_ap_index = &spot_ap_to_greatspn_ap_index;
    std::string expr = "### " + formula;

    istringstream *p_buffer = new istringstream(expr);
    initialize_lexer(p_buffer);
    parsing_HOA_edge = true;
    Formula *stateFormula = (Formula*)parse_formula();
    deinitialize_lexer();
    delete p_buffer;
    parsing_HOA_edge = false;
    // stateFormula->addOwner();

    // cout << "parse_spot_formula: " << formula << " parsed as: " << *stateFormula << endl;

    p_greatspn_atomic_propositions = nullptr;
    p_spot_ap_to_greatspn_ap_index = nullptr;

    return stateFormula;
};


//-----------------------------------------------------------------------------

// count number of nested parenthesis in property string
unsigned int max_nested_par(const std::string prop)
{
    unsigned int cnt = 0;
    unsigned int max = 0;

    for(const char& c: prop) {
        switch(c) {
        case '(':
        case '[':
            if(++cnt > max) max = cnt;
            break;
        case ')':
        case ']':
            --cnt;
            break;
        default:
            break;
        }
    }
    return max;
}

unsigned int num_path_ops(const std::string prop)
{
    unsigned int cnt = 0;
    std::string ops = "GFUX";
    for(const char& c: prop) {
        if(std::find(ops.begin(), ops.end(), c) != ops.end()) cnt++;
    }
    return cnt;
}

//-----------------------------------------------------------------------------

void model_check_query(Context& ctx, const ctl_query_t& query, int sem_id, bool *set_regr_failed) 
{
    // Modify the evaluation context for the current query
    ctx.language   = query.lang;
    ctx.always_use_SatELTL = query.always_use_SatELTL;
    ctx.stutter_EG = true; //(query.lang == Language::CTL);
    ctx.stutter_EX = (query.lang == Language::LTL);

    std::list<dd_edge> ctx_fair_sets = ctx.fair_sets;
    if (query.is_fairness_constraint) {
        // Fairness constraints are evaluated without other active fairness constraints
        ctx.fair_sets.clear();
    }

    // cout << s_languageName[query.lang] << "   Stutter EG = " << ctx.stutter_EG << endl;

    // Compute the result
    result_t result;
    clock_t startMDD = clock();
    BaseFormula *formula = parse_and_evaluate_formula(ctx, query.line, &result);
    clock_t endMDD2 = clock();

    // Compute the cardinality
    cardinality_t satset_card = -1, rs_card = -1;
    bool compute_card = CTL_print_sat_sets || invoked_from_gui();
    if (formula && formula->isBoolFormula() && (query.lang!=Language::LTL) && compute_card) {
        dd_edge dd(dynamic_cast<Formula*>(formula)->getMDD(ctx));
        apply(INTERSECTION, ctx.RS, dd, dd);
        apply(CARDINALITY, dd, cardinality_ref(satset_card));
        apply(CARDINALITY, ctx.RS, cardinality_ref(rs_card));
    }

    // stop the killing timer after query evaluation
    if (is_mc_parallel()) {
        alarm(0);
        signal(SIGALRM, SIG_DFL);
    }

    // From this point on, there should be only (fast) result visualization


    // Show the result of this CTL formula
    if (formula == NULL) { // parse error
        semaphore_sentinel sent(sem_id);
        if (!running_for_MCC()) {
            cout<<"Parse error."<<endl;
            if (CTL_quiet) {
                cout << s_languageName[query.lang] << ": ";
                if (!query.name.empty())
                    cout << query.name << "  ";
                cout << "  is  -" << endl;
            }
            else {
                cout << "Language: " << s_languageName[query.lang] << "\n";
                if (!query.name.empty())
                    cout << "Formula: " << query.name << "\n";
                cout << "Evaluation: -" << endl;
            }
        }
        else {
            if (!query.name.empty())
                cout << "FORMULA " << query.name << " CANNOT_COMPUTE " << endl;
        }
    }
    else { // parsed & evaluated
        semaphore_sentinel sent(sem_id);
        const char* ctl_endl = (CTL_quiet ? "" : "\n");

        // We have a valid evaluated CTL formula
        bool is_int_formula = formula->isIntFormula();

        // Regression test
        const char *regression_res = "";
        bool failed_regression = false;
        if (query.expected && (*query.expected).which() == result.which()) { // Regression test
            if ((*query.expected) == result)
                regression_res = "[[OK]]";
            else {
                regression_res = "[[FAILED]]";
                failed_regression = true;
                *set_regr_failed = true;
            }
        }

        if (!running_for_MCC()) {
            if (CTL_quiet) {
                // compact single-line result
                cout << s_languageName[query.lang] << ": ";
                if (!query.name.empty())
                    cout << query.name;
                else
                    cout << query.line;
                cout << "  is  " << left << setw(8) 
                    << format_result(result, false) << regression_res;
                cout << "  Time: "<<std::fixed<<setprecision(3)<<(double(endMDD2-startMDD))/CLOCKS_PER_SEC;
                if (satset_card >= 0)
                    cout << "  SAT=" << satset_card;
                cout << endl;
            }
            else {
                // multiline result descriptor
                cout << "\n-------------------------------------------------------------------------------\n";
                if (!query.name.empty())
                    cout << "Formula: " << query.name << "\n";
                cout << "  " << *formula << endl;
                cout << "Language: " << s_languageName[query.lang] << "\n";
                cout << "Evaluation: " << left << setw(8) 
                     << format_result(result, false) << regression_res << endl;
                if (satset_card >= 0)
                    cout << "SAT=" << satset_card << endl;
                // if (!is_int_formula)
                //     cout<<"Sat-set generation time: "<<std::fixed<<setprecision(3)<<(double(endMDD-startMDD))/CLOCKS_PER_SEC<<" sec"<<endl;
                cout<<"Evaluation time: "<<std::fixed<<setprecision(3)<<(double(endMDD2-startMDD))/CLOCKS_PER_SEC<<" sec"<<endl;
                cout << "-------------------------------------------------------------------------------\n";
            }

            // Counter-example/witness generation
            if (print_CTL_counterexamples && !is_int_formula) {
                Formula* state_formula = dynamic_cast<Formula*>(formula);
                cout << "\nGenerated " << (boost::get<bool>(result) ? "witness: " : "counter-example: ") << endl;
                vector<int> state0(npl + 1);
                enumerator it0(g_rsrg->getInitMark());
                const int* tmp =it0.getAssignments();
                std::copy(tmp, tmp+npl + 1, state0.begin());
                
                TraceType traceTy = (boost::get<bool>(result) ? TT_WITNESS : TT_COUNTEREXAMPLE);
                TreeTraceNode *ttn = state_formula->generateTrace(state0, traceTy);
                print_banner(" Trace ");
                cout << "Initial state is: ";
                CTLMDD::getInstance()->print_state(state0.data());
                cout << endl;
                ttn->print_trace();
                cout << endl;
                delete ttn;
            }
        }
        else { // compact MCC result
            cout << "FORMULA " << query.name << " "
                 << format_result(result, true)
                 << " " << MCC_TECHNIQUES << "\n";
        }


        if (failed_regression) {
            if (!query.name.empty())
                cerr << "Expected CTL value of " << query.name;
            else
                cerr << "Expected CTL value";
            cerr << " is " << *query.expected << ", found "<<result<<endl;
            // throw rgmedd_exception("Regression test failed.");
        }

        if (invoked_from_gui()) {
            cout << "#{GUI}# RESULT " << query.name << " " 
                 << format_result(result, true);
            if (satset_card >= 0) {
                cout << " "<<satset_card<<"/"<<rs_card; // single additional string
            }
            cout  << endl;
        }

        ctx.fair_sets = ctx_fair_sets; // restore fairness constraints
        if (!is_int_formula && query.is_fairness_constraint) {
            Formula* state_formula = dynamic_cast<Formula*>(formula);
            // Add the sat-set as a new fairness constraint in the global evaluation context
            ctx.add_fairness_constraint(state_formula->getStoredMDD());
        }

        // Release the memory occupied by the CTL formula tree
        formula->removeOwner();
        formula = nullptr;
    }

    if (!running_for_MCC() && !CTL_quiet)
        cout << endl;
}

//-----------------------------------------------------------------------------

// Handle a query evaluation timeout induced by an ALARM signal
void handle_sigalarm_quit_query_eval(int) {
    if (!running_for_MCC())
        printf("\nTimed out. Killing query evaluation...\n");
    exit(EXIT_TIMEOUT_MC_QUERY);
}

//-----------------------------------------------------------------------------

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

std::string::const_iterator to_iterator(const std::string& str, size_t pos) {
    if (pos == std::string::npos)
        return str.end();
    return str.begin() + pos;
}

//-----------------------------------------------------------------------------

Language language_from_string(const char* str) {
    if      (0==strcmp(str, "CTL"))  return Language::CTL;
    else if (0==strcmp(str, "LTL"))  return Language::LTL;
    else if (0==strcmp(str, "CTL*"))  return Language::CTLSTAR;
    else if (0==strcmp(str, "CTLSTAR"))  return Language::CTLSTAR;
    else if (0==strcmp(str, "FAIRNESS"))  return Language::CTLSTAR;
    return Language::NUM_LANGUAGES; // unknown
}

//-----------------------------------------------------------------------------

// Parse input ctl queries read from file. Print the formula result on screen.
bool do_model_checking(RSRG *r) {
    g_rsrg = r;
    std::string filename = g_rsrg->getPropName();
    ifstream in;
    if (filename == "")
        filename = g_rsrg->getNetName() + std::string("ctl");
    in.open(filename.c_str());
    if (!in)
    {
        cout<<"Error opening CTL file: "<<filename<<"\n";
        if (running_for_MCC())
            cout<<"CANNOT_COMPUTE"<<endl;
        exit(EXIT_FAILURE);
    }
    // inizializzo la classe CTL dove ho i riferimenti a rs foreste e domini per tutte le altre classi
    CTLMDD::getInstance()->CTLinit();
    // filename = filename + ".output";
    // ofstream fout;
    // if (output_flag)
    // {
    //     fout.open(filename.c_str());
    //     if (!fout)
    //     {
    //         cout<<"Error opening output CTL file: "<<filename<<"\n";
    //         if (running_for_MCC())
    //             cout<<"CANNOT_COMPUTE"<<endl;
    //         exit(EXIT_FAILURE);
    //     }
    // }

    // read all lines from the input file, and separate queries from comments.
    std::vector<ctl_query_t> queries;
    {
        std::string name, line;
        Language lang = Language::CTL;
        bool is_fair = true;
        bool always_use_SatELTL = eval_CTL_using_SatELTL;
        while(!in.eof()) {
            getline(in, line);

            // remove any comment
            auto end = to_iterator(line, line.find_first_of("%"));
            line = std::string(line.cbegin(), end);
            rtrim(line);

            if (!line.empty()) {
                if (line.rfind("FORMULA:", 0) == 0) { // formula identifier
                    line = std::string(line, 8);
                    ltrim(line);
                    name = line;
                }
                else if (line.rfind("LANGUAGE:", 0) == 0) { // language name
                    line = std::string(line, 9);
                    ltrim(line);
                    lang = language_from_string(line.c_str());
                    is_fair = (0 == strcmp(line.c_str(), "FAIRNESS"));
                    if (lang == NUM_LANGUAGES) {
                        cerr << "Unknown language:" << line<< endl;
                    }
                }
                else if (0 == strcmp(line.c_str(), "ALWAYS_USE_SatELTL")) {
                    always_use_SatELTL = true;
                }
                else if (0 == strcmp(line.c_str(), "USE_SatELTL_FOR_LTL_ONLY")) {
                    always_use_SatELTL = false;
                }
                else { // query line
                    ctl_query_t q;
                    q.name  = name;
                    q.line  = line;
                    q.score = 0;
                    q.lang  = lang;
                    q.is_fairness_constraint = is_fair;
                    q.always_use_SatELTL = always_use_SatELTL;

                    // cout << s_languageName[q.lang]<<" " << q.name<<":    "<<q.line<<endl;
                    queries.emplace_back(std::move(q));
                    name = "";
                    is_fair = false;
                    always_use_SatELTL = eval_CTL_using_SatELTL;
                } 
            }
        }
        in.close();
    }

    // sort CTL/CTL* queries according to a complexity function heuristics
    std::vector<size_t> query_order(queries.size());
    std::iota(query_order.begin(), query_order.end(), 0);
    if (sort_CTL_queries) {
        // sort(queries.begin(), queries.end(),
        //      [] (const auto& f, const auto& g) -> bool {
        //          // count max num. nested parenthesis
        //          const auto mpf = max_nested_par(get<0>(f));
        //          const auto mpg = max_nested_par(get<0>(g));

        //          // count num. path operators
        //          const auto nof = num_path_ops(get<0>(f));
        //          const auto nog = num_path_ops(get<0>(g));

        //          return mpf + nof < mpg + nog;
        //      });
        // if (!running_for_MCC() && !CTL_quiet)
        //     cout << "--- Sorted " << queries.size() << " queries ---" << endl;
    }

    // carica i risultati attesi per le query
    for (size_t i=0; i<queries.size(); i++) {
        auto kv = g_rsrg->expected_results.find(queries[i].name);
        if (kv != g_rsrg->expected_results.end())
            queries[i].expected = kv->second;
        if (!CTL_quiet && !running_for_MCC()) {
            // const auto n = get<1>(p);
            if (queries[i].expected)
                cout << queries[i].name 
                     << "\texpecting: " << *(queries[i].expected)
                     << ":\t" << queries[i].line << endl;
            else
                cout << queries[i].name << ":\t " << queries[i].line << endl;
        }
    }

    // Get requirements
    bool hasCTL=false, hasLTL=false, hasCTLSTAR=false;
    for (auto&& q : queries) {
        switch (q.lang) {
            case Language::CTL:      hasCTL=true;        break;
            case Language::LTL:      hasLTL=true;        break;
            case Language::CTLSTAR:  hasCTLSTAR=true;    break;
            default: throw rgmedd_exception();
        }
    }
    // bool implicitNextForCTL = true;

    // Evaluate the availability of the RS
    bool need_rs = (hasCTLSTAR || hasCTL || hasLTL);
    if (need_rs && !g_rsrg->has_RS()) {
        cout << "RS is not built. Could not evaluate formulas." << endl;
        return false;
    }
    if (need_rs && !g_rsrg->has_finite_RS()) {
        if (!running_for_MCC())
            cout << "State space is infinite. Could not evaluate formulas." << endl;
        else
            cout << "CANNOT_COMPUTE" << endl;
        return false;
    }

    bool need_monoNSF = !implicitNextForCTLstar;
//    bool need_monoNSF = (((hasLTL||hasCTLSTAR) && !LTL_implicit_RSxBA) || (hasCTL && !implicitNextForCTL));
//    bool need_monoNSF = (hasLTL || hasCTLSTAR || (hasCTL && !implicitNextForCTL));
    if (need_monoNSF)
        g_rsrg->prepareNSFforCTL();

    bool need_eventNSF = implicitNextForCTLstar;
    // bool need_eventNSF = (hasCTL && implicitNextForCTL) || ((hasLTL||hasCTLSTAR) && LTL_implicit_RSxBA);
    if (need_eventNSF) {
        g_rsrg->prepareEventMxDsForCTL();
    }

    // cout << "hasCTL: " << hasCTL << endl;
    // cout << "hasLTL: " << hasLTL << endl;
    // cout << "hasCTLSTAR: " << hasCTLSTAR << endl;
    // cout << "need_monoNSF: " << need_monoNSF << endl;
    // cout << "need_eventNSF: " << need_eventNSF << endl;

    // Prepare the shared evaluation context
    unique_ptr<VirtualNSF> vNSF;
    if (implicitNextForCTLstar) {
        cout << "MODEL CHECKING USING IMPLICIT NEXT." << endl;
        vNSF = make_unique<ByEventVirtualNSF>(g_rsrg);  // BySeparateEventVirtualNSF OLD, ByEventVirtualNSF NEW
    }
    else {
        vNSF = make_unique<MonoVirtualNSF>(g_rsrg->getNSF());
    }
    
    Context ctx(g_rsrg, g_rsrg->getRS(), 
                mdd_potential_state_set(g_rsrg, g_rsrg->getForestMDD(), true),
                std::move(vNSF), 
                false, false, /* EG/EX stuttering is by-query */
                false, // SatELTL usage is decided per query.
                Language::NUM_LANGUAGES, 
                CTL_atoms_use_potential_state_space ? StateSetPolicy::POTENTIAL : StateSetPolicy::ACTUAL,
                true);


    // Query evaluation
    int sem_id = -1;
    bool failed_regressions = false;
    if (!is_mc_parallel()) {
        //=================================
        // Sequential query evaluation
        //=================================
        for (size_t step=0; step<2; step++) { // step 0=fairness constraints, step 1=queries
            for(const ctl_query_t& q: queries) {
                if (q.is_fairness_constraint == (step==0))
                    model_check_query(ctx, q, sem_id, &failed_regressions);
            } 
        }
        if (!running_for_MCC()) {
            cout << "Formula cache size: " << CTLMDD::getInstance()->cache_size() << endl;
        }
    }
    else {
        //=================================
        // Parallel query evaluation
        //=================================
        sem_id = semaphore_create();

        std::function<void(int, int)> query_ends = [&queries](int i, int exitcode) {
            if (!running_for_MCC())
                cout << "EXITCODE " << queries[i].name << " " << WEXITSTATUS(exitcode) << " " << endl;
        };

        // PHASE 1: both time and memory bound
        std::vector<int> exitcodes;
        int qid = parallel_exec(g_par_mc_num_parallel_procs,
                                queries.size(), nullptr,
                                exitcodes, query_ends, 
                                false && !running_for_MCC());
        if (qid >= 0) { // working subprocess            
            if (g_par_mc_max_time_round0 > 0) {
                // Start the timer
                signal(SIGALRM, handle_sigalarm_quit_query_eval);
                alarm(g_par_mc_max_time_round0);
            }
            if (g_par_mc_max_MB_statespace > 0)
                constraint_address_space(g_par_mc_max_MB_statespace);

            model_check_query(ctx, queries[qid], sem_id, &failed_regressions);
            exit(failed_regressions ? EXIT_FAILURE_REGRESSION : EXIT_SUCCESS);
        }
        // cout << "\n\nPHASE 2\n\n" << endl;

        // all parallel tasks have finished: collect the unfinished queries
        size_t n_timedout = 0;
        for (size_t i=0; i<queries.size(); i++) {
            if (WEXITSTATUS(exitcodes[i]) == EXIT_TIMEOUT_MC_QUERY)
                n_timedout++;
            else if (WEXITSTATUS(exitcodes[i]) != 0 && running_for_MCC() 
                     && !queries[i].name.empty()) {
                cout << "FORMULA " << queries[i].name 
                     << " CANNOT_COMPUTE " << endl;
            }
            if (WEXITSTATUS(exitcodes[i]) == EXIT_FAILURE_REGRESSION )
                failed_regressions = true;
        }

        if (n_timedout > 0) {
            std::vector<int> exitcodes_ph2(queries.size(), -1);
            // PHASE 2 - restart timed-out queries with unbounded time
            if (g_par_mc_do_second_round) {
                qid = parallel_exec(g_par_mc_num_parallel_procs,
                                    queries.size(), nullptr,
                                    exitcodes_ph2, query_ends, 
                                    false && !running_for_MCC());
                if (qid >= 0) { // working subprocess
                    if (WEXITSTATUS(exitcodes[qid]) != EXIT_TIMEOUT_MC_QUERY)
                        exit(EXIT_SUCCESS); // already completed
                    
                    if (g_par_mc_max_MB_statespace > 0)
                        constraint_address_space(g_par_mc_max_MB_statespace);

                    model_check_query(ctx, queries[qid], sem_id, &failed_regressions);
                    exit(failed_regressions ? EXIT_FAILURE_REGRESSION : EXIT_SUCCESS);
                }
            }

            // PHASE 3: notify unevaluated queries
            for (size_t i=0; i<queries.size(); i++) {
                if (WEXITSTATUS(exitcodes[i]) == EXIT_TIMEOUT_MC_QUERY && 
                    WEXITSTATUS(exitcodes_ph2[i]) != 0) 
                {
                    if (!queries[i].name.empty() && running_for_MCC())
                        cout << "FORMULA " << queries[i].name 
                             << " CANNOT_COMPUTE " << endl;
                }
                if (WEXITSTATUS(exitcodes_ph2[i]) == EXIT_FAILURE_REGRESSION)
                    failed_regressions = true;
            }
        }

        semaphore_close(sem_id);
    }

    // if (fout)
    //     fout.close();
    cout << "Ok." << endl;
    return failed_regressions;
}

//-----------------------------------------------------------------------------
