//---------------------------------------------------------------------------------------
#ifndef __HEURISTIC_OBJECTIVE_FINDER_H__
#define __HEURISTIC_OBJECTIVE_FINDER_H__
//---------------------------------------------------------------------------------------

// Heuristic optimization of a combinatorial NP-hard problem
// using Boltzmann simulated annealing procedure.
namespace optimization_finder {

    struct conf {
        size_t max_tentatives = 50000;
        size_t max_seconds = 20;
        double init_temperature_mult = 2.0;
        double temperature_delta = 0.0005;
    };

    enum message {
        IMPROVEMENT,   // Got a new solution with a better score
        TIME_EXCEEDED, // Quit due to exceeded time.
        REACHED_LOCAL_MINIMUM, // End of annealing
    };

    // Try improving a target objective using the score function
    template<class Objective, class ScoreFn, class NextFn, class MsgFn>
    void
    optimize(Objective& best_objective, ScoreFn&& get_score, 
             NextFn&& generate, MsgFn&& msg, const conf& conf) 
    {
        typedef decltype(get_score(best_objective))  Score;
        // using namespace nbsdx::concurrent;
        clock_t time_start = clock();    

        // Measure the initial score
        Score score = get_score(best_objective);

        // Initial temperature and annealing delta
        double temperature = score * conf.init_temperature_mult;
        const double deltaT = score * conf.temperature_delta;

        // The current objective during the iterations.
        // Note that the score of the current objective could be worst than
        // the score of the best_objective, since the algorithm allows detours.
        Objective curr_objective = best_objective;
        Objective new_objective = best_objective;

        // score of the best observed solution (assigned to best_objective)
        Score best_score = score;
        msg(IMPROVEMENT, 0, score, score);

        size_t num_iter = 0;
        while (num_iter < conf.max_tentatives &&
               best_score < temperature) 
        {
            num_iter++;

            // Make a new objective with a neighbourhood search
            generate(curr_objective, new_objective);
            Score new_score = get_score(new_objective);
            bool accept = false;

            if (new_score < score) { // Got an improvement, always accept
                accept = true;
            }
            else if (new_score < temperature) {
                // Accept probabilistically the new state. Use Boltzmann annealing
                double lambda = (new_score - score) / temperature;
                double probability = 1 - exp(-lambda);
                // cout << "probability = " << probability << endl;
                double x = genrand64_real1(); // in range [0,1]

                // Accept if x < e^{- deltaE / T}
                accept = (x < probability);
            }

            if (accept) { // switch the current objective
                curr_objective = new_objective;
                score = new_score;
                if (score < best_score) {
                    msg(IMPROVEMENT, num_iter, score, new_score);
                    best_score = score;
                    best_objective = curr_objective;
                }
            }

            clock_t elapsed = clock() - time_start;
            if (elapsed > conf.max_seconds * CLOCKS_PER_SEC) {
                msg(TIME_EXCEEDED, num_iter, score, score);
                break;
            }
            // Reduce the temperature and continue the iterations.
            temperature -= deltaT;
        }

        msg(REACHED_LOCAL_MINIMUM, num_iter, score, score);
    }

}; // namespace optimization_finder

//---------------------------------------------------------------------------------------

// // Heuristic optimization of a combinatorial NP-hard problem
// // using modified simulated annealing/taboo search procedure.
// namespace optimization_finder {

//     struct conf {
//         size_t max_tentatives = 50000;
//         size_t num_failures_before_detours = 10; // 5
//         size_t min_local_serch_tentatives = 50; // 0
//         size_t max_seconds = 20;
//         size_t max_consecutive_failures = 2000;
//     };

//     enum message {
//         IMPROVEMENT,   // Got a new solution with a better score
//         TIME_EXCEEDED, // Quit due to exceeded time.
//         REACHED_LOCAL_MINIMUM, // Too many consecutive failures, probably a local minimum
//     };

//     // Try improving a target objective using the score function
//     template<class Objective, class ScoreFn, class NextFn, class MsgFn>
//     void
//     optimize(Objective& objective, ScoreFn&& get_score, 
//              NextFn&& generate, MsgFn&& msg, const conf& conf) 
//     {
//         typedef decltype(get_score(objective))  Score;
//         // using namespace nbsdx::concurrent;
//         clock_t time_start = clock();    

//         // Measure the initial score
//         Score score = get_score(objective);

//         Objective new_objective = objective;
//         size_t num_iter = 0;
//         size_t num_failed_to_detour = 0;
//         size_t num_last_failed = 0;
//         size_t local_search_tentatives = 0;
//         while (num_iter < conf.max_tentatives) {
//             num_iter++;

//             // Make a new objective with a neighbourhood search
//             generate(objective, new_objective);
//             Score new_score = get_score(new_objective);

//             if (new_score < score) { // Got an improvement!
//                 msg(IMPROVEMENT, num_iter, score, new_score);
//                 // improvement();
//                 objective = new_objective;
//                 score = new_score;
//                 num_failed_to_detour = 0;
//                 local_search_tentatives = 0;
//                 num_last_failed = 0;
//             }
//             else {
//                 num_last_failed++;
//                 if (local_search_tentatives < conf.min_local_serch_tentatives) {
//                     local_search_tentatives++;
//                     new_objective = objective;
//                     // if (local_search_tentatives == min_local_serch_tentatives) {
//                     //     cout << "start of detour at iteration " << num_iter << endl;
//                     // }
//                 }
//                 // Implement taboo search: normally move to neighbouring states of lower score values, 
//                 // but will take uphill moves when it finds itself stuck in a local minimum.
//                 else if (num_failed_to_detour < conf.num_failures_before_detours) {
//                     // Try again, we may still have a chance of finding a better 
//                     // objective in the next steps.
//                     num_failed_to_detour++;
//                 }
//                 else {
//                     // Restart back from objective
//                     new_objective = objective;
//                     num_failed_to_detour = 0;
//                 }
//             }
//             clock_t elapsed = clock() - time_start;
//             if (elapsed > conf.max_seconds * CLOCKS_PER_SEC) {
//                 msg(TIME_EXCEEDED, num_iter, score, score);
//                 break;
//             }
//             if (num_last_failed > conf.max_consecutive_failures) {
//                 msg(REACHED_LOCAL_MINIMUM, num_iter, score, score);
//                 break;
//             }
//         }
//         // return score;
//     }

// }; // namespace optimization_finder

//---------------------------------------------------------------------------------------
#endif //__HEURISTIC_OBJECTIVE_FINDER_H__
