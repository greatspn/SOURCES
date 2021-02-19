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
        String vGraphCmd = null;
       
        int measureNum = 0;
        String measCmd = "";
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
                        measCmd += " -measure "+measName+" \""+spnMeas+"\"";
                        step.entries.add(entry);
                        break;
                    case RG:
                        entry = new PdfResultEntry("RG", evalBind, 
                                new File(getGspnFile().getAbsoluteFile()+"-RG-"+step.stepNum+".pdf"));
                        params.maxDotMarkings.checkExprCorrectness(getContext(), getPage().targetGspn, null);
                        vGraphCmd = " -dot-F " + quotedFn("-RG-"+step.stepNum);
                        vGraphCmd += " -max-dot-markings " + params.maxDotMarkings.evaluate
                             (getContext(), EvaluationArguments.NO_ARGS).getScalarInt();
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
            String rgBuildCmd;
            switch (params.mode) {
                case EGSPN:     rgBuildCmd = "GSPNRG";   break;
                case SWN_ORD:   rgBuildCmd = "WNRG";     break;
                case SWN_SYM:   rgBuildCmd = "WNSRG";    break;
                default:        throw new IllegalStateException();
            }
            rgBuildCmd = startOfCommand() + useGreatSPN_binary(rgBuildCmd) + " " + quotedFn(null);
            rgBuildCmd += getParamBindingCmd(currBind, true, true);
            rgBuildCmd += hasMeasures ? " -m" : " ";
            rgBuildCmd += hasStat ? " -gui-stat" : " ";
            // Unfortunately, only WNRG and WNSRG support the v_graph switch -d
            if (vGraphCmd != null && params.mode != SolverMode.EGSPN)
                rgBuildCmd += vGraphCmd;

            // Step 0: generate RG
            step.addCmd(rgBuildCmd);

            if (hasMeasures) { // Solution of the Markov chain and measure computation
                // Step 1: empty .gst file
                step.addCmd(startOfCommand() + "/bin/cp /dev/null "+quotedFn(".gst"));

                // Step 2: invoke swn_stndrd
                step.addCmd(startOfCommand() + useGreatSPN_binary("swn_stndrd") + " " + quotedFn(null));

                // Step 3: numerical solution of the CTMC
                params.epsilon.checkExprCorrectness(getContext(), getPage().targetGspn, null);
                double epsilon = params.epsilon.evaluate(getContext(), EvaluationArguments.NO_ARGS)
                        .getScalarRealOrIntAsReal();
                params.maxIters.checkExprCorrectness(getContext(), getPage().targetGspn, null);
                int maxIters = params.maxIters.evaluate(getContext(), EvaluationArguments.NO_ARGS)
                        .getScalarInt();
                if (params.solTime == SolutionTime.STEADY_STATE) {
                    // Use Gauss-Seidel
                    step.addCmd(startOfCommand() + useGreatSPN_binary("swn_ggsc") + " " + quotedFn(null) +
                                " -e"+epsilon+ " -i"+maxIters);
                }
                else {
                    params.timeT.checkExprCorrectness(getContext(), getPage().targetGspn, null);
                    double timeT = params.timeT.evaluate(getContext(), EvaluationArguments.NO_ARGS)
                            .getScalarRealOrIntAsReal();
                    // Use transient uniformization at time params.timeT
                    step.addCmd(startOfCommand() + useGreatSPN_binary("swn_ntrs") + " " + quotedFn(null) + 
                                " " +timeT+ " -e"+epsilon+ " -i"+maxIters);
                }

                // Step 4: copy .epd into .mpd
                step.addCmd(startOfCommand() + "/bin/cp "+quotedFn(".epd")+" "+quotedFn(".mpd"));

                // Step 5: prepare measures (use modified mark/rate params and extra defs)
                step.addCmd(startOfCommand() + useGreatSPN_binary("swn_gst_prep") + " " + quotedFn(null) +
                            getParamBindingCmd(currBind, true, true) + measCmd);

                // Step 6: compute measures
                step.addCmd(startOfCommand() + useGreatSPN_binary("swn_gst_stndrd") + " " + quotedFn(null) +
                            " -append " + quotedFn(".sta"));
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
            String cmd = startOfCommand() + useGreatSPN_binary(rgBuildCmd) + " " + quotedFn(null);
            cmd += getParamBindingCmd(currBind, true, true);
            cmd += hasStat ? " -gui-stat" : "";
            cmd += measCmd;

            if (params.useFirstTrLength) {
                params.firstTrLength.checkExprCorrectness(getContext(), getPage().targetGspn, null);
                double firstTr = params.firstTrLength.evaluate(getContext(), EvaluationArguments.NO_ARGS)
                        .getScalarRealOrIntAsReal();
                cmd += " -f "+firstTr;
            }
            if (params.solTime == SolutionTime.TRANSIENT) {
                params.timeT.checkExprCorrectness(getContext(), getPage().targetGspn, null);
                double timeT = params.timeT.evaluate(getContext(), EvaluationArguments.NO_ARGS)
                        .getScalarRealOrIntAsReal();
                cmd += " -t "+timeT;
            }
            if (params.useBatchConstraints) {
                params.minBatch.checkExprCorrectness(getContext(), getPage().targetGspn, null);
                params.maxBatch.checkExprCorrectness(getContext(), getPage().targetGspn, null);
                double minBatch = params.minBatch.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarRealOrIntAsReal();
                double maxBatch = params.maxBatch.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarRealOrIntAsReal();
                if (params.batchLengthMode == GreatSPNSolverParams.BatchLengthMode.BY_EVENT_COUNT)
                    cmd += " -m "+(int)minBatch+" -M "+(int)maxBatch;
                else
                    cmd += " -d "+minBatch+" -D "+maxBatch;                    
            }
            params.approxPercentage.checkExprCorrectness(getContext(), getPage().targetGspn, null);
            int approx = params.approxPercentage.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarInt();
            cmd += " -a "+approx;

            cmd += " -c "+params.confidence.getConfidencePercentage();

            if (params.useSeed) {
                params.seed.checkExprCorrectness(getContext(), getPage().targetGspn, null);
                int seed = params.seed.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarInt();
                cmd += " -s "+seed;
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
}
