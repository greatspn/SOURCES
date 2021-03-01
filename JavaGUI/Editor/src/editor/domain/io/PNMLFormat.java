/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.io;

import common.Tuple;
import editor.domain.Edge;
import editor.domain.LabelDecor;
import editor.domain.NetObject;
import static editor.domain.NetObject.isAlphanumericIdentifier;
import editor.domain.Node;
import editor.domain.elements.ColorClass;
import editor.domain.elements.ColorVar;
import editor.domain.elements.ConstantID;
import editor.domain.elements.GspnEdge;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import editor.domain.elements.TemplateVariable;
import editor.domain.elements.TokenType;
import editor.domain.elements.Transition;
import editor.domain.grammar.EvaluationException;
import editor.domain.grammar.ExpressionLanguage;
import editor.domain.grammar.ParserContext;
import java.awt.geom.Point2D;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Map;
import java.util.Queue;
import java.util.Scanner;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;
import javax.swing.filechooser.FileFilter;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

/**
 *
 * @author elvio
 */
public class PNMLFormat {
    // The file filter
    public static final FileFilter fileFilter 
            = new FileNameExtensionFilter("PNML File (*.pnml)", new String[]{"pnml"});
    
    // Escape a string in XML format
    private static String escapeXml(String input) {
        StringBuilder sb = new StringBuilder();
        
        for (int i=0; i<input.length(); i++) {
            char ch = input.charAt(i);
            switch (ch) {
                case '\"':  sb.append("&quot;");  break;
                case '\'':  sb.append("&apos;");  break;
                case '<':   sb.append("&lt;");    break;
                case '>':   sb.append("&gt;");    break;
                case '&':   sb.append("&amp;");   break;  
                default:
                    sb.append(ch);
            }
        }
        return sb.toString();
    }
    
    private static int scaleCoord(double c) {
        return (int)(c * 10.0);
    }
    
    private static double unscaleCoord(String c) {
        return (Double.valueOf(c) / 10.0);
    }
    
    private static String ensureInt(String expr, String what, ArrayList<String> log) {
        if (NetObject.isInteger(expr)) {
            return expr;
        }
        log.add("Could not export correctly " + what + " with value '" + expr + "'");
        return "1";
    }
    
    // Makes an ID from a string (removed any character other than letters, numbers or _)
    private static String makeId(String text) {
        StringBuilder sb = new StringBuilder();
        for (int i=0; i<text.length(); i++) {
            char ch = text.charAt(i);
            if (Character.isLetterOrDigit(ch) || ch=='_')
                sb.append(ch);
        }
        return sb.toString();
    }
    
    private static void writeTags(PrintWriter pnml, Node n, boolean saveGraphics) {
        if (n.numTags() > 0) {
            pnml.println("\t\t\t\t<toolspecific tool=\"GreatSPN-tags\" version=\"3\">");
            for (int i=0; i<n.numTags(); i++)
                pnml.println("\t\t\t\t\t<tag>"+n.getTag(i)+"</tag>");
            
            if (saveGraphics) {
                int labelX = scaleCoord(n.getSuperPosTagsDecor().getInternalPosX());
                int labelY = scaleCoord(n.getSuperPosTagsDecor().getInternalPosY());
                pnml.println("\t\t\t\t\t<graphics>");
                pnml.println("\t\t\t\t\t\t<offset x=\""+labelX+"\" y=\""+labelY+"\"/>");
                pnml.println("\t\t\t\t\t</graphics>");
            }
            pnml.println("\t\t\t\t</toolspecific>");
        }
    }
    
