/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.io;

import common.UnixPrintWriter;
import editor.domain.elements.ConstantID;
import editor.domain.Edge;
import editor.domain.elements.GspnEdge;
import editor.domain.elements.GspnPage;
import editor.domain.LabelDecor;
import editor.domain.NetObject;
import editor.domain.Node;
import editor.domain.elements.BaseID;
import editor.domain.elements.ColorClass;
import editor.domain.elements.ColorVar;
import editor.domain.elements.Place;
import editor.domain.elements.TemplateVariable;
import editor.domain.elements.TokenType;
import editor.domain.elements.Transition;
import editor.domain.elements.ParsedColorSubclass;
import editor.domain.grammar.ExpressionLanguage;
import editor.domain.grammar.ParserContext;
import java.awt.geom.Point2D;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Properties;
import java.util.Scanner;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.swing.filechooser.FileFilter;
import javax.swing.filechooser.FileNameExtensionFilter;

/**
 *
 * @author elvio
 */
public class GreatSpnFormat {

    public static final FileFilter fileFilter = new FileNameExtensionFilter("GreatSPN .net file (*.net)", new String[]{"net"});

    // Transform an integer expression in GreatSPN format.
    // GreatSPN either accepts a positive integer or a (negative) marking parameter index
    public static String intOrMpar(String expr, String what, GspnPage gspn, ArrayList<String> log) {
        if (expr.isEmpty())
            return "0";
        if (NetObject.isInteger(expr)) {
            return expr;
        }

        int markParNum = 1;
        for (Node mpar : gspn.nodes) {
            if (mpar instanceof ConstantID) {
                ConstantID con = (ConstantID) mpar;
                if (con.isInNeutralDomain() && con.isIntConst()) {
                    if (con.getUniqueName().equals(expr)) {
                        expr = "-" + markParNum;
                        return expr;
                    }
                    markParNum++;
                }
            }
            else if (mpar instanceof TemplateVariable) {
                TemplateVariable tvar = (TemplateVariable)mpar;
                if (tvar.getType() == TemplateVariable.Type.INTEGER) {
                    if (tvar.getUniqueName().equals(expr)) {
                        expr = "-" + markParNum;
                        return expr;
                    }
                    markParNum++;
                }
            }
        }
        log.add("Could not export correctly " + what + " with value '" + expr + "'");
        return "1";
    }

    // Transform a real expression in GreatSPN format.
    // GreatSPN either accepts a positive double or a (negative) rate parameter index
    public static String realOrRpar(String expr, String what, GspnPage gspn, ArrayList<String> log) {
        if (NetObject.isDouble(expr)) {
            return expr;
        }

        int rateParNum = 1;
        for (Node rpar : gspn.nodes) {
            if (rpar instanceof ConstantID) {
                ConstantID con = (ConstantID) rpar;
                if (con.isInNeutralDomain() && con.isRealConst()) {
                    if (con.getUniqueName().equals(expr)) {
                        expr = "-" + rateParNum;
                        return expr;
                    }
                    rateParNum++;
                }
            }
            else if (rpar instanceof TemplateVariable) {
                TemplateVariable tvar = (TemplateVariable)rpar;
                if (tvar.getType() == TemplateVariable.Type.REAL) {
                    if (tvar.getUniqueName().equals(expr)) {
                        expr = "-" + rateParNum;
                        return expr;
                    }
                    rateParNum++;
                }
            }
        }
        if (log != null) {
            log.add("Could not export correctly " + what + " with value '" + expr + "'");
            return "1.0";
        }
        else return null;
    }

    public static String ensureInt(String expr, String what, ArrayList<String> log) {
        if (NetObject.isInteger(expr)) {
            return expr;
        }
        log.add("Could not export correctly " + what + " with value '" + expr + "'");
        return "1";
    }

    public static String ensureReal(String expr, String what, ArrayList<String> log) {
        if (NetObject.isDouble(expr)) {
            return expr;
        }
        log.add("Could not export correctly " + what + " with value '" + expr + "'");
        return "1.0";
    }

    private static double scaleCoord(double coord) {
        return coord / 6.0;
    }

    private static final String[][] old2new = {
        /*{ "!", "++" }, { "^", "--" },*/
        { "not", "!" }, { "and", "&&" }, { "or", "||" },
        { "~", "!" }, { "&", "&&" },
        { "<>", "!=" }, { "/=", "!=" }, { "=", "==" }
    };

    // converts an expression from the editor language to the greatspn language
    private static String exportExpr(String expr, Set<ColorClass> allColorClasses, ArrayList<String> log) {
        ArrayList<SimpleLexer.Token> list = SimpleLexer.lex(expr);
        token_loop:
        for (int i=0; i<list.size(); i++) {
            SimpleLexer.Token prevTok = (i == 0 ? null : list.get(i-1));
            SimpleLexer.Token tok = list.get(i);
            SimpleLexer.Token nextTok = (i == list.size() - 1 ? null : list.get(i+1));

            if ((tok.data.equals("in") || tok.data.equals("!in")) &&
                prevTok != null &&  prevTok.type == SimpleLexer.TokenType.ID)
            {
                // beware: "in" is an ID, while "!in" is an OP.
                //  x  in Class   ->   d(x) =  Class
                //  x !in Class   ->   d(x) <> Class
                String t2 = tok.data, t1= prevTok.data;
                tok.data = tok.data.equals("in") ? "=" : "<>";
                prevTok.data = "d(" + prevTok.data +")";
                // Check that nextTok is actually a Color class or a static subclass
                boolean isNextTokCC = false;
                for (ColorClass cc : allColorClasses)
                    if (cc.isSimpleClass())
                        if (cc.getUniqueName().equals(nextTok.data) ||
                            cc.testHasStaticSubclassNamed(nextTok.data))
                            isNextTokCC = true;
                if (!isNextTokCC) {
                    log.add("Expression: \""+t1+" "+t2+" "+nextTok.data+"\" cannot be translated in the GreatSPN format. "
                            + "Color variable "+t1+" can only be tested with the 'in' operator against a static subclass.");
                }
                i++; // skip Class
                continue;
            }

            switch (tok.type) {
                case ID: {
                    if (tok.data.equals("All")) {
                        tok.data = "S";
                        break;
                    }

                    // subclass -> S subclass
                    for (ColorClass cc : allColorClasses)
                        if (cc.isSimpleClass() && cc.testHasStaticSubclassNamed(tok.data)) {
                            tok.data = "S " + tok.data;
                            continue token_loop;
                        }
                }
                break;

                case OP: {
                    if (tok.data.equals("++") && prevTok != null) { // x++ -> !x
                        prevTok.data = "!" + prevTok.data;
                        tok.clear();
                        break;
                    }
                    if (tok.data.equals("--") && prevTok != null) { // x-- -> ^x
                        prevTok.data = "^" + prevTok.data;
                        tok.clear();
                        break;
                    }
                    for (String[] pair : old2new) {
                        if (tok.data.equals(pair[1])) {
                            tok.data = pair[0];
                            break;
                        }
                    }
                }
                break;
            }
        }
        return SimpleLexer.unlex(list);
    }

