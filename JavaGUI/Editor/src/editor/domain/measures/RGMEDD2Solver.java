/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import editor.domain.grammar.EvaluatedBinding;
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
public class RGMEDD2Solver extends SolverInvokator {
    
    @Override
    void prepareStepForBinding(SolutionStep step, TemplateBinding currBind, 
                               AbstractMeasure[] measures) throws IOException
    {
        assert getPage().solverParams instanceof RGMEDD2SolverParams;
        RGMEDD2SolverParams params = (RGMEDD2SolverParams)getPage().solverParams;
        EvaluatedBinding evalBind = currBind.createEvaluated(getContext());
        RGMEDD2SolverParams.VariableOrder varOrder = params.varOrder;
        boolean callPinvar = varOrder.usesPinvars();
        int s = 0;
        step.cmdLines = new String[callPinvar ? 4 : 2];
        
        if (callPinvar) {
            // Add a pinvar invocation step
            step.cmdLines[s] = useGreatSPN_binary("pinvar") + " " + quotedFn(null) + " -detect-exp ";
            step.cmdLines[s] += getParamBindingCmd(currBind, true, false);
            s++;
            // Add a struct invocation step (write also mpar switches)
            step.cmdLines[s] = useGreatSPN_binary("struct") + " " + quotedFn(null) + " -only-bnd ";
            step.cmdLines[s] += getParamBindingCmd(currBind, true, false);
            s++;
        }
        
        // Generate the P-basis
        step.cmdLines[s] = useGreatSPN_binary("DSPN-Tool") + " -load "+ quotedFn(null) + " -pbasis ";
        s++;
        
        step.cmdLines[s] = useGreatSPN_binary("RGMEDD2") + " " + quotedFn(null);
        step.cmdLines[s] += " " + varOrder.getCmdOption() + " ";
        if (params.genCounterExamples)
            step.cmdLines[s] += " -c";
        
        // Add the command for parameter bindings
        step.cmdLines[s] += getParamBindingCmd(currBind, true, true);
        
        // Format measures
        int measureNum = 0;
        File ctlFilename = new File(getGspnFile().getAbsolutePath()+".ctl");
        BufferedWriter ctlWriter = null;
        boolean hasStat = false;
        String ddCmd = "", incCmd = "";
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
                        
                    case DD: {
                        String f = getGspnFile().getAbsoluteFile()+"-DD-"+step.stepNum;
                        entry = new PdfResultEntry("DD", evalBind, new File(f+".pdf"));
                        ddCmd = " -dot-F " + quotedFn("-DD-"+step.stepNum)+" ";
                        step.entries.add(entry);
                        break;
                    }
                    
                    case INC: {
                        String f = getGspnFile().getAbsoluteFile()+"-INC-"+step.stepNum;
                        entry = new PdfResultEntry("INC", evalBind, new File(f+".pdf"));
                        incCmd = " -inc-F " + quotedFn("-INC-"+step.stepNum)+" ";
                        step.entries.add(entry);
                        break;
                    }
                        
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
        step.cmdLines[s] += ddCmd;
        step.cmdLines[s] += incCmd;
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
