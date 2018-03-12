/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import editor.domain.grammar.EvaluatedBinding;
import editor.domain.grammar.EvaluationArguments;
import editor.domain.grammar.TemplateBinding;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Map;

/**
 *
 * @author elvio
 */
public class RGMEDDSolver extends SolverInvokator {
    
    @Override
    void prepareStepForBinding(SolutionStep step, TemplateBinding currBind, 
                               AbstractMeasure[] measures) throws IOException
    {
        assert getPage().solverParams instanceof RGMEDDSolverParams;
        RGMEDDSolverParams params = (RGMEDDSolverParams)getPage().solverParams;
        EvaluatedBinding evalBind = currBind.createEvaluated(getContext());
        boolean callPinvar = (params.plBoundMeth == RGMEDDSolverParams.PlaceBoundMethod.FROM_PINVAR);
        int s = 0;
        step.cmdLines = new String[callPinvar ? 3 : 1];
        
        if (callPinvar) {
            // Add a pinvar invocation step
            step.cmdLines[s] = useGreatSPN_binary("pinvar") + " " + quotedFn(null) + " ";
            step.cmdLines[s] += getParamBindingCmd(currBind, true, false);
            s++;
            // Add a struct invocation step (write also mpar switches)
            step.cmdLines[s] = useGreatSPN_binary("struct") + " " + quotedFn(null) + " -only-bnd ";
            step.cmdLines[s] += getParamBindingCmd(currBind, true, false);
            s++;
        }
        
        step.cmdLines[s] = useGreatSPN_binary("RGMEDD") + " " + quotedFn(null);
        if (params.plBoundMeth == RGMEDDSolverParams.PlaceBoundMethod.MANUALLY_SPECIFIED) {
            params.placeBound.checkExprCorrectness(getContext(), getPage().targetGspn, null);
            step.cmdLines[s] += " -B "+params.placeBound.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarInt();
        }
        switch (params.varOrder) {
            case NO_ORDER: break;
            case HEURISTIC_PINV:        step.cmdLines[s] += " -P";           break;
            case HEURISTIC_FORCE_WES1:  step.cmdLines[s] += " -FORCE-WES1";  break;
            case HEURISTIC_FORCE_PINV:  step.cmdLines[s] += " -FORCE-P";     break;
            case FROM_FILE:             step.cmdLines[s] += " -F";           break;
        }
        if (params.genCounterExamples)
            step.cmdLines[s] += " -c";
        
        // Add the command for parameter bindings
        step.cmdLines[s] += getParamBindingCmd(currBind, true, true);
        
        // Format measures
        int measureNum = 0;
        File ctlFilename = new File(getGspnFile().getAbsolutePath()+".ctl");
        BufferedWriter ctlWriter = null;
        boolean hasStat = false;
        for (AbstractMeasure meas : measures) {
            String measName = "MEASURE"+(measureNum++);
            ResultEntry entry;
            if (meas instanceof FormulaMeasure) {
                FormulaMeasure fm = (FormulaMeasure)meas;
                switch (fm.getLanguage()) {
                    case CTL:
                        if (ctlWriter == null) {
                            ctlWriter = new BufferedWriter(new FileWriter(ctlFilename));
                        }
                        entry = new ModelCheckingResultEntry(measName, evalBind);
                        ctlWriter.append("% ").append(measName).append("\n");
//                        String exCTL = fm.getFormula().convertLang(getContext(), EvaluationArguments.NO_ARGS, 
//                                        ExpressionLanguage.GREATSPN);
                        ctlWriter.append(fm.getFormula().getExpr()).append("\n");
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
            step.cmdLines[s] += " -gui-stat";
        if (ctlWriter != null) { // there are CTL measures to compute
            ctlWriter.close();
            step.cmdLines[s] += " -C";
        }
    }

    @Override
    void readCommand(String command, int stepNum, AbstractMeasure[] measures) {
        defaultReadCommand(command, stepNum, measures);
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