    // imports an expression in the greatspn language
    private static String importExpr(String expr, Set<ColorClass> allColorClasses) {
        ArrayList<SimpleLexer.Token> list = SimpleLexer.lex(expr);
        token_loop:
        for (int i=0; i<list.size(); i++) {
            SimpleLexer.Token prevTok = (i == 0 ? null : list.get(i-1));
            SimpleLexer.Token tok = list.get(i);
            SimpleLexer.Token nextTok = (i == list.size() - 1 ? null : list.get(i+1));
            switch (tok.type) {
                case ID: {

                    if (tok.data.equals("S") && nextTok != null) {
                        // S subclass -> subclass
                        for (ColorClass cc : allColorClasses)
                            if (cc.isSimpleClass() && cc.testHasStaticSubclassNamed(nextTok.data)) {
                                list.remove(i);
                                continue token_loop;
                            }
                        // otherwise transform S -> All
                        tok.data = "All";
                        break;
                    }
                    //   d(x) = Class    ->   x in  Class
                    //   d(x) <> Class   ->   x !in Class
                    if (tok.data.equals("d") && (list.size() - i) > 4 &&
                             list.get(i + 1).is(SimpleLexer.TokenType.OP, "(") &&
                             list.get(i + 2).type == SimpleLexer.TokenType.ID &&
                             list.get(i + 3).is(SimpleLexer.TokenType.OP, ")") &&
                             (list.get(i + 4).is(SimpleLexer.TokenType.OP, "=") ||
                              list.get(i + 4).is(SimpleLexer.TokenType.OP, "<>")))
                    {
                        boolean eq = list.get(i + 4).is(SimpleLexer.TokenType.OP, "=");
                        tok.clear();
                        list.get(i + 4).data = eq ? " in " : " !in ";
                        list.get(i+3).clear();
                        list.get(i+1).clear();
                        break;
                    }
                    // Boolean operators: and, or, not
                    if (tok.data.equals("and")) {
                        tok.data = "&&";
                        tok.type = SimpleLexer.TokenType.OP;
                        break;
                    }
                    if (tok.data.equals("or")) {
                        tok.data = "||";
                        tok.type = SimpleLexer.TokenType.OP;
                        break;
                    }
                    if (tok.data.equals("not")) {
                        tok.data = "!";
                        tok.type = SimpleLexer.TokenType.OP;
                        break;
                    }
                }
                break;

                case OP: {
                    if (tok.data.equals("!") && nextTok != null) { // !x -> x++
                        tok.data = "++";
                        list.set(i, nextTok);
                        list.set(i + 1, tok);
                        break;
                    }
                    if (tok.data.equals("^") && nextTok != null) { // ^x -> x--
                        tok.data = "--";
                        list.set(i, nextTok);
                        list.set(i + 1, tok);
                        break;
                    }
                    for (String[] pair : old2new) {
                        if (tok.data.equals(pair[0])) {
                            tok.data = pair[1];
                            break;
                        }
                    }
                }
                break;
            }
        }
        return SimpleLexer.unlex(list);
    }

    // Converts an ordinary marking to a short marking, if possible,
    // otherwise returns the orinary marking as-is.
    //   ordinary_marking :=  coeff  '<'  marking_list  '>'
    //       marking_list :=  'S' ( ',' 'S' )*
    //      short_marking :=  coeff  'S'
    //              coeff := could be an integer or a marking parameter
    private static String ordinaryToShortMarking(String mark) {
//        System.out.print("ordinaryToShortMarking: "+mark +"  ->  ");
        Pattern p = Pattern.compile("(?<coeff>[0-9a-zA-Z_]*)\\s*<\\s*S(\\s*,\\s*S\\s*)*\\s*>\\s*$");
        Matcher m = p.matcher(mark);
        if (!m.matches())
            return mark; // Not convertible to short marking
        String coeff = m.group("coeff"); // null or coefficient
        if (coeff == null)
            return "S";
        else
            return  coeff + " S";
    }

//    public static void main(String[] args) {
//        System.out.println(ordinaryToShortMarking("<S>"));
//        System.out.println(ordinaryToShortMarking("23<S>"));
//        System.out.println(ordinaryToShortMarking("<S,S k>"));
//        System.out.println(ordinaryToShortMarking("<S,S,S,S>"));
//        System.out.println(ordinaryToShortMarking("3<S,S>"));
//        System.out.println(ordinaryToShortMarking("2<S> + <S>"));
//    }

    // Converts a short marking into an ordinary marking
    private static String shortToOrdinaryMarking(String mark, String colorDomainDef) {
//        System.out.print("shortToOrdinaryMarking("+mark+", "+colorDomainDef+")  ->  ");
        String coeff = null;
        if (!mark.trim().equals("S")) {
            Pattern p = Pattern.compile("(?<coeff>[0-9a-zA-Z_]*)\\s*S\\s*$");
            Matcher m = p.matcher(mark);
            if (!m.matches())
                return mark; // not a short marking
            coeff = m.group("coeff");
        }

        StringBuilder ordMark = new StringBuilder();
        if (coeff != null)
            ordMark.append(coeff).append(" ");
        ordMark.append("<");
        int numClassesInDomain = 1;
        for (int i=0; i<colorDomainDef.length(); i++)
            if (colorDomainDef.charAt(i) == '*')
                numClassesInDomain++;
        for (int d=0; d<numClassesInDomain; d++)
            ordMark.append(d==0 ? "": ",").append("S");
        ordMark.append(">");
        return ordMark.toString();
    }

    // Some old tools in the GreatSPN toolchain expect transitions to be ordered,
    // from timed to immediate, and immediate sorted by increasing priority
    public static Transition[] getTransitionsInOrder(GspnPage gspn) {
        ArrayList<Transition> trns = new ArrayList<>();
        for (Node node : gspn.nodes) {
            if (node instanceof Transition) {
                Transition trn = (Transition) node;
                trns.add(trn);
            }
        }
        Collections.sort(trns, new Comparator<Transition>() {
            @Override
            public int compare(Transition t1, Transition t2) {
                return Integer.compare(getIntPriorityOf(t1), getIntPriorityOf(t2));
            }
        });
        return trns.toArray(new Transition[trns.size()]);
    }

    private static int getIntPriorityOf(Transition t) {
        if (!t.hasPriority())
            return 0; // Timed transition has priority 0
        try {
            return Integer.parseInt(t.getPriority());
        }
        catch (NumberFormatException e) {
            return Integer.MAX_VALUE;
        }
    }

