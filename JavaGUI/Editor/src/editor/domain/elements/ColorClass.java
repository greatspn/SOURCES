/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.elements;

import editor.domain.EditableValue;
import editor.domain.Expr;
import editor.domain.LabelDecor;
import static editor.domain.LabelDecor.DEFAULT_TEXT_SIZE;
import editor.domain.NetObject;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.grammar.EvaluationException;
import editor.domain.grammar.ExprRewriter;
import editor.domain.grammar.FormulaPayload;
import editor.domain.grammar.NodeNamespace;
import editor.domain.grammar.ParserContext;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;
import java.awt.geom.Point2D;
import java.io.Serializable;
import java.util.Set;
import org.w3c.dom.Element;

/** A color class (or domain) definition, with form:  class ID = definition
 *
 * @author elvio
 */
public class ColorClass extends BaseID implements Serializable, FormulaPayload {

    class ColorClassDefExpr extends Expr {
        public ColorClassDefExpr(String def) { super(def); rebuildParsedData(def); }
        @Override protected String getExprDescr() { 
            return "Definition of color class " + getUniqueName() + ".";
        }
        @Override protected int getParseFlags() { 
            return ParserContext.PF_CONST_EXPR | ParserContext.PF_NO_STATIC_SUBCLASS_ID;
        }
        @Override protected ParserContext.ParserEntryPoint getParseRule(String exprText) { 
            return ParserContext.ParserEntryPoint.COLOR_CLASS_DEF; 
        }
        @Override
        public void setExpr(String newExpr) {
            super.setExpr(newExpr); 
            rebuildParsedData(newExpr);
        }
    }
    
    // The header string (class/domain)
    private final NonEditableTextLabel headerText = new NonEditableTextLabel("-");
    // The color class definition
    private final ColorClassDefExpr classDef = new ColorClassDefExpr("");

    public ColorClass() {
        setLabelSequence(new LabelDecor[]{
            this.headerText,
            new ConstNameLabel(DEFAULT_TEXT_SIZE),
            new NonEditableTextLabel(" = "),
            new EditableExpressionLabel() {
                @Override Expr getValueExpr() { return classDef; }
            }
        });
    }
    
    public ColorClass(String name, Point2D pos, String def) {
        this();
        initializeNode(pos, name);
        this.classDef.setExpr(def);
    }
    
    @Override
    public void checkNodeCorrectness(NetPage page, ParserContext context) {
        super.checkNodeCorrectness(page, context);
        classDef.checkExprCorrectness(context, page, this);
        
        if (isParseDataOk() && isCrossDomain()) {
            // Relink color classes into the transient crossProdClasses[] array
            // So that we have the references to the ColorClass's of this domain
            crossProdClasses = new ColorClass[crossProdNames.length];
            for (int i=0; i<crossProdClasses.length; i++) {
                Node n = page.getNodeByUniqueName(crossProdNames[i]);
                if (n == null || !(n instanceof ColorClass)) {
                    page.addPageError("\""+crossProdNames[i]+"\" is not a valid color class name.", this);
                }
                else {
                    ColorClass refClass = (ColorClass)n;
                    if (refClass.isCrossDomain()) {
                        page.addPageError("\""+crossProdNames[i]+"\" is not a simple color class."
                                + "A cross domain may only be defined as product of simple color classes.", 
                                this);
                    }
                    else crossProdClasses[i] = refClass;
                }
            }            
        }
    }

    @Override
    public void rewriteNode(ParserContext context, ExprRewriter rewriter) {
        classDef.rewrite(context, rewriter);
    }
    
    public Expr getColorClassDef() {
        return classDef;
    }
    
    @Override
    public int getUniqueNameFontStyle() { return STYLE_ITALIC; }
    
//    @Override public GroupClass getGroupClass() { return GroupClass.COLOR_CLASS; }
    @Override public boolean hasSuperPosTags() { return false; }
    
    
    //=========================================================================
    // Support for adding color domains to other domain.elements
    //=========================================================================
    
    // Special name of the neutral color.
    public final static String UNCOLORED_DOMAIN_NAME = "<Neutral>";
    