    public static String exportGspn(GspnPage gspn, File pnmlFile, boolean saveGraphics) throws Exception {
        if (!gspn.isPageCorrect())
            throw new UnsupportedOperationException("GSPN must be correct before exporting.");
        
        ParserContext context = new ParserContext(gspn);
        ArrayList<String> log = new ArrayList<>();
        
        try {
            try {
                gspn.compileParsedInfo(context);
            }
            catch (EvaluationException e) {
                throw new UnsupportedOperationException("PNML exporter does not support "
                        + "parametric color classes. All color classes bounds must be defined.");
            }
            
            PrintWriter pnml = new PrintWriter(new BufferedOutputStream(new FileOutputStream(pnmlFile)));
            
            String netTypeUrl;
            if (gspn.gspnHasColors())
                netTypeUrl = "http://www.pnml.org/version-2009/grammar/symmetricnet";
            else
                netTypeUrl = "http://www.pnml.org/version-2009/grammar/ptnet";
            
            // Write PNML header
            pnml.println("<?xml version=\"1.0\"?>");
            pnml.println("<pnml xmlns=\"http://www.pnml.org/version-2009/grammar/pnml\">");
            pnml.println("\t<!-- Written by GreatSPN Editor. -->");
            pnml.println("\t<net id=\""+makeId(gspn.getPageName())+"\" type=\""+netTypeUrl+"\">");
            pnml.println("\t\t<name>");
            pnml.println("\t\t\t<text>"+escapeXml(gspn.getPageName())+"</text>");
            pnml.println("\t\t</name>");
            
            // Write color declarations: color classes and color variables
            if (gspn.gspnHasColors()) {
                pnml.println("\t\t<declaration>");
                pnml.println("\t\t\t<structure>");
                pnml.println("\t\t\t\t<declarations>");
                pnml.println("\t\t\t\t\t<!-- Declaration of user-defined color classes (sorts) -->");                
                // Dot color class (if any place is neutral)
                for (Node node : gspn.nodes) {
                    if (node instanceof Place) {
                        if (((Place) node).isInNeutralDomain()) {
                            pnml.println("\t\t\t\t\t<namedsort id=\"dot\" name=\"Dot\">");
                            pnml.println("\t\t\t\t\t\t<dot/>");
                            pnml.println("\t\t\t\t\t</namedsort>");
                            break;
                        }
                    }
                }
                // Color classes
                for (Node node : gspn.nodes) {
                    if (node instanceof ColorClass) {
                        ColorClass cc = (ColorClass)node;
                        String ccName = escapeXml(cc.getUniqueName());
                        pnml.println("\t\t\t\t\t<namedsort id=\""+ccName+"\" name=\""+ccName+"\">");
                        if (cc.isSimpleClass()) {
                            if (cc.numSubClasses() == 1 && cc.getSubclass(0).isInterval()) {
                                // Write a <finiteintrange>
                                pnml.println("\t\t\t\t\t\t<finiteintrange "
                                        + "start=\""+cc.getSubclass(0).getStartRangeExpr()+"\" "
                                        + "end=\""+cc.getSubclass(0).getEndRangeExpr()+"\"/>");
                            }
                            else {
                                // Write a <*enumeration>
                                String enumType = cc.isCircular() ? "cyclicenumeration" : "finiteenumeration";

                                pnml.println("\t\t\t\t\t\t<"+enumType+">");
                                int numColors = cc.numColors();
                                for (int i=0; i<numColors; i++) {
                                    String clrName = escapeXml(cc.getColorName(i));
                                    pnml.println("\t\t\t\t\t\t\t<feconstant id=\""+clrName+"\" name=\""+clrName+"\"/>");
                                }
                                pnml.println("\t\t\t\t\t\t</"+enumType+">");
                            }
                        }
                        else { // Product color class
                            pnml.println("\t\t\t\t\t\t<productsort>");
                            for (int i=0; i<cc.getNumClassesInDomain(); i++) {
                                pnml.println("\t\t\t\t\t\t\t<usersort declaration=\""+
                                        escapeXml(cc.getColorClassName(i))+"\"/>");
                            }
                            pnml.println("\t\t\t\t\t\t</productsort>");
                        }
                        pnml.println("\t\t\t\t\t</namedsort>");
                    }
                }
                // Color variables
                pnml.println("\t\t\t\t\t<!-- Declaration of user-defined color variables -->");                
                for (Node node : gspn.nodes) {
                    if (node instanceof ColorVar) {
                        ColorVar cvar = (ColorVar)node;
                        String cvarName = escapeXml(cvar.getUniqueName());
                        pnml.println("\t\t\t\t\t<variabledecl id=\""+cvarName+"\" name=\""+cvarName+"\">");
                        pnml.println("\t\t\t\t\t\t<usersort declaration=\""+
                                escapeXml(cvar.getDomainExpr().getExpr())+"\"/>");
                        pnml.println("\t\t\t\t\t</variabledecl>");
                    }
                }
                pnml.println("\t\t\t\t</declarations>");
                pnml.println("\t\t\t</structure>");
                pnml.println("\t\t</declaration>");
            }
            
            // Write page header
            pnml.println("\t\t<page id=\"page0\">");
            pnml.println("\t\t\t<name>");
            pnml.println("\t\t\t\t<text>DefaultPage</text>");
            pnml.println("\t\t\t</name>");

            for (Node node : gspn.nodes) {
                if (node instanceof ConstantID || node instanceof TemplateVariable) {
                    log.add("Parameter "+node.getUniqueName()+" is not convertible in PNML format.");
                }
            }

            // Write Places
            pnml.println("\t\t\t<!-- List of places -->");                
            for (Node node : gspn.nodes) {
                if (node instanceof Place) {
                    Place plc = (Place) node;
                    if (plc.isContinuous())
                        log.add("Continuous place "+plc.getUniqueName()+" is not convertible in PNML format.");

                    String plcName = escapeXml(plc.getUniqueName());
                    int labelX = scaleCoord(plc.getUniqueNameDecor().getInternalPosX());
                    int labelY = scaleCoord(plc.getUniqueNameDecor().getInternalPosY());

                    pnml.println("\t\t\t<place id=\""+plcName+"\">");
                    pnml.println("\t\t\t\t<name>");
                    if (saveGraphics) {
                        pnml.println("\t\t\t\t\t<graphics>");
                        pnml.println("\t\t\t\t\t\t<offset x=\""+labelX+"\" y=\""+labelY+"\"/>");
                        pnml.println("\t\t\t\t\t</graphics>");
                    }
                    pnml.println("\t\t\t\t\t<text>"+plcName+"</text>");
                    pnml.println("\t\t\t\t</name>");
                    if (saveGraphics) {
                        pnml.println("\t\t\t\t<graphics>");
                        pnml.println("\t\t\t\t\t<position x=\""+scaleCoord(plc.getX())+
                                "\" y=\""+scaleCoord(plc.getY())+"\"/>");
                        pnml.println("\t\t\t\t</graphics>");
                    }
                    
                    if (plc.isInColorDomain()) {
                        pnml.println("\t\t\t\t<type>");
                        pnml.println("\t\t\t\t\t<text>"+escapeXml(plc.getColorDomainName())+"</text>");
                        pnml.println("\t\t\t\t\t<structure>");
                        pnml.println("\t\t\t\t\t\t<usersort declaration=\""+escapeXml(plc.getColorDomainName())+"\"/>");
                        pnml.println("\t\t\t\t\t</structure>");
                        pnml.println("\t\t\t\t</type>");
                    }

                    String initMarkText = plc.getInitMarkingEditable().getValue().toString();
                    if (!initMarkText.isEmpty()) {
                        if (gspn.gspnHasColors()) {
                            String initMarkExpr = plc.convertInitMarkingLang(
                                    context, ExpressionLanguage.PNML);
                            if (plc.isInNeutralDomain())
                                initMarkExpr = "<numberof><subterm>"+initMarkExpr+
                                        "</subterm><subterm><dotconstant/></subterm></numberof>";

                            pnml.println("\t\t\t\t<hlinitialMarking>");
                            pnml.println("\t\t\t\t\t<text>"+escapeXml(initMarkText)+"</text>");
                            pnml.println("\t\t\t\t\t<structure>");
                            pnml.println("\t\t\t\t\t\t"+initMarkExpr);
                            pnml.println("\t\t\t\t\t</structure>");
                            pnml.println("\t\t\t\t</hlinitialMarking>");
                        }
                        else {
                            String initMark = ensureInt(plc.getInitMarkingEditable().getValue().toString(), 
                                                        "initial marking", log);
                            pnml.println("\t\t\t\t<initialMarking>");
                            pnml.println("\t\t\t\t\t<text>"+initMark+"</text>");
                            pnml.println("\t\t\t\t</initialMarking>");
                        }
                    }
                    writeTags(pnml, plc, saveGraphics);
                    pnml.println("\t\t\t</place>");
                }
            }

            // Write transitions
            pnml.println("\t\t\t<!-- List of transitions -->");                
            for (Node node : gspn.nodes) {
                if (node instanceof Transition) {
                    Transition trn = (Transition) node;
                    if (trn.isContinuous())
                        log.add("Continuous transition "+trn.getUniqueName()+" is not convertible in PNML format.");

                    String trnName = escapeXml(trn.getUniqueName());
                    int labelX = scaleCoord(trn.getUniqueNameDecor().getInternalPosX());
                    int labelY = scaleCoord(trn.getUniqueNameDecor().getInternalPosY());

                    pnml.println("\t\t\t<transition id=\""+trnName+"\">");
                    pnml.println("\t\t\t\t<name>");
                    if (saveGraphics) {
                        pnml.println("\t\t\t\t\t<graphics>");
                        pnml.println("\t\t\t\t\t\t<offset x=\""+labelX+"\" y=\""+labelY+"\"/>");
                        pnml.println("\t\t\t\t\t</graphics>");
                    }
                    pnml.println("\t\t\t\t\t<text>"+trnName+"</text>");
                    pnml.println("\t\t\t\t</name>");
                    if (saveGraphics) {
                        pnml.println("\t\t\t\t<graphics>");
                        pnml.println("\t\t\t\t\t<position x=\""+scaleCoord(trn.getX())+
                                "\" y=\""+scaleCoord(trn.getY())+"\"/>");
                        pnml.println("\t\t\t\t</graphics>");
                    }
                    
                    if (trn.hasGuard() && !trn.getGuard().equals("True")) {
                        pnml.println("\t\t\t\t<condition>");
                        pnml.println("\t\t\t\t\t<text>"+escapeXml(trn.getGuard())+"</text>");
                        pnml.println("\t\t\t\t\t<structure>");
                        pnml.println("\t\t\t\t\t\t"+trn.convertGuardLang(context, null, ExpressionLanguage.PNML));
                        pnml.println("\t\t\t\t\t</structure>");
                        pnml.println("\t\t\t\t</condition>");
                    }
                    
                    writeTags(pnml, trn, saveGraphics);
                    pnml.println("\t\t\t</transition>");
                }
            }

            // Write arcs
            int arcId = 1;
            pnml.println("\t\t\t<!-- List of arcs -->");                
            for (Edge edge : gspn.edges) {
                if (edge instanceof GspnEdge) {
                    GspnEdge arc = (GspnEdge)edge;
                    String from = escapeXml(arc.getTailNode().getUniqueName());
                    String to = escapeXml(arc.getHeadNode().getUniqueName());

                    pnml.println("\t\t\t<arc id=\"id"+(arcId++)+"\" "
                            + "source=\""+from+"\" target=\""+to+"\">");
                    
                    if (arc.numPoints() > 2 && saveGraphics) {
                        pnml.println("\t\t\t\t<graphics>");
                        for (int i=1; i<arc.numPoints()-1; i++) {
                            Point2D pt = arc.points.get(i);
                            pnml.println("\t\t\t\t\t<position x=\""+scaleCoord(pt.getX())+
                                    "\" y=\""+scaleCoord(pt.getY())+"\"/>");
                        }
                        pnml.println("\t\t\t\t</graphics>");
                    }
                    
                    if (gspn.gspnHasColors()) {
                        String arcMult = arc.convertMultiplicityLang(context, ExpressionLanguage.PNML);
                        if (arc.getConnectedPlace().isInNeutralDomain())
                                arcMult = "<numberof><subterm>"+arcMult+
                                        "</subterm><subterm><dotconstant/></subterm></numberof>";
                        
                        pnml.println("\t\t\t\t<hlinscription>");
                        pnml.println("\t\t\t\t\t<text>"+escapeXml(arc.getMultiplicity())+"</text>");
                        pnml.println("\t\t\t\t\t<structure>");
                        pnml.println("\t\t\t\t\t\t"+arcMult);
                        pnml.println("\t\t\t\t\t</structure>");
                        pnml.println("\t\t\t\t</hlinscription>");
                    }
                    else {
                        if (!arc.getMultiplicity().equals("1")) {
                            String intMult = ensureInt(arc.getMultiplicity(), "arc multiplicity", log);
                            pnml.println("\t\t\t\t<inscription>");
                            pnml.println("\t\t\t\t\t<text>"+intMult+"</text>");
                            pnml.println("\t\t\t\t</inscription>");
                        }
                    }
                    if (arc.getEdgeKind() == GspnEdge.Kind.INHIBITOR)
                        pnml.println("\t\t\t\t<type value=\"inhibitor\"/>");
                    pnml.println("\t\t\t</arc>");
                }
            }

            // Write PNML footer
            pnml.println("\t\t</page>");
            pnml.println("\t</net>");
            pnml.println("</pnml>");
            pnml.close();
        }
        finally {
            gspn.compileParsedInfo(null);
        }

        if (log.isEmpty()) {
            return null; // Everything went ok
        }
        else {
            String message = "Detected problems in the exported PNML net.\n\n";
            for (String s : log) {
                message += s + "\n";
            }
            return message;
        }
    }
    
    
//    // Unescape a string from XML format
//    private static String unescapeXml(String input) {
//        StringBuilder sb = new StringBuilder();
//        
//        for (int i=0; i<input.length(); i++) {
//            char ch = input.charAt(i);
//            switch (ch) {
//                case '&':
//                    if (input.startsWith("quot;", i+1)) {
//                        sb.append("&");
//                        i += 6;
//                    }
//                    else if (input.startsWith("apos;", i+1)) {
//                        sb.append("\'");
//                        i += 6;
//                    }
//                    else if (input.startsWith("lt;", i+1)) {
//                        sb.append("<");
//                        i += 4;
//                    }
//                    else if (input.startsWith("gt;", i+1)) {
//                        sb.append(">");
//                        i += 4;
//                    }
//                    else if (input.startsWith("amp;", i+1)) {
//                        sb.append("<");
//                        i += 5;
//                    }
//                    else throw new UnsupportedOperationException("Cannot unescape XML string \""+input+"\"");
//                default:
//                    sb.append(ch);
//            }
//        }
//        return sb.toString();
//    }
//    
    //==========================================================================
    