    private static int getMaxNameLength(GspnPage gspn, Class nodeClass) {
        int maxLen = 0;
        for (Node node : gspn.nodes) {
            if (node.getClass() == nodeClass) {
                int tagLen = node.getSuperPosTags().isEmpty() ? 0 : node.getSuperPosTags().length() + 1;
                maxLen = Math.max(maxLen, node.getUniqueName().length() + tagLen);
            }
        }
        return maxLen;
    }

    private static void printPaddedName(PrintWriter out, Node node, int maxNameLength) {
        String name = node.getUniqueName();
        String tag = node.getSuperPosTags();
        int tagLen = tag.isEmpty() ? 0 : tag.length() + 1;
        int pad = maxNameLength - name.length() - tagLen;
        out.print(name);
        if (!tag.isEmpty()) {
            out.print("|");
            out.print(tag);
        }
        for (int i=0; i<pad; i++)
            out.print(" ");
    }

//    public static void main(String[] args) {
//        System.out.println(shortToOrdinaryMarking("S", "C"));
//        System.out.println(shortToOrdinaryMarking("3 S", "C1*C2*C3"));
//    }

    // Parameter useExt means that we are using the new extensions of the format that
    // are not compatible with the old GreatSPN GUI.
    public static String exportGspn(GspnPage gspn, File netFile, File defFile, 
                                    boolean useExt, boolean useMDepArcs) throws Exception 
    {
        if (!gspn.isPageCorrect()) {
            throw new UnsupportedOperationException("GSPN must be correct before exporting.");
        }
        System.out.println("exportGspn useExt="+useExt);

        Properties sysProps = System.getProperties();
        Object oldLineSep = null;
        try {
            ParserContext context = new ParserContext(gspn);
            oldLineSep = sysProps.setProperty("line.separator", "\n");

            ArrayList<String> log = new ArrayList<>();

            // Always save the net/def files with Unix line endings using the UnixPrintWriter
            PrintWriter net = new UnixPrintWriter(new BufferedOutputStream(new FileOutputStream(netFile)));
            PrintWriter def = new UnixPrintWriter(new BufferedOutputStream(new FileOutputStream(defFile)));

            net.println("|0|\n|");

            ArrayList<String> defs = new ArrayList<>();
            int numMarkDefs = 0;

            ArrayList<String> markDepDefs = new ArrayList<>();
            int maxPlaceNameLen = getMaxNameLength(gspn, Place.class);
            int maxTransitionNameLen = getMaxNameLength(gspn, Transition.class);

            // Start writing the .net file
            int numMarkPars = 0, numPlaces = 0, numTrns = 0, numRatePars = 0;
            Set<Integer> groups = new TreeSet<>();
            for (Node node : gspn.nodes) {
                if (node instanceof ConstantID) {
                    ConstantID con = (ConstantID)node;
                    if (con.isIntConst() && con.isInNeutralDomain()) {
                        numMarkPars++;
                    }
                    else if (con.isRealConst() && con.isInNeutralDomain()) {
                        numRatePars++;
                    }
                }
                if (node instanceof TemplateVariable) {
                    TemplateVariable tvar = (TemplateVariable)node;
                    if (tvar.getType() == TemplateVariable.Type.INTEGER)
                        numMarkPars++;
                    else if (tvar.getType() == TemplateVariable.Type.REAL)
                        numRatePars++;
                }
                numPlaces += (node instanceof Place) ? 1 : 0;
                if (node instanceof Transition) {
                    Transition trn = (Transition) node;
                    numTrns++;
                    if (trn.isImmediate()) {
                        groups.add(Integer.parseInt(ensureInt(trn.getPriority(), "transition priority", log)));
                    }
                }
            }

            // Print the color definitions
            Set<ColorClass> allColorClasses = new HashSet<>();
            for (Node node : gspn.nodes) {
                if (node instanceof ColorClass) {
                    ColorClass cc = (ColorClass)node;
                    allColorClasses.add(cc);
                    if (!cc.isSimpleClass())
                        continue;
                    String[] subclassNames = new String[cc.numSubClasses()];
                    String allSubclasses = "";
                    for (int i=0; i<cc.numSubClasses(); i++) {
                        subclassNames[i] = (cc.getSubclass(i).isNamed() ?
                                            cc.getSubclass(i).name : cc.getUniqueName()+"_"+i);
                        allSubclasses += (i==0 ? "" : ",") + subclassNames[i];
                    }
                    String colorDef = "("+cc.getUniqueName()+" c " +
                                      scaleCoord(cc.getX()) + " " +
                                      scaleCoord(cc.getY()) + " " +
                                      " (@c\n" + (cc.isCircular() ? "o " : "u ") +
                                      allSubclasses + "\n))";
                    defs.add(colorDef);
                    for (int i=0; i<cc.numSubClasses(); i++) {
                        ParsedColorSubclass pcs = cc.getSubclass(i);
                        String subClassDef;
                        if (pcs.isInterval())
                            subClassDef = (pcs.getIntervalPrefix() == null ? "" : pcs.getIntervalPrefix())+
                                          "{"+pcs.getStartRangeExpr()+"-"+pcs.getEndRangeExpr()+"}";
                        else {
                            subClassDef = "{";
                            for (int j=0; j<pcs.getNumColors(); j++)
                                subClassDef += (j==0 ? "" : ",") + pcs.getColorName(j);
                            subClassDef += "}";
                        }
                        colorDef = "("+subclassNames[i]+" c " +
                                      scaleCoord(cc.getX() + 4*(i+1)) + " " +
                                      scaleCoord(cc.getY()) + " " +
                                   " (@c\n"+subClassDef+"\n))";
                        defs.add(colorDef);
                    }
                }
            }

            // Export the int multiset constants in the .def file
            Map<String, Integer> intMsetMarks = new HashMap<>();
            for (Node node : gspn.nodes) {
                if (node instanceof ConstantID) {
                    ConstantID con = (ConstantID)node;
                    if (con.isInNeutralDomain())
                        continue;
                    if (con.isIntConst()) {
                        String colMark = exportExpr(con.getConstantExpr().getExpr(), allColorClasses, log);
                        colMark = ordinaryToShortMarking(colMark);
                        String markDef = "(m"+numMarkDefs+" m "+scaleCoord(con.getX())+
                                         " "+scaleCoord(con.getY())+" (@m\n"+colMark+"\n))";
                        defs.add(markDef);
                        numMarkDefs++;
                        intMsetMarks.put(node.getUniqueName(), defs.size());
                    }
                    else {
                        log.add("Real multiset constants are not supported in net/def format. Could not export '" + con.getUniqueName() + "'.");
                    }
                }
            }


            // Print the count header
            net.println("f " + numMarkPars + " " + numPlaces + " " + numRatePars + " " + numTrns + " " + groups.size() + " 0 0");

            // Print marking parameters
            Map<String, Integer> mpar2pos = new TreeMap<>();
            for (Node node : gspn.nodes) {
                String mparValue = null;
                if (node instanceof ConstantID) {
                    ConstantID con = (ConstantID) node;
                    if (con.isIntConst() && con.isInNeutralDomain()) {
                        mparValue = ensureInt(con.getConstantExpr().getExpr(), "marking parameter", log);
                    }
                }
                else if (node instanceof TemplateVariable) {
                    TemplateVariable tvar = (TemplateVariable)node;
                    if (tvar.getType() == TemplateVariable.Type.INTEGER) {
                        mparValue = "-7134";
                    }
                }
                if (mparValue != null) {
                    mpar2pos.put(node.getUniqueName(), mpar2pos.size());
                    net.println(node.getUniqueName() + " " + mparValue + " "
                                + scaleCoord(node.getCenterX()) + " " + scaleCoord(node.getCenterY()) + " 0");
                }
            }

            // Print places
            Map<Place, Integer> plc2pos = new HashMap<>();
            for (Node node : gspn.nodes) {
                if (node instanceof Place) {
                    Place plc = (Place) node;
                    if (plc.isContinuous())
                        log.add("Continuous place "+plc.getUniqueName()+" is not convertible in GreatSPN format.");
                    String initMarkExpr;
                    String color = "", domain = "";
                    if (plc.isInColorDomain()) {
                        ColorClass cc = plc.getColorDomain();
                        for (int i=0; i<cc.getNumClassesInDomain(); i++)
                            domain += (i==0? "" : ",") + cc.getColorClassName(i);
                        color = " " + scaleCoord(plc.getCenterX() + 1.2) +
                                " " + scaleCoord(plc.getCenterY() + 1.0) +
                                " " + domain;
                        String colMark = plc.getInitMarkingEditable().getValue().toString();
                        if (colMark.isEmpty())
                            initMarkExpr = "0";
                        else if (intMsetMarks.containsKey(colMark)) {
                            initMarkExpr = "" + (-10000 - intMsetMarks.get(colMark));
                        }
                        else { // create a marking definition
                            colMark = exportExpr(colMark, allColorClasses, log);
                            colMark = ordinaryToShortMarking(colMark);
                            String markDef = "(m_"+numMarkDefs+" m 10.0 "+(2.0+numMarkDefs)+" (@m\n"+colMark+"\n))";
                            defs.add(markDef);
                            initMarkExpr = "" + (-10000 - defs.size());
                            numMarkDefs++;
                        }
                    }
                    else {
                        initMarkExpr = intOrMpar(plc.getInitMarkingEditable().getValue().toString(),
                                                 "initial marking", gspn, log);
                    }
                    printPaddedName(net, plc, maxPlaceNameLen);
                    net.println(" " + initMarkExpr + " "
                            + scaleCoord(plc.getCenterX()) + " " + scaleCoord(plc.getCenterY()) + " "
                            + scaleCoord(plc.getCenterX() + plc.getUniqueNameDecor().getRelativeX() + 0.33 - domain.length()/2.0) + " "
                            + scaleCoord(plc.getCenterY() + plc.getUniqueNameDecor().getRelativeY() + 0.33) + " 0"+color);
                    plc2pos.put(plc, plc2pos.size() + 1);
                }
            }

            // Print rate parameters
            for (Node node : gspn.nodes) {
                String rparValue = null;
                if (node instanceof ConstantID) {
                    ConstantID con = (ConstantID) node;
                    if (con.isRealConst() && con.isInNeutralDomain()) {
                        rparValue = ensureReal(con.getConstantExpr().getExpr(), "rate parameter", log);
                    }
                }
                else if (node instanceof TemplateVariable) {
                    TemplateVariable tvar = (TemplateVariable)node;
                    if (tvar.getType() == TemplateVariable.Type.REAL) {
                        rparValue = "-7134.0";
                    }
                }
                if (rparValue != null)
                    net.println(node.getUniqueName() + " " + rparValue + " "
                            + scaleCoord(node.getCenterX()) + " " + scaleCoord(node.getCenterY()) + " 0");

            }

            //Print groups
            Map<Integer, Integer> prio2grp = new TreeMap<>();
            for (int prio : groups) {
                prio2grp.put(prio, prio2grp.size() + 1);
                net.println("G" + prio2grp.size() + " 0.0 0.0 " + prio);
            }

            // Print transitions
            Map<Transition, EdgeList> trn2edgeList = buildTransitionsEdgeLists(gspn);
            int trnNum = 0;
            Transition[] trnsList = getTransitionsInOrder(gspn);
            for (Transition trn : trnsList) {
                String delay;
                int prio;
                int enabDeg;
                if (trn.isImmediate()) {
                    delay = trn.getWeight();
                    prio = prio2grp.get(Integer.parseInt(ensureInt(trn.getPriority(), "transition priority", log)));
                    enabDeg = 1;
                }
                else if (trn.isGeneral()) {
                    String simpleDet = trn.getSimpleDeterministicDelay();
                    if (simpleDet != null)
                        delay = simpleDet;
                    else {
                        markDepDefs.add("|"+(trnNum+1)+" \n"+trn.getDelay());
                        delay = "-5.100000e+02";
                    }
                    prio = 127;
                    enabDeg = 0;
                }
                else if (trn.isContinuous()) {
                    log.add("Continuous transition "+trn.getUniqueName()+" is not convertible in GreatSPN format.");
                    delay = "1.0";
                    prio = 0;
                    enabDeg = 0;
                }
                else {
                    delay = trn.getDelay();
                    prio = 0;
                    enabDeg = (trn.isInfiniteServers() ? 0
                               : Integer.parseInt(ensureInt(trn.getNumServers(), "number of servers", log)));
                }

                if (trn.isExponential()) {
                    delay = realOrRpar(delay, "transition coefficient", gspn, null);
                    if (delay == null) { // marking-dependent rate
                        delay = trn.convertDelayLang(context, null, ExpressionLanguage.GREATSPN);
                        markDepDefs.add("|"+(trnNum+1)+"\n"+delay);
                        delay = "-5.100000e+02";
                        enabDeg = 1;
                    }
                }
                else if (trn.isImmediate() && useExt) {
                    delay = realOrRpar(delay, "transition coefficient", gspn, null);
                    if (delay == null) { // marking-dependent rate
                        delay = trn.convertWeightLang(context, null, ExpressionLanguage.GREATSPN);
                        markDepDefs.add("|"+(trnNum+1)+"\n"+delay);
                        delay = "-5.100000e+02";
                    }
                }
                else // can only be a constant or a rate parameter
                    delay = realOrRpar(delay, "transition coefficient", gspn, log);

                int degrees = (int) Math.round((trn.getRotation() - Math.PI / 16) / (Math.PI / 4) + 8) % 4;
                int orientation;
                switch (degrees) {
                    case 0:
                        orientation = 1;
                        break;
                    case 1:
                        orientation = 3;
                        break;
                    case 2:
                        orientation = 0;
                        break;
                    case 3:
                        orientation = 2;
                        break;
                    default:
                        throw new IllegalStateException("Wrong orientation index.");
                }
//                System.out.println(trn.getUniqueName()+"  "+trn.getRotation() + "   "+(degrees/Math.PI*180)+"  "+orientation);
//                System.out.println(trn.getUniqueName()+"  "+trn.getRotation() + "   "+degrees+"  "+orientation+"   : "
//                                   +(trn.getRotation() - Math.PI/16) / (Math.PI/4));

                StringBuilder inArcs = new StringBuilder();
                StringBuilder outArcs = new StringBuilder();
                StringBuilder inhibArcs = new StringBuilder();
                int numInputArcs = 0, numOutpurArcs = 0, numInhibArcs = 0;
                for (EdgeList edgeList = trn2edgeList.get(trn);
                    edgeList != null; edgeList = edgeList.next)
                {
                    GspnEdge arc = edgeList.edge;
                    int plcIndex;
                    StringBuilder sb;
                    switch (arc.getEdgeKind()) {
                        case INPUT:
                            if (arc.getHeadNode() != trn) {
                                continue;
                            }
                            numInputArcs++;
                            sb = inArcs;
                            plcIndex = plc2pos.get((Place) arc.getTailNode());
                            break;

                        case OUTPUT:
                            if (arc.getTailNode() != trn) {
                                continue;
                            }
                            numOutpurArcs++;
                            sb = outArcs;
                            plcIndex = plc2pos.get((Place) arc.getHeadNode());
                            break;

                        case INHIBITOR:
                            if (arc.getHeadNode() != trn) {
                                continue;
                            }
                            numInhibArcs++;
                            sb = inhibArcs;
                            plcIndex = plc2pos.get((Place) arc.getTailNode());
                            break;

                        default:
                            throw new IllegalStateException();
                    }

                    sb.append("   ");
                    sb.append(arc.isBroken ? "-" : "");
                    boolean isColored = !arc.getColorDomainOfConnectedPlace().isNeutralDomain();
                    boolean hasMdepExpr = false;
                    if (isColored)
                        sb.append("1");
                    else if (useMDepArcs && !NetObject.isInteger(arc.getMultiplicity())) {
                        hasMdepExpr = true;
                        sb.append("1");
                    }
                    else if (useExt && mpar2pos.containsKey(arc.getMultiplicity()))
                        sb.append(20000 + mpar2pos.get(arc.getMultiplicity()));
                    else
                        sb.append(ensureInt(arc.getMultiplicity(), "arc multiplicity", log));
                    sb.append(" ");
                    sb.append(plcIndex);
                    sb.append(" ");
                    sb.append(arc.numPoints() - 2);
                    sb.append(" 0");
                    if (isColored || hasMdepExpr)
                        sb.append(" 0.000000 0.000000 ").append(exportExpr(arc.getMultiplicity(), allColorClasses, log));
                    sb.append("\n");
                    for (int pt = 1; pt < arc.numPoints() - 1; pt++) {
                        int pInd = pt;
                        if (arc.getEdgeKind() != GspnEdge.Kind.OUTPUT) {
                            pInd = arc.numPoints() - pt - 1;
                        }
                        sb.append(scaleCoord(arc.points.get(pInd).getX()));
                        sb.append(" ");
                        sb.append(scaleCoord(arc.points.get(pInd).getY()));
                        sb.append("\n");
                    }
                }

                String guard = "";
                if (trn.hasGuard() && !trn.getGuard().equalsIgnoreCase("True"))
                    guard = " " + scaleCoord(trn.getCenterX() + trn.getGuardDecor().getRelativeX()) +
                            " " + scaleCoord(trn.getCenterY() + trn.getGuardDecor().getRelativeY()) +
                            " [" + exportExpr(trn.getGuard(), allColorClasses, log) + "]";

                printPaddedName(net, trn, maxTransitionNameLen);
                net.println(" " + delay + " " + enabDeg + " " + prio + " " + numInputArcs + " " + orientation + " "
                        + scaleCoord(trn.getCenterX()) + " " + scaleCoord(trn.getCenterY()) + " "
                        + scaleCoord(trn.getCenterX() + trn.getUniqueNameDecor().getRelativeX() + 0.5) + " "
                        + scaleCoord(trn.getCenterY() + trn.getUniqueNameDecor().getRelativeY() + 1.0) + " "
                        + scaleCoord(trn.getCenterX() + trn.getDelayDecor().getRelativeX()) + " "
                        + scaleCoord(trn.getCenterY() + trn.getDelayDecor().getRelativeY()) + " 0" + guard);
                net.print(inArcs.toString());
                net.println("   " + numOutpurArcs);
                net.print(outArcs.toString());
                net.println("   " + numInhibArcs);
                net.print(inhibArcs.toString());
                trnNum++;
            }

            // Write the .def file. First write the marking-dependent functions
            for (String d : markDepDefs)
                def.println(d);
            def.println("|256\n%\n|");
            // The write the SWN definitions
            for (String d : defs)
                def.println(d);

            net.close();
            def.close();

            if (log.isEmpty()) {
                return null; // Everything went ok
            }
            else {
                String message = "Detected problems in the exported GreatSPN net.\n\n";
                for (String s : log) {
                    message += s + "\n";
                }
                return message;
            }
        }
        finally {
            // Reset the line separator
            if( oldLineSep != null ) {
                sysProps.put("line.separator", oldLineSep);
            }
        }
    }