    public static interface DomainHolder {
        public ColorClass getColorDomain();
        public boolean isInNeutralDomain();
        public boolean isInColorDomain();
        public EditableValue getColorDomainEditable();
        public String getColorDomainName();
    }
    
    public static abstract class ColorDomainExprBase extends Expr implements DomainHolder {
        public ColorDomainExprBase(String expr) { super(expr); }
//        @Override protected String getExprDescr() { 
//            return "Color domain of constant " + getUniqueName() + ".";
//        }
        @Override protected int getParseFlags() { return ParserContext.PF_CONST_EXPR; }
        @Override protected ParserContext.ParserEntryPoint getParseRule(String exprText) { 
            return ParserContext.ParserEntryPoint.PLACE_COLOR_DOMAIN_DEF; 
        }
        @Override
        public boolean isValidExpr(ParserContext context, String text) {
            if (text.isEmpty() || text.equals(UNCOLORED_DOMAIN_NAME))
                return true; // Accept the empty color domain
            return super.isValidExpr(context, text); 
        }
        @Override
        public void setExpr(String newExpr) {
            if (newExpr.equals(UNCOLORED_DOMAIN_NAME))
                newExpr = "";
            super.setExpr(newExpr); 
        }
        
        @Override public ColorClass getColorDomain() {
            if (isInNeutralDomain())
                return NeutralColorClass.INSTANCE;
            if (isFormattedAndCorrect()) {
                return (ColorClass)getParserPayload();
            }
            return null; // undetermined
        }
        @Override public boolean isInNeutralDomain() {
            return getExpr().isEmpty();
        }
        @Override public boolean isInColorDomain() {
            return !isInNeutralDomain();
        }
        @Override public EditableValue getColorDomainEditable() {
            return getEditableValue();
        }
        @Override public String getColorDomainName() {
            return getExpr();
        }
    }
    
    //=========================================================================
    // Parsed class information:
    //=========================================================================
    
    // Is a simple class definition, or a product of multiple classes?
    private boolean isCrossDomain = false;
    // Is the definition parsed correctly by the rebuildParsedData() method?
    protected boolean isParseDataValid = false;
    // The static subclasses of a simple class (or null for product classes)
    private ParsedColorSubclass[] subclasses = null;
    // List of classes in the product definition.
    private String[] crossProdNames = null;
    private transient ColorClass[] crossProdClasses = null;
    // Is this simple class ordered? (Not defined for domain classes)
    private Boolean isOrdered = null;
    // Is this class representing the identity of an agent? (for NetLogo support)
    private Boolean isAgent = null;
    
    public boolean isCrossDomain() {
//        assert isParseDataOk();
        return isCrossDomain;
    }
    
    public boolean isNeutralDomain() {
        return false;
    }
    
    public boolean isSimpleClass() {
//        assert isParseDataOk();
        return !isCrossDomain;
    }
    
    public boolean isSimpleFiniteIntRange() {
        // Is this color class simple, and defined by one range prefix{start..end} ?
        return isSimpleClass() && numSubClasses() == 1 && getSubclass(0).isInterval();
    }
    
    // Are colors circular (i.e. ordered)? Otherwise, they are unordered.
    // Only defined for simple color class, not for color domains.
    public boolean isCircular() {
        assert isSimpleClass();
        return isOrdered == true;
    }
    
    public boolean isAgent() {
        return isAgent;
    }
    
    public boolean isParseDataOk() {
        return isParseDataValid;
    }
    
    public int getNumClassesInDomain() { 
//        assert isParseDataOk();
        return isCrossDomain ? crossProdNames.length : 1;
    }
    
    // Return the number of colors in a simple color class, or assert if it is not simple
    public int numColors() {
        assert isParseDataOk() && isSimpleClass();
        int count = 0;
        for (ParsedColorSubclass cs : subclasses)
            count += cs.getNumColors();
        return count;
    }
    
