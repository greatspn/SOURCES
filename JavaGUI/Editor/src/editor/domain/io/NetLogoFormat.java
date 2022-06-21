/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.io;

import common.Tuple;
import common.Util;
import editor.domain.Edge;
import editor.domain.Node;
import editor.domain.composition.MultiNetPage;
import editor.domain.elements.ColorClass;
import editor.domain.elements.ConstantID;
import editor.domain.elements.GspnEdge;
import editor.domain.elements.GspnPage;
import editor.domain.elements.ParsedColorSubclass;
import editor.domain.elements.Place;
import editor.domain.elements.Transition;
import editor.domain.grammar.DomainElement;
import editor.domain.grammar.ExpressionLanguage;
import editor.domain.grammar.ParserContext;
import editor.domain.values.EvaluatedFormula;
import editor.domain.values.ValuedMultiSet;
import java.awt.Color;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import javax.swing.filechooser.FileFilter;
import javax.swing.filechooser.FileNameExtensionFilter;

/**
 *
 * @author elvio
 */
public class NetLogoFormat {
    
     // The file filter
    public static final FileFilter fileFilter 
            = new FileNameExtensionFilter("NetLogo File (*.nlogo)", new String[]{"nlogo"});
    
    enum AgentFiringStatus {
        CREATED, MODIFIED, KILLED, SUPPORT;
    }
    
    static class FiringAgent {

        public FiringAgent(Transition trn, GspnEdge inEdge, GspnEdge outEdge, String agentName, ColorClass agentClass, String[] inColorVars, String[] outColorVars, AgentFiringStatus firingStatus) {
            this.trn = trn;
            this.inEdge = inEdge;
            this.outEdge = outEdge;
            this.agentName = agentName;
            this.agentClass = agentClass;
            this.inColorVars = inColorVars;
            this.outColorVars = outColorVars;
            this.firingStatus = firingStatus;
        }

        public Transition trn;
        public GspnEdge inEdge, outEdge;
        public String agentName;
        public ColorClass agentClass;
        public String[] inColorVars, outColorVars;
        public AgentFiringStatus firingStatus;

        @Override
        public String toString() {
            String s = "FiringAgent("+trn.getUniqueName() +
                        " from:" + (inEdge != null ? inEdge.getConnectedPlace().getUniqueName() : "null") +
                        " to:" + (outEdge != null ? outEdge.getConnectedPlace().getUniqueName() : "null") +
                        " agentName:" + agentName + "["+agentClass.getUniqueName()+"]";
            if (inColorVars != null) {
                s += " inColorVar:[ ";
                for (String cv : inColorVars)
                    s += cv + " ";
                s += "]";
            }
            if (outColorVars != null) {
                s += " outColorVars:[ ";
                for (String cv : outColorVars)
                    s += cv + " ";
                s += "]";
            }
            return s + "] status:" + firingStatus + ")";
        }
        
