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
import java.util.ArrayList;
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
//        boolean callPinvar = varOrder.usesPinvars();
        
//        if (callPinvar) {
//            // Add a pinvar invocation step
//            step.addOptionalCmd(useGreatSPN_binary("pinvar") + " " + quotedFn(null) + 
//                        " -detect-exp " + getParamBindingCmd(currBind, true, false));
//            // Add a struct invocation step (write also mpar switches)
//            step.addCmd(useGreatSPN_binary("struct") + " " + quotedFn(null) + 
//                        " -only-bnd " + getParamBindingCmd(currBind, true, false));
//        }

        // Generate the P-basis, the P-semiflows and the bounds
        ArrayList<String> cmd = startOfCommand();
        cmd.add(useGreatSPN_binary("DSPN-Tool"));
        cmd.add("-load");
        cmd.add(makeDefaultModelFilenameForCmd(null));
        cmd.addAll(getParamBindingCmdArgs(currBind, true, true));
        cmd.add("-pbasis");
        cmd.add("-detect-exp");
        cmd.add("-psfl");
        cmd.add("-bnd");
        step.addOptionalCmd(cmd);
//        step.addOptionalCmd(startOfCommand() + useGreatSPN_binary("DSPN-Tool") + 
//                            " -load "+ quotedFn(null) + 
//                            getParamBindingCmd(currBind, true, true) +
//                            " -pbasis -detect-exp -psfl -bnd ");

        
        // Generate the bounds from the ILP with a 5-seconds timeout
        cmd = startOfCommand();
        cmd.add(useGreatSPN_binary("DSPN-Tool"));
        cmd.add("-load");
        cmd.add(makeDefaultModelFilenameForCmd(null));
        cmd.addAll(getParamBindingCmdArgs(currBind, true, true));
        cmd.add("-load-bnd");
        cmd.add("-timeout");
        cmd.add("5");
        cmd.add("-ilp-bnd");
        step.addOptionalCmd(cmd);
//        step.addOptionalCmd(startOfCommand() + useGreatSPN_binary("DSPN-Tool") +
//                            " -load "+ quotedFn(null) + 
//                            getParamBindingCmd(currBind, true, true) +
//                            " -load-bnd -timeout 5 -ilp-bnd\" '");
        
        boolean isV3 = getRGMEDDName().equals("RGMEDD3");
        cmd = startOfCommand();
        cmd.add(useGreatSPN_binary(getRGMEDDName()));
        cmd.add(makeDefaultModelFilenameForCmd(null));
        cmd.add(varOrder.getCmdOption());
        if (params.genCounterExamples)
            cmd.add("-c");
        if (!isV3)
            cmd.add("-satsets");
        cmd.addAll(getParamBindingCmdArgs(currBind, true, true));
//        String rgmeddCmd = (startOfCommand() + useGreatSPN_binary(getRGMEDDName()) + 
//                            " " + quotedFn(null));
//        rgmeddCmd += " " + varOrder.getCmdOption() + " ";
//        if (params.genCounterExamples)
//            rgmeddCmd += " -c";
//        if (!isV3)
//            rgmeddCmd += " -satsets";
        
        // Add the command for parameter bindings
//        rgmeddCmd += getParamBindingCmd(currBind, true, true);
        
        // Format measures
        int measureNum = 0;
        File ctlFilename = new File(getGspnFile().getAbsolutePath()+".ctl");
        BufferedWriter ctlWriter = null;
        boolean hasStat = false;
        ArrayList<String> ddCmd = new ArrayList<>();
        ArrayList<String> incCmd = new ArrayList<>();
        for (AbstractMeasure meas : measures) {
            String measName = "MEASURE"+(measureNum++);
            ResultEntry entry;
            if (meas instanceof FormulaMeasure) {
                FormulaMeasure fm = (FormulaMeasure)meas;
                switch (fm.getLanguage()) {
                    case CTL:
                    case LTL:
                    case CTLSTAR:
                    case FAIRNESS:
                        if (ctlWriter == null) {
                            ctlWriter = new BufferedWriter(new FileWriter(ctlFilename));
                        }
                        entry = new ModelCheckingResultEntry(measName, evalBind);
                        if (isV3) {
                            ctlWriter.append("% ").append(measName).append("\n");
                            ctlWriter.append(fm.getFormula().getExpr()).append("\n");
                        }
                        else {
                            ctlWriter.append("FORMULA: ").append(measName).append("\n");
                            ctlWriter.append("LANGUAGE: ").append(fm.getLanguage().toString()).append("\n");
                            ctlWriter.append(fm.getFormula().getExpr()).append("\n\n");
                        }
//                        String exCTL = fm.getFormula().convertLang(getContext(), EvaluationArguments.NO_ARGS, 
//                                        ExpressionLanguage.GREATSPN);
//                        ctlWriter.append(fm.getFormula().getExpr()).append("\n");
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
                        String f = getGspnFile().getAbsoluteFile().toString()+"-DD-"+step.stepNum;
                        entry = new PdfResultEntry("DD", evalBind, new File(f+".pdf"));
                        ddCmd.add("-dot-F");
                        ddCmd.add( makeDefaultModelFilenameForCmd("-DD-"+step.stepNum));
//                        ddCmd = " -dot-F " + quotedFn("-DD-"+step.stepNum)+" ";
                        step.entries.add(entry);
                        break;
                    }
                    
                    case INC: {
                        String f = getGspnFile().getAbsoluteFile()+"-INC-"+step.stepNum;
                        entry = new PdfResultEntry("INC", evalBind, new File(f+".pdf"));
                        incCmd.add("-inc-F");
                        incCmd.add( makeDefaultModelFilenameForCmd("-INC-"+step.stepNum));
//                        incCmd = " -inc-F " + quotedFn("-INC-"+step.stepNum)+" ";
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
            cmd.add("-gui-stat");
        if (ctlWriter != null) { // there are CTL measures to compute
            ctlWriter.close();
            cmd.add("-C");
        }
        cmd.addAll(ddCmd);
        cmd.addAll(incCmd);
//        rgmeddCmd += ddCmd;
//        rgmeddCmd += incCmd;
        step.addCmd(cmd);
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