    static private Element getSingleSubElementByTagName(Element parent, String name) {
        for (org.w3c.dom.Node child = parent.getFirstChild(); 
             child != null; child = child.getNextSibling()) 
        {
            if (child instanceof Element && ((Element)child).getTagName().equals(name))
                return (Element)child;
        }
        return null;
    }
    
    private static boolean extractGraphicPosition(Element elem, Point2D pos, String positionTag) {
        Element gfxElem = getSingleSubElementByTagName(elem, "graphics");
        if (gfxElem != null) {
            Element offsetElem = getSingleSubElementByTagName(gfxElem, positionTag);
            if (offsetElem != null) {
                pos.setLocation(unscaleCoord(offsetElem.getAttribute("x")), 
                                unscaleCoord(offsetElem.getAttribute("y")));
                return true;
            }
        }
        return false;
    }
    
    private static String extractGraphicsLabelAndPosition(Element elem, Point2D pos, Point2D labelOffset) {
        if (labelOffset != null)
            labelOffset.setLocation(0, 0);

        if (pos != null) {
            pos.setLocation(0, 0);
            extractGraphicPosition(elem, pos, "position");
        }
        
        Element nameElem = getSingleSubElementByTagName(elem, "name");
        if (nameElem != null) {
            if (labelOffset != null)
                extractGraphicPosition(nameElem, labelOffset, "offset");
            // Read the id
            Element textElem = getSingleSubElementByTagName(nameElem, "text");
            if (textElem != null) {
                return textElem.getTextContent();
            }
        }
        return null;
    }
    
