/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import common.UnixPrintWriter;
import common.Util;
import editor.Main;
import editor.domain.elements.GspnPage;
import editor.domain.grammar.EvaluatedBinding;
import editor.domain.grammar.EvaluationArguments;
import editor.domain.grammar.TemplateBinding;
import editor.domain.io.ApnnFormat;
import editor.domain.struct.StructInfo;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Map;
import java.util.Scanner;

/**
 *
 * @author elvio
 */
public class NSolve extends SolverInvokator {
    
    @Override
    void prepareStepForBinding(SolutionStep step, TemplateBinding currBind, 
                               AbstractMeasure[] measures) throws IOException 
    {
        NSolveParams params = (NSolveParams)getPage().solverParams;
        EvaluatedBinding evalBind = currBind.createEvaluated(getContext());
        
        // Save the APNN file for this step
        GspnPage gspn = getPage().targetGspn;
        String baseName = getSolutionDir()+File.separator+"step"+step.stepNum+"net";
        File apnnFile = new File(baseName+".apnn");
        File paramFile = new File(baseName+".param");
        File gtransFile = new File(baseName+".gtrans");
        
        asyncLogStdout("\033[0mPREPARE APNN FOR STEP "+step.stepNum+"\n");
        
        StructInfo struct = StructInfo.computeStructInfo(getDlg(), gspn, currBind, getContext());
        try {
            String log = ApnnFormat.exportGspn(gspn, apnnFile, struct, getContext());
            if (log != null)
                throw new UnsupportedOperationException("Error saving the APNN file for step: "+step.stepNum+
                        "\nReason:\n"+log);
        }
        catch (Exception e) {
            Main.logException(e, true);
            throw new IOException("Cannot save APNN file. "+e.getMessage());
        }
        
        // Initialize parameter expressions
        params.maxLocalStates.checkExprCorrectness(getContext(), gspn, null);
        params.maxTotalStates.checkExprCorrectness(getContext(), gspn, null);
        params.maxIters.checkExprCorrectness(getContext(), gspn, null);
        params.maxCpuTime.checkExprCorrectness(getContext(), gspn, null);
        params.maxMemory.checkExprCorrectness(getContext(), gspn, null);
        params.relaxFactor.checkExprCorrectness(getContext(), gspn, null);
        params.epsilon.checkExprCorrectness(getContext(), gspn, null);
        
        // Save the parameter file
        PrintWriter paramOut = new UnixPrintWriter(paramFile);
        paramOut.println("MODELFILE "+baseName);
        paramOut.println("TRS_EXPLORATION NO");
        paramOut.println("TRS_FORMAT NO");
        paramOut.println("MAT_FORMAT BASIC");
        paramOut.println("MAX_NO_OF_STATES_LOCAL "+params.maxLocalStates.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarInt());
        paramOut.println("MAX_NO_OF_STATES_TOTAL "+params.maxTotalStates.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarInt());
        paramOut.println("MEMORY "+params.maxMemory.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarInt());
        paramOut.println("NUMERICAL_ANALYSIS "+params.numericalMethod);
        paramOut.println("RESULTS ALL");
        paramOut.println("MAX_NO_OF_ITERATIONS "+params.maxIters.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarInt());
        paramOut.println("MAX_CPU_TIME "+params.maxCpuTime.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarInt());
        paramOut.println("RELAXATIONFACTOR "+params.relaxFactor.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarRealOrIntAsReal());
        paramOut.println("CONVERGENCE_EPSILON "+params.epsilon.evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarRealOrIntAsReal());
        paramOut.close();
        
        gtransFile.delete();
            
        // Prepare measures
        boolean hasAll = false, hasStat = false;
        for (AbstractMeasure meas : measures) {
            ResultEntry entry;
            if (meas instanceof FormulaMeasure) {
                FormulaMeasure fm = (FormulaMeasure)meas;
                switch (fm.getLanguage()) {
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
        
        // Setup the NSolve command
        step.addCmd("/bin/sh "+useNSolve_binary("nsolvefornm.sh")+" "+baseName);
    }

    @Override
    void modifyEnvironmentVars(Map<String, String> env) {
        env.put("homedir", getPathToNSolve() + File.separator + "bin");
    }

    @Override
    void readCommand(String command, int stepNum, AbstractMeasure[] measures) {
//        System.out.print(stepNum+ ": Reading command: '"+command+"'");
        if (command.startsWith("END_STEP")) {
            // Read ALL measures from the .pop files
            for (AbstractMeasure meas : measures) {
                if (meas instanceof FormulaMeasure) {
                    FormulaMeasure fm = (FormulaMeasure)meas;
                    if (fm.getLanguage() == FormulaLanguage.ALL) {
                        // Read token distribution of places 
                        String baseName = getSolutionDir()+File.separator+"step"+stepNum+"net";

                        for (int k=1; true; k++) {
                            File popFile = new File(baseName+k+".pop");
//                            System.out.println("Try reading "+popFile.getAbsolutePath());
                            if (!popFile.exists())
                                break;
                            readMeasuresFromPopFile(fm.getResults().table.get(stepNum), popFile);
                        }
       
                    }
                }
            }
        }
        // Default action
        defaultReadCommand(command, stepNum, measures);
    }
        
    @Override
    void startOfStep(SolutionStep step) {
    }

    @Override
    void endOfStep(SolutionStep step, boolean interrupted, boolean allStepsCompleted) {
        step.completed = (!interrupted && allStepsCompleted);
    }
    
    //==========================================================================

    void readMeasuresFromPopFile(ResultEntry all, File popFile) {
        try {
            Scanner pop = new Scanner(popFile);
            String placeName = null;
            
            while (pop.hasNext()) {
                String tag = pop.next();
                if (tag.equals("%") && pop.hasNext()) {
                    tag = pop.next();
                    switch (tag) {
                        case "Place":
                            if (pop.hasNext()) {
                                String[] parts = pop.next().split("\\.");
                                if (parts.length == 3) {
                                    placeName = parts[1];
//                                    System.out.println("PLACE "+placeName);
                                }
                            }
                        break;
                            
                        case "Mean":
                            pop.nextLine(); // Skip rest of the line
                            tag = pop.next();
                            if (tag.equals("%") && pop.hasNext() && placeName!=null) {
                                String mean = pop.next();
                                all.parseResult(new Scanner("MEAN "+placeName+" = "+mean), 
                                                getPage().targetGspn);
                            }
                            break;
                            
                        case "Distribution":
                            pop.nextLine(); // Skip rest of the line
                            tag = pop.next();
                            if (tag.equals("%") && pop.hasNextInt()&& placeName!=null) {
                                int numVals = pop.nextInt();
                                StringBuilder sb = new StringBuilder();
                                sb.append("DISTRIB ").append(placeName).append(" ");
                                sb.append(numVals).append(" =");
                                for (int i=0; i<numVals; i++) {
                                    pop.nextLine(); // Skip rest of the line
                                    pop.next(); // skip '%'
                                    pop.next(); // skip count
                                    if (pop.hasNext()) {
                                        String prob = pop.next();
                                        sb.append(" ").append(prob);
                                    }
                                }
                                all.parseResult(new Scanner(sb.toString()), getPage().targetGspn);
                            }
                            break;
                    }
                }
                // Skip rest of the line
                pop.nextLine();
            }
        } 
        catch (FileNotFoundException ex) {
        }
    }
    
    //==========================================================================
    // NSolve tool installation directory finder
    //==========================================================================

    private static final String NSOLVE_DIR = "nsolve_dir";
    public static void setPathToNSolve(String path) {
        Util.getPreferences().put(NSOLVE_DIR, path);
    }
    public static String getPathToNSolve() {
        return Util.getPreferences().get(NSOLVE_DIR, "/home/user/NMSPN");
    }
    
    public static String useNSolve_binary(String binName) {
        // Append the command to the step.cmd string
        String cmd = getPathToNSolve() + File.separator + "bin" + File.separator + binName;
        if (!new File(cmd).canExecute()) {
            throw new IllegalStateException("The path to the "+binName+" solver of NSolve is not set.\n"
                    + "Check that the NSolve path is set correctly (from the Edit > Options menu) and"
                    + "verify that the NSolve installation is not damaged.");
        }
        return cmd;
    }
}
