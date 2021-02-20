/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import editor.domain.grammar.EvaluatedBinding;
import editor.domain.grammar.EvaluationArguments;
import editor.domain.grammar.TemplateBinding;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
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
        ArrayList<String> cmd = startOfCommand();
        cmd.add(useGreatSPN_binary("DSPN-Tool"));
//        String cmd = useGreatSPN_binary("DSPN-Tool");
//        if (!new File(cmd).canExecute()) {
//            throw new IllegalStateException("The path to the MC4CSLTA solver is not set.\n"
//                    + "You must first set the solver path from the Edit > Options menu.");
//        }
        cmd.add("-load");
        cmd.add(quotedFn(null));
//        cmd += " -load " + quotedFn(null);
        ArrayList<String> performanceIndexTime = new ArrayList<>();
        
        // Add the solver parameters
        assert getPage().solverParams instanceof MC4CSLTASolverParams;
        
        MC4CSLTASolverParams params = (MC4CSLTASolverParams)getPage().solverParams;
        EvaluatedBinding evalBind = currBind.createEvaluated(getContext());
        params.epsilon.checkExprCorrectness(getContext(), getPage().targetGspn, null);
        cmd.add("-epsilon");
        cmd.add(""+params.epsilon.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarRealOrIntAsReal());
        if (params.onTheFly)
            cmd.add("-on-the-fly");
        
//        cmd += " -epsilon "+params.epsilon.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarRealOrIntAsReal();
//        cmd += params.onTheFly ? " -on-the-fly" : "";
//            cmd += params.mcSolType==MC4CSLTASolverParams.McSolutionType.FORWARD ?
//                            " -fmc" : " -bmc";
        switch (params.mrpMethod) {
            case EXPLICIT:         cmd.add("-e"); break;
            case IMPLICIT:         cmd.add("-i"); break;
            case COMPONENT_METHOD: cmd.add("-i"); cmd.add("-scc"); break;
        }
        switch (params.linAlgo) {
            case FORWARD_SOR:  cmd.add("-forward-sor"); break;
            case BACKWARD_SOR: cmd.add("-backward-sor"); break;
            case JOR:          cmd.add("-jor"); break;
            case GMRES:        cmd.add("-gmres"); break;
            case CGS:          cmd.add("-cgs"); break;
            case BICGSTAB:     cmd.add("-bicgstab"); break;
            case POWER_METHOD: cmd.add("-i-power"); break;
            default: 
                throw new UnsupportedOperationException();
        }

        // What stochastic solution will be computed
        if (params.piTime == MC4CSLTASolverParams.PerformanceIndexTime.STEADY_STATE)
            performanceIndexTime.add("-s");
        else {
            params.timeT.checkExprCorrectness(getContext(), getPage().targetGspn, null);
            performanceIndexTime.add("-t ");
            performanceIndexTime.add(""+params.timeT.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarReal());
        }
        
        // Extra RG/TRG arguments
        ArrayList<String> extraTrgArgs = new ArrayList<>();
        if (params.dotClusters)
            extraTrgArgs.add("-with-clusters");
        params.maxDotMarkings.checkExprCorrectness(getContext(), getPage().targetGspn, null);
        extraTrgArgs.add("-max-dot-markings");
        extraTrgArgs.add(""+params.maxDotMarkings.evaluate
                         (getContext(), EvaluationArguments.NO_ARGS).getScalarInt());

        
        // Add the command for parameter bindings
        cmd.addAll(getParamBindingCmdArgs(currBind, true, true));
//        cmd += getParamBindingCmd(currBind, true, true);
        
        // Prepare the measures passed on the command line
        int measureNum = 0;
        ArrayList<String> measCmd = new ArrayList<>();
        ArrayList<String> rgDot = new ArrayList<>();
        ArrayList<String> trgDot = new ArrayList<>();
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
                        measCmd.add("-cslta0-X");
                        measCmd.add(measName);
                        measCmd.add(fm.getFormula().getExpr());
//                        measCmd += " -cslta0-X "+measName+" \"" + fm.getFormula().getExpr() +"\"";
                        step.entries.add(entry);
                        break;
                    case PERFORMANCE_INDEX:
                        hasMeasures = true;
                        entry = new ScalarResultEntry(measName, evalBind);
                        measCmd.add("-measure");
                        measCmd.add(measName);
                        measCmd.add(fm.getFormula().getExpr());
//                        measCmd += " -measure "+measName+" \""+fm.getFormula().getExpr() +"\"";
                        step.entries.add(entry);
                        break;
                    case TRG:
                        hasTRG = true;
                        entry = new PdfResultEntry("TRG", evalBind, 
                                new File(getGspnFile().getAbsoluteFile()+"-TRG-"+step.stepNum+".pdf"));
                        trgDot.add("-dot-F");
                        trgDot.add(quotedFn("-TRG-"+step.stepNum));
//                        trgDot = " -dot-F " + quotedFn("-TRG-"+step.stepNum);
                        step.entries.add(entry);
                        break;
                    case RG:
                        hasRG = true;
                        entry = new PdfResultEntry("RG", evalBind, 
                                new File(getGspnFile().getAbsoluteFile()+"-RG-"+step.stepNum+".pdf"));
                        rgDot.add("-new-rg");
                        rgDot.add("-dot-F");
                        rgDot.add(quotedFn("-RG-"+step.stepNum));
//                        rgDot = " -new-rg -dot-F " + quotedFn("-RG-"+step.stepNum); // require new RG construction
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
            cmd.add("-gui-stat");
        if (hasCslta) {
            cmd.add("-dta-path");
            cmd.add(getSolutionDir().getAbsolutePath());
        }
        cmd.addAll(measCmd);
        if (hasMeasures || hasAll)
            cmd.addAll(performanceIndexTime);
        else if (hasStat && !(hasRG || hasTRG)) // No measure bu have stats, just build the TRG
            cmd.add("-new-trg");
        if (hasAll)
            cmd.add("-all-measures");
        if (hasRG || hasTRG) 
            cmd.addAll(extraTrgArgs);
        if (hasTRG)
            cmd.addAll(trgDot);
        if (hasRG) // keep last since it rebuilds the reachability graph with vanishings.
            cmd.addAll(rgDot);
        
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