    private static void extractTags(Element elem, Node node, Point2D labelOffset) {
        String tags = "";
        NodeList subElems = elem.getElementsByTagName("toolspecific");
        for (int i=0; i<subElems.getLength(); i++) {
            Element toolSpec = (Element)subElems.item(i);
            if (toolSpec.hasAttribute("tool") && toolSpec.getAttribute("tool").equals("GreatSPN-tags")) {
                // read tags
                NodeList allTags = elem.getElementsByTagName("tag");
                for (int j=0; j<allTags.getLength(); j++) {
                    Element tag = (Element)allTags.item(j);
                    if (tag.getTextContent() != null && !tag.getTextContent().isEmpty())
                        tags += (tags.isEmpty()?"":"|") + tag.getTextContent();
                }
                if (extractGraphicPosition(toolSpec, labelOffset, "offset"))
                    node.getSuperPosTagsDecor().setInternalPos(labelOffset.getX(), labelOffset.getY());
            }
        }
        node.setSuperPosTags(tags);
    }
    
    // convert PNML identifiers into PNML names
    private static String convertId2Name(String id, Map<String, String> id2name) {
        if (id2name.containsKey(id))
            return id2name.get(id);
        return id;
    }
    
    // Save an id -> name mapping, ensuring sanity and uniqueness of the name
    private static String saveIdName(String id, String _nameOpt, 
                                     Map<String, String> id2name, 
                                     Set<String> knownNames)
    {
        // Use the id if the name is not present
        String name = (_nameOpt == null) ? id : _nameOpt;
        
        // Make the name a valid alphanumeric identifier
        if (!isAlphanumericIdentifier(name)) {
            StringBuilder sb = new StringBuilder();
            for (int i=0; i<name.length(); i++) {
                char ch = name.charAt(i);
                if (Character.isLetterOrDigit(ch) || ch=='_')
                    sb.append(ch);
                else //if (ch==' ')
                    sb.append('_');
            }
            name = sb.toString();
        }
        
        // Ensure uniqueness of this name
        String uniqName = name;
        int i=0;
        while (knownNames.contains(uniqName)) {
            uniqName = name + "__" + i;
        }
//        if (!uniqName.equals(name)) {
//            System.out.println("Name conflict: "+name+" -> "+uniqName);
//        }
        
        id2name.put(id, uniqName);
        knownNames.add(uniqName);
        return uniqName;
    }
   
    // Wrap into parenthesis the expression if the outside expr has a greater precedence
    
