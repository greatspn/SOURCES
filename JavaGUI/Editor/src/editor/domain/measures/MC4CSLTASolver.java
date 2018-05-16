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
        String cmd = useGreatSPN_binary("DSPN-Tool");
        if (!new File(cmd).canExecute()) {
            throw new IllegalStateException("The path to the MC4CSLTA solver is not set.\n"
                    + "You must first set the solver path from the Edit > Options menu.");
        }
        cmd += " -load " + quotedFn(null);
        String performanceIndexTime;
        
        // Add the solver parameters
        assert getPage().solverParams instanceof MC4CSLTASolverParams;
        
        MC4CSLTASolverParams params = (MC4CSLTASolverParams)getPage().solverParams;
        EvaluatedBinding evalBind = currBind.createEvaluated(getContext());
        params.epsilon.checkExprCorrectness(getContext(), getPage().targetGspn, null);
        cmd += " -epsilon "+params.epsilon.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarRealOrIntAsReal();
        cmd += params.onTheFly ? " -on-the-fly" : "";
//            cmd += params.mcSolType==MC4CSLTASolverParams.McSolutionType.FORWARD ?
//                            " -fmc" : " -bmc";
        switch (params.mrpMethod) {
            case EXPLICIT:         cmd += " -e"; break;
            case IMPLICIT:         cmd += " -i"; break;
            case COMPONENT_METHOD: cmd += " -i -scc"; break;
        }
        switch (params.linAlgo) {
            case FORWARD_SOR:  cmd += " -forward-sor"; break;
            case BACKWARD_SOR: cmd += " -backward-sor"; break;
            case JOR:          cmd += " -jor"; break;
            case GMRES:        cmd += " -gmres"; break;
            case CGS:          cmd += " -cgs"; break;
            case BICGSTAB:     cmd += " -bicgstab"; break;
            case POWER_METHOD: cmd += " -i-power"; break;
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
        cmd += getParamBindingCmd(currBind, true, true);
        
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
            cmd += " -gui-stat";
        if (hasCslta)
            cmd += " -dta-path \""+getSolutionDir().getAbsolutePath()+"\"";
        cmd += measCmd;
        if (hasMeasures || hasAll)
            cmd += performanceIndexTime;
        else if (hasStat && !(hasRG || hasTRG)) // No measure bu have stats, just build the TRG
            cmd += " -new-trg";
        if (hasAll)
            cmd += " -all-measures";
        if (hasRG || hasTRG) 
            cmd += extraTrgArgs;
        if (hasTRG)
            cmd += trgDot;
        if (hasRG) // keep last since it rebuilds the reachability graph with vanishings.
            cmd += rgDot;
        
        step.addCmd(cmd);
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