    public String getColorName(int color) {
        assert isParseDataOk() && isSimpleClass();
        for (ParsedColorSubclass subclass : subclasses) {
            int numColors = subclass.getNumColors();
            if (color >= numColors) {
                color -= numColors;
            } else {
                return subclass.getColorName(color);
            }
        }
        throw new IllegalStateException("Color index is too high.");
    }
    
    
    // This method evaluates the expressions of color ranges for static subclasses.
    // A color class like:  class C = c{1 .. N}   can only be evaluated
    // when the value of N is available.
    public void evaluateColorRange(ParserContext evalContext) {
        assert isParseDataOk();
        try {
            if (isSimpleClass()) {
                // Evaluate range values.
                // Note that range value may be evaluated only if all template constants
                // have an assigned value, i.e. at evaluation time (token game, solution, ...).
                for (ParsedColorSubclass subclass : subclasses)
                    subclass.evaluateSubclassRange(evalContext);
            }
    //        else { // cross domain
    //        }
        }
        catch (EvaluationException e) {
            throw new EvaluationException("Cannot evaluate color class "+getUniqueName()+". "+e.getMessage());
        }
    }
    
    public ParsedColorSubclass getSubclass(int i) {
        assert isParseDataOk() && isSimpleClass();
        return subclasses[i];
    }
    
    public final int numSubClasses() { 
        assert isParseDataOk() && isSimpleClass();
        return subclasses.length; 
    }
    
    public String getColorClassName(int i) {
        assert isParseDataOk();
        if (isSimpleClass() && i == 0)
            return this.getUniqueName();
        
        assert isCrossDomain();
        if (crossProdClasses != null) // compiled info
            return crossProdNames[i];
        
        String[] names = classDef.getExpr().split("\\*");
        if (i >= 0 && i < names.length)
            return names[i].trim();
        return getUniqueName()+"["+i+"]";
    }
    
    public ColorClass getColorClass(int i) {
        assert isParseDataOk();
        if (isSimpleClass() && i == 0)
            return this;
        
        assert isCrossDomain() && crossProdClasses != null;
        ColorClass cc = crossProdClasses[i];
        // For now, we do not support cross domains that are defined from other cross domains.
        assert cc==null/* in case of syntactic errors*/ || cc.isSimpleClass();
        return cc;
    }
    
    public int getColorIndex(String colorName) {
        assert isParseDataOk() && isSimpleClass();
        int base = 0;
        for (ParsedColorSubclass subclass : subclasses) {
            int pos = subclass.getColorIndex(colorName);
            if (pos >= 0)
                return pos + base;
            else
                base += subclass.getNumColors();
        }
        throw new IllegalStateException("Should not happen.");
    }
    
    // Simple test to check if this class is simple and defines a static subclass
    public boolean testHasStaticSubclassNamed(String subclassName) {
        assert isSimpleClass();
        if (subclasses == null)
            return false;
        for (ParsedColorSubclass scd : subclasses)
            if (scd.isNamed() && scd.name.equals(subclassName))
                    return true;
        return false;
    }
    
    // Simple test to check there is a color with a specified name
    public boolean testHasColorNamed(NodeNamespace nspace, String colorName) {
        assert isSimpleClass();
        if (subclasses == null)
            return false;
        for (ParsedColorSubclass scd : subclasses)
            if (scd.testHasColorNamed(nspace, colorName))
                return true;
        return false;
    }
    
    // Extract the variables that are used by the colorclass definition
    // only intervals of static subclasses (simple classes) may be parametric.
    public void getDependentVariables(Set<String> varNames) {
        if (isParseDataOk() && isSimpleClass()) {
            for (int sb=0; sb<numSubClasses(); sb++) {
                ParsedColorSubclass pcs = getSubclass(sb);
                if (pcs.isInterval()) {
                    if (isAlphanumericIdentifier(pcs.getStartRangeExpr()))
                        varNames.add(pcs.getStartRangeExpr());
                    if (isAlphanumericIdentifier(pcs.getEndRangeExpr()))
                        varNames.add(pcs.getEndRangeExpr());
                }
            }
        }
    }
    
    
    static final String[] PREFIX_WORD = { "ordered", "unordered", "enum", "circular", "agent" };
    static final Boolean[] IS_ORDERED = { true, false, false, true, null };
    static final Boolean[] IS_AGENT   = { null, null, null, null, true };

