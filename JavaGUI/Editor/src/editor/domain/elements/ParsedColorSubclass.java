/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.elements;

import editor.domain.NetObject;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.grammar.EvaluationArguments;
import editor.domain.grammar.EvaluationException;
import editor.domain.grammar.FormulaPayload;
import editor.domain.grammar.NodeNamespace;
import editor.domain.grammar.ParserContext;
import editor.domain.values.EvaluatedFormula;
import java.io.Serializable;
import java.util.List;
import java.util.Set;

/** An immutable static subclass in a simple color domain.
 *
 */
public class ParsedColorSubclass implements Serializable, FormulaPayload {
    // The subclass name, or null if it is anonymous
    public final String name;
    
    // Def. mode 1: The list of color names.
    private final String[] colors;
    
    // Def. mode 2: prefix { start .. end }
    private final String prefix; // could be null
    private final String startRange, endRange;
    private int startValue = -1, endValue = -1;

    public boolean isNamed() {
        return name != null;
    }
    
    public boolean isList() {
        return colors != null;
    }
    
    public boolean isInterval() {
        return startRange != null;
    }
    
    public int getNumColors() {
        if (isList())
            return colors.length;
        
        assert startValue != -1 && endValue != -1;
        return endValue - startValue + 1;
    }
    
    public void evaluateSubclassRange(ParserContext context) throws EvaluationException {
        if (context == null) {
            startValue = endValue = -1;
            return;
        }
        if (isList())
            return;
        try {
            startValue = evaluateStartRange(context);
            endValue = evaluateEndRange(context);
            if (startValue > endValue)
                throw new EvaluationException("Invalid range {"+startValue+".."+endValue+"} in static subclass definition.");
        }
        catch (EvaluationException e) {
            startValue = endValue = -1;
            throw e;
        }
    }

    public String getIntervalPrefix() {
        assert isInterval();
        return prefix;
    }
    
    // requires a compiled subclass - returns -1 if the color is not present
    public int getColorIndex(String colorName) {
        if (isList()) {
            for (int i=0; i<colors.length; i++)
                if (colors[i].equals(colorName))
                    return i;
        }
        else {
            if (prefix != null) { 
                if (colorName.length() < prefix.length() || !colorName.startsWith(prefix))
                    return -1;
                colorName = colorName.substring(prefix.length());
            }
            try {
                int nameVal = Integer.parseInt(colorName);
                if (startValue <= nameVal && nameVal <= endValue)
                    return nameVal - startValue;
            }
            catch (NumberFormatException e) {
                return -1;
            }
        }
        return -1;
    }
    
    public String getColorName(int i) {
        if (isList())
            return colors[i];
        
        if (prefix != null)
            return prefix + (startValue + i);
        else
            return "" + (startValue + i);
    }
    
    public int evaluateStartRange(ParserContext context) throws EvaluationException {
        int start = evaluateIntExpr(context, startRange, "Start range of color class");
        if (start < 0)
            throw new EvaluationException("Negative start range in static subclass definition.");
        return start;
    }
    
    public int evaluateEndRange(ParserContext context) throws EvaluationException {
        int end = evaluateIntExpr(context, endRange, "End range of color class");
        if (end < 0)
            throw new EvaluationException("Negative end range in static subclass definition.");
        return end;
    }
    
    private static int evaluateIntExpr(ParserContext context, String expr, String stackLabel) 
            throws EvaluationException 
    {
        if (NetObject.isInteger(expr)) {
            return Integer.parseInt(expr);
        }
        else if (NetObject.isAlphanumericIdentifier(expr)) {
            Node node = context.getNodeByUniqueName(expr);
            if (node != null && node instanceof ConstantID) {
                ConstantID con = (ConstantID)node;
                if (con.isInNeutralDomain() && con.isIntConst() &&
                    NetObject.isInteger(con.getConstantExpr().getExpr()))
                    return Integer.parseInt(con.getConstantExpr().getExpr());
            }
        }
        EvaluatedFormula e;
        e = context.evaluate(expr, ParserContext.ParserEntryPoint.INT_EXPR, 
                             EvaluationArguments.NO_ARGS, stackLabel, 
                             ParserContext.PF_CONST_EXPR);
        return e.getScalarInt();
    }