    private final static int PREC_EXPR = 0;
    private final static int PREC_ADD = 1;
    private final static int PREC_MULT = 2;
    private final static int PREC_IMPLY = 3;
    private final static int PREC_OR = 4;
    private final static int PREC_AND = 5;
    private final static int PREC_LTGT = 6;
    private final static int PREC_EQ = 7;
    
    
    // Transform PNML/XML expressions into GreatSPN expression
    private static String readExpr(Element elem, int outPrecedence, Map<String, String> id2name) {
        
        String op = null, expr = "";
        int precedence = outPrecedence;

        // Binary operators
        switch (elem.getTagName()) {
            case "add":                op = " + ";    precedence = PREC_ADD;    break;
            case "subtract":           op = " - ";    precedence = PREC_ADD;    break;
            case "and":                op = " && ";   precedence = PREC_AND;    break;
            case "or":                 op = " || ";   precedence = PREC_OR;     break;
            case "imply":              op = " -> ";   precedence = PREC_IMPLY;  break;
            case "equality":           op = " == ";   precedence = PREC_EQ;     break;
            case "inequality":         op = " != ";   precedence = PREC_EQ;     break;
            case "greaterthan":        op = " > ";    precedence = PREC_LTGT;   break;
            case "greaterthanorequal": op = " >= ";   precedence = PREC_LTGT;   break;
            case "lessthan":           op = " < ";    precedence = PREC_LTGT;   break;
            case "lessthanorequal":    op = " <= ";   precedence = PREC_LTGT;   break;
        }
        if (op != null) {
//            System.out.println("op = "+op+"  prec="+precedence+"  outPrec="+outPrecedence);
            int count = 0;
            for (org.w3c.dom.Node subterm = elem.getFirstChild(); 
                 subterm != null; subterm = subterm.getNextSibling())
            {
                if (!(subterm instanceof Element))
                    continue;
                if (count++ > 0)
                    expr += op;
                expr += readExpr((Element)subterm, precedence, id2name);
            }
            if (precedence < outPrecedence)
                return "(" + expr + ")";
            return expr;
        }
        else if (elem.getTagName().equals("tuple")) {
            int count = 0;
            for (org.w3c.dom.Node subterm = elem.getFirstChild(); 
                 subterm != null; subterm = subterm.getNextSibling())
            {
                if (!(subterm instanceof Element))
                    continue;
                String term = readExpr((Element)subterm, PREC_EXPR, id2name);
                if (count == 0 && term.equals("<dotconstant>"))
                    return "";
                expr += ((count == 0) ? "<": ", ") + term;
                count++;
            }
            return expr + ">";
        }
        else if (elem.getTagName().equals("numberof"))
        {
            // numberof is a tuple with a multiplicity, like: N <x,y>
            int numTupleTerms = 0;
            String mult = null;
            StringBuilder tuples = new StringBuilder();
            for (org.w3c.dom.Node subterm = elem.getFirstChild(); 
                 subterm != null; subterm = subterm.getNextSibling())
            {
                if (!(subterm instanceof Element))
                    continue;
                Element tupleTerm = getSingleSubElementByTagName((Element)subterm, "tuple");
                if (mult == null && tupleTerm == null) { // multiplicity
                    mult = readExpr((Element)subterm, PREC_EXPR, id2name);
                }
                else { // tuple terms
                    String term = readExpr((Element)subterm, PREC_EXPR, id2name);
                    if (term.equals("<dotconstant>"))
                        return mult;
                    // Quick fix: some terms are not wrapped in a <tuple>. Wrap them.
                    if (!term.startsWith("<"))
                        term = "<" + term + ">";
                    if (numTupleTerms > 0)
                        tuples.append(" + ");
                    if (mult!=null && !mult.equals("1"))
                        tuples.append(mult);
                    tuples.append(term);
                    numTupleTerms++;
                }
            }
            if (numTupleTerms < 1)
                throw new UnsupportedOperationException("Incomplete <numberof> tag.");
            if (numTupleTerms > 2) // more than one tuple
                return "(" + tuples.toString() + ")";
            return tuples.toString();
        }
        // Useless term - recursive descend
        else if (elem.getTagName().equals("structure") || 
                 elem.getTagName().equals("subterm")) 
        {
            for (org.w3c.dom.Node child0 = elem.getFirstChild();
                 child0 != null; child0 = child0.getNextSibling())
            {
                if (child0 instanceof Element)
                    return readExpr((Element)child0, precedence, id2name);
            }
            return "<<missing expression in structure tag>>";
        }
        else if (elem.getTagName().equals("successor") || 
                 elem.getTagName().equals("predecessor")) 
        {
            Element subterm = getSingleSubElementByTagName(elem, "subterm");
            if (subterm == null)
                throw new UnsupportedOperationException("Missing subterms in unary operator");
            op = (elem.getTagName().equals("successor")) ? "++" : "--";
            return readExpr(subterm, PREC_EXPR, id2name) + op;
        }
        else if (elem.getTagName().equals("numberconstant")) {
            return elem.getAttribute("value");
        }
        else if (elem.getTagName().equals("booleanconstant")) {
            return elem.getAttribute("value").equals("true") ? "True" : "False";
        }
        else if (elem.getTagName().equals("finiteintrangeconstant")) {
            Element firDecl = getSingleSubElementByTagName(elem,"finiteintrange");
            if (firDecl == null)
                throw new UnsupportedOperationException("Missing <finiteintrange> inside <finiteintrangeconstant>");
            String start = firDecl.getAttribute("start");
            String end = firDecl.getAttribute("end");
            String declName = "finiteintrange:"+start+".."+end;
            if (!id2name.containsKey(declName))
                throw new UnsupportedOperationException("A <finiteintrange> inside a "
                        + "<finiteintrangeconstant> term is not declared");
            return convertId2Name(declName, id2name) + elem.getAttribute("value");
        }
        else if (elem.getTagName().equals("variable")) { // a ColorVar element
            return convertId2Name(elem.getAttribute("refvariable"), id2name);
        }
        else if (elem.getTagName().equals("useroperator")) { // a single color element
            return convertId2Name(elem.getAttribute("declaration"), id2name);
        }
        else if (elem.getTagName().equals("dotconstant")) { // neutral color element
            return "<dotconstant>";
        }
        else if (elem.getTagName().equals("all")) { // special 'All' color term
            return "All";
        }
        else throw new UnsupportedOperationException("Unknown tag <"+elem.getTagName()+"> in PNML expression.");
    }
    
    // Parse a PNML expression tree, with exception handling
    private static String parseExpr(Element elem, Map<String, String> id2name, ArrayList<String> log, 
                                    String whatIs, String nodeName) 
    {
        try {
            String expr = readExpr(elem, PREC_EXPR, id2name);
            // Sanitize some special cases
            if (expr.equals("All"))
                return "<All>";
//            System.out.println(whatIs+" of "+nodeName+" is: "+expr);
            return expr;
        }
        catch (Exception e) {
            log.add("Cannot retrieve "+whatIs+" of "+nodeName+": "+e.getMessage());
            return "???";
        }
    }
    
    
    public static class NuPNUnit {
        public String unitName;
        public String numUnits; // # of units, initialized only for the root unit
        public ArrayList<NuPNUnit> subunits = new ArrayList<>();
        public ArrayList<String> places = new ArrayList<>();

