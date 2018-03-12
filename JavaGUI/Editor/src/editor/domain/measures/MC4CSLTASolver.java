/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import common.Util;
import editor.domain.grammar.EvaluatedBinding;
import editor.domain.grammar.EvaluationArguments;
import editor.domain.grammar.TemplateBinding;
import java.io.File;
import java.io.IOException;
import java.util.Map;

/**
 *
 * @author elvio
 */
public class MC4CSLTASolver extends SolverInvokator {
    
    @Override
    void prepareStepForBinding(SolutionStep step, TemplateBinding currBind,
                               AbstractMeasure[] measures) throws IOException
    {
        step.cmdLines = new String[1];
        step.cmdLines[0] = useGreatSPN_binary("DSPN-Tool");
        if (!new File(step.cmdLines[0]).canExecute()) {
            throw new IllegalStateException("The path to the MC4CSLTA solver is not set.\n"
                    + "You must first set the solver path from the Edit > Options menu.");
        }
        step.cmdLines[0] += " -load " + quotedFn(null);
        String performanceIndexTime;
        
        // Add the solver parameters
        assert getPage().solverParams instanceof MC4CSLTASolverParams;
        
        MC4CSLTASolverParams params = (MC4CSLTASolverParams)getPage().solverParams;
        EvaluatedBinding evalBind = currBind.createEvaluated(getContext());
        params.epsilon.checkExprCorrectness(getContext(), getPage().targetGspn, null);
        step.cmdLines[0] += " -epsilon "+params.epsilon.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarRealOrIntAsReal();
        step.cmdLines[0] += params.onTheFly ? " -on-the-fly" : "";
//            step.cmdLines[0] += params.mcSolType==MC4CSLTASolverParams.McSolutionType.FORWARD ?
//                            " -fmc" : " -bmc";
        switch (params.mrpMethod) {
            case EXPLICIT:         step.cmdLines[0] += " -e"; break;
            case IMPLICIT:         step.cmdLines[0] += " -i"; break;
            case COMPONENT_METHOD: step.cmdLines[0] += " -i -scc"; break;
        }
        switch (params.linAlgo) {
            case FORWARD_SOR:  step.cmdLines[0] += " -forward-sor"; break;
            case BACKWARD_SOR: step.cmdLines[0] += " -backward-sor"; break;
            case JOR:          step.cmdLines[0] += " -jor"; break;
            case GMRES:        step.cmdLines[0] += " -gmres"; break;
            case CGS:          step.cmdLines[0] += " -cgs"; break;
            case BICGSTAB:     step.cmdLines[0] += " -bicgstab"; break;
            case POWER_METHOD: step.cmdLines[0] += " -i-power"; break;
            default: 
                throw new UnsupportedOperationException();
        }

        // What stochastic solution will be computed
        if (params.piTime == MC4CSLTASolverParams.PerformanceIndexTime.STEADY_STATE)
            performanceIndexTime = " -s";
        else {
            params.timeT.checkExprCorrectness(getContext(), getPage().targetGspn, null);
            performanceIndexTime = " -t "+params.timeT.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarReal();
        }
        
        // Extra RG/TRG arguments
        String extraTrgArgs = "";
        if (params.dotClusters)
            extraTrgArgs += " -with-clusters";
        params.maxDotMarkings.checkExprCorrectness(getContext(), getPage().targetGspn, null);
        extraTrgArgs += " -max-dot-markings "+params.maxDotMarkings.evaluate
                    (getContext(), EvaluationArguments.NO_ARGS).getScalarInt();

        
        // Add the command for parameter bindings
        step.cmdLines[0] += getParamBindingCmd(currBind, true, true);
        
        // Prepare the measures passed on the command line
        int measureNum = 0;
        String measCmd = "", rgDot = "", trgDot = "";
        boolean hasMeasures = false, hasCslta = false, hasAll = false;
        boolean hasRG = false, hasTRG = false, hasStat = false;
        for (AbstractMeasure meas : measures) {
            String measName = "MEASURE"+(measureNum++);
            ResultEntry entry;
            if (meas instanceof FormulaMeasure) {
                FormulaMeasure fm = (FormulaMeasure)meas;
                switch (fm.getLanguage()) {
                    case CSLTA:
                        hasCslta = true;
                        entry = new StochasticMcResult(measName, evalBind);
                        measCmd += " -cslta0-X "+measName+" \"" + fm.getFormula().getExpr() +"\"";
                        step.entries.add(entry);
                        break;
                    case PERFORMANCE_INDEX:
                        hasMeasures = true;
                        entry = new ScalarResultEntry(measName, evalBind);
                        measCmd += " -measure "+measName+" \""+fm.getFormula().getExpr() +"\"";
                        step.entries.add(entry);
                        break;
                    case TRG:
                        hasTRG = true;
                        entry = new PdfResultEntry("TRG", evalBind, 
                                new File(getGspnFile().getAbsoluteFile()+"-TRG-"+step.stepNum+".pdf"));
                        trgDot = " -dot-F " + quotedFn("-TRG-"+step.stepNum);
                        step.entries.add(entry);
                        break;
                    case RG:
                        hasRG = true;
                        entry = new PdfResultEntry("RG", evalBind, 
                                new File(getGspnFile().getAbsoluteFile()+"-RG-"+step.stepNum+".pdf"));
                        rgDot = " -new-rg -dot-F " + quotedFn("-RG-"+step.stepNum); // require new RG construction
                        step.entries.add(entry);
                        break;
                    case ALL:
                        if (hasAll)
                            throw new UnsupportedOperationException("Only one ALL measure can be computed at once.");
                        hasAll = true;
                        entry = new AllMeasuresResult("ALL", evalBind);
                        step.entries.add(entry);
                        break;
                    case STAT:
                        if (hasStat)
                            throw new UnsupportedOperationException("Only one STAT measure can be computed at once.");
                        hasStat = true;
                        entry = new StatResultEntry("STAT", evalBind);
                        step.entries.add(entry);
                        break;
                    default:
                        throw new UnsupportedOperationException("Language "+fm.getLanguage()+" is not supported by the tool.");
                }
            }
            else throw new UnsupportedOperationException("missing measure impl.");
            meas.getResults().table.add(entry);
        }
        
        if (hasStat)
            step.cmdLines[0] += " -gui-stat";
        if (hasCslta)
            step.cmdLines[0] += " -dta-path \""+getSolutionDir().getAbsolutePath()+"\"";
        step.cmdLines[0] += measCmd;
        if (hasMeasures || hasAll)
            step.cmdLines[0] += performanceIndexTime;
        else if (hasStat && !(hasRG || hasTRG)) // No measure bu have stats, just build the TRG
            step.cmdLines[0] += " -new-trg";
        if (hasAll)
            step.cmdLines[0] += " -all-measures";
        if (hasRG || hasTRG) 
            step.cmdLines[0] += extraTrgArgs;
        if (hasTRG)
            step.cmdLines[0] += trgDot;
        if (hasRG) // keep last since it rebuilds the reachability graph with vanishings.
            step.cmdLines[0] += rgDot;
    }

    @Override
    void readCommand(String commandLine, int stepNum, AbstractMeasure[] measures) {
        defaultReadCommand(commandLine, stepNum, measures);
    }

    @Override
    void modifyEnvironmentVars(Map<String, String> env) {
    }

    @Override
    void startOfStep(SolutionStep step) {
    }

    @Override
    void endOfStep(SolutionStep step, boolean interrupted, boolean allStepsCompleted) {
        step.completed =  (!interrupted && allStepsCompleted);
    }
}