    private static class EdgeList {
        GspnEdge edge;
        EdgeList next;

        public EdgeList(GspnEdge edge, EdgeList next) {
            this.edge = edge;
            this.next = next;
        }

        public static EdgeList reverse(EdgeList head) {
            // if head is null or only one node, it's reverse of itself.
            if ( (head==null) || (head.next == null) ) return head;

            // reverse the sub-list leaving the head node.
            EdgeList reverse = reverse(head.next);

            // head.next still points to the last element of reversed sub-list.
            // so move the head to end.
            head.next.next = head;

            // point last node to nil, (get rid of cycles)
            head.next = null;
            return reverse;
        }
    }

    // Map that assigns to each transition a linked list of edges
    private static Map<Transition, EdgeList> buildTransitionsEdgeLists(GspnPage gspn) {
        Map<Transition, EdgeList> trn2edgeList = new HashMap<>();
//        for (Node node : gspn.nodes) {
//            if (node instanceof Transition)
//                trn2edgeList.put((Transition)node, null);
//        }

        for (Edge edge : gspn.edges) {
            if (edge instanceof GspnEdge) {
                GspnEdge arc = (GspnEdge) edge;
                Transition trn = arc.getConnectedTransition();
                EdgeList edgeList = trn2edgeList.get(trn);
                edgeList = new EdgeList(arc, edgeList);
                trn2edgeList.put(trn, edgeList);
            }
        }

        // Preserve edge order - reverse the lists.
        for (Map.Entry<Transition, EdgeList> e : trn2edgeList.entrySet()) {
            e.setValue(EdgeList.reverse(e.getValue()));
        }

        return trn2edgeList;
    }