    public ParsedColorSubclass(String name, List<String> colors) {
        this.name = name;
        this.colors = new String[colors.size()];
        int i = 0;
        for (String s : colors)
            this.colors[i++] = s;
        this.prefix = this.startRange = this.endRange = null;
    }
    
    public ParsedColorSubclass(String name, String[] colors) {
        this.name = name;
        this.colors = colors;
        this.prefix = this.startRange = this.endRange = null;
    }

    public ParsedColorSubclass(String name, String prefix, String startRange, String endRange) {
        this.name = name;
        this.prefix = prefix;
        this.startRange = startRange;
        this.endRange = endRange;
        this.colors = null;
    }

    public String getStartRangeExpr() {
        return startRange;
    }

    public String getEndRangeExpr() {
        return endRange;
    }
    
    
    private int tryDecodeRange(NodeNamespace nspace, String rangeExpr) throws NumberFormatException {
        if (NetObject.isInteger(rangeExpr))
            return Integer.parseInt(rangeExpr);
        
        // Try to search for an integer constant
        Node node = (Node)nspace.getNodeByUniqueName(rangeExpr);
        if (node != null && node instanceof ConstantID) {
            ConstantID con = (ConstantID)node;
            if (con.isInNeutralDomain() && con.isIntConst() &&
                NetObject.isInteger(con.getConstantExpr().getExpr()))
                return Integer.parseInt(con.getConstantExpr().getExpr());
        }
        return -1;
    }
    
    public boolean testHasColorNamed(NodeNamespace nspace, String colorName) {
        if (isList()) {
            for (String clr : colors)
                if (clr.equals(colorName))
                    return true;
        }
        else {
            if (prefix != null) { 
                if (colorName.length() < prefix.length() || !colorName.startsWith(prefix))
                    return false;
                colorName = colorName.substring(prefix.length());
            }
            try {    
                int nameVal = Integer.parseInt(colorName);
                int decodedStart = tryDecodeRange(nspace, startRange);
                if (decodedStart > 0) {
                    int decodedEnd = tryDecodeRange(nspace, endRange);
                        
                    return (decodedStart <= nameVal && nameVal <= decodedEnd);
                }
            }
            catch (NumberFormatException e) {
                return false;
            }
        }
        return false;
    }
    
    public void testHasDuplicatedColorNames(NetPage page, ColorClass cc, Set<String> knownNames) {
        if (isList()) {
            for (String clr : colors)
                testColorNameUniqueness(clr, page, cc, knownNames);
        }
        else {
            try {   
                final int MAX_COLORS = 500;
                int decodedStart = tryDecodeRange(page, startRange);
                if (decodedStart > 0) {
                    int decodedEnd = tryDecodeRange(page, endRange);
                    if (decodedEnd > decodedStart && (decodedEnd - decodedStart) < MAX_COLORS) {
                        for (int i=decodedStart; i<=decodedEnd; i++) {
                            String clr = (prefix == null ? "" : prefix) + i;
                            testColorNameUniqueness(clr, page, cc, knownNames);
                        }
                    } 
                }
            }
            catch (NumberFormatException e) {
            }
        }
    }
    
    private void testColorNameUniqueness(String colorName, NetPage page, ColorClass cc, Set<String> knownNames) {
        if (knownNames.contains(colorName)) {
            page.addPageError("Color <"+colorName+">"+(isNamed() ? " in static subclass "+name : "") +
                              " does not have a unique name.", cc);
        }
        else knownNames.add(colorName);
        
        if (NetObject.isIdentifierKeyword(colorName, ParsedColorSubclass.class)) {
            page.addPageWarning("Color identifier <"+colorName+">"+(isNamed() ? " in static subclass "+name : "") +
                              " is a keyword and should not be used.", cc);
        }
    }
}
