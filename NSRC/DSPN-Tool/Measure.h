/*
 *  Measure.h
 *
 *  Performance index computation for Petri Net
 *
 *  Created by Elvio Amparore
 *
 */
//=============================================================================
#ifndef __MEASURE_H__
#define __MEASURE_H__
//=============================================================================

struct Measure;
struct PetriNetSolution;

typedef struct PetriNetSolution		PetriNetSolution;
typedef struct Measure				Measure;
typedef struct RewardVectors		RewardVectors;

// Forward declaration
struct RG;
struct RG2;

//=============================================================================
//     Measure Definitions
//=============================================================================

// Solution vector o a Petri Net
struct PetriNetSolution {
    ublas::vector<double>	stateProbs;	 // State probabilities
    ublas::vector<double>   firingFreq;  // Transition firing frequencies
};

// Reward vector
struct RewardVectors {
    ublas::vector<double>	rateRewards;	 // State rewards
    ublas::vector<double>   impulseRewards;  // Transition firing rewards
};

//=============================================================================

// Preprocessed Measure expression base class
struct PpMeasureExpr : public PetriNetObject {
    // Evaluate this measure against the solution vector
    virtual double EvaluateMeasure(const PetriNetSolution &pns) = 0;
    // Construct the rate reward and the impulse reward vectors
    virtual void SetupRewardVectors(size_t numStates, RewardVectors &rv) = 0;
};

//=============================================================================

// Preprocessed measure
struct PpMeasure {
    string						name;
    string						printedExpr;
    shared_ptr<PpMeasureExpr>	expr;
    double						result;
};

//=============================================================================

// Base class of each measure expression
struct MeasureExpr : public PetriNetObject {
    // Preprocessing: compute rewards associated to TRG markings
    virtual shared_ptr<PpMeasureExpr> Preprocess(const PN &pn, const RG &rg) = 0;
    virtual shared_ptr<PpMeasureExpr> Preprocess(const PN &pn, const RG2 &rg) = 0;
    // Converts this measure in the SPNica format
    virtual void PrintInSPNicaFormat(ostream &os, const PN &pn) const = 0;
};

//=============================================================================

void PreprocessMeasures(const PN &pn, RG &rg, VerboseLevel verboseLvl);
void PreprocessMeasures(const PN &pn, RG2 &rg, VerboseLevel verboseLvl);

void ComputeMeasures(RG &rg, const PetriNetSolution &sol, VerboseLevel verboseLvl);
void ComputeMeasures(RG2 &rg, const PetriNetSolution &sol, VerboseLevel verboseLvl);

void SaveMeasureValuesOnFile(RG &rg, ofstream &res, VerboseLevel verboseLvl);
void SaveMeasureValuesOnFile(RG2 &rg, ofstream &res, VerboseLevel verboseLvl);

void SaveStateProbsOnFile(const PetriNetSolution &sol, ofstream &res, VerboseLevel verboseLvl);

void GetMeasureRewardVectors(RG &rg, const char *measureName, RewardVectors &rv);
void GetMeasureRewardVectors(RG2 &rg, const char *measureName, RewardVectors &rv);

void CompareMeasuresAgainstFile(const PetriNetSolution &sol, ifstream &wrf,
                                double epsilon);

void ComputeAllBasicMeasures(const PN &pn, const RG2 &rg, const PetriNetSolution &pns);

//=============================================================================



//=============================================================================
#endif   // __MEASURE_H__