    // Additional parse method. This method does not substitute the semantic parser,
    // but it is used to collect the color class definition for semantic use.
    private void rebuildParsedData(final String expr) {
        isParseDataValid = true;
        crossProdNames = null;
        crossProdClasses = null;
        subclasses = null;
        isOrdered = null;
        isAgent = null;
        isCrossDomain = expr.contains("*");
        
        headerText.setValue(null, null, isCrossDomain ? "\\text{\\textbf{domain }}" : 
                                                        "\\text{\\textbf{class }}");

//        System.out.println("class = "+getUniqueName()+"   def = "+expr+"  isCross = "+isCrossDomain);
        if (isCrossDomain) {
            crossProdNames = expr.split("\\*");
            for (int i=0; i<crossProdNames.length; i++) {
                crossProdNames[i] = crossProdNames[i].trim();
                validateId(crossProdNames[i]);
//                System.out.println("  crossProdNames["+i+"] = "+crossProdNames[i]);
            }
        }
        else {
            String classDefExpr = expr.trim();
            // Initial keyword for ordered/unordered class
            for (int i=0; i<PREFIX_WORD.length; i++) {
                if (classDefExpr.startsWith(PREFIX_WORD[i])) {
                    classDefExpr = classDefExpr.substring(PREFIX_WORD[i].length()).trim();
                    if (IS_ORDERED[i] != null)
                        isOrdered = IS_ORDERED[i];
                    if (IS_AGENT[i] != null)
                        isAgent = IS_AGENT[i];
                }
            }
            if (isOrdered == null)
                isOrdered = false; // by default, color classes are unordered.
            if (isAgent == null)
                isAgent = false; // by default, color classes are not NetLogo agents.
            
            String[] subclassDefs = classDefExpr.split("\\+");
            subclasses = new ParsedColorSubclass[subclassDefs.length];
            int scCount=0;
            for (String def : subclassDefs) {
//                SubclassDef scd = new SubclassDef();
                ParsedColorSubclass subclass;
                String name = null;
                int posBrack = def.lastIndexOf("}");
                if (posBrack != -1) {
                    String isName = def.substring(posBrack+1, def.length());
                    int posIs = isName.indexOf("is ");
                    if (posIs != -1) {
                        name = isName.substring(posIs + 3).trim();
                        validateId(name);
                    }
                    def = def.substring(0, posBrack); // remove the '}'
                }
                else 
                    isParseDataValid = false;
//                System.out.println("  subclass "+name);
                int ddotPos = def.indexOf("..");
                int openBrackPos = def.indexOf("{");
                if (ddotPos == -1) {
                    // Defined as a list
                    String[] list = def.replace("{", "").split(",");
                    for (int j=0; j<list.length; j++) {
                        list[j] = list[j].trim();
                        validateId(list[j]);
//                        System.out.println("      list item "+list[j]);
                    }
                    subclass = new ParsedColorSubclass(name, list);
                }
                else {
                    // Defined as an interval
                    String prefix = null, start, end;
                    if (openBrackPos != -1) {
                        prefix = def.substring(0, openBrackPos).trim();
                        validateId(prefix);
                        def = def.substring(openBrackPos+1, def.length());
                    }
                    String[] interv = def.split("\\.\\.");
                    if (interv.length == 2) {
                        start = interv[0].trim();
//                            validameteoetId(scd.start);
                        end = interv[1].trim();
                    }
                    else {
                        isParseDataValid = false;
                        start = "1";
                        end = "1";
                    }
//                    System.out.println("      prefix = "+prefix+"  start = "+start+"  end = "+end);
                    subclass = new ParsedColorSubclass(name, prefix, start, end);
                }
                subclasses[scCount++] = subclass;
            }
        }
    }

    private void validateId(String id) {
        isParseDataValid = isParseDataValid && id!= null && id.length() > 0 && 
                  NetObject.isAlphanumericIdentifier(id);
    }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        super.exchangeXML(el, exDir); 
        
        bindXMLAttrib(this, el, exDir, "definition", "classDef.@Expr", null, String.class);
    }
}