    private static void readToken(Scanner sc, String token) {
        String s = sc.next();
        if (s.equals(token)) {
            return;
        }
        throw new RuntimeException("Unexpected \"" + s + "\" - expected \"" + token + "\".");
    }

    private static double readCoord(Scanner sc) {
        return Double.parseDouble(sc.next()) * 6.0;
    }

    private static Point2D readPos(Scanner sc) {
        return new Point2D.Double(readCoord(sc), readCoord(sc));
    }

    // A definition in the .def file.
    private static class Def {
        String name, type /*= c,f,m*/, expr;
        Point2D pos;

        public Def(String name, String type, String expr, Point2D pos) {
            this.name = name;
            this.type = type;
            this.expr = expr;
            this.pos = pos;
        }
    }

    private static void skip_layers(Scanner net) {
        int num;
        do {
            num = net.nextInt();
        }
        while (num != 0);
    }

    private static String sanitizeName(String name) {
        return name.replace("'", "_prime").replace("-", "_");
    }

    public static String importGspn(GspnPage gspn, File netFile, File defFile) throws Exception {
        ArrayList<String> log = new ArrayList<>();
        Scanner net = new Scanner(netFile);
        Scanner def = new Scanner(defFile);
        String line;

        // ------ Read the .def file ------
        Map<Integer, String> markDepDefs = new HashMap<>();
        ArrayList<Def> definitions = new ArrayList<>();
        int numExtraColDef = 0;

        // Read marking-dependent functions
        while(def.hasNextLine()) {
            line = def.nextLine().trim();
            if (line.equals("|256"))
                break; // mark-dep functions section ends
            if (line.startsWith("|")) {
                int trnIndex = Integer.parseInt(line.substring(1));
                line = def.nextLine().trim();
                markDepDefs.put(trnIndex, line);
                System.out.println("reading mark dep def: "+line);
            }
        }

        // Skip all lines until the '|' line
        def.nextLine();
        do {
            line = def.nextLine().trim();
        } while (def.hasNextLine() && !line.equals("|"));

        // Read color definitions
        while (def.hasNext()) {
            String name = def.next();
            if (name.isEmpty() || !name.startsWith("("))
                break;
            String type = def.next();
            assert "c".equals(type) || "m".equals(type) || "f".equals(type);
            Point2D pos = readPos(def);
            def.nextLine(); // skip
            String expr = def.nextLine();
            def.nextLine(); // skip
//            System.out.println("name="+name+" type="+type+" expr="+expr);
            definitions.add(new Def(name.substring(1), type, expr, pos));
        }

        // Create color classes
        Set<ColorClass> allColorClasses = new HashSet<>();
        for (Def d : definitions) {
            if (d.type.equals("c") && !d.expr.contains("{")) { // color class definition
                Scanner ccDef = new Scanner(d.expr.replace(",", " "));
                String uo = ccDef.next();
                assert uo.equals("u") || uo.equals("o");
                String classDef = uo.equals("o") ? "circular " : "";
                int subCount = 0;
                // Read static subclasses
                while (ccDef.hasNext()) {
                    classDef += (subCount++ == 0 ? "" : " + ");
                    String subclassName = ccDef.next();
                    for (Def sub : definitions) {
                        if (sub.name.equals(subclassName)) {
                            classDef += sub.expr.replace("-", "..");
                            break;
                        }
                    }
                    if (!subclassName.startsWith(d.name+"_"))
                        classDef += " is " + subclassName;
                }
                ColorClass cc = new ColorClass(d.name, d.pos, classDef);
                gspn.nodes.add(cc);
                allColorClasses.add(cc);
            }
        }

        // ------ Read the .net file ------
        // Skip the initial .net header
        net.nextLine();
        do {
            line = net.nextLine();
        } while (net.hasNextLine() && (line.isEmpty() || line.charAt(0) != '|'));

        // Header with object counters
        readToken(net, "f");
        int numMarkPars = net.nextInt();
        int numPlcs = net.nextInt();
        int numRatePars = net.nextInt();
        int numTrns = net.nextInt();
        int numGrps = net.nextInt();
        int csetNum = net.nextInt();
        // Very old net/def files could not have this number.
        int layerNum = 0;
        boolean hasLayers = net.hasNextInt();
        if (hasLayers)
            layerNum = net.nextInt();

        // Read marking parameters
        BaseID[] markPars = new BaseID[numMarkPars];
        for (int i = 0; i < numMarkPars; i++) {
            String name = net.next();
            int value = net.nextInt();
            Point2D pos = readPos(net);
            net.nextLine();

            BaseID mpar;
            if (value == -7134) // template marking parameter
                mpar = new TemplateVariable(TemplateVariable.Type.INTEGER, name, "", pos);
            else
                mpar = new ConstantID(ConstantID.ConstType.INTEGER, name, "" + value, "", pos);
            mpar.setNodePosition(pos.getX() - (mpar.getCenterX() - mpar.getX()),
                                 pos.getY() - (mpar.getCenterY() - mpar.getY()));
            gspn.nodes.add(mpar);
            markPars[i] = mpar;
        }

        // Read places
        Place[] places = new Place[numPlcs];
        for (int i = 0; i < numPlcs; i++) {
            String name = net.next(), superPosTags = "";
            if (name.contains("|")) {
                // split the name from the superposition tags
                int pos = name.indexOf("|");
                superPosTags = name.substring(pos + 1);
                name = name.substring(0, pos);
            }
            name = sanitizeName(name);
            String initMarkStr = net.next();
            int initMark = Integer.parseInt(ensureInt(initMarkStr, "Initial Marking cannot be parsed as INT.", log));
            Point2D pos = readPos(net);
            Point2D label = readPos(net);
            skip_layers(net);
            String colDef = net.nextLine().trim(), colorDomain = "", initMarkExpr;
            if (!colDef.isEmpty()) {
                Scanner colScan = new Scanner(colDef);
                colScan.next(); // skip X
                colScan.next(); // skip Y
                colorDomain = colScan.nextLine().trim();
                if (colorDomain.contains(",")) { // cross domain
                    // Should we add a cross domain?
                    String classDef = colorDomain.replace(",", " * ");
                    colorDomain = colorDomain.replace(",", "x");
                    if (null == gspn.getNodeByUniqueName(colorDomain)) {
                        ColorClass ccdom = new ColorClass(colorDomain,
                                new Point2D.Double(20, 3+numExtraColDef++), classDef);
                        gspn.nodes.add(ccdom);
                    }
                }
                if (initMark == 0)
                    initMarkExpr = "";
                else {
                    assert initMark < -10000 && definitions.get(-initMark - 10001).type.equals("m");
                    Def d = definitions.get(-initMark - 10001);
                    String defExpr = shortToOrdinaryMarking(d.expr, colorDomain);
                    if (d.name.startsWith("m_")) {
                        // copy the expression in initMark
                        initMarkExpr = importExpr(defExpr, allColorClasses);
                    }
                    else {
                        // Create int multiset constants
                        initMarkExpr = d.name;
                        if (gspn.getNodeByUniqueName(d.name) == null) {
                            ConstantID con = new ConstantID(ConstantID.ConstType.INTEGER, d.name,
                                                            importExpr(defExpr, allColorClasses),
                                                            colorDomain, d.pos);
                            gspn.nodes.add(con);
                        }
                    }
                    //  S(?!\\w)  means  'S' not followed by a word character (a-zA-Z0-9)
//                    initMarkExpr = (initMarkExpr.replaceAll("S(?!\\w)", "All").replaceAll("\\s+", " ")).replace(" ", ",");
                }
            }
            else { // neutral domain
                initMarkExpr = (initMark >= 0 ? "" + initMark : markPars[-initMark - 1].getUniqueName());
            }

            pos.setLocation(pos.getX() - Place.PLACE_RADIUS, pos.getY() - Place.PLACE_RADIUS);
            Place plc = new Place(name, initMarkExpr, TokenType.DISCRETE, colorDomain, "", pos);
            plc.setSuperPosTags(superPosTags);
            LabelDecor lab = plc.getUniqueNameDecor();
            lab.setInternalPos(label.getX() - plc.getCenterX() - 0.33 + lab.getWidth() / 2 + colorDomain.length()/2.0,
                               label.getY() - plc.getCenterY() - 0.33 + lab.getHeight() / 2);
            gspn.nodes.add(plc);
            places[i] = plc;
        }

        // Read rate parameters
        BaseID[] ratePars = new BaseID[numRatePars];
        for (int i = 0; i < numRatePars; i++) {
            String name = net.next();
            double value = Double.parseDouble(net.next());
            Point2D pos = readPos(net);
            net.nextLine();

            BaseID rpar;
            if (value == -7134.0) // template variable
                rpar = new TemplateVariable(TemplateVariable.Type.REAL, name, "", pos);
            else
                rpar = new ConstantID(ConstantID.ConstType.REAL, name, "" + value, "", pos);
            rpar.setNodePosition(pos.getX() - (rpar.getCenterX() - rpar.getX()),
                                 pos.getY() - (rpar.getCenterY() - rpar.getY()));
            gspn.nodes.add(rpar);
            ratePars[i] = rpar;
        }

        // Read Groups
        int[] prioGroups = new int[numGrps];
        for (int i = 0; i < numGrps; i++) {
            net.next();
            net.next();
            net.next();
            prioGroups[i] = net.nextInt();
            net.nextLine();
        }

        // Read transitions
        for (int i = 0; i < numTrns; i++) {
            String name = net.next(), superPosTags = "";
            if (name.contains("|")) {
                // split the name from the superposition tags
                int pos = name.indexOf("|");
                superPosTags = name.substring(pos + 1);
                name = name.substring(0, pos);
            }
            name = sanitizeName(name);
            String delayMark = net.next();
//            double delayIndex = Double.parseDouble(delayMark);
            int enabDegree = net.nextInt();
            int trnPrGroup = net.nextInt();
            int numArcs = net.nextInt();
            int orientation = net.nextInt();
            Point2D pos = readPos(net);
            Point2D tagPos = readPos(net);
            Point2D ratePos = readPos(net);
            skip_layers(net);
            String colGuard = net.nextLine().trim();

            String delay = delayMark;
            if (delayMark.equals("*"))
                delay = net.nextLine().trim();

            Transition.Type tt;
            int prio;
            String weight = "1.0";
            String numServers;
            if (enabDegree == 1 && trnPrGroup > 0) {
                double delayIndex = Double.parseDouble(delayMark);
                tt = Transition.Type.IMM;
                prio = prioGroups[trnPrGroup - 1];
                if (delayIndex < 0) {
                    weight = ratePars[-(int) delayIndex - 1].getUniqueName();
                }
                else
                    weight = "" + delayIndex;
                numServers = "1";
                delay = "1.0";
            }
            else if (enabDegree == 0 && trnPrGroup == 127) {
                tt = Transition.Type.GEN;
                prio = 0;
                numServers = "1";
                double delayIndex = Double.parseDouble(delayMark);
                if (delayIndex == -5.100000e+02) {
                    // Read the marking dependent function from the def file
                    delay = importExpr(markDepDefs.get(i+1), allColorClasses);
                }
                else if (!delayMark.equals("*")) // Constant deterministic duration
                    delay = "I[" + delay + "]";
            }
            else if (trnPrGroup == 0) {
                double delayIndex = Double.parseDouble(delayMark);
                tt = Transition.Type.EXP;
                prio = 0;
                if (delayIndex == -5.100000e+02) {
                    // Read the marking dependent function from the def file
                    delay = importExpr(markDepDefs.get(i+1), allColorClasses);
                }
                else if (delayIndex < 0) {
                    delay = ratePars[-(int) delayIndex - 1].getUniqueName();
                }
                if (enabDegree == 0) {
                    numServers = "Infinite";
                }
                else if (enabDegree > 0) {
                    if (delayIndex == -5.100000e+02) {
                        // Marking dependent function
                        numServers = "Infinite";
                    }
                    else {
                        numServers = "" + enabDegree;
                    }
                }
                else { // enabDegree < 0
                    throw new RuntimeException("Load dependent transitions are not implemented.");
                }
            }
            else {
                throw new RuntimeException("Unrecognized transition type.");
            }

            double degree;
            switch (orientation) {
                case 0:
                    degree = 2;
                    break; // 90/270 degrees
                case 1:
                    degree = 0;
                    break; // 0/180 degrees
                case 2:
                    degree = 3;
                    break; // 135/315 degrees
                case 3:
                    degree = 1;
                    break; // 45/225 degrees
                default:
                    throw new IllegalStateException("Wrong orientation index.");
            }
            degree *= Math.PI / 4.0;

            String guard = "True";
            if (!colGuard.isEmpty()) {
                Scanner guardScan = new Scanner(colGuard);
                guardScan.next(); // skip X
                guardScan.next(); // skip Y
                guard = importExpr(guardScan.nextLine().trim().replaceAll("^\\[", "").replaceAll("\\]$", ""), allColorClasses);
            }

            Transition trn = new Transition(name, tt, delay, "" + prio,
                                            weight, numServers, guard, degree, pos);
            trn.setSuperPosTags(superPosTags);
            trn.setNodePosition(pos.getX() - (trn.getCenterX() - trn.getX()),
                                pos.getY() - (trn.getCenterY() - trn.getY()));
            LabelDecor tag = trn.getUniqueNameDecor();
            tag.setInternalPos(tagPos.getX() - trn.getCenterX() - 0.5 + tag.getWidth() / 2,
                               tagPos.getY() - trn.getCenterY() - 1.0 + tag.getHeight() / 2);
            LabelDecor labRate = trn.getDelayDecor();
            labRate.setInternalPos(ratePos.getX() - trn.getCenterX(),
                                   ratePos.getY() - trn.getCenterY() + labRate.getHeight() / 2);
            gspn.nodes.add(trn);

            // Read Arcs
            for (int ak = 0; ak < 3; ak++) {
                for (int n = 0; n < numArcs; n++) {
                    int mult = net.nextInt();
                    boolean broken = (mult < 0);
                    Place pl = places[net.nextInt() - 1];
                    int numPoints = net.nextInt();
                    skip_layers(net);
                    String arcDef = net.nextLine().trim(), multExpr;
                    if (!arcDef.isEmpty()) { // colored
                        Scanner arcDefScan = new Scanner(arcDef);
                        arcDefScan.next(); // skip X
                        arcDefScan.next(); // skip Y
                        multExpr = importExpr(arcDefScan.nextLine().trim(), allColorClasses);

                        ColorClass cc = (ColorClass)gspn.getNodeByUniqueName(pl.getColorDomainName());
                        if (cc == null) {
                            log.add("Missing color class '"+pl.getColorDomainName()+
                                    "' of place "+pl.getUniqueName());
                        }
                        else {
                            // Deduce the type of the color variables
                            int numDeducedVars = deduceColorVarsFromExpr(gspn, cc, multExpr, numExtraColDef);
                            numExtraColDef += numDeducedVars;
                        }
                    }
                    else { // neutral
                        multExpr = "" + Math.abs(mult);
                        if (mult == 0) {
                            throw new RuntimeException("A constant arc multiplicity of 0 is not accepted.");
                        }
                    }

                    ArrayList<Point2D> points = new ArrayList<>();
                    points.add(new Point2D.Double(0, 0));
                    for (int pt = 0; pt < numPoints; pt++) {
                        points.add(readPos(net));
                    }
                    points.add(new Point2D.Double(0, 0));

                    if (ak != 1) {
                        // Reverse the order of points[]
                        for (int k = 0; k < points.size() / 2; k++) {
                            Point2D pp = points.get(k);
                            points.set(k, points.get(points.size() - k - 1));
                            points.set(points.size() - k - 1, pp);
                        }
                    }

                    GspnEdge edge = null;
                    switch (ak) {
                        case 0: // input
                            edge = new GspnEdge(pl, 0, trn, 0, points, broken,
                                                GspnEdge.Kind.INPUT, multExpr);
                            break;
                        case 1: // output
                            edge = new GspnEdge(trn, 0, pl, 0, points, broken,
                                                GspnEdge.Kind.OUTPUT, multExpr);
                            break;
                        case 2: // inhibitor
                            edge = new GspnEdge(pl, 0, trn, 0, points, broken,
                                                GspnEdge.Kind.INHIBITOR, multExpr);
                            break;
                    }

                    gspn.edges.add(edge);
                }
                if (ak != 2) {
                    numArcs = net.nextInt();
                }
            }
        }

        net.close();
        def.close();

        if (log.isEmpty()) {
            return null; // Everything went ok
        }
        else {
            String message = "Detected problems while importing a GreatSPN net.\n\n";
            for (String s : log) {
                message += s + "\n";
            }
            return message;
        }
    }


