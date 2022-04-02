/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import editor.domain.grammar.EvaluatedBinding;
import editor.domain.grammar.EvaluationArguments;
import editor.domain.grammar.ExpressionLanguage;
import editor.domain.grammar.TemplateBinding;
import editor.domain.measures.GreatSPNSolverParams.SolutionTime;
import editor.domain.measures.GreatSPNSolverParams.SolverMode;
import static editor.domain.measures.GreatSPNSolverParams.SolverMode.EGSPN;
import static editor.domain.measures.GreatSPNSolverParams.SolverMode.SWN_ORD;
import static editor.domain.measures.GreatSPNSolverParams.SolverMode.SWN_SYM;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Map;

/** GreatSPN toolchain invokator for:  [gspn|swn]_[ord|sym]_[rg|tr|sim]
 *
 * @author elvio
 */
public class GreatSPNSolver extends SolverInvokator {
        
    @Override
    void prepareStepForBinding(SolutionStep step, TemplateBinding currBind, 
                               AbstractMeasure[] measures) throws IOException 
    {
        GreatSPNSolverParams params = (GreatSPNSolverParams)getPage().solverParams;
        EvaluatedBinding evalBind = currBind.createEvaluated(getContext());
        
        // Check if we need to invoke the v_graph command in the tool chain of this step
        ArrayList<String> vGraphCmd = new ArrayList<>();
       
        int measureNum = 0;
        ArrayList<String> measCmd = new ArrayList<>();
        boolean hasAll = false, hasMeasures = false, hasStat = false;
        for (AbstractMeasure meas : measures) {
            String measName = "MEASURE" + (measureNum++);
            ResultEntry entry;
            if (meas instanceof FormulaMeasure) {
                FormulaMeasure fm = (FormulaMeasure)meas;
                switch (fm.getLanguage()) {
                    case PERFORMANCE_INDEX:
                        hasMeasures = true;
                        entry = new ScalarResultEntry(measName, evalBind);
                        String spnMeas = fm.getFormula().convertLang(getContext(), EvaluationArguments.NO_ARGS, 
                                        ExpressionLanguage.GREATSPN);
                        measCmd.add("-measure");
                        measCmd.add(measName);
                        measCmd.add(spnMeas);
//                        measCmd += " -measure "+measName+" \""+spnMeas+"\"";
                        step.entries.add(entry);
                        break;
                    case RG:
                        entry = new PdfResultEntry("RG", evalBind, 
                                new File(getGspnFile().getAbsoluteFile()+"-RG-"+step.stepNum+".pdf"));
                        params.maxDotMarkings.checkExprCorrectness(getContext(), getPage().targetGspn, null);
                        vGraphCmd.add("-dot-F");
                        vGraphCmd.add(makeDefaultModelFilenameForCmd("-RG-"+step.stepNum));
                        vGraphCmd.add("-max-dot-markings");
                        vGraphCmd.add(""+params.maxDotMarkings.evaluate
                                      (getContext(), EvaluationArguments.NO_ARGS).getScalarInt());
//                        vGraphCmd = " -dot-F " + quotedFn("-RG-"+step.stepNum);
//                        vGraphCmd += " -max-dot-markings " + params.maxDotMarkings.evaluate
//                             (getContext(), EvaluationArguments.NO_ARGS).getScalarInt();
                        step.entries.add(entry);
                        break;
                    case ALL:
                        hasMeasures = true;
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
        
        if (!params.mode.isSimulation()) {
            //------------------------------------------------------------------
            // Build the state space explicitly
            //------------------------------------------------------------------
            String rgBuildCmd = null;
            switch (params.mode) {
                case EGSPN:     rgBuildCmd = "GSPNRG";   break;
                case SWN_ORD:   rgBuildCmd = "WNRG";     break;
                case SWN_SYM:   rgBuildCmd = "WNSRG";    break;
                default:        throw new IllegalStateException();
            }
            ArrayList<String> cmd = startOfCommand();
            cmd.add(useGreatSPN_binary(rgBuildCmd));
            cmd.add(makeDefaultModelFilenameForCmd(null));
            cmd.addAll(getParamBindingCmdArgs(currBind, true, true));
            if (hasMeasures)
                cmd.add("-m");
            if (hasStat)
                cmd.add("-gui-stat");
//            rgBuildCmd += getParamBindingCmd(currBind, true, true);
//            rgBuildCmd += hasMeasures ? " -m" : " ";
//            rgBuildCmd += hasStat ? " -gui-stat" : " ";
            // Unfortunately, only WNRG and WNSRG support the v_graph switch -d
            if (params.mode != SolverMode.EGSPN)
                cmd.addAll(vGraphCmd);

            // Step 0: generate RG
            step.addCmd(cmd);

            if (hasMeasures) { // Solution of the Markov chain and measure computation
                // Step 1: empty .gst file
                cmd = createEmptyFileCmd(makeDefaultModelFilenameForCmd(".gst"));
                step.addCmd(cmd);
//                step.addCmd(startOfCommand() + "/bin/cp /dev/null "+quotedFn(".gst"));

                // Step 2: invoke swn_stndrd
                cmd = startOfCommand();
                cmd.add(useGreatSPN_binary("swn_stndrd"));
                cmd.add(makeDefaultModelFilenameForCmd(null));
                step.addCmd(cmd);
//                step.addCmd(startOfCommand() + useGreatSPN_binary("swn_stndrd") + " " + quotedFn(null));

                // Step 3: numerical solution of the CTMC
                params.epsilon.checkExprCorrectness(getContext(), getPage().targetGspn, null);
                double epsilon = params.epsilon.evaluate(getContext(), EvaluationArguments.NO_ARGS)
                        .getScalarRealOrIntAsReal();
                params.maxIters.checkExprCorrectness(getContext(), getPage().targetGspn, null);
                int maxIters = params.maxIters.evaluate(getContext(), EvaluationArguments.NO_ARGS)
                        .getScalarInt();
                if (params.solTime == SolutionTime.STEADY_STATE) {
                    // Use Gauss-Seidel
                    cmd = startOfCommand();
                    cmd.add(useGreatSPN_binary("swn_ggsc"));
                    cmd.add(makeDefaultModelFilenameForCmd(null));
                    cmd.add("-e"+epsilon);
                    cmd.add("-i"+maxIters);
                    step.addCmd(cmd);
//                    step.addCmd(startOfCommand() + useGreatSPN_binary("swn_ggsc") + " " + quotedFn(null) +
//                                " -e"+epsilon+ " -i"+maxIters);
                }
                else {
                    params.timeT.checkExprCorrectness(getContext(), getPage().targetGspn, null);
                    double timeT = params.timeT.evaluate(getContext(), EvaluationArguments.NO_ARGS)
                            .getScalarRealOrIntAsReal();
                    // Use transient uniformization at time params.timeT
                    cmd = startOfCommand();
                    cmd.add(useGreatSPN_binary("swn_ntrs"));
                    cmd.add(makeDefaultModelFilenameForCmd(null));
                    cmd.add(""+timeT);
                    cmd.add("-e"+epsilon);
                    cmd.add("-i"+maxIters);
                    step.addCmd(cmd);
//                    step.addCmd(startOfCommand() + useGreatSPN_binary("swn_ntrs") + " " + quotedFn(null) + 
//                                " " +timeT+ " -e"+epsilon+ " -i"+maxIters);
                }

                // Step 4: copy .epd into .mpd
                cmd = createCopyFileCmd(makeDefaultModelFilenameForCmd(".epd"),
                                        makeDefaultModelFilenameForCmd(".mpd"));
                step.addCmd(cmd);
//                step.addCmd(startOfCommand() + "/bin/cp "+quotedFn(".epd")+" "+quotedFn(".mpd"));

                // Step 5: prepare measures (use modified mark/rate params and extra defs)
                cmd = startOfCommand();
                cmd.add(useGreatSPN_binary("swn_gst_prep"));
                cmd.add(makeDefaultModelFilenameForCmd(null));
                cmd.addAll(getParamBindingCmdArgs(currBind, true, true));
                cmd.addAll(measCmd);
                step.addCmd(cmd);
//                step.addCmd(startOfCommand() + useGreatSPN_binary("swn_gst_prep") + " " + quotedFn(null) +
//                            getParamBindingCmd(currBind, true, true) + measCmd);

                // Step 6: compute measures
                cmd = startOfCommand();
                cmd.add(useGreatSPN_binary("swn_gst_stndrd"));
                cmd.add(makeDefaultModelFilenameForCmd(null));
                cmd.add("-append");
                cmd.add(makeDefaultModelFilenameForCmd(".sta"));
                step.addCmd(cmd);
//                step.addCmd(startOfCommand() + useGreatSPN_binary("swn_gst_stndrd") + " " + quotedFn(null) +
//                            " -append " + quotedFn(".sta"));
            }
        }
        else {
            //------------------------------------------------------------------
            // Use simulation
            //------------------------------------------------------------------
            String rgBuildCmd;
            switch (params.mode) {
                case EGSPN_SIMUL:       rgBuildCmd = "GSPNSIM";   break;
                case SWN_ORD_SIMUL:     rgBuildCmd = "WNSIM";   break;
                case SWN_SYM_SIMUL:     rgBuildCmd = "WNSYMB";   break;
                default:        throw new IllegalStateException();
            }
            ArrayList<String> cmd = startOfCommand();
            cmd.add(useGreatSPN_binary(rgBuildCmd));
            cmd.add(makeDefaultModelFilenameForCmd(null));
            cmd.addAll(getParamBindingCmdArgs(currBind, true, true));
            if (hasStat)
                cmd.add("-gui-stat");
            cmd.addAll(measCmd);
//            String cmd = startOfCommand() + useGreatSPN_binary(rgBuildCmd) + " " + quotedFn(null);
//            cmd += getParamBindingCmd(currBind, true, true);
//            cmd += hasStat ? " -gui-stat" : "";
//            cmd += measCmd;

            if (params.useFirstTrLength) {
                params.firstTrLength.checkExprCorrectness(getContext(), getPage().targetGspn, null);
                double firstTr = params.firstTrLength.evaluate(getContext(), EvaluationArguments.NO_ARGS)
                        .getScalarRealOrIntAsReal();
                cmd.add("-f");
                cmd.add(""+firstTr);
//                cmd += " -f "+firstTr;
            }
            if (params.solTime == SolutionTime.TRANSIENT) {
                params.timeT.checkExprCorrectness(getContext(), getPage().targetGspn, null);
                double timeT = params.timeT.evaluate(getContext(), EvaluationArguments.NO_ARGS)
                        .getScalarRealOrIntAsReal();
                cmd.add("-t");
                cmd.add(""+timeT);
//                cmd += " -t "+timeT;
            }
            if (params.useBatchConstraints) {
                params.minBatch.checkExprCorrectness(getContext(), getPage().targetGspn, null);
                params.maxBatch.checkExprCorrectness(getContext(), getPage().targetGspn, null);
                double minBatch = params.minBatch.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarRealOrIntAsReal();
                double maxBatch = params.maxBatch.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarRealOrIntAsReal();
                if (params.batchLengthMode == GreatSPNSolverParams.BatchLengthMode.BY_EVENT_COUNT) {
                    cmd.add("-m");
                    cmd.add(""+(int)minBatch);
                    cmd.add("-M");
                    cmd.add(""+(int)maxBatch);
//                    cmd += " -m "+(int)minBatch+" -M "+(int)maxBatch;
                }
                else {
                    cmd.add("-d");
                    cmd.add(""+minBatch);
                    cmd.add("-D");
                    cmd.add(""+maxBatch);
//                    cmd += " -d "+minBatch+" -D "+maxBatch;                    
                }
            }
            params.approxPercentage.checkExprCorrectness(getContext(), getPage().targetGspn, null);
            int approx = params.approxPercentage.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarInt();
            cmd.add("-a");
            cmd.add(""+approx);
//            cmd += " -a "+approx;

            cmd.add("-c");
            cmd.add(""+params.confidence.getConfidencePercentage());
//            cmd += " -c "+params.confidence.getConfidencePercentage();

            if (params.useSeed) {
                params.seed.checkExprCorrectness(getContext(), getPage().targetGspn, null);
                int seed = params.seed.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarInt();
                cmd.add("-s");
                cmd.add(""+seed);
//                cmd += " -s "+seed;
            }
            
            step.addCmd(cmd);
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
        step.completed = (!interrupted && allStepsCompleted);
    }
    
    @Override
    boolean enableSupportForMDepArcsInNetDef() {
        return false;
    }
}