        public boolean isInputAgent() {
            return inColorVars != null;
        }
    }
    
    
    private static final String[] SUPPORT_ATTRIBUTES = { null, "attrValue" };
    
 
    public static String export(GspnPage gspn, File file, 
                                ParserContext context,
                                boolean verbose) 
            throws Exception 
    {
        ArrayList<String> log = new ArrayList<>();
        PrintWriter pw = new PrintWriter(new BufferedOutputStream(new FileOutputStream(file)));
        
        // Determine which color classes are agent classes
        ArrayList<ColorClass> agentsColorClasses = new ArrayList<>();
        Set<ColorClass> attrColorClasses = new HashSet<>();
        for (Node node : gspn.nodes) {
            if (node instanceof ColorClass) {
                ColorClass cc = (ColorClass)node;
                if (cc.isSimpleClass()) {
                    if (cc.isAgent())
                        agentsColorClasses.add(cc);
                    else
                        attrColorClasses.add(cc);
                }
            }
        }
        String[] agentsColorList = new String[agentsColorClasses.size()];
        for (int i=0; i<agentsColorClasses.size(); i++)
            agentsColorList[i] = agentsColorClasses.get(i).getUniqueName();
        
        // Assign to each agent class a unique color
        Map<String, Color> agentClass2color = new HashMap<>();
        for (int i=0; i<agentsColorList.length; i++) {
            agentClass2color.put(agentsColorList[i], MultiNetPage.BLUE_PALETTE[ i % MultiNetPage.BLUE_PALETTE.length ]);
        }
        
//        // Get all simple color classes that are not agent classes
//        Set<ColorClass> attrColorClasses = new HashSet<>();
//        for (Node node : gspn.nodes) {
//            if (node instanceof ColorClass) {
//                ColorClass cc = (ColorClass)node;
//                if (cc.isSimpleClass()) {
//                    boolean isAgent = cc.is;
//                    for (String agentClass : agentsColorList) {
//                        if (agentClass.equals(cc.getUniqueName())) {
//                            isAgent = true;
//                            break;
//                        }
//                    }
//                    if (!isAgent)
//                        attrColorClasses.add(cc);
//                }
//            }
//        }
        
        
        // Get all mark/rate parameters as global variables
        String varNames = "";
        String varSetup = "";
        for (Node node : gspn.nodes) {
            if (node instanceof ConstantID) {
                ConstantID cid = (ConstantID)node;
                varNames += cid.getUniqueName()+" ";
                varSetup += "  set "+cid.getUniqueName()+" "+cid.getConstantExpr().getExpr()+"\n";
            }
        }
        // Declare names of all the static color subclasses
        for (ColorClass clrClass : attrColorClasses) {
            for (int sc=0; sc<clrClass.numSubClasses(); sc++) {
                ParsedColorSubclass pcsc = clrClass.getSubclass(sc);
                if (pcsc.isNamed()) {
                    varNames += pcsc.name+" ";
                }
            }
        }
        pw.println();
//        System.out.println("varNames="+varNames);
//        System.out.println("varSetup="+varSetup);
        
        // Preamble
        pw.println(";; preamble");
        pw.println("extensions [rnd]");
        pw.print("globals ["+varNames+"time gammatot");
        for (Node node : gspn.nodes) {
            if (node instanceof Place) {
                Place plc = (Place)node;
                pw.print(" "+plc.getUniqueName());
            }
        }
        pw.println("]\n");
        
        // Declare all agents
        pw.println(";; declare main agent classes");
        for (String agentClass : agentsColorList) {
            pw.println("breed ["+agentClass+" a_"+agentClass+"]");
        }
        pw.println(";; declare support classes (for attributes)");
        for (ColorClass agentClass : attrColorClasses) {
            pw.println("breed ["+agentClass.getUniqueName()+" a_"+agentClass.getUniqueName()+"_attr]");
        }
        pw.println();
        
        // set of agent classes
        Set<String> agentClasses = new HashSet<>();
        for (String ac : agentsColorList) {
            agentClasses.add(ac);
            Node ncc = gspn.getNodeByUniqueName(ac);
            if (ncc!=null && ncc instanceof ColorClass) {
                ColorClass cc = (ColorClass)ncc;
                if (!cc.isSimpleClass())
                    log.add("Agent class "+ac+" is not a simple class.");
            }
            else
                log.add("Agent class "+ac+" does not exist as a color class.");
        }
        
        // agent class -> attributes
        Map<String, Set<Tuple<String, Integer>>> agentAttrs = new HashMap<>();
        // domain color classes -> attribute names
        Map<ColorClass, String[]> domain2attrs = new HashMap<>();
        
        // Derive agent attributes from the color domains
        for (Node node : gspn.nodes) {
            if (node instanceof ColorClass) {
                ColorClass cc = (ColorClass)node;
                if (cc.isCrossDomain()) {
                    String agentClass = cc.getColorClassName(0);
                    String[] attrNames = new String[cc.getNumClassesInDomain()];
                    boolean haveAttr = agentAttrs.containsKey(agentClass);
                    Set<Tuple<String, Integer>> attributes;
                    if (haveAttr)
                        attributes = agentAttrs.get(agentClass);
                    else
                        attributes = new HashSet<>();
                    if (!agentClasses.contains(agentClass)) {
                        log.add("Color "+agentClass+
                                " that appears as first color in domain "+cc.getUniqueName()+
                                " is not an agent class.");
                    }
                    Map<String, Integer> colorOccurrences = new HashMap<>();
                    for (int i=1; i<cc.getNumClassesInDomain(); i++) {
                        String colorAttr = cc.getColorClassName(i);
                        int rep = 0;                        
                        if (colorOccurrences.containsKey(colorAttr)) {
                            rep = colorOccurrences.get(colorAttr) + 1;
                            colorOccurrences.replace(colorAttr, rep);
                        }
                        else {
                            colorOccurrences.put(colorAttr, rep);
                        }
                        attributes.add(new Tuple<>(colorAttr, rep));
                        attrNames[i] = colorAttr+"_"+rep;
                    }
                    
                    if (!haveAttr)
                        agentAttrs.put(agentClass, attributes);
                    domain2attrs.put(cc, attrNames);
                }
                else if (cc.isSimpleClass() && agentClasses.contains(cc.getColorClassName(0))) {
                    String[] attrNames = new String[cc.getNumClassesInDomain()];
                    domain2attrs.put(cc, attrNames);
                }
            }
        }
        
        // Sanity check for places
        for (Node node : gspn.nodes) {
            if (node instanceof Place) {
                Place pl = (Place)node;
                ColorClass plDom = pl.getColorDomain();
                
                if (plDom==null || plDom.isNeutralDomain()) {
                    log.add("Place "+pl.getUniqueName()+
                            " does not have a color domain.");
                }
                String agentClass = plDom.getColorClassName(0);
                if (!agentClasses.contains(agentClass)) {
                    log.add("Place "+pl.getUniqueName()+
                            " does not belong to an agent class.");
                }
            }
        }
        // Do not continue if problems were detected so far.
        if (!log.isEmpty())
            return reportLog(log, pw);

        // Pre-process all input arcs. 
        // transition -> input/output/inhibitor edges + color vars[]
        Map<Transition, ArrayList<Tuple<GspnEdge, String[]>>> trn2inAgents = new HashMap<>();
        Map<Transition, ArrayList<Tuple<GspnEdge, String[]>>> trn2outAgents = new HashMap<>();
        Map<Transition, ArrayList<Tuple<GspnEdge, String[]>>> trn2inhibAgents = new HashMap<>();
        // Each arc expresses a sum of agents involved in the transition
        for (Edge edge : gspn.edges) {
            if (edge instanceof GspnEdge) {
                GspnEdge e = (GspnEdge)edge;
                if (e.getConnectedPlace().isInNeutralDomain())
                    continue;
                
                Map<Transition, ArrayList<Tuple<GspnEdge, String[]>>> trnAgentMap;
                switch (e.getEdgeKind()) {
                    case INPUT:     trnAgentMap = trn2inAgents;    break;
                    case OUTPUT:    trnAgentMap = trn2outAgents;   break;
                    case INHIBITOR: trnAgentMap = trn2inhibAgents; break;
                    default: throw new IllegalStateException();
                }
                
                ColorClass dom = e.getConnectedPlace().getColorDomain();
                assert domain2attrs.containsKey(dom);
                
//                System.out.println("convert: " + e.getMultiplicity());
                String nlMult = e.convertMultiplicityLang(context, ExpressionLanguage.NETLOGO);
//                System.out.println(e.getMultiplicity()+" ==> "+nlMult);
                String[] allTupleTerms = splitTupleSum(nlMult, log);
                // Separate sum terms
//                ArrayList<String[]> tuples = new ArrayList<>();
                for (String tupleTerm : allTupleTerms) {
                    String[] colorVars = tupleTerm.split(",");
                    for (int i=0; i<colorVars.length; i++)
                        colorVars[i] = stripVarTerm(colorVars[i], log);
//                    tuples.add(colorVars);
                    
                    Transition trn = e.getConnectedTransition();
                    if (!trnAgentMap.containsKey(trn)) {
                        trnAgentMap.put(trn, new ArrayList<>());
                    }
                    trnAgentMap.get(trn).add(new Tuple<>(e, colorVars));
                }
            }
        }

        
        
        // transition firing transformation
        Map<Transition, ArrayList<FiringAgent>> allFiringAgents = new HashMap<>();
        for (Node node : gspn.nodes) {
            if (node instanceof Transition) {
                Transition trn = (Transition)node;
                ArrayList<Tuple<GspnEdge, String[]>> allInAgents = trn2inAgents.get(trn);
                ArrayList<Tuple<GspnEdge, String[]>> allOutAgents = trn2outAgents.get(trn);
                
                ArrayList<FiringAgent> firingAgents = new ArrayList<>();
                
                if (allOutAgents != null) {
                    for (Tuple<GspnEdge, String[]> outAgent : allOutAgents) {
                        boolean isNew = true;
                        if (allInAgents != null) {
                            for (Tuple<GspnEdge, String[]> inAgent : allInAgents) {
                                if (outAgent.y[0].equals(inAgent.y[0])) {
//                                  // modified
                                    firingAgents.add(new FiringAgent(trn, inAgent.x, outAgent.x, 
                                            inAgent.y[0], inAgent.x.getConnectedPlace().getColorDomain().getColorClass(0),
                                            inAgent.y, outAgent.y,
                                            AgentFiringStatus.MODIFIED));
                                    isNew = false;
                                    break;
                                }
                            }
                        }
                        if (isNew) {
//                          // newly created
                            firingAgents.add(new FiringAgent(trn, null, outAgent.x, 
                                            outAgent.y[0], outAgent.x.getConnectedPlace().getColorDomain().getColorClass(0),
                                            null, outAgent.y,
                                            AgentFiringStatus.CREATED));
                        }
                    }
                }
                if (allInAgents != null) {
                    for (Tuple<GspnEdge, String[]> inAgent : allInAgents) {
                        // check if inAgent was transformed
                        boolean isFound = false;
                        for (FiringAgent fa : firingAgents) {
                            if (fa.agentName.equals(inAgent.y[0])) {
                                isFound = true;
                                break;
                            }
                        }
                        if (!isFound) {
//                              // killed
                            firingAgents.add(new FiringAgent(trn, inAgent.x, null, 
                                        inAgent.y[0], inAgent.x.getConnectedPlace().getColorDomain().getColorClass(0),
                                        inAgent.y, null,
                                        AgentFiringStatus.KILLED));
                        }
                    }
                }
                
                // Enumerate all bounded variables
                Set<String> boundedVars = new HashSet<>();
                for (FiringAgent fa : firingAgents)
                    if (fa.inColorVars != null)
                        for (int i=1; i<fa.inColorVars.length; i++)
                            boundedVars.add(fa.inColorVars[i]);
                // Now add all support agents to deal with the free variables
                ArrayList<FiringAgent> supportAgents = new ArrayList<>();
                for (FiringAgent fa : firingAgents)
                    if (fa.outColorVars != null) {
                        for (int i=1; i<fa.outColorVars.length; i++) {
                            if (!boundedVars.contains(fa.outColorVars[i])) {
                                // Add new agent to the transition
                                String[] attrs = new String[]{ fa.outColorVars[i] };
                                supportAgents.add(new FiringAgent(trn, null, null, 
                                            fa.outColorVars[i], 
                                            fa.outEdge.getConnectedPlace().getColorDomain().getColorClass(i),
                                            attrs, attrs,
                                            AgentFiringStatus.SUPPORT));
                                
                                boundedVars.add(fa.outColorVars[i]);
                            }
                        }
                    }
                firingAgents.addAll(supportAgents);

                allFiringAgents.put(trn, firingAgents);
            }
        }
        
        
        for (Map.Entry<Transition, ArrayList<FiringAgent>> e1 : allFiringAgents.entrySet()) {
            System.out.println("Transition: "+e1.getKey().getUniqueName());
            for (FiringAgent fa : e1.getValue()) {
                System.out.println("  "+fa);
            }
        }
        
        
        // Determine transition leaders
        Map<Transition, String> leaderOfTrn = new HashMap<>();
        // agent class -> transitions for which it is the leader
        Map<String, ArrayList<Transition>> transitionsOfLeaderAgentClass = new HashMap<>();
        for (String agentClass : agentClasses) 
            transitionsOfLeaderAgentClass.put(agentClass, new ArrayList<>());
        for (Map.Entry<Transition, ArrayList<FiringAgent>> e1 : allFiringAgents.entrySet()) {
            for (FiringAgent agent : e1.getValue()) {
                if (agent.isInputAgent()) {
                    String agentClassName = agent.agentClass.getUniqueName();
                    leaderOfTrn.put(e1.getKey(), agentClassName);
                    transitionsOfLeaderAgentClass.get(agentClassName).add(e1.getKey());
                    System.out.println(agentClassName+" is leader of transition "+e1.getKey().getUniqueName());
                    break;
                }
            }
        }
        

        ///////////////////////////////////////////
        // Write agent attributes
        Map<String, ArrayList<String>> agentAttrSeq = new HashMap<>();
        pw.println(";; Agent attributes");
        for (String agentClass : agentClasses) {
            pw.print(agentClass+"-own [");
            Set<Tuple<String, Integer>> attributes = agentAttrs.get(agentClass);
            ArrayList<String> attrPos = new ArrayList<>();
            if (attributes != null) {
                for (Tuple<String, Integer> attr : attributes) {
                    pw.print(attr.x+"_"+attr.y+" ");
                    attrPos.add(attr.x+"_"+attr.y);
                }
            }
            pw.println("place myrate totrate]");
            agentAttrSeq.put(agentClass, attrPos);
        }
        pw.println(";; Support agent attributes");
        for (ColorClass agentClass : attrColorClasses) {
            pw.println(agentClass.getUniqueName()+"-own [attrValue]");
        }
        pw.println();
        
        
        ///////////////////////////////////////////
        // Model Setup procedure
        pw.println(";; model setup procedure");
        pw.println("to setup");
        pw.println("  ca ;; clear all");
        pw.println("  if seed != -1 [ random-seed SEED ]");
        pw.println("  reset-ticks");
        pw.print(varSetup);
        
        pw.println("  ;; place identifiers");
        int plcId = 1000;
        for (Node node : gspn.nodes) {
            if (node instanceof Place) {
                Place plc = (Place)node;
                pw.println("  set "+plc.getUniqueName()+" "+(plcId++));
            }
        }
        pw.println();
        
        pw.println("  ;; setup initial marking");
        // write initial markings as sprout directives
        for (Node node : gspn.nodes) {
            if (node instanceof Place) {
                Place pl = (Place)node;
                ColorClass plDom = pl.getColorDomain();
                if (plDom==null || plDom.isNeutralDomain()) 
                    continue;
                String agentClassName = plDom.getColorClassName(0);
                String initMark = pl.getInitMarkingExpr();
                if (initMark.isEmpty())
                    continue;
                String[] attrs = domain2attrs.get(plDom);
                
                EvaluatedFormula em0 = pl.evaluateInitMarking(context);
                assert em0.isMultiSetInt();
                ValuedMultiSet msm0 = (ValuedMultiSet)em0;
                
                Map<ArrayList<Integer>, Integer> m0AttrReplicas = new HashMap<>();
                for (int ii=0; ii<msm0.numElements(); ii++) {
                    DomainElement domEl = msm0.getElement(ii);
                    assert plDom == domEl.getDomain();
                    int elMult = msm0.getValue(ii).getScalarInt();
                    ArrayList<Integer> initAttrs = new ArrayList<>(plDom.getNumClassesInDomain());
                    initAttrs.add(-1);
                    for (int colNum=1; colNum<plDom.getNumClassesInDomain(); colNum++) {
                        ColorClass simpleColor = plDom.getColorClass(colNum);
                        int colIndex = domEl.getColor(colNum);
                        initAttrs.add(colIndex);
                    }
                    // Merge together with all the other agents that have the same initial attributes
                    elMult += m0AttrReplicas.getOrDefault(initAttrs, 0);
                    m0AttrReplicas.put(initAttrs, elMult);
                }
                // Write initial marking code
                for (Map.Entry<ArrayList<Integer>, Integer> ee : m0AttrReplicas.entrySet()) {
                    pw.println("  create-"+agentClassName+" "+ee.getValue()+" [");
                    pw.println("    set place "+pl.getUniqueName());
                    for (int colNum=1; colNum<plDom.getNumClassesInDomain(); colNum++) {
                        ColorClass simpleColor = plDom.getColorClass(colNum);
                        pw.println("    set "+attrs[colNum]+" "+ee.getKey().get(colNum));
                    }
                    Color agentClr = agentClass2color.get(agentClassName);
                    pw.println("    set color ["+agentClr.getRed()+" "+agentClr.getGreen()+" "+agentClr.getBlue()+"]");
                    pw.println("  ]");
                }
                /*for (int ii=0; ii<msm0.numElements(); ii++) {
                    DomainElement domEl = msm0.getElement(ii);
                    assert plDom == domEl.getDomain();
                    int elMult = msm0.getValue(ii).getScalarInt();
                    
                    pw.println("  create-"+agentClassName+" "+elMult+" [");
                    pw.println("    set place "+pl.getUniqueName());
                    for (int colNum=1; colNum<plDom.getNumClassesInDomain(); colNum++) {
                        ColorClass simpleColor = plDom.getColorClass(colNum);
                        int colIndex = domEl.getColor(colNum);
                        pw.println("    set "+attrs[colNum]+" "+colIndex);
                    }
                    pw.println("  ]");
                }*/
            }
        }
        pw.println();        
        pw.println("  ;; setup static color subclasses of attribute color classes");
        for (ColorClass clrClass : attrColorClasses) {
            for (int sc=0; sc<clrClass.numSubClasses(); sc++) {
                ParsedColorSubclass pcsc = clrClass.getSubclass(sc);
                if (pcsc.isNamed()) {
                    pw.print("  set "+pcsc.name+" (list ");
                    for (int cc=0; cc<pcsc.getNumColors(); cc++) {
                        int clrIndex = clrClass.getColorIndex(pcsc.getColorName(cc));
                        pw.print(clrIndex+" ");
                    }
                    pw.println(")");
                }
            }
        }
        pw.println();

        pw.println("  ;; setup support agents");
        pw.println("  let attrCounter 0");
        for (ColorClass agentClass : attrColorClasses) {
            pw.println("  set attrCounter 0");
            pw.println("  create-"+agentClass.getUniqueName()+" "+agentClass.numColors()+" [");
            pw.println("    set attrValue attrCounter");
            pw.println("    set attrCounter attrCounter + 1");
            pw.println("    hide-turtle");
            pw.println("  ]");
        }
        pw.println();
        
        pw.println("  set time 0.0");
        pw.println("end\n");
        
        
        ///////////////////////////////////////////
        // Ask all agents to initialize their myrate list
        pw.println("to go");
        pw.println("if STOPTIME > 0 AND time > STOPTIME [ STOP ]");
        pw.println("let _A1 0");
        pw.println(";; ask agents to initialize myrate");
        for (String agentClass : agentClasses) {
            ArrayList<Transition> leadersOf = transitionsOfLeaderAgentClass.get(agentClass);
            if (leadersOf.size() > 0) {
                pw.print("ask "+agentClass+" [set myrate (list ");
                for (int i=0; i<leadersOf.size(); i++)
                    pw.print(" 0");
                pw.print(" ) set totrate 0.0 ");
                pw.println("]");
            }
        }
//        pw.println("ask turtles [set myrate [] ]");
        pw.println();
        
        ///////////////////////////////////////////
        // Compute transition enablings
        for (Node node : gspn.nodes) {
            if (node instanceof Transition) {
                Transition trn = (Transition)node;
                Map<String, String> varConv = new HashMap<>();
                Set<String> knownVars = new HashSet<>();
                int ind=0;
                int agentNum = 1;
                
//                System.out.println("transition: "+trn.getUniqueName());
                ArrayList<FiringAgent> firingAgents = allFiringAgents.get(trn);
                ArrayList<FiringAgent> inputFiringAgents = new ArrayList<>();
                for (FiringAgent agent : firingAgents) {
                    if (agent.isInputAgent())
                        inputFiringAgents.add(agent);
                }
                
                String leaderAgentClass = leaderOfTrn.get(trn);
                assert inputFiringAgents.isEmpty() || leaderAgentClass.equals(inputFiringAgents.get(0).agentClass.getUniqueName());
                pw.println(";; transition "+trn.getUniqueName());

                // cycle through all input agents
                for (FiringAgent agent : inputFiringAgents) {
                    String[] attrs = (agent.firingStatus==AgentFiringStatus.SUPPORT ? SUPPORT_ATTRIBUTES : 
                                        domain2attrs.get(agent.inEdge.getConnectedPlace().getColorDomain()));
                    String whoOfSelf = (agent.firingStatus==AgentFiringStatus.SUPPORT ? "attrValue" : "who");
                    String guard = "";
                    varConv.put("($"+agent.agentName+"$)", "["+whoOfSelf+"] of self");
                    for (int i=1; i<agent.inColorVars.length; i++)
                        varConv.put("($"+agent.inColorVars[i]+"$)", attrs[i]);
//                        varConv.put("($"+agent.inColorVars[i]+"$)", "["+attrs[i]+"] of self");

                    for (int i=0; i<agent.inColorVars.length; i++)
                        knownVars.add(agent.inColorVars[i]);
                    String nlGuard = trn.dropGuardSubTerms(context, knownVars, ExpressionLanguage.NETLOGO);
                    if (!nlGuard.isEmpty()) {
                        for (Map.Entry<String, String> ee : varConv.entrySet()) {
                            nlGuard = nlGuard.replace(ee.getKey(), ee.getValue());
                        }
                        guard = nlGuard;
                    }
                    if (agent.inEdge != null)
                        guard = "place = "+agent.inEdge.getConnectedPlace().getUniqueName() 
                                + (guard.isEmpty() ? "" : " AND (" + guard + ")");


                    indent(pw, ind); 
                    pw.println((agentNum==1 ? "set" : "let")+" _A"+agentNum+" "+agent.agentClass.getUniqueName()+
                             " with ["+guard+"]"); 
                    indent(pw, ind); pw.println("if any? _A"+agentNum+" ["); ind++;

                    if (inputFiringAgents.size() == 1) { // single agent transition
                        indent(pw, ind);  pw.println("ask _A"+agentNum+" ["); ind++;
                        if (agentNum == 1) {
                             indent(pw, ind); pw.println("let countInstances 1");
                        }
                    }
//                    else if (agentNum == inputFiringAgents.size()) { // last agent set
//                        indent(pw, ind); pw.println("set countInstances  countInstances + (count A"+agentNum+")");
//                    }
                    else {
                        // ask the selected agent
                        indent(pw, ind);  pw.println("ask _A"+agentNum+" ["); ind++;
                        if (agentNum == 1) {
                             indent(pw, ind); pw.println("let countInstances 0");
                        }
                        // name self
                        indent(pw, ind); 
                        pw.println("let "+agent.agentName+" ["+whoOfSelf+"] of self");
                        varConv.put("($"+agent.agentName+"$)", agent.agentName);
                        // name all attributes
                        for (int i=1; i<agent.inColorVars.length; i++) {
                            indent(pw, ind); 
                            pw.println("let "+agent.inColorVars[i]+" ["+attrs[i]+"] of self");
                            varConv.put("($"+agent.inColorVars[i]+"$)", agent.inColorVars[i]);
                        }
                        
                        if (agentNum == inputFiringAgents.size()) { // last agent sets the counter
                            indent(pw, ind); pw.println("set countInstances  countInstances + 1");
                        }
                    }
                    agentNum++;
                }
                
                while (ind > 0) {
                    if (ind == 2) {
                        // determine the transition index inside leadersOfTrn
                        int myratePos = transitionsOfLeaderAgentClass.get(leaderAgentClass).indexOf(trn);                        
                        indent(pw, ind); pw.println(";; summing up all rates");
                        String delayExpr = trn.convertDelayLang(context, null, ExpressionLanguage.NETLOGO);
                        indent(pw, ind); pw.println("set myrate replace-item "+myratePos+" myrate (countInstances * ("+delayExpr+"))");
                    }
                    ind--;
                    indent(pw, ind); pw.println("]");
                }                
            }
        }
        pw.println();
        
        ///////////////////////////////////////////
        // Now ask all agents to sum the rates into a single totrate variable
        pw.println(";; ask agents to update their myrate values, and then update gammatot");
        pw.print("let allAgents (turtle-set ");
        for (String agentClass : agentClasses)
            pw.print(agentClass+" ");
        pw.println(")");
        pw.println("ask allAgents [set totrate sum myrate]");
        pw.println("set gammatot sum [totrate] of allAgents");
        pw.println("if gammatot = 0 [stop]");
        pw.println("let increment ((-1 / gammatot) * ln(random-float 1))");
        pw.println("set time  time + increment");
        pw.println();

        // Select the next agent that will perform an action
        pw.println(";; select the next agent that will perform an action");
        pw.println("let chosenAgent rnd:weighted-one-of allAgents [totrate]");
        pw.println();
        
        
        ///////////////////////////////////////////
        // Switch over the chosen agent class to activate a transition firing
        pw.println(";; switch over the class of the chosen agent");
        pw.println("ask chosenAgent [");
        pw.println("  let bindingSelected false");
        pw.println("  (if-else ");
        for (String agentClass : agentClasses) {
            ArrayList<Transition> leadersOf = transitionsOfLeaderAgentClass.get(agentClass);
            if (leadersOf.isEmpty())
                continue; // this agent is never a leader of any transition
            pw.println("  is-a_"+agentClass+"? self [");
            // select the enabling transition
            pw.println("    ;; select the next action performed by the chosen "+agentClass);
            pw.println("    let indices  n-values (length myrate) [ i -> i ]");
            pw.println("    let pairs (map list indices myrate)");
            pw.println("    let nextaction  first rnd:weighted-one-of-list pairs [ [var_P] -> last var_P ]");
            pw.println("    (");
            pw.println("      if-else");
            // evaluate the transitions where the agent is a leader
            for (int iit=0; iit<leadersOf.size(); iit++) {
                Transition trn = leadersOf.get(iit);
                pw.println("      nextaction = "+iit+" [");
                pw.println("        ;; chosenAgent is leader of "+trn.getUniqueName());
                int ind = 4;
                pw.println("        let targetRate random-float (item "+iit+" myrate)");
                
                
                
                // cycle through all input agents
                ArrayList<FiringAgent> firingAgents = allFiringAgents.get(trn);
                ArrayList<FiringAgent> inputFiringAgents = new ArrayList<>();
                for (FiringAgent agent : firingAgents) {
                    if (agent.isInputAgent())
                        inputFiringAgents.add(agent);
                }
                
                Map<String, String> varConv = new HashMap<>();
                Set<String> knownVars = new HashSet<>();
                int agentNum = 1;
                for (FiringAgent agent : inputFiringAgents) {
                    String[] attrs = (agent.firingStatus==AgentFiringStatus.SUPPORT ? SUPPORT_ATTRIBUTES : 
                                        domain2attrs.get(agent.inEdge.getConnectedPlace().getColorDomain()));
                    String whoOfSelf = (agent.firingStatus==AgentFiringStatus.SUPPORT ? "attrValue" : "who");
                    String guard = "";
                    varConv.put("($"+agent.agentName+"$)", "["+whoOfSelf+"] of self");
                    for (int i=1; i<agent.inColorVars.length; i++)
                        varConv.put("($"+agent.inColorVars[i]+"$)", attrs[i]);
//                        varConv.put("($"+agent.inColorVars[i]+"$)", "["+attrs[i]+"] of self");
                    
                    for (int i=0; i<agent.inColorVars.length; i++)
                        knownVars.add(agent.inColorVars[i]);
                    String nlGuard = trn.dropGuardSubTerms(context, knownVars, ExpressionLanguage.NETLOGO);
                    if (!nlGuard.isEmpty()) {
                        for (Map.Entry<String, String> ee : varConv.entrySet()) {
                            nlGuard = nlGuard.replace(ee.getKey(), ee.getValue());
                        }
                        guard = nlGuard; 
                    }
                    if (agent.inEdge != null)
                        guard = "place = "+agent.inEdge.getConnectedPlace().getUniqueName() 
                                + (guard.isEmpty() ? "" : " AND (" + guard + ")");

                    indent(pw, ind); 
                    pw.println("let _AA"+agentNum+" "+agent.agentClass.getUniqueName()+" with ["+guard+"]"); 
                    indent(pw, ind); pw.println("if any? _AA"+agentNum+" ["); ind++;

                    // ask the selected agent
                    indent(pw, ind);  pw.println("ask _AA"+agentNum+" ["); ind++;
                    indent(pw, ind);  pw.println("if NOT bindingSelected ["); ind++;
                    // name self
                    indent(pw, ind); 
                    pw.println("let "+agent.agentName+" ["+whoOfSelf+"] of self");
                    varConv.put("($"+agent.agentName+"$)", agent.agentName);
                    // name all attributes
                    for (int i=1; i<agent.inColorVars.length; i++) {
                        indent(pw, ind); 
                        pw.println("let "+agent.inColorVars[i]+" ["+attrs[i]+"] of self");
                        varConv.put("($"+agent.inColorVars[i]+"$)", agent.inColorVars[i]);
                    }
                    
                    if (agentNum == inputFiringAgents.size()) { // last agent may perform the firing
                        String delayExpr = trn.convertDelayLang(context, null, ExpressionLanguage.NETLOGO);
                        indent(pw, ind); pw.println("set targetRate targetRate - ("+delayExpr+")");
                        indent(pw, ind); pw.println("if-else targetRate > 0 [ ]");
                        indent(pw, ind); pw.println("[");
                        ind++;
                        
                        ///////////////////////////////////////////
                        // Transition firing
                        indent(pw, ind); pw.println(";; fire this binding");
                        indent(pw, ind); pw.println("set bindingSelected true\n");
                        
                        for (FiringAgent firing : firingAgents) {
                            String[] outAttrs = null;
                            if (firing.outEdge != null)
                                outAttrs = (firing.firingStatus==AgentFiringStatus.SUPPORT ? SUPPORT_ATTRIBUTES : 
                                            domain2attrs.get(firing.outEdge.getConnectedPlace().getColorDomain()));
                            
                            switch (firing.firingStatus) {
                                case CREATED:
                                    indent(pw, ind); pw.println(";; agent "+firing.agentName+" is new");
                                    indent(pw, ind); pw.println("hatch-"+firing.agentClass.getUniqueName()+" 1 [ "); ind++;
                                    indent(pw, ind); pw.println("set place "+firing.outEdge.getConnectedPlace().getUniqueName());
                                    for (int i=1; i<firing.outColorVars.length; i++) {
                                        indent(pw, ind); pw.println("set "+outAttrs[i]+" "+firing.outColorVars[i]);
                                    }
                                    Color agentClr = agentClass2color.get(firing.agentClass.getUniqueName());
                                    indent(pw, ind); pw.println("set color ["+agentClr.getRed()+" "+agentClr.getGreen()+" "+agentClr.getBlue()+"]");
                                    indent(pw, ind); pw.println("set myrate 0");
                                    indent(pw, ind); pw.println("set totrate 0");
                                    ind --;
                                    indent(pw, ind); pw.println("]\n");
                                    break;
                                    
                                case MODIFIED:
                                    indent(pw, ind); pw.println(";; agent "+firing.agentName+" is modified");
                                    indent(pw, ind); pw.println("ask turtle "+firing.agentName+" ["); ind++;
                                    indent(pw, ind); pw.println("set place "+firing.outEdge.getConnectedPlace().getUniqueName());
                                    for (int i=1; i<outAttrs.length; i++) {
                                        indent(pw, ind); pw.println("set "+outAttrs[i]+" "+firing.outColorVars[i]);
                                    }
                                    ind --;
                                    indent(pw, ind); pw.println("]\n");
                                    break;
                                    
                                case KILLED:
                                    indent(pw, ind); pw.println(";; agent "+firing.agentName+" is killed");
                                    indent(pw, ind); pw.println("ask turtle "+firing.agentName+" [die]\n");
                                    break;
                                    
                                case SUPPORT:
                                    break;
                            }
                        }
                    }
                    
                    agentNum++;
                }
                
                while (ind > 4) {
                    ind--;
                    indent(pw, ind); pw.println("]");
                }
                
                pw.println("      ]");
            }
            pw.println("    )");
            
            pw.println("  ]"); // end of if-agent? case
        }
        pw.println(")]");
        pw.println("tick");
        pw.println();
        pw.println("end\n");
        
        // Write the fixed footer with the GUI commands
        String footer = Util.loadTextDoc("domain/io/NetLogoFooter.txt", "");
        // Modify the footer adding one PEN object for every agent
        StringBuilder pens = new StringBuilder();
        for (int i=0; i<agentsColorList.length; i++) {
            // Global count of all agents of a certain type
            Color color = MultiNetPage.BLUE_PALETTE[i % MultiNetPage.BLUE_PALETTE.length];
            pens.append("\"").append(agentsColorList[i])
                .append("\" 1.0 0 ").append(color.getRGB())
                .append(" true \"\" \"plotxy time (count ")
                .append(agentsColorList[i]).append(")\"\n");
            // Detailed count of all agents in a place
            StringBuilder detailedPens = new StringBuilder();
            int plCount = 0;
            for (Node node : gspn.nodes) {
                if (node instanceof Place) {
                    Place pl = (Place)node;
                    ColorClass plDom = pl.getColorDomain();
                    if (!plDom.isNeutralDomain() && 
                            plDom.getColorClass(0).getUniqueName().equals(agentsColorList[i])) 
                    {
                        color = MultiNetPage.RED_PALETTE[plCount % MultiNetPage.RED_PALETTE.length];
                        detailedPens.append("\"").append(agentsColorList[i]).append("_").append(pl.getUniqueName())
                            .append("\" 1.0 0 ").append(color.getRGB())
                            .append(" true \"\" \"plotxy time (count ")
                            .append(agentsColorList[i]).append(" with [Place = ")
                            .append(pl.getUniqueName()).append("])\"\n");
                        plCount++;
                    }
                }
            }
            if (plCount >= 2)
                pens.append(detailedPens.toString());
        }
        footer = footer.replace("#PENS\n", "PENS\n"+pens.toString());
        pw.println(footer);
        
        
        return reportLog(log, pw);
    }
    
    
    private static void indent(PrintWriter pw, int indentLevel) {
        for (int i=0; i<indentLevel; i++)
            pw.print("  ");
    }
    
    
    private static String[] splitTupleSum(String tupleSum, ArrayList<String> log) {
        ArrayList<String> tuples = new ArrayList<>();
        
        int start=-1;
        for (int i=0; i<tupleSum.length(); i++) {
            if (tupleSum.charAt(i) == '<') {
                if (start != -1) {
                    log.add("Cannot nest tuples: "+tupleSum);
                }
                start = i;
            }
            else if (tupleSum.charAt(i) == '>') {
                if (start == -1) {
                    log.add("Closed tuple: "+tupleSum);
                }
                else {
                    String term = tupleSum.substring(start+1, i).trim();
//                    System.out.println("term = "+term);
                    tuples.add(term);
                    start = -1;
                }
            }
        }
        return tuples.toArray(new String[tuples.size()]);
    }
    