    // Deduce color variable types from an arc expression.
    // Example: given an arc expression: <x, y++>  connected with a place of color domain C=C1*C2,
    // the function deduces that exists a color variable named x of type C1, and
    // another color variable named y of type C2
    // Returns the number of deduced color vars
    private static int deduceColorVarsFromExpr(GspnPage gspn, ColorClass colorDom,
                                               String expr, int numExtraColDef)
    {
        int numDeducedVars = 0;

        while (true) {
            // Extract the tuple part of the expression. For instance, the expr
            //     2<x,y> + 3<x++, z>
            // will extract:    x,y         and:   x++, z
            int startOfParen = expr.indexOf('<');
            if (startOfParen == -1)
                break;
            int endOfParen = expr.indexOf('>', startOfParen);
            if (endOfParen == -1)
                break;

            // Extract the tuple from expr
            String tuple = expr.substring(startOfParen+1, endOfParen);
            expr = expr.substring(endOfParen+1);

            String[] terms = tuple.split(",");
            // We must have one term per color class in domain to continue parsing
            if (terms.length == colorDom.getNumClassesInDomain()) {
                for (int d=0; d<colorDom.getNumClassesInDomain(); d++) {
                    String colorClassName = colorDom.getColorClassName(d);
                    Node ccNode = gspn.getNodeByUniqueName(colorClassName);
                    if (!(ccNode instanceof ColorClass))
                        continue;
                    ColorClass cc = (ColorClass)ccNode;
                    if (!cc.isSimpleClass())
                        continue;
                    // Remove trailing spaces and increment/decrement operators
                    // so that a term like x++ will only keep the color var x.
                    String colVar = terms[d];
                    colVar = colVar.replaceAll("\\+\\+", "");
                    colVar = colVar.replaceAll("--", "");
                    colVar = colVar.trim();

                    // Is the term that remains a valid identifier?
                    if (NetObject.isAlphanumericIdentifier(colVar) &&
                        !cc.testHasStaticSubclassNamed(colVar) &&
                        !cc.testHasColorNamed(gspn, colVar))
                    {
//                        System.out.println("Deduce "+colVar+" of type "+cc.getUniqueName());
                        Node cv = gspn.getNodeByUniqueName(colVar);
                        if (cv == null) { // A color variable named colVar does not exists yet
                            // Create a new color variable.
                            cv = new ColorVar(colVar, new Point2D.Double(20, 3+numExtraColDef+numDeducedVars),
                                              cc.getUniqueName());
                            numDeducedVars++;
                            gspn.nodes.add(cv);
                        }
                    }
                }
            }
        }
        return numDeducedVars;
    }


//    public static void main(String[] args) throws Exception {
//        Util.initApplication(PREF_ROOT_KEY, "/org/unito/mainprefs");
//        LatexProvider.initializeProvider(false);
//        File net = new File("/Users/elvio/Downloads/rete_colorata_errore 14.28.52/master_slave_E_b.net");
//        File def = new File("/Users/elvio/Downloads/rete_colorata_errore 14.28.52/master_slave_E_b.def");
//        GspnPage gspn = new GspnPage();
//
//        importGspn(gspn, net, def);
//    }

}