        public NuPNUnit(String unitName) {
            this.unitName = unitName;
        }
    };
        
    
    // Loads a PNML file into the editor.
    // Supports 2009 version of PNML, with both P/T and symmetric nets.
    public static String importPNML(GspnPage gspn, File pnmlFile, 
                                    Map<String, String> id2name,
                                    NuPNUnit[] rootUnit /*optional*/) throws Exception 
    {
        gspn.viewProfile.viewRatesDelays = false;
        ArrayList<String> log = new ArrayList<>();
        
        // PNML associates o each entity a id and an optional name.
        // We only use names, so let's convert ids into names. Also, finite int
        // ranges get a name in this table
        id2name.clear();
        Set<String> knownNames = new TreeSet<>();
        
        DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
	DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
	Document doc = dBuilder.parse(pnmlFile);
        final XmlExchangeDirection exDir = new XmlExchangeDirection(true, doc);
        
	//optional, but recommended
	doc.getDocumentElement().normalize();
        Point2D.Double pos = new Point2D.Double();
        Point2D.Double labelOffset = new Point2D.Double();

        if (!doc.getDocumentElement().getNodeName().equals("pnml"))
            throw new UnsupportedOperationException("Cannot find PNML XML root.");
        Element pnmlRootElem = doc.getDocumentElement();
        
        Element netRootElem = getSingleSubElementByTagName(pnmlRootElem, "net");
        if (netRootElem == null)
            throw new UnsupportedOperationException("Missing <net> tag in PNML file.");
        String netType = netRootElem.getAttribute("type");
        if (!netType.equals("http://www.pnml.org/version-2009/grammar/ptnet") &&
            !netType.equals("http://www.pnml.org/version-2009/grammar/symmetricnet") &&
            !netType.equals("http://www.pnml.org/version-2009/grammar/highlevelnet"))
            log.add("Unknown PNML version: "+netType);
        
        // Retrieve net name
        String netName = netRootElem.getAttribute("id");
        Element netNameElem = getSingleSubElementByTagName(netRootElem, "name");
        if (netNameElem != null){
            Element textElem = getSingleSubElementByTagName(netNameElem, "text");
            if (textElem != null) {
                netName = textElem.getTextContent();
//                netName = sanitizeName(netName, log, "Net");
            }
        }
        gspn.setPageName(netName);
        
        // Read declarations
        int numExtraColDef=0;
        String dotSortName = null;
        NodeList allDeclarations = netRootElem.getElementsByTagName("declaration");
        for (int i=0; i<allDeclarations.getLength(); i++) {
            Element declStruct = getSingleSubElementByTagName((Element)allDeclarations.item(i), "structure");
            if (declStruct != null) {
                
                // Read partitions first
                Map<String, ArrayList<Tuple<String, ArrayList<String>>>> staticSubClasses;
                staticSubClasses = new TreeMap<>();
                NodeList allPartitions = declStruct.getElementsByTagName("partition");
                for (int j=0; j<allPartitions.getLength(); j++) {
                    Element partition = (Element)allPartitions.item(j);
                    Element usDecl = getSingleSubElementByTagName(partition, "usersort");
                    if (usDecl != null) {
                        String clrClassId = usDecl.getAttribute("declaration");
//                        System.out.println("partition "+clrClassId);
                        NodeList partitionElems = partition.getElementsByTagName("partitionelement");
                        ArrayList<Tuple<String, ArrayList<String>>> subclasses = new ArrayList<>();
                        for (int el=0; el<partitionElems.getLength(); el++) {
                            Element partitionElem = (Element)partitionElems.item(el);
                            String partId = partitionElem.getAttribute("id");
                            String partName = saveIdName(partId, partitionElem.getAttribute("name"), 
                                                         id2name, knownNames);
//                            System.out.println("  partitionElem "+partId);
                            ArrayList<String> colors = new ArrayList<>();
                            NodeList colorList = partitionElem.getElementsByTagName("useroperator");
                            for (int c=0; c<colorList.getLength(); c++) {
                                Element colorDecl = (Element)colorList.item(c);
                                String colorId = colorDecl.getAttribute("declaration");
//                                System.out.println("    colorId "+colorId);
                                colors.add(colorId);
                            }
                            Tuple<String, ArrayList<String>> subclass = new Tuple<>(partId, colors);
                            subclasses.add(subclass);
                        }
                        staticSubClasses.put(clrClassId, subclasses);
                    }
                }
                
                // Read named sorts (i.e. color classes)
                NodeList allNamedSorts = declStruct.getElementsByTagName("namedsort");
                for (int j=0; j<allNamedSorts.getLength(); j++) {
                    Element namedSort = (Element)allNamedSorts.item(j);
                    String clrClassId = namedSort.getAttribute("id");
                    String clrClassName = saveIdName(clrClassId, namedSort.getAttribute("name"), 
                                                     id2name, knownNames);
//                    if (namedSort.hasAttribute("name")) {
//                        clrClassName = namedSort.getAttribute("name");
//                        saveIdName(clrClassId, clrClassName, id2name, knownNames);
////                        clrClassName = sanitizeName(clrClassName, knownNames);
////                        id2name.put(clrClassId, clrClassName);
////                        knownNames.add(clrClassName);
//                    }
//                    if (namedSort.hasAttribute("id") && namedSort.hasAttribute("name"))
//                        id2name.put(namedSort.getAttribute("id"), namedSort.getAttribute("name"));
                    
                    // Is this a finite enumeration, a cyclic enum or a product?
                    Element feDecl = getSingleSubElementByTagName(namedSort, "finiteenumeration");
                    Element ceDecl = getSingleSubElementByTagName(namedSort, "cyclicenumeration");
                    Element firDecl = getSingleSubElementByTagName(namedSort, "finiteintrange");
                    Element prDecl = getSingleSubElementByTagName(namedSort, "productsort");
                    Element dotDecl = getSingleSubElementByTagName(namedSort, "dot");
                    
                    if (feDecl != null || ceDecl != null) {
                        Element decl = (feDecl != null) ? feDecl : ceDecl;
                        // Read the <feconstants>
                        NodeList allColors = decl.getElementsByTagName("feconstant");
                        ArrayList<String> allClrNames = new ArrayList<>();
                        for (int k=0; k<allColors.getLength(); k++) {
                            Element color = (Element)allColors.item(k);
                            String clrId = color.getAttribute("id");
                            String clrName0 = color.getAttribute("name");
                            if (clrName0 != null && Character.isDigit(clrName0.charAt(0)))
                                clrName0 = clrClassName + clrName0;
                            String clrName = saveIdName(clrId, clrName0, id2name, knownNames);
                            allClrNames.add(clrName);
                        }
                        // generate color definition
                        boolean makeSimpleDef = true;
                        String def = (feDecl != null) ? "enum" : "circular";
                        if (staticSubClasses.containsKey(clrClassId)) {
                            ArrayList<Tuple<String, ArrayList<String>>> subclasses = staticSubClasses.get(clrClassId);
                            int numClrs = 0;
                            for (Tuple<String, ArrayList<String>> t : subclasses)
                                numClrs += t.y.size();
                            if (numClrs != allClrNames.size()) {
                                log.add("Only fully partitioned color classes are supported. "
                                        + "Cannot import definition of "+clrClassName);
                            }
                            else {
                                for (int s=0; s<subclasses.size(); s++) {
                                    def += (s==0 ? "" : " + ") ;
                                    ArrayList<String> clrsInSubclass = subclasses.get(s).y;
                                    for (int k=0; k<clrsInSubclass.size(); k++) {
                                        String clrName = convertId2Name(clrsInSubclass.get(k), id2name);
                                        def += (k==0 ? " {" : ", ") + clrName;
                                    }
                                    String subclassName = convertId2Name(subclasses.get(s).x, id2name);
                                    def += "} is " + subclassName;
                                }
                                makeSimpleDef = false;
                            }
                        }
                        if (makeSimpleDef) {
                            for (int k=0; k<allClrNames.size(); k++)
                                def += (k==0 ? " {" : ", ") + allClrNames.get(k);
                            def += "}";
                        }
//                        System.out.println(clrClassName+" = "+def);
                        
                        ColorClass cc = new ColorClass(clrClassName, new Point2D.Double(20, 3+numExtraColDef++), def);
                        gspn.nodes.add(cc);
                    }
                    else if (firDecl != null) { // colors are a range of integers
                        String start = firDecl.getAttribute("start");
                        String end = firDecl.getAttribute("end");
                        String def = "c"+clrClassName+"{"+start+" .. "+end+"}";
                        ColorClass cc = new ColorClass(clrClassName, new Point2D.Double(20, 3+numExtraColDef++), def);
                        id2name.put("finiteintrange:"+start+".."+end, "c"+clrClassName);
                        knownNames.add("c"+clrClassName);
                        gspn.nodes.add(cc);
                    }
                    else if (prDecl != null) {
                        String def = "";
                        NodeList allTerms = prDecl.getElementsByTagName("usersort");
                        for (int k=0; k<allTerms.getLength(); k++) {
                            String ccName = ((Element)allTerms.item(k)).getAttribute("declaration");
                            def += (k==0 ? "" : " * ") + convertId2Name(ccName, id2name);
                        }
                        ColorClass cc = new ColorClass(clrClassName, new Point2D.Double(20, 3+numExtraColDef++), def);
                        gspn.nodes.add(cc);
                    }
                    else if (dotDecl != null) { // We have the dot class
                        dotSortName = clrClassName;
                    }
                    else {
                        log.add("Unknown type of namedsort with id="+namedSort.getAttribute("id"));
                    }
                }
                
                // Read variable declarations (i.e. color variables)
                NodeList allVarDecls = declStruct.getElementsByTagName("variabledecl");
                for (int j=0; j<allVarDecls.getLength(); j++) {
                    Element varDecl = (Element)allVarDecls.item(j);
                    String clrVarId = varDecl.getAttribute("id");
                    String clrVarName = saveIdName(clrVarId, varDecl.getAttribute("name"), id2name, knownNames);;
                    // Get the color domain of this variable
                    String colDomain = "??";
                    Element userSortElem = getSingleSubElementByTagName(varDecl, "usersort");
                    if (userSortElem != null) {
                        colDomain = convertId2Name(userSortElem.getAttribute("declaration"), id2name);
                    }
                    
                    ColorVar cvar = new ColorVar(clrVarName, new Point2D.Double(20, 3+numExtraColDef++), colDomain);
                    gspn.nodes.add(cvar);
                }
            }
        }
        
        // Place/transition map for re-linking the edges
        Map<String, Place> places = new TreeMap<>();
        Map<String, Transition> transitions = new TreeMap<>();
        
        
        // Read all the pages together
        NodeList pageRootList = pnmlRootElem.getElementsByTagName("page");
        for (int pageNum=0; pageNum<pageRootList.getLength(); pageNum++) {
            if (!(pageRootList.item(pageNum) instanceof Element))
                continue;
            Element pageElem = (Element)pageRootList.item(pageNum);
            
            // Prepare id -> name translation map for this page
            for (org.w3c.dom.Node child = pageElem.getFirstChild(); 
                    child != null; child = child.getNextSibling())
            {
                if (child instanceof Element && ((Element) child).hasAttribute("id")) {
                    String name = extractGraphicsLabelAndPosition((Element) child, null, null);
                    if (name != null)
                        id2name.put(((Element) child).getAttribute("id"), name);
                }
            }
            
            // Read places
            NodeList placeList = pageElem.getElementsByTagName("place");
            for (int i=0; i<placeList.getLength(); i++) {
                Element plcElem = (Element)placeList.item(i);
     
                String id = plcElem.getAttribute("id");
                String name = extractGraphicsLabelAndPosition(plcElem, pos, labelOffset);
                name = saveIdName(id, name, id2name, knownNames);
                
                // Read color domain
                String colorDomain = "";
                Element typeElem = getSingleSubElementByTagName(plcElem, "type");
                if (typeElem != null) {
                    Element structElem = getSingleSubElementByTagName(typeElem, "structure");
                    if (structElem != null) {
                        Element usersortElem = getSingleSubElementByTagName(structElem, "usersort");
                        if (usersortElem != null)
                            colorDomain = convertId2Name(usersortElem.getAttribute("declaration"), id2name);
                        if (colorDomain.equals(dotSortName))
                            colorDomain = "";
                    }
                }
                
                // Read P/T initial marking
                String initMarkExpr = "0";
                Element initMarkPTElem = getSingleSubElementByTagName(plcElem, "initialMarking");
                if (initMarkPTElem != null) {
                    Element textInitMarkElem = getSingleSubElementByTagName(initMarkPTElem, "text");
                    if (textInitMarkElem != null)
                        initMarkExpr = textInitMarkElem.getTextContent();
                }
                // Read colored initial marking
                Element initMarkClrElem = getSingleSubElementByTagName(plcElem, "hlinitialMarking");
                if (initMarkClrElem != null) {
                    Element structElem = getSingleSubElementByTagName(initMarkClrElem, "structure");
                    if (structElem != null) {
                        // convert XML tree into PNPRO/GreatSPN expression
                        initMarkExpr = parseExpr(structElem, id2name, log, "initial marking", name);
//                        System.out.println(id+" "+initMarkExpr);
                    }
                }
                                
                Place plc = new Place(name, initMarkExpr, TokenType.DISCRETE, colorDomain, "", pos);
                extractTags(plcElem, plc, labelOffset);
                LabelDecor lab = plc.getUniqueNameDecor();
                lab.setInternalPos(labelOffset.x, labelOffset.y);
                gspn.nodes.add(plc);
                places.put(plc.getUniqueName(), plc);
            }
            
            // Read transitions
            NodeList trnList = pageElem.getElementsByTagName("transition");
            for (int i=0; i<trnList.getLength(); i++) {
                Element trnElem = (Element)trnList.item(i);
     
                String id = trnElem.getAttribute("id");
                String name = extractGraphicsLabelAndPosition(trnElem, pos, labelOffset);
                name = saveIdName(id, name, id2name, knownNames);;
                
                String guard = "True";
                Element conditionElem = getSingleSubElementByTagName(trnElem, "condition");
                if (conditionElem != null) {
                    Element structElem = getSingleSubElementByTagName(conditionElem, "structure");
                    if (structElem != null) {
                        guard = parseExpr(structElem, id2name, log, "transition guard", name);
                    }
                }
                
                Transition trn = new Transition(name, Transition.Type.EXP, "1.0", "0",
                                                "1.0", "Infinite", guard, 0.0, pos);
                extractTags(trnElem, trn, labelOffset);
                LabelDecor lab = trn.getUniqueNameDecor();
                lab.setInternalPos(labelOffset.x, labelOffset.y);
                gspn.nodes.add(trn);
                transitions.put(trn.getUniqueName(), trn);
            }
            
            // Read arcs
            NodeList arcList = pageElem.getElementsByTagName("arc");
            for (int i=0; i<arcList.getLength(); i++) {
                Element arcElem = (Element)arcList.item(i);
                String source = convertId2Name(arcElem.getAttribute("source"), id2name);
                String target = convertId2Name(arcElem.getAttribute("target"), id2name);
                Node head, tail;
                GspnEdge.Kind kind;
                
                if (places.containsKey(source) && transitions.containsKey(target)) {
                    // Input or inhibitor edge
                    tail = places.get(source);
                    head = transitions.get(target);
                    
                    Element arcType = getSingleSubElementByTagName(arcElem, "type");
                    if (arcType != null && 
                        arcType.hasAttribute("value") && 
                        arcType.getAttribute("value").equals("inhibitor"))
                        kind = GspnEdge.Kind.INHIBITOR;
                    else
                        kind = GspnEdge.Kind.INPUT;
                }
                else if (transitions.containsKey(source) && places.containsKey(target)) {
                    // Output edge
                    tail = transitions.get(source);
                    head = places.get(target);
                    kind = GspnEdge.Kind.OUTPUT;
                }
                else {
                    log.add("Cannot load incomplete arc with id="+arcElem.getAttribute("id"));
                    continue;
                }
                
                // Read points
                ArrayList<Point2D> points = new ArrayList<>();
                points.add(new Point2D.Double(0, 0));
                Element gfxElem = getSingleSubElementByTagName(arcElem, "graphics");
                if (gfxElem != null) {
                    for (org.w3c.dom.Node child = gfxElem.getFirstChild();
                         child != null; child = child.getNextSibling())
                    {
                        if (!(child instanceof Element))
                            continue;
                        Element posElem = (Element)child;
                        if (!posElem.getTagName().equals("position"))
                            continue;
                        points.add(new Point2D.Double(unscaleCoord(posElem.getAttribute("x")),
                                                      unscaleCoord(posElem.getAttribute("y"))));
                    }
                }
                points.add(new Point2D.Double(0, 0));
                
                // Read multiplicity
                String multExpr = "1";
                Element inscriptionElem = getSingleSubElementByTagName(arcElem, "inscription");
                if (inscriptionElem != null) {
                    Element valueElem = getSingleSubElementByTagName(inscriptionElem, "value");
                    if (valueElem != null)
                        multExpr = valueElem.getTextContent();
                    else { // Some PNML have a text-only field instead of having a value field
                        Element textElem = getSingleSubElementByTagName(inscriptionElem, "text");
                        if (textElem != null) 
                            multExpr = textElem.getTextContent();
                    }
                    
                }
                Element hlInscriptionElem = getSingleSubElementByTagName(arcElem, "hlinscription");
                if (hlInscriptionElem != null) {
                    Element structElem = getSingleSubElementByTagName(hlInscriptionElem, "structure");
                    if (structElem != null) {
                        // convert XML tree into PNPRO/GreatSPN expression
                        multExpr = parseExpr(structElem, id2name, log, "multiplicity", "arc");
                    }
                }
                
                GspnEdge edge = new GspnEdge(tail, 0, head, 0, points, false, kind, multExpr);
                gspn.edges.add(edge);
            }
            
            // Read Nested Units Petri Net declarations
            if (rootUnit != null) 
                readNuPNUnits(pageElem, rootUnit);
            
        } // page reading cycle
                
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
    
    
    private static void readNuPNUnits(Element pageElem, NuPNUnit[] rootUnit) {
        NodeList toolSpecificElems = pageElem.getElementsByTagName("toolspecific");
        for (int i=0; i<toolSpecificElems.getLength(); i++) {
            Element toolspec = (Element)toolSpecificElems.item(i);
            if (!toolspec.getAttribute("tool").equals("nupn"))
                continue; // not a NuPN declaration
            if (!toolspec.getAttribute("version").equals("1.1")) {
                System.out.println("WARNING: NuPN version is not 1.1!!");
            }

            Element nupnStruct = getSingleSubElementByTagName(toolspec, "structure");
            if (nupnStruct == null)
                continue;
            
            String rootName = nupnStruct.getAttribute("root");
            String numUnits = nupnStruct.getAttribute("units");
            rootUnit[0] = new NuPNUnit(rootName);
            rootUnit[0].numUnits = numUnits;
            Queue<NuPNUnit> visitQueue = new ArrayDeque<>();
            visitQueue.add(rootUnit[0]);
            
            NodeList unitList = pageElem.getElementsByTagName("unit");            
            while (!visitQueue.isEmpty()) {
                NuPNUnit unit = visitQueue.poll();
                for (int j=0; j<unitList.getLength(); j++) {
                    Element unitEl = (Element)unitList.item(j);
                    if (!unitEl.getAttribute("id").equals(unit.unitName))
                        continue; // not the unit we want
                    
                    // Retrieve the list of places inside this unit
                    Element places = getSingleSubElementByTagName(unitEl, "places");
                    if (places != null) {
                        Scanner scanner = new Scanner(places.getTextContent());
                        while (scanner.hasNext())
                            unit.places.add(scanner.next());
                    }
                    
                    // Retrieve nested units
                    Element nestedUnits = getSingleSubElementByTagName(unitEl, "subunits");
                    if (nestedUnits != null) {
                        Scanner scanner = new Scanner(nestedUnits.getTextContent());
                        while (scanner.hasNext()) {
                            String nuName = scanner.next();
                            NuPNUnit nu = new NuPNUnit(nuName);
                            unit.subunits.add(nu);
                            visitQueue.add(nu);
                        }
                    }
                }
            }
        }
    }
    
    
    public static void main(String[] args) throws Exception {
        File f = new File("/Users/elvio/PNML/SWN.pnml");
        GspnPage gspn = new GspnPage();
        Map<String, String> id2name = new TreeMap<>();
        importPNML(gspn, f, id2name, null);
    }
}