    // Given a term in the form " ($var$)  ", returns "var"
    private static String stripVarTerm(String term, ArrayList<String> log) {
        term = term.strip();
        if (term.startsWith("($") && term.endsWith("$)")) {
            term = term.substring(2, term.length()-2);
            if (term.contains("$")) {
                log.add("The term "+term+" was not parsed correctly.");
            }
        }
        else {
            log.add("Could not identify the variable in "+term);
        }
        return term;
    }
    
    
    private static int countColorNum(ColorClass cc, String clrExpr, ArrayList<String> log) {
        int num = 0;
        String[] clrTerms = clrExpr.split("\\+");
        for (String clrTerm : clrTerms) {
            boolean found = false;
            int numSubClasses = cc.numSubClasses();
//            System.out.println("numSubClasses="+numSubClasses);
            // Try if clrTerm is a single color name
            for (int sc=0; sc<numSubClasses && !found; sc++) {
                ParsedColorSubclass pcsc = cc.getSubclass(sc);
//                System.out.println("confronto 1: "+pcsc.name);
                if (clrTerm.equals(pcsc.name)) {
                    // clrTerm is a static subclass
//                    System.out.println("color term "+clrTerm+" is a static subclass");
                    num += pcsc.getNumColors();
                    found = true;
                }
//                System.out.println("Parsed static subclass "+pcsc.name+" has "+pcsc.getNumColors()+" colors.");
                for (int c=0; c<pcsc.getNumColors() && !found; c++) {
//                    System.out.println("confronto 2: "+pcsc.getColorName(c));
                    if (pcsc.getColorName(c).equals(clrTerm)) {
                        // clrTerm is a single named color
//                        System.out.println("color term "+clrTerm+" is a named single color");
                        num += 1;
                        found = true;
                    }
                }
            }
            if (!found) {
                log.add("Color expression "+clrExpr+" is not valid.");
            }
        }
        return num;
    }
    
    // close the print writer stream and prepare the report to be returned
    private static String reportLog(ArrayList<String> log, PrintWriter pw) {
        pw.close();
        if (log.isEmpty()) {
            return null; // Everything went ok
        }
        else {
            String message = "Detected problems while exporting the NetLogo model.\n\n";
            for (String s : log) {
                message += s + "\n";
            }
            return message;
        }       
    }
}
