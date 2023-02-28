/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.grammar;

import editor.domain.elements.ParsedColorSubclass;
import editor.domain.AlternateNameFunction;
import editor.domain.NetObject;
import editor.domain.Node;
import editor.domain.elements.ClockVar;
import editor.domain.elements.ColorClass;
import editor.domain.elements.ColorVar;
import editor.domain.elements.ConstantID;
import editor.domain.elements.DtaSignature;
import editor.domain.elements.Place;
import editor.domain.elements.TemplateVariable;
import editor.domain.elements.Transition;
import static editor.domain.grammar.ExpressionLanguage.GRML;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Stack;
import org.antlr.v4.runtime.ParserRuleContext;
import org.antlr.v4.runtime.Token;
import org.antlr.v4.runtime.tree.ParseTree;
import org.antlr.v4.runtime.tree.TerminalNode;

/**
 * The semantic parser of the editor. This semantic parser is used to check the
 * correctness of the expressions during the editing phase. Expressions are
 * checked for type correctness, and in some case they could provide back some
 * parsing informations to the caller (the payload). In addition, the LaTeX form
 * of the expression is prepared, that is used by the editor to show the
 * formula.
 *
 * @author elvio
 */
public class SemanticParser extends ExprLangBaseVisitor<FormattedFormula> {

    // The context in which formulas are checked and parsed.
    private ParserContext context;

    // The parse flags
    private final int parseFlags;

    // The language in which the resulting FormattedFormula will be produced
    protected ExpressionLanguage lang = ExpressionLanguage.LATEX;

    // Global flag that enables strict evaluation of color term expressions.
    // These are disabled just for the Model checking context, where some 
    // PNML file has loose color expressions.
    // Disallowed color expressions are: 
    //   - tests with < or > on colors in circular classes, and do not
    //   - next/prev color using ++/-- on enumerative color classess.
    public static boolean strictColorExpressionChecks = true;

    public SemanticParser(ParserContext context, int parseFlags) {
        this.context = context;
        this.parseFlags = parseFlags;
    }

    // Concatenate two or more FormattedFormula's into a new one
    private FormattedFormula format(boolean isSimpleTerm, Object... args) {
        return formatPayload(isSimpleTerm, null, args);
    }

    private FormattedFormula formatPayload(boolean isSimpleTerm, FormulaPayload payload, Object... args) {
        StringBuilder sb = new StringBuilder();
        for (Object arg : args) {
            sb.append(getObjectFormula(arg));
        }
        return new FormattedFormula(lang, isSimpleTerm, sb.toString(), payload);
    }

    private String getObjectFormula(Object obj) {
        if (obj instanceof FormattedFormula) {
            return ((FormattedFormula) obj).getFormula();
        }
        if (obj instanceof Node) {
            switch (lang) {
                case LATEX:
                    return ((Node) obj).getVisualizedUniqueName();
                case PNPRO:
                case GREATSPN:
                case APNN:
                    return ((Node) obj).getUniqueName();
                case GRML:
                    if (obj instanceof ColorVar) {
                        return "<attribute name=\"name\">" + ((ColorVar) obj).getUniqueName() + "</attribute>";
                    }
                    throw new UnsupportedOperationException("Cannot convert entity in GRML format.");
                case PNML:
                    if (obj instanceof ColorVar) {
                        return "<variable refvariable=\"" + ((ColorVar) obj).getUniqueName() + "\"/>";
                    }
                    throw new UnsupportedOperationException("Cannot convert entity in PNML format.");
                case NETLOGO:
                    if (obj instanceof ColorVar) {
                        return "($" + ((ColorVar) obj).getUniqueName() + "$)";
                    }
                    return ((Node) obj).getUniqueName();
//                    throw new UnsupportedOperationException("Cannot convert entity in PNML format."); 
                case CPP:
                    return ((Node) obj).getUniqueName();
                default:
                    throw new UnsupportedOperationException("getObjectFormula");
            }
        }
        if (obj instanceof ParserRuleContext) {
            return visit((ParserRuleContext) obj).getFormula();
        }
        if (obj == null) {
            return "(null)";
        }
        return obj.toString();
    }

    private void requireLatexLanguage() {
        if (lang != ExpressionLanguage.LATEX) {
            throw new UnsupportedOperationException("Parse rule is supported only in LaTeX mode.");
        }
    }

    //==========================================================================
    //  Unary and binary functions:
    //==========================================================================
    static enum OperatorPos {
        PREFIX, //  OP  <term>
        PREFIX_SIMPLETERM, //  OP  <simpleterm>  
        POSTFIX, //  <term>  OP
        POSTFIX_SIMPLETERM, //  <simpleterm>  OP
        FUNCTION, //  OP  <term>  CLOSE   or   START <term> BETWEEN <term> END
        // binary operator only
        FUNCTION_FIRST_SIMPLETERM       //  START <simpleterm> BETWEEN <term> END
    }

    static class UnaryFunct {

        public UnaryFunct(boolean isSimpleTerm, int opCode, String opName, OperatorPos pos) {
            this.isSimpleTerm = isSimpleTerm;
            this.opCode = opCode;
            this.opName = opName;
            this.opClose = null;
            this.pos = pos;
        }

        public UnaryFunct(boolean isSimpleTerm, int opCode, String opName, String opClose) {
            this.isSimpleTerm = isSimpleTerm;
            this.opCode = opCode;
            this.opName = opName;
            this.opClose = opClose;
            this.pos = OperatorPos.FUNCTION;
        }

        boolean isSimpleTerm;
        int opCode;
        String opName, opClose;
        OperatorPos pos;
    };
    static final UnaryFunct latexUnaryFunctions[] = {
        new UnaryFunct(true, ExprLangParser.OP_PAREN, "\\left( ", " \\right)"),
        new UnaryFunct(false, ExprLangParser.SUB, " - ", OperatorPos.PREFIX_SIMPLETERM),
        new UnaryFunct(false, ExprLangParser.POSTINCR, " {+}{+} ", OperatorPos.POSTFIX_SIMPLETERM),
        new UnaryFunct(false, ExprLangParser.POSTDECR, " {-}{-} ", OperatorPos.POSTFIX_SIMPLETERM),
        new UnaryFunct(true, ExprLangParser.CEIL_FN, "\\left\\lceil ", " \\right\\rceil"),
        new UnaryFunct(true, ExprLangParser.FLOOR_FN, "\\left\\lfloor ", " \\right\\rfloor"),
        new UnaryFunct(true, ExprLangParser.ROUND_FN, "\\mathrm{Round}\\left[ ", " \\right]"),
        new UnaryFunct(true, ExprLangParser.ABS_FN, "\\left\\vert ", " \\right\\vert"),
        new UnaryFunct(false, ExprLangParser.FACTORIAL_FN, "!", OperatorPos.POSTFIX_SIMPLETERM),
        new UnaryFunct(false, ExprLangParser.EXP_FN, "\\mathrm{e}^{", "}"),
        new UnaryFunct(true, ExprLangParser.SQRT_FN, "\\sqrt{", " }"),
        new UnaryFunct(false, ExprLangParser.SIN_FN, "\\sin{", " }"),
        new UnaryFunct(false, ExprLangParser.COS_FN, "\\cos{", " }"),
        new UnaryFunct(false, ExprLangParser.TAN_FN, "\\tan{", " }"),
        new UnaryFunct(false, ExprLangParser.ARCSIN_FN, "\\asin{", " }"),
        new UnaryFunct(false, ExprLangParser.ARCCOS_FN, "\\acos{", " }"),
        new UnaryFunct(false, ExprLangParser.ARCTAN_FN, "\\atan{", " }"),
        new UnaryFunct(false, ExprLangParser.NOT, "\\neg ", OperatorPos.PREFIX_SIMPLETERM),
        new UnaryFunct(true, ExprLangParser.MULTISET_CARD, "\\mathrm{Card}[", " ]"),
        new UnaryFunct(true, ExprLangParser.MULTISET_SUBCLASS, "\\mathrm{Subclass}[", " ]"),
        new UnaryFunct(true, ExprLangParser.COLOR_ORDINAL, "\\mathrm{CN}[", "]"),
        // General event PDF operator
        new UnaryFunct(true, ExprLangParser.DIRAC_DELTA_FN, "\\mathrm{\\mathbf{I}}[", "]"),};
    static final UnaryFunct greatspnUnaryFunctions[] = {
        new UnaryFunct(true, ExprLangParser.OP_PAREN, "(", ")"),
        new UnaryFunct(false, ExprLangParser.SUB, "-", OperatorPos.PREFIX_SIMPLETERM),
        new UnaryFunct(false, ExprLangParser.POSTINCR, "!", OperatorPos.PREFIX_SIMPLETERM),
        new UnaryFunct(false, ExprLangParser.POSTDECR, "^", OperatorPos.PREFIX_SIMPLETERM),
        new UnaryFunct(true, ExprLangParser.MULTISET_CARD, "", ""),
        new UnaryFunct(true, ExprLangParser.EXP_FN, "e^", OperatorPos.PREFIX_SIMPLETERM),
        new UnaryFunct(false, ExprLangParser.NOT, "! ", OperatorPos.PREFIX_SIMPLETERM),
        new UnaryFunct(false, ExprLangParser.LOG_FN, "log(", ")"),
        new UnaryFunct(true, ExprLangParser.SQRT_FN, "sqrt(", ")"),
        new UnaryFunct(true, ExprLangParser.ROUND_FN, "round(", ")"),
        new UnaryFunct(true, ExprLangParser.CEIL_FN, "ceil(", " )"),
        new UnaryFunct(true, ExprLangParser.FLOOR_FN, "floor(", ")"),
        new UnaryFunct(true, ExprLangParser.ABS_FN, "abs(", " )"),
        // General event PDF operator
        new UnaryFunct(true, ExprLangParser.DIRAC_DELTA_FN, "DiracDelta[", "]"),};
    static final UnaryFunct pnproUnaryFunctions[] = {
        new UnaryFunct(true, ExprLangParser.OP_PAREN, "(", ")"),
        new UnaryFunct(false, ExprLangParser.SUB, "-", OperatorPos.PREFIX_SIMPLETERM),
        new UnaryFunct(false, ExprLangParser.POSTINCR, "++", OperatorPos.POSTFIX_SIMPLETERM),
        new UnaryFunct(false, ExprLangParser.POSTDECR, "--", OperatorPos.POSTFIX_SIMPLETERM),
        new UnaryFunct(true, ExprLangParser.CEIL_FN, "Ceil[", "]"),
        new UnaryFunct(true, ExprLangParser.FLOOR_FN, "Floor[", "]"),
        new UnaryFunct(true, ExprLangParser.ROUND_FN, "Round[", "]"),
        new UnaryFunct(true, ExprLangParser.ABS_FN, "Abs[", "]"),
        new UnaryFunct(false, ExprLangParser.FACTORIAL_FN, "!", OperatorPos.POSTFIX_SIMPLETERM),
        new UnaryFunct(false, ExprLangParser.EXP_FN, "Exp[", "]"),
        new UnaryFunct(true, ExprLangParser.SQRT_FN, "Sqrt[", "]"),
        new UnaryFunct(false, ExprLangParser.SIN_FN, "Sin[", "]"),
        new UnaryFunct(false, ExprLangParser.COS_FN, "Cos[", "]"),
        new UnaryFunct(false, ExprLangParser.TAN_FN, "Tan[", "]"),
        new UnaryFunct(false, ExprLangParser.ARCSIN_FN, "Asin[", "]"),
        new UnaryFunct(false, ExprLangParser.ARCCOS_FN, "Acos[", "]"),
        new UnaryFunct(false, ExprLangParser.ARCTAN_FN, "Atan[", "]"),
        new UnaryFunct(false, ExprLangParser.NOT, "!", OperatorPos.PREFIX_SIMPLETERM),
        new UnaryFunct(true, ExprLangParser.MULTISET_CARD, "Card[", "]"),
        new UnaryFunct(true, ExprLangParser.MULTISET_SUBCLASS, "Subclass[", "]"),
        new UnaryFunct(true, ExprLangParser.COLOR_ORDINAL, "CN[", "]"),
        // General event PDF operator
        new UnaryFunct(true, ExprLangParser.DIRAC_DELTA_FN, "I[", "]"),};
    static final UnaryFunct apnnUnaryFunctions[] = {
        new UnaryFunct(true, ExprLangParser.OP_PAREN, "(", ")"),
        new UnaryFunct(false, ExprLangParser.SUB, " - ", OperatorPos.PREFIX_SIMPLETERM),
        //        new UnaryFunct(false, ExprLangParser.POSTINCR, "!", OperatorPos.PREFIX_SIMPLETERM),
        //        new UnaryFunct(false, ExprLangParser.POSTDECR, "^", OperatorPos.PREFIX_SIMPLETERM),
        new UnaryFunct(true, ExprLangParser.CEIL_FN, "Math.ceil(", ")"),
        new UnaryFunct(true, ExprLangParser.FLOOR_FN, "Math.floor(", ")"),
        new UnaryFunct(true, ExprLangParser.ROUND_FN, "Math.round(", ")"),
        new UnaryFunct(true, ExprLangParser.ABS_FN, "Math.abs(", ")"),
        new UnaryFunct(false, ExprLangParser.FACTORIAL_FN, "Math.fact(", ")"),
        new UnaryFunct(false, ExprLangParser.EXP_FN, "Math.exp(", ")"),
        new UnaryFunct(true, ExprLangParser.SQRT_FN, "Math.sqrt(", ")"),
        new UnaryFunct(false, ExprLangParser.SIN_FN, "Math.sin(", ")"),
        new UnaryFunct(false, ExprLangParser.COS_FN, "Math.cos(", ")"),
        new UnaryFunct(false, ExprLangParser.TAN_FN, "Math.tan(", ")"),
        new UnaryFunct(false, ExprLangParser.ARCSIN_FN, "Math.asin(", ")"),
        new UnaryFunct(false, ExprLangParser.ARCCOS_FN, "Math.acos(", ")"),
        new UnaryFunct(false, ExprLangParser.ARCTAN_FN, "Math.atan(", ")"),
        new UnaryFunct(false, ExprLangParser.NOT, "!", OperatorPos.PREFIX_SIMPLETERM),
        //        new UnaryFunct(true, ExprLangParser.MULTISET_CARD, "Card[", "]"),
        //        new UnaryFunct(true, ExprLangParser.MULTISET_SUBCLASS, "d(", ")"),
        new UnaryFunct(true, ExprLangParser.DIRAC_DELTA_FN, "\\dist{const}{", "}"),};

    static final UnaryFunct grmlUnaryFunctions[] = {
        new UnaryFunct(true, ExprLangParser.OP_PAREN, "<attribute name=\"expr\">", "</attribute>"),
        new UnaryFunct(false, ExprLangParser.SUB, "<attribute name=\"-\"><attribute name=\"numValue\">0</attribute>", "</attribute>"),
        //        new UnaryFunct(false, ExprLangParser.POSTINCR, "!", OperatorPos.PREFIX_SIMPLETERM),
        //        new UnaryFunct(false, ExprLangParser.POSTDECR, "^", OperatorPos.PREFIX_SIMPLETERM),
        new UnaryFunct(true, ExprLangParser.CEIL_FN, "<attribute name=\"ceil\">", "</attribute>"),
        new UnaryFunct(true, ExprLangParser.FLOOR_FN, "<attribute name=\"floor\">", "</attribute>"),
        new UnaryFunct(true, ExprLangParser.ROUND_FN, "Math.round(", ")"),
        new UnaryFunct(true, ExprLangParser.ABS_FN, "Math.abs(", ")"),
        new UnaryFunct(false, ExprLangParser.FACTORIAL_FN, "Math.fact(", ")"),
        new UnaryFunct(false, ExprLangParser.EXP_FN, "Math.exp(", ")"),
        new UnaryFunct(true, ExprLangParser.SQRT_FN, "Math.sqrt(", ")"),
        new UnaryFunct(false, ExprLangParser.NOT, "<attribute name=\"not\">", "</attribute>"),
        new UnaryFunct(true, ExprLangParser.POSTINCR, "<attribute name=\"function\"><attribute name=\"++\">", "</attribute></attribute>"),
        new UnaryFunct(true, ExprLangParser.POSTDECR, "<attribute name=\"function\"><attribute name=\"--\">", "</attribute></attribute>"),
        //        new UnaryFunct(true, ExprLangParser.MULTISET_CARD, "Card[", "]"),
        //        new UnaryFunct(true, ExprLangParser.MULTISET_SUBCLASS, "d(", ")"),
        new UnaryFunct(true, ExprLangParser.DIRAC_DELTA_FN,
        "\t\t<attribute name=\"type\">DETERMINISTIC</attribute>\n"
        + "\t\t<attribute name=\"param\"><attribute name=\"expr\">", "</attribute></attribute>"),};

    static final UnaryFunct pnmlUnaryFunctions[] = {
        new UnaryFunct(true, ExprLangParser.OP_PAREN, "", ""),
        new UnaryFunct(true, ExprLangParser.SUB, "-", ""),
        new UnaryFunct(true, ExprLangParser.POSTINCR, "<successor><subterm>", "</subterm></successor>"),
        new UnaryFunct(true, ExprLangParser.POSTDECR, "<predecessor><subterm>", "</subterm></predecessor>"),
        new UnaryFunct(true, ExprLangParser.NOT, "<not><subterm>", "</subterm></not>"),};

    static final UnaryFunct netlogoUnaryFunctions[] = {
        new UnaryFunct(true, ExprLangParser.OP_PAREN, "( ", " )"),
        new UnaryFunct(false, ExprLangParser.SUB, " - ", OperatorPos.PREFIX_SIMPLETERM),
        new UnaryFunct(false, ExprLangParser.POSTINCR, " + 1", OperatorPos.POSTFIX_SIMPLETERM),
        new UnaryFunct(false, ExprLangParser.POSTDECR, " - 1", OperatorPos.POSTFIX_SIMPLETERM),
        //        new UnaryFunct(true, ExprLangParser.CEIL_FN, "\\left\\lceil ", " \\right\\rceil"),
        //        new UnaryFunct(true, ExprLangParser.FLOOR_FN, "\\left\\lfloor ", " \\right\\rfloor"),
        //        new UnaryFunct(true, ExprLangParser.ROUND_FN, "\\mathrm{Round}\\left[ ", " \\right]"),
        //        new UnaryFunct(true, ExprLangParser.ABS_FN, "\\left\\vert ", " \\right\\vert"),
        //        new UnaryFunct(false, ExprLangParser.FACTORIAL_FN, "!", OperatorPos.POSTFIX_SIMPLETERM),
        //        new UnaryFunct(false, ExprLangParser.EXP_FN, "\\mathrm{e}^{", "}"),
        //        new UnaryFunct(true, ExprLangParser.SQRT_FN, "\\sqrt{", " }"),
        //        new UnaryFunct(false, ExprLangParser.SIN_FN, "\\sin{", " }"),
        //        new UnaryFunct(false, ExprLangParser.COS_FN, "\\cos{", " }"),
        //        new UnaryFunct(false, ExprLangParser.TAN_FN, "\\tan{", " }"),
        //        new UnaryFunct(false, ExprLangParser.ARCSIN_FN, "\\asin{", " }"),
        //        new UnaryFunct(false, ExprLangParser.ARCCOS_FN, "\\acos{", " }"),
        //        new UnaryFunct(false, ExprLangParser.ARCTAN_FN, "\\atan{", " }"),
        new UnaryFunct(false, ExprLangParser.NOT, "NOT ", OperatorPos.PREFIX_SIMPLETERM), //        new UnaryFunct(true, ExprLangParser.MULTISET_CARD, "\\mathrm{Card}[", " ]"),
    //        new UnaryFunct(true, ExprLangParser.MULTISET_SUBCLASS, "\\mathrm{Subclass}[", " ]"),
    //        new UnaryFunct(true, ExprLangParser.COLOR_ORDINAL, "\\mathrm{CN}[", "]"),
    };

    static final UnaryFunct cppUnaryFunctions[] = {
        new UnaryFunct(true, ExprLangParser.OP_PAREN, "(", ")"),
        new UnaryFunct(false, ExprLangParser.SUB, "-", OperatorPos.PREFIX_SIMPLETERM),
        new UnaryFunct(false, ExprLangParser.NOT, "! ", OperatorPos.PREFIX_SIMPLETERM),
        new UnaryFunct(false, ExprLangParser.POSTINCR, "++", OperatorPos.POSTFIX_SIMPLETERM),
        new UnaryFunct(false, ExprLangParser.POSTDECR, "--", OperatorPos.POSTFIX_SIMPLETERM),
        new UnaryFunct(false, ExprLangParser.EXP_FN, "exp(", ")"),
        new UnaryFunct(false, ExprLangParser.LOG_FN, "log(", ")"),
        new UnaryFunct(true, ExprLangParser.SQRT_FN, "sqrt(", ")"),
        new UnaryFunct(true, ExprLangParser.ROUND_FN, "round(", ")"),
        new UnaryFunct(true, ExprLangParser.CEIL_FN, "ceil(", " )"),
        new UnaryFunct(true, ExprLangParser.FLOOR_FN, "floor(", ")"),
        new UnaryFunct(true, ExprLangParser.ABS_FN, "abs(", " )"),
        // come gestire le dirac delta e le rectangular function in cpp?
        new UnaryFunct(true, ExprLangParser.DIRAC_DELTA_FN, "Dirac_delta(", ")"),};

    public FormattedFormula formatUnaryFn(int unaryIntRealFn, FormattedFormula expr) {
        UnaryFunct[] functs;
        switch (lang) {
            //case LATEX:     functs = latexUnaryFunctions;       break;
            case LATEX:
                functs = latexUnaryFunctions;
                break;
            case PNPRO:
                functs = pnproUnaryFunctions;
                break;
            case GREATSPN:
                functs = greatspnUnaryFunctions;
                break;
            case APNN:
                functs = apnnUnaryFunctions;
                break;
            case GRML:
                functs = grmlUnaryFunctions;
                break;
            case PNML:
                functs = pnmlUnaryFunctions;
                break;
            case NETLOGO:
                functs = netlogoUnaryFunctions;
                break;
            case CPP:
                functs = cppUnaryFunctions;
                break;
            default:
                throw new UnsupportedOperationException("formatUnaryFn: Unsupported language");
        }
        for (UnaryFunct uf : functs) {
            if (uf.opCode == unaryIntRealFn) {
                switch (uf.pos) {
                    case PREFIX_SIMPLETERM:
                        return format(uf.isSimpleTerm, uf.opName, expr.getAsSimpleTerm());

                    case FUNCTION:
                        return format(uf.isSimpleTerm, uf.opName, expr, uf.opClose);

                    case POSTFIX_SIMPLETERM:
                        return format(uf.isSimpleTerm, expr.getAsSimpleTerm(), uf.opName);
                }
            }
        }
        throw new UnsupportedOperationException("formatUnaryFn = " + unaryIntRealFn + " for " + lang);
    }

    static class BinaryFunct {

        public BinaryFunct(boolean isSimpleTerm, int opCode, String between, OperatorPos pos) {
            this.isSimpleTerm = isSimpleTerm;
            this.opCode = opCode;
            this.start = "";
            this.between = between;
            this.end = "";
            this.pos = pos;
        }

        public BinaryFunct(boolean isSimpleTerm, int opCode, String start,
                String between, String end, OperatorPos pos) {
            this.isSimpleTerm = isSimpleTerm;
            this.opCode = opCode;
            this.start = start;
            this.between = between;
            this.end = end;
            this.pos = pos;
        }
        boolean isSimpleTerm;
        int opCode;
        String start, between, end;
        OperatorPos pos;
    };

    static final BinaryFunct latexBinaryFunctions[] = {
        new BinaryFunct(false, ExprLangParser.ADD, " + ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.SUB, " - ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.MUL, " \\cdot ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.DIV, " / ", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.MAX_FN, "\\max\\left( ", ", ", "\\right)", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.MIN_FN, "\\min\\left( ", ", ", "\\right)", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.MOD_FN, " \\hspace{3pt}\\mathrm{mod}\\hspace{3pt} ", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.BINOMIAL_FN, "\\binom{ ", "}{", "}", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.POW_FN, "{", "}^{", "}", OperatorPos.FUNCTION_FIRST_SIMPLETERM),
        new BinaryFunct(true, ExprLangParser.FRACT_FN, "\\frac{ ", "}{", "}", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.AND, " \\wedge ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.OR, " \\vee ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.IMPLY, " \\rightarrow ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.BIIMPLY, " \\leftrightarrow ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.EQUAL, " = ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.NOT_EQUAL, " \\not= ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.LESS, " < ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.LESS_EQ, " \\leq ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.GREATER, " > ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.GREATER_EQ, " \\geq ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.CONTAINS, " \\in ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.DONT_CONTAINS, " \\not\\in ", OperatorPos.FUNCTION),
        // General event PDF functions
        new BinaryFunct(true, ExprLangParser.RECT_FN, " \\mathrm{\\mathbf{R}}\\left[", ", ", "\\right]", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.UNIFORM_FN, " \\mathrm{Uniform}\\left[", ", ", "\\right]", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.TRIANGULAR_FN, " \\mathrm{Triangular}\\left[", ", ", "\\right]", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.ERLANG_FN, " \\mathrm{Erlang}\\left[", ", ", "\\right]", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.TRUNCATED_EXP_FN, " \\mathrm{TruncatedExp}\\left[", ", ", "\\right]", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.PARETO_FN, " \\mathrm{Pareto}\\left[", ", ", "\\right]", OperatorPos.FUNCTION),};
    static final BinaryFunct pnproBinaryFunctions[] = {
        new BinaryFunct(false, ExprLangParser.ADD, " + ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.SUB, " - ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.MUL, " * ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.DIV, " / ", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.MAX_FN, "Max[", ", ", "]", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.MIN_FN, "Min[", ", ", "]", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.MOD_FN, "Mod[", ", ", "]", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.BINOMIAL_FN, "Binom[ ", ", ", "]", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.POW_FN, "Pow[", ", ", "]", OperatorPos.FUNCTION_FIRST_SIMPLETERM),
        new BinaryFunct(true, ExprLangParser.FRACT_FN, " / ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.AND, " && ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.OR, " || ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.IMPLY, " -> ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.BIIMPLY, " <-> ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.EQUAL, " = ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.NOT_EQUAL, " != ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.LESS, " < ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.LESS_EQ, " <= ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.GREATER, " > ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.GREATER_EQ, " >= ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.CONTAINS, " in ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.DONT_CONTAINS, " !in ", OperatorPos.FUNCTION),
        // General event PDF functions
        new BinaryFunct(true, ExprLangParser.RECT_FN, "R[", ", ", "]", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.UNIFORM_FN, "Uniform[", ", ", "]", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.TRIANGULAR_FN, "Triangular[", ", ", "]", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.ERLANG_FN, "Erlang[", ", ", "]", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.TRUNCATED_EXP_FN, "TruncatedExp[", ", ", "]", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.PARETO_FN, "Pareto[", ", ", "]", OperatorPos.FUNCTION),};
    static final BinaryFunct greatspnBinaryFunctions[] = {
        new BinaryFunct(false, ExprLangParser.ADD, " + ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.SUB, " - ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.MUL, " * ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.DIV, " / ", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.MAX_FN, "Max[", ", ", "]", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.MIN_FN, "Min[", ", ", "]", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.MOD_FN, "Mod[", ", ", "]", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.BINOMIAL_FN, "Binom[ ", ", ", "]", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.POW_FN, "Pow[", ", ", "]", OperatorPos.FUNCTION_FIRST_SIMPLETERM),
        new BinaryFunct(true, ExprLangParser.FRACT_FN, " / ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.AND, " & ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.OR, " | ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.IMPLY, " -> ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.BIIMPLY, " <-> ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.EQUAL, " = ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.NOT_EQUAL, " /= ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.LESS, " < ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.LESS_EQ, " <= ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.GREATER, " > ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.GREATER_EQ, " >= ", OperatorPos.FUNCTION),
        // General event PDF functions
        new BinaryFunct(true, ExprLangParser.RECT_FN, "Rect[", ", ", "]", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.UNIFORM_FN, "Uniform[", ", ", "]", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.TRIANGULAR_FN, "Triangular[", ", ", "]", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.ERLANG_FN, "Erlang[", ", ", "]", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.TRUNCATED_EXP_FN, "TruncatedExp[", ", ", "]", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.PARETO_FN, "Pareto[", ", ", "]", OperatorPos.FUNCTION),};
    static final BinaryFunct apnnBinaryFunctions[] = {
        new BinaryFunct(false, ExprLangParser.ADD, " + ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.SUB, " - ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.MUL, " * ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.DIV, " / ", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.MAX_FN, "Math.max(", ", ", ")", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.MIN_FN, "Math.min(", ", ", ")", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.MOD_FN, " % ", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.BINOMIAL_FN, "Math.binom( ", ", ", ")", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.POW_FN, "Math.pow(", ", ", ")", OperatorPos.FUNCTION_FIRST_SIMPLETERM),
        new BinaryFunct(true, ExprLangParser.FRACT_FN, " / ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.AND, " andalso ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.OR, " orelse ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.EQUAL, " = ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.NOT_EQUAL, " <> ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.LESS, " < ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.LESS_EQ, " <= ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.GREATER, " > ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.GREATER_EQ, " >= ", OperatorPos.FUNCTION),
        // General PDF functions 
        //new BinaryFunct(true, ExprLangParser.RECT_FN, "Rect[", ", ", "]", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.UNIFORM_FN, "\\dist{uniform}{", " ", "}", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.TRIANGULAR_FN, "\\dist{trian}{", " ", "}", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.ERLANG_FN, "\\dist{erlang}{", " ", "}", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.TRUNCATED_EXP_FN, "\\dist{truncexp}{", " ", "}", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.PARETO_FN, "\\dist{pareto}{", " ", "}", OperatorPos.FUNCTION),};

    static final BinaryFunct grmlBinaryFunctions[] = {
        new BinaryFunct(false, ExprLangParser.ADD, "<attribute name=\"+\">", " ", "</attribute>", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.SUB, "<attribute name=\"-\">", " ", "</attribute>", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.MUL, "<attribute name=\"*\">", " ", "</attribute>", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.DIV, "<attribute name=\"/\">", " ", "</attribute>", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.MAX_FN, "<attribute name=\"max\">", " ", "</attribute>", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.MIN_FN, "<attribute name=\"min\">", " ", "</attribute>", OperatorPos.FUNCTION),
        //        new BinaryFunct(false, ExprLangParser.MOD_FN, "Mod[", ", ", "]", OperatorPos.FUNCTION),
        //        new BinaryFunct(true, ExprLangParser.BINOMIAL_FN, "Binom[ ", ", ", "]", OperatorPos.FUNCTION),
        //        new BinaryFunct(false, ExprLangParser.POW_FN, "Pow[", ", ", "]", OperatorPos.FUNCTION_FIRST_SIMPLETERM),
        new BinaryFunct(true, ExprLangParser.FRACT_FN, "<attribute name=\"/\">", " ", "</attribute>", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.AND, "<attribute name=\"and\">", " ", "</attribute>", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.OR, "<attribute name=\"or\">", " ", "</attribute>", OperatorPos.FUNCTION),
        //        new BinaryFunct(false, ExprLangParser.IMPLY, " -> ", OperatorPos.FUNCTION),
        //        new BinaryFunct(false, ExprLangParser.BIIMPLY, " <-> ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.EQUAL, "<attribute name=\"equal\">", " ", "</attribute>", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.NOT_EQUAL, "<attribute name=\"notEqual\">", " ", "</attribute>", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.LESS, " < ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.LESS_EQ, " <= ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.GREATER, " > ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.GREATER_EQ, " >= ", OperatorPos.FUNCTION),
        // General event PDF functions
        //        new BinaryFunct(true, ExprLangParser.RECT_FN, "R[", ", ", "]", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.UNIFORM_FN,
        "\t\t<attribute name=\"type\">UNIFORM</attribute>\n"
        + "\t\t<attribute name=\"param\"><attribute name=\"expr\">", "</attribute></attribute>\n"
        + "\t\t<attribute name=\"param\"><attribute name=\"expr\">", "</attribute></attribute>", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.TRIANGULAR_FN,
        "\t\t<attribute name=\"type\">TRIANGULAR</attribute>\n"
        + "\t\t<attribute name=\"param\"><attribute name=\"expr\">", "</attribute></attribute>\n"
        + "\t\t<attribute name=\"param\"><attribute name=\"expr\">", "</attribute></attribute>", OperatorPos.FUNCTION),
        new BinaryFunct(true, ExprLangParser.ERLANG_FN,
        "\t\t<attribute name=\"type\">ERLANG</attribute>\n"
        + "\t\t<attribute name=\"param\"><attribute name=\"expr\">", "</attribute></attribute>\n"
        + "\t\t<attribute name=\"param\"><attribute name=\"expr\">", "</attribute></attribute>", OperatorPos.FUNCTION),};

    static final BinaryFunct pnmlBinaryFunctions[] = {
        new BinaryFunct(false, ExprLangParser.ADD, "<add><subterm>", "</subterm><subterm>", "</subterm></add>", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.SUB, "<subtract><subterm>", "</subterm><subterm>", "</subterm></subtract>", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.AND, "<and><subterm>", "</subterm><subterm>", "</subterm></and>", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.OR, "<or><subterm>", "</subterm><subterm>", "</subterm></or>", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.IMPLY, "<imply><subterm>", "</subterm><subterm>", "</subterm></imply>", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.EQUAL, "<equality><subterm>", "</subterm><subterm>", "</subterm></equality>", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.NOT_EQUAL, "<inequality><subterm>", "</subterm><subterm>", "</subterm></inequality>", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.LESS, "<lessthan><subterm>", "</subterm><subterm>", "</subterm></lessthan>", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.LESS_EQ, "<lessthanorequal><subterm>", "</subterm><subterm>", "</subterm></lessthanorequal>", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.GREATER, "<greaterthan><subterm>", "</subterm><subterm>", "</subterm></greaterthan>", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.GREATER_EQ, "<greaterthanorequal><subterm>", "</subterm><subterm>", "</subterm></greaterthanorequal>", OperatorPos.FUNCTION),};
    static final BinaryFunct netlogoBinaryFunctions[] = {
        new BinaryFunct(false, ExprLangParser.ADD, " + ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.SUB, " - ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.MUL, " * ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.DIV, " / ", OperatorPos.FUNCTION),
        //        new BinaryFunct(true, ExprLangParser.MAX_FN, "Max[", ", ", "]", OperatorPos.FUNCTION),
        //        new BinaryFunct(true, ExprLangParser.MIN_FN, "Min[", ", ", "]", OperatorPos.FUNCTION),
        //        new BinaryFunct(false, ExprLangParser.MOD_FN, "Mod[", ", ", "]", OperatorPos.FUNCTION),
        //        new BinaryFunct(true, ExprLangParser.BINOMIAL_FN, "Binom[ ", ", ", "]", OperatorPos.FUNCTION),
        //        new BinaryFunct(false, ExprLangParser.POW_FN, "Pow[", ", ", "]", OperatorPos.FUNCTION_FIRST_SIMPLETERM),
        //        new BinaryFunct(true, ExprLangParser.FRACT_FN, " / ", OperatorPos.FUNCTION),

        new BinaryFunct(false, ExprLangParser.AND, " AND ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.OR, " OR ", OperatorPos.FUNCTION),
        //        new BinaryFunct(false, ExprLangParser.IMPLY, " -> ", OperatorPos.FUNCTION),
        //        new BinaryFunct(false, ExprLangParser.BIIMPLY, " <-> ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.EQUAL, " = ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.NOT_EQUAL, " != ", OperatorPos.FUNCTION),
        //        new BinaryFunct(false, ExprLangParser.LESS, " < ", OperatorPos.FUNCTION),
        //        new BinaryFunct(false, ExprLangParser.LESS_EQ, " <= ", OperatorPos.FUNCTION),
        //        new BinaryFunct(false, ExprLangParser.GREATER, " > ", OperatorPos.FUNCTION),
        //        new BinaryFunct(false, ExprLangParser.GREATER_EQ, " >= ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.CONTAINS, " ( member? ", " ", " )", OperatorPos.FUNCTION_FIRST_SIMPLETERM),
        new BinaryFunct(false, ExprLangParser.DONT_CONTAINS, " ( NOT member? ", " ", " )", OperatorPos.FUNCTION_FIRST_SIMPLETERM), // General event PDF functions
    //        new BinaryFunct(true, ExprLangParser.RECT_FN, "R[", ", ", "]", OperatorPos.FUNCTION),
    //        new BinaryFunct(true, ExprLangParser.UNIFORM_FN, "Uniform[", ", ", "]", OperatorPos.FUNCTION),
    //        new BinaryFunct(true, ExprLangParser.TRIANGULAR_FN, "Triangular[", ", ", "]", OperatorPos.FUNCTION),
    //        new BinaryFunct(true, ExprLangParser.ERLANG_FN, "Erlang[", ", ", "]", OperatorPos.FUNCTION),
    //        new BinaryFunct(true, ExprLangParser.TRUNCATED_EXP_FN, "TruncatedExp[", ", ", "]", OperatorPos.FUNCTION),
    //        new BinaryFunct(true, ExprLangParser.PARETO_FN, "Pareto[", ", ", "]", OperatorPos.FUNCTION),
    };

    static final BinaryFunct cppBinaryFunctions[] = {
        new BinaryFunct(false, ExprLangParser.ADD, " + ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.SUB, " - ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.MUL, " * ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.DIV, " / ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.FRACT_FN, " / ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.AND, " && ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.OR, " || ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.EQUAL, " == ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.NOT_EQUAL, " != ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.LESS, " < ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.LESS_EQ, " <= ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.GREATER, " > ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.GREATER_EQ, " >= ", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.MAX_FN, "max(", ", ", ")", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.MIN_FN, "min(", ", ", ")", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.POW_FN, "pow(", ", ", ")", OperatorPos.FUNCTION),
        //come gestire in cpp le distribuzioni?
        new BinaryFunct(false, ExprLangParser.RECT_FN, "uniform_real_distribution<double> unf_dis(", ", ", ");\n unf_dis(generator)", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.TRUNCATED_EXP_FN, "exponential_distribution<> exp_dis(", ", ", ")\n exp_dis(generator)", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.TRIANGULAR_FN, "Triangular(", ", ", ")", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.PARETO_FN, "Pareto(", ", ", ")", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.UNIFORM_FN, "uniform_real_distribution<double> unf_dis(", ", ", ");\n distribution(generator)", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.ERLANG_FN, "Erlang(", ", ", ")", OperatorPos.FUNCTION),
        new BinaryFunct(false, ExprLangParser.BINOMIAL_FN, "binomial_distribution<> bin_dis(", ", ", ")\n bin_dis(generator)", OperatorPos.FUNCTION),};

    public FormattedFormula formatBinaryFn(int binaryIntFn, FormattedFormula expr0,
            FormattedFormula expr1) {
        BinaryFunct[] functs;
        switch (lang) {
            //case LATEX:     functs = latexBinaryFunctions;    break;
            case LATEX:
                functs = latexBinaryFunctions;
                break;
            case PNPRO:
                functs = pnproBinaryFunctions;
                break;
            case GREATSPN:
                functs = greatspnBinaryFunctions;
                break;
            case APNN:
                functs = apnnBinaryFunctions;
                break;
            case GRML:
                functs = grmlBinaryFunctions;
                break;
            case PNML:
                functs = pnmlBinaryFunctions;
                break;
            case NETLOGO:
                functs = netlogoBinaryFunctions;
                break;
            case CPP:
                functs = cppBinaryFunctions;
                break;
            default:
                throw new UnsupportedOperationException("formatBinaryFn: Unsupported language");
        }
        for (BinaryFunct bf : functs) {
            if (bf.opCode == binaryIntFn) {
                switch (bf.pos) {
                    case FUNCTION:
                        return format(bf.isSimpleTerm, bf.start, expr0, bf.between, expr1, bf.end);
                    case FUNCTION_FIRST_SIMPLETERM:
                        return format(bf.isSimpleTerm, bf.start, expr0.getAsSimpleTerm(),
                                bf.between, expr1, bf.end);
                }
            }
        }
        throw new UnsupportedOperationException("formatBinaryFn = " + binaryIntFn);
    }

    public FormattedFormula formatIfThenElse(boolean preferIfFormat,
            FormattedFormula cond,
            FormattedFormula e1,
            FormattedFormula e2) {
        switch (lang) {
            case LATEX:
                if (preferIfFormat) {
                    return format(true, "\\mathrm{If}\\left[", cond, ", ",
                            e1, ", ", e2, "\\right]");
                } else {
                    return format(true, "\\textrm{when } ", cond, " :\\textrm{ }",
                            e1, "; \\textrm{ ever } ", e2, ";");
                }
            case GREATSPN:
                return format(true, "when ", cond, ": ", e1, "; ever ", e2, ";");

            case PNPRO:
                return format(true, "If[", cond, ", ", e1, ", ", e2, "]");

            case APNN:
                return format(true, "if ", cond, " then ", e1, " else ", e2);

            default:
                throw new UnsupportedOperationException("formatIfThenElse: unsupported lang");
        }
    }

//    public static FormattedFormula formatUnaryIntRealFn(int unaryIntRealFn, FormattedFormula expr) {
//        switch (unaryIntRealFn) {
//            case ExprLangParser.CEIL_FN:
//                return format(true, "\\left\\lceil ", expr, " \\right\\rceil");
//            case ExprLangParser.FLOOR_FN:
//                return format(true, "\\left\\lfloor ", expr, " \\right\\rfloor");
//            case ExprLangParser.ROUND_FN:
//                return format(true, "\\mathrm{Round}\\left[ ", expr, " \\right]");
//            default:
//                throw new UnsupportedOperationException();
//        }
//    }
//    public static FormattedFormula formatBinaryIntFn(int binaryIntFn, FormattedFormula expr0, FormattedFormula expr1) {
//        switch (binaryIntFn) {
//            case ExprLangParser.MAX_FN:
//                return format(true, "\\max\\left( ", expr0, ", ", expr1, " \\right)");
//            case ExprLangParser.MIN_FN:
//                return format(true, "\\min\\left( ", expr0, ", ", expr1, " \\right)");
//            case ExprLangParser.MOD_FN:
//                return format(false, expr0, " \\mod ", expr1);
//            case ExprLangParser.BINOMIAL_FN:
//                return format(true, "\\binom{", expr0, "}{", expr1, "}");
//            case ExprLangParser.POW_FN:
//                return format(false, "{", expr0.getAsSimpleTerm(), "}^{", expr1, "}");
//            case ExprLangParser.FRACT_FN:
//                return format(true, "\\frac{", expr0, "}{", expr1, "}");
//            default:
//                throw new UnsupportedOperationException();
//        }      
//    }
//    public static FormattedFormula formatUnaryIntFn(int unaryFn, FormattedFormula expr) {
//        switch (unaryFn) {
//            case ExprLangParser.ABS_FN:
//                return format(true, "\\left\\vert ", expr, " \\right\\vert");
//            case ExprLangLexer.FACTORIAL_FN:
//                return format(false, expr.getAsSimpleTerm(), "!");
//            default:
//                throw new UnsupportedOperationException();
//        }
//    }   
//    public static FormattedFormula formatUnaryRealFn(int unaryRealFn, FormattedFormula expr) {
//        switch (unaryRealFn) {
//            case ExprLangParser.EXP_FN:
//                return format(false, "\\mathrm{e}^{", expr, "}");
//            case ExprLangParser.SQRT_FN:
//                return format(true, "\\sqrt{", expr, "}");
//            case ExprLangParser.SIN_FN:
//                return format(false, "\\sin{", expr.getAsSimpleTerm(), "}");
//            case ExprLangParser.COS_FN:
//                return format(false, "\\cos{", expr.getAsSimpleTerm(), "}");
//            case ExprLangParser.TAN_FN:
//                return format(false, "\\tan{", expr.getAsSimpleTerm(), "}");
//            case ExprLangParser.ARCSIN_FN:
//                return format(false, "\\arcsin{", expr.getAsSimpleTerm(), "}");
//            case ExprLangParser.ARCCOS_FN:
//                return format(false, "\\arccos{", expr.getAsSimpleTerm(), "}");
//            case ExprLangParser.ARCTAN_FN:
//                return format(false, "\\arctan{", expr.getAsSimpleTerm(), "}");
//            default:
//                throw new UnsupportedOperationException();
//        }
//    }
//    public static String getLatexCmpOp(int op) {
//        switch (op) {
//            case ExprLangLexer.EQUAL:       return " = ";
//            case ExprLangLexer.NOT_EQUAL:   return " \\not= ";
//            case ExprLangLexer.LESS:        return " < ";
//            case ExprLangLexer.LESS_EQ:     return " \\leq ";
//            case ExprLangLexer.GREATER:     return " > ";
//            case ExprLangLexer.GREATER_EQ:  return " \\geq ";
//            default:
//                throw new UnsupportedOperationException();
//        }
//    }    
    // Format an unknown id in red and add a new parse error.
    private FormattedFormula unknownId(TerminalNode ID) {
        requireLatexLanguage();
        context.addNewError("Unknown ID '" + ID.getText() + "' at " + ID.getSymbol().getLine()
                + ":" + (ID.getSymbol().getCharPositionInLine() + 1) + ".");
        return new FormattedFormula(lang, true, "\\mathrm{\\textcolor{FF0000}{" + ID.getText() + "}}");
    }

    private void ruleNotAvailableForConstExpr(String whatIs) {
        if ((parseFlags & ParserContext.PF_CONST_EXPR) != 0) {
            context.addNewError("Could not specify a " + whatIs + " in a constant expression.");
        }
    }

    private void ensureSameDomain(FormattedFormula f0, FormattedFormula f1) {
        if (f0.getPayload() != null && f1.getPayload() != null) {
            assert f0.getPayload() instanceof ColorClass;
            assert f1.getPayload() instanceof ColorClass;
            if (f0.getPayload() != f1.getPayload()) {
                context.addNewError("Incompatible types: expression " + f0.getFormula() + " with type "
                        + ((ColorClass) f0.getPayload()).getUniqueName()
                        + " is different from expression" + f1.getFormula() + " with type "
                        + ((ColorClass) f1.getPayload()).getUniqueName());
            }
        }
    }

//    @Override
//    public FormattedFormula visitErrorNode(ErrorNode node) {
//        context.addNewError("Error at '"+node.getText()+"' at "+node.getSymbol().getLine()+
//                            ":"+(node.getSymbol().getCharPositionInLine()+1)+".");
//        return new FormattedFormula(true, "\\mathrm{\\textcolor{FF0000}{"+node.getText()+"}}");
//    }
    //==========================================================================
    //  Entry points:
    //==========================================================================
    private void throwIfIncomplete(TerminalNode EOF, Token stop) {
        if (EOF != null) {
            return;
        }
        throw new ExprLangParserException("Syntax error or incomplete expression"
                + " after line " + stop.getLine()
                + ":" + stop.getCharPositionInLine() + ".");
    }

    @Override
    public FormattedFormula visitMainRealExpr(ExprLangParser.MainRealExprContext ctx) {
        throwIfIncomplete(ctx.EOF(), ctx.getStop());
        return visit(ctx.realExpr());
    }

    @Override
    public FormattedFormula visitMainRealExprList(ExprLangParser.MainRealExprListContext ctx) {
        throwIfIncomplete(ctx.EOF(), ctx.getStop());
        FormattedFormula ff = visit(ctx.realExpr(0));
        Iterator<ExprLangParser.RealExprContext> it = ctx.realExpr().iterator();
        if (it.hasNext()) {
            it.next();
        }
        while (it.hasNext()) {
            ff = format(false, ff, ", ", visit(it.next()));
        }
        return ff;
    }

    @Override
    public FormattedFormula visitMainBoolExpr(ExprLangParser.MainBoolExprContext ctx) {
        throwIfIncomplete(ctx.EOF(), ctx.getStop());
        return visit(ctx.boolExpr());
    }

    @Override
    public FormattedFormula visitMainIntExpr(ExprLangParser.MainIntExprContext ctx) {
        throwIfIncomplete(ctx.EOF(), ctx.getStop());
        return visit(ctx.intExpr());
    }

    @Override
    public FormattedFormula visitMainIntExprList(ExprLangParser.MainIntExprListContext ctx) {
        throwIfIncomplete(ctx.EOF(), ctx.getStop());
        FormattedFormula ff = visit(ctx.intExpr(0));
        Iterator<ExprLangParser.IntExprContext> it = ctx.intExpr().iterator();
        if (it.hasNext()) {
            it.next();
        }
        while (it.hasNext()) {
            ff = format(false, ff, ", ", visit(it.next()));
        }
        return ff;
    }

    @Override
    public FormattedFormula visitMainIntOrInfiniteExpr(ExprLangParser.MainIntOrInfiniteExprContext ctx) {
        throwIfIncomplete(ctx.EOF(), ctx.getStop());
        if (ctx.INFINITY() != null) {
            switch (lang) {
                case LATEX:
                    return new FormattedFormula(lang, true, "\\infty");
                case PNPRO:
                    return new FormattedFormula(lang, true, "Infinite");
                case GRML:
                    return new FormattedFormula(lang, true, "inf");
                default:
                    throw new UnsupportedOperationException();
            }
        }
        return visit(ctx.intExpr());
    }

    @Override
    public FormattedFormula visitMainClockDef(ExprLangParser.MainClockDefContext ctx) {
        requireLatexLanguage();
        return new FormattedFormula(lang, true, "\\mathrm{clock}");
    }

    @Override
    public FormattedFormula visitMainActSet(ExprLangParser.MainActSetContext ctx) {
        throwIfIncomplete(ctx.EOF(), ctx.getStop());
        return visit(ctx.actSet());
    }

    @Override
    public FormattedFormula visitMainActBinding(ExprLangParser.MainActBindingContext ctx) {
        throwIfIncomplete(ctx.EOF(), ctx.getStop());
        return visit(ctx.actBinding());
    }

    @Override
    public FormattedFormula visitMainVarFlow(ExprLangParser.MainVarFlowContext ctx) {
        throwIfIncomplete(ctx.EOF(), ctx.getStop());
        return visit(ctx.varFlowList());
    }

    @Override
    public FormattedFormula visitMainStatePropExpr(ExprLangParser.MainStatePropExprContext ctx) {
        throwIfIncomplete(ctx.EOF(), ctx.getStop());
        return visit(ctx.boolExpr());
//        return format(false, "\\mathbf{", visit(ctx.boolExpr()), "}");
    }

    @Override
    public FormattedFormula visitMainClockConstr(ExprLangParser.MainClockConstrContext ctx) {
        throwIfIncomplete(ctx.EOF(), ctx.getStop());
        if (ctx.varUpdateList() == null) // No update set
        {
            return visit(ctx.clockConstr());
        }
        switch (lang) {
            case LATEX:
                return format(true, "\\begin{array}{c} \\raisebox{-5pt}{",
                        visit(ctx.clockConstr()), "} \\\\ \\{",
                        visit(ctx.varUpdateList()), "\\}\\end{array}");
            default:
                throw new UnsupportedOperationException();
        }
    }

    @Override
    public FormattedFormula visitMainCsltaExpr(ExprLangParser.MainCsltaExprContext ctx) {
        throwIfIncomplete(ctx.EOF(), ctx.getStop());
        return visit(ctx.csltaExpr());
    }

    @Override
    public FormattedFormula visitMainMeasure(ExprLangParser.MainMeasureContext ctx) {
        throwIfIncomplete(ctx.EOF(), ctx.getStop());
        return visit(ctx.measure());
    }

    @Override
    public FormattedFormula visitMainColorClassDef(ExprLangParser.MainColorClassDefContext ctx) {
        throwIfIncomplete(ctx.EOF(), ctx.getStop());
        return visit(ctx.colorClassDef());
    }

    @Override
    public FormattedFormula visitMainColorVarDef(ExprLangParser.MainColorVarDefContext ctx) {
        throwIfIncomplete(ctx.EOF(), ctx.getStop());
        return visit(ctx.colorVarDef());
    }

    @Override
    public FormattedFormula visitMainPlaceColorDomainDef(ExprLangParser.MainPlaceColorDomainDefContext ctx) {
        throwIfIncomplete(ctx.EOF(), ctx.getStop());
        return visit(ctx.placeColorDomainDef());
    }

    @Override
    public FormattedFormula visitMainIntMSetExpr(ExprLangParser.MainIntMSetExprContext ctx) {
        throwIfIncomplete(ctx.EOF(), ctx.getStop());
        return visit(ctx.intMSetExpr());
    }

    @Override
    public FormattedFormula visitMainRealMSetExpr(ExprLangParser.MainRealMSetExprContext ctx) {
        throwIfIncomplete(ctx.EOF(), ctx.getStop());
        return visit(ctx.realMSetExpr());
    }

    @Override
    public FormattedFormula visitMainTagRewriteList(ExprLangParser.MainTagRewriteListContext ctx) {
        throwIfIncomplete(ctx.EOF(), ctx.getStop());
        return visit(ctx.tagRewriteList());
    }

    //==========================================================================
    //  Integer expressions:
    //==========================================================================
    @Override
    public FormattedFormula visitIntConstLiteral(ExprLangParser.IntConstLiteralContext ctx) {
        switch (lang) {
            case GRML:
                return format(true, "<attribute name=\"intValue\">" + ctx.INT().getText() + "</attribute>");
            case APNN:
            case LATEX:
            case GREATSPN:
            case PNPRO:
                return format(true, ctx.INT().getText());
            case PNML:
                return format(true, "<numberconstant value=\"" + ctx.INT().getText() + "\">"
                        + "<positive/></numberconstant>");
            case CPP:
                return format(true, ctx.INT().getText());
            default:
                throw new UnsupportedOperationException();
        }
    }

    @Override
    public FormattedFormula visitIntConstId(ExprLangParser.IntConstIdContext ctx) {
        ConstantID intConstId = (ConstantID) context.getNodeByUniqueName(ctx.INT_CONST_ID().getText());
        switch (lang) {
            case GRML:
                return format(true, "<attribute name=\"name\">" + intConstId.getUniqueName() + "</attribute>");
            case APNN:
            case LATEX:
            case GREATSPN:
            case PNPRO:
            case CPP:
                return format(true, intConstId);
            default:
                throw new UnsupportedOperationException();
        }
    }

    @Override
    public FormattedFormula visitIntTemplateId(ExprLangParser.IntTemplateIdContext ctx) {
        TemplateVariable intVarId = (TemplateVariable) context.getNodeByUniqueName(ctx.INT_TEMPLATE_ID().getText());
        switch (lang) {
            case GRML:
                return format(true, "<attribute name=\"name\">" + intVarId.getUniqueName() + "</attribute>");
            case APNN:
            case LATEX:
            case GREATSPN:
            case PNPRO:
                return format(true, intVarId);
            default:
                throw new UnsupportedOperationException();
        }
    }

    @Override
    public FormattedFormula visitIntExprPlaceMarking(ExprLangParser.IntExprPlaceMarkingContext ctx) {
        ruleNotAvailableForConstExpr("place marking");
        Place place = (Place) context.getNodeByUniqueName(ctx.INT_PLACE_ID().getText());
        switch (lang) { // same for visitRealExprPlaceMarking
            case LATEX:
                return format(true, "\\#", place.getUniqueNameDecor().getVisualizedValue());
            case GREATSPN:
            case PNPRO:
                return format(true, "#" + ctx.INT_PLACE_ID().getText());
            case APNN:
                return format(true, ctx.INT_PLACE_ID().getText());
            case GRML:
                return format(true, "<attribute name=\"name\">" + place.getUniqueName() + "</attribute>");
            case CPP:
                return format(true, "Value[" + ctx.INT_PLACE_ID().getText() + "_place]");
            default:
                throw new UnsupportedOperationException("visitIntExprPlaceMarking");
        }
    }

    @Override
    public FormattedFormula visitIntExprParen(ExprLangParser.IntExprParenContext ctx) {
        return formatUnaryFn(ExprLangParser.OP_PAREN, visit(ctx.intExpr()));
    }

    @Override
    public FormattedFormula visitIntExprAddSub(ExprLangParser.IntExprAddSubContext ctx) {
        return formatBinaryFn(ctx.op.getType(), visit(ctx.intExpr(0)), visit(ctx.intExpr(1)));
    }

    @Override
    public FormattedFormula visitIntExprConst(ExprLangParser.IntExprConstContext ctx) {
        return visit(ctx.intConst());
    }

    @Override
    public FormattedFormula visitIntExprUnaryRealFn(ExprLangParser.IntExprUnaryRealFnContext ctx) {
        return formatUnaryFn(ctx.unaryIntRealFn().fn.getType(), visit(ctx.realExpr()));
    }

    @Override
    public FormattedFormula visitIntExprCond(ExprLangParser.IntExprCondContext ctx) {
        return formatIfThenElse(true, visit(ctx.boolExpr()), visit(ctx.intExpr(0)), visit(ctx.intExpr(1)));
    }

    @Override
    public FormattedFormula visitIntExprCond2(ExprLangParser.IntExprCond2Context ctx) {
        return formatIfThenElse(false, visit(ctx.boolExpr()), visit(ctx.intExpr(0)), visit(ctx.intExpr(1)));
    }

    @Override
    public FormattedFormula visitIntExprBinaryFn(ExprLangParser.IntExprBinaryFnContext ctx) {
        return formatBinaryFn(ctx.binaryIntFn().fn.getType(), visit(ctx.intExpr(0)), visit(ctx.intExpr(1)));
    }

    @Override
    public FormattedFormula visitIntExprUnaryFn(ExprLangParser.IntExprUnaryFnContext ctx) {
        return formatUnaryFn(ctx.unaryIntFn().fn.getType(), visit(ctx.intExpr()));
    }

    @Override
    public FormattedFormula visitIntExprNegate(ExprLangParser.IntExprNegateContext ctx) {
        return formatUnaryFn(ExprLangParser.SUB, visit(ctx.intExpr()));
    }

    @Override
    public FormattedFormula visitIntExprMulDiv(ExprLangParser.IntExprMulDivContext ctx) {
        return formatBinaryFn(ctx.op.getType(), visit(ctx.intExpr(0)), visit(ctx.intExpr(1)));
    }

    @Override
    public FormattedFormula visitIntExprCardMSet(ExprLangParser.IntExprCardMSetContext ctx) {
        ColorClass fixedDomain = context.colorDomainOfExpr;
        try {
            context.colorDomainOfExpr = null;
            return formatUnaryFn(ExprLangParser.MULTISET_CARD, visit(ctx.intMSetExpr()));
        } finally {
            context.colorDomainOfExpr = fixedDomain;
        }
    }

    @Override
    public FormattedFormula visitIntExprColorNum(ExprLangParser.IntExprColorNumContext ctx) {
        ColorClass fixedDomain = context.colorDomainOfExpr;
        try {
            context.colorDomainOfExpr = null;
            return formatUnaryFn(ExprLangParser.COLOR_ORDINAL, visit(ctx.colorTerm()));
        } finally {
            context.colorDomainOfExpr = fixedDomain;
        }
    }

    @Override
    public FormattedFormula visitIntExprCTLBound(ExprLangParser.IntExprCTLBoundContext ctx) {
        StringBuilder placeList = new StringBuilder();
        for (TerminalNode place : ctx.INT_PLACE_ID()) {
            if (placeList.length() > 0) {
                placeList.append(", ");
            }
            placeList.append(visit(place).getFormula());
        }
        switch (lang) {
            case LATEX:
                return format(true, "\\mathrm{bounds}(", placeList.toString(), ")");
            case GREATSPN:
                return format(true, "\\mathrm{bounds}(", placeList.toString(), ")");
            default:
                throw new UnsupportedOperationException();
        }
    }

    @Override
    public FormattedFormula visitIntExprUnknownId(ExprLangParser.IntExprUnknownIdContext ctx) {
        requireLatexLanguage();
        return unknownId(ctx.ID());
    }

    @Override
    public FormattedFormula visitIntExprUnknownPlaceId(ExprLangParser.IntExprUnknownPlaceIdContext ctx) {
        requireLatexLanguage();
        return format(true, "\\#", unknownId(ctx.ID()));
    }

    //==========================================================================
    //  Real expressions:
    //==========================================================================
    @Override
    public FormattedFormula visitRealConstLiteral(ExprLangParser.RealConstLiteralContext ctx) {
        switch (lang) {
            case GRML:
                return format(true, "<attribute name=\"numValue\">" + ctx.REAL().getText() + "</attribute>");
            case APNN:
            case LATEX:
            case GREATSPN:
            case PNPRO:
            case CPP:
                return format(true, ctx.REAL().getText());
            default:
                throw new UnsupportedOperationException();
        }

    }

    @Override
    public FormattedFormula visitRealConstId(ExprLangParser.RealConstIdContext ctx) {
        ConstantID realConstId = (ConstantID) context.getNodeByUniqueName(ctx.REAL_CONST_ID().getText());
        switch (lang) {
            case GRML:
                return format(true, "<attribute name=\"name\">" + realConstId.getUniqueName() + "</attribute>");
            case APNN:
            case LATEX:
            case GREATSPN:
            case PNPRO:
            case NETLOGO:
            case CPP:
                return format(true, realConstId);
            default:
                throw new UnsupportedOperationException();
        }

    }

    @Override
    public FormattedFormula visitRealTemplateId(ExprLangParser.RealTemplateIdContext ctx) {
        TemplateVariable realVarId = (TemplateVariable) context.getNodeByUniqueName(ctx.REAL_TEMPLATE_ID().getText());
        switch (lang) {
            case GRML:
                return format(true, "<attribute name=\"name\">" + realVarId.getUniqueName() + "</attribute>");
            case APNN:
            case LATEX:
            case GREATSPN:
            case PNPRO:
            case CPP:
                return format(true, realVarId);
            default:
                throw new UnsupportedOperationException();
        }

    }

    @Override
    public FormattedFormula visitRealExprPlaceMarking(ExprLangParser.RealExprPlaceMarkingContext ctx) {
        ruleNotAvailableForConstExpr("place marking");
        Place place = (Place) context.getNodeByUniqueName(ctx.REAL_PLACE_ID().getText());
        switch (lang) { // same for visitIntExprPlaceMarking
            case LATEX:
                return format(true, "\\#", place.getUniqueNameDecor().getVisualizedValue());
            case GREATSPN:
            case PNPRO:
                return format(true, "#" + ctx.REAL_PLACE_ID().getText());
            case APNN:
                return format(true, ctx.REAL_PLACE_ID().getText());
            case GRML:
                return format(true, "<attribute name=\"name\">" + place.getUniqueName() + "</attribute>");
            case CPP:
                return format(true, "Value[" + ctx.REAL_PLACE_ID().getText() + "_place]");
            default:
                throw new UnsupportedOperationException("visitRealExprPlaceMarking");
        }
    }

    @Override
    public FormattedFormula visitRealExprNegate(ExprLangParser.RealExprNegateContext ctx) {
        return formatUnaryFn(ExprLangParser.SUB, visit(ctx.realExpr()));
    }

    @Override
    public FormattedFormula visitRealExprBinaryFn(ExprLangParser.RealExprBinaryFnContext ctx) {
        int fn;
        if (ctx.binaryIntFn() != null) {
            fn = ctx.binaryIntFn().fn.getType();
        } else if (ctx.binaryRealFn() != null) {
            fn = ctx.binaryRealFn().fn.getType();
        } else {
            throw new UnsupportedOperationException();
        }

        return formatBinaryFn(fn, visit(ctx.realExpr(0)), visit(ctx.realExpr(1)));
    }

    @Override
    public FormattedFormula visitRealExprAddSub(ExprLangParser.RealExprAddSubContext ctx) {
        return formatBinaryFn(ctx.op.getType(), visit(ctx.realExpr(0)), visit(ctx.realExpr(1)));
    }

    @Override
    public FormattedFormula visitRealExprInt(ExprLangParser.RealExprIntContext ctx) {
        return visit(ctx.intExpr());
    }

    @Override
    public FormattedFormula visitRealExprUnaryFn(ExprLangParser.RealExprUnaryFnContext ctx) {
        int fn;
        if (ctx.unaryIntFn() != null) {
            fn = ctx.unaryIntFn().fn.getType();
        } else if (ctx.unaryRealFn() != null) {
            fn = ctx.unaryRealFn().fn.getType();
        } else {
            throw new UnsupportedOperationException();
        }
        return formatUnaryFn(fn, visit(ctx.realExpr()));
    }

    @Override
    public FormattedFormula visitRealFromIntConst(ExprLangParser.RealFromIntConstContext ctx) {
        return visit(ctx.intConst());
    }

    @Override
    public FormattedFormula visitRealExprMulDiv(ExprLangParser.RealExprMulDivContext ctx) {
        return formatBinaryFn(ctx.op.getType(), visit(ctx.realExpr(0)), visit(ctx.realExpr(1)));
    }

    @Override
    public FormattedFormula visitRealExprParen(ExprLangParser.RealExprParenContext ctx) {
        return formatUnaryFn(ExprLangParser.OP_PAREN, visit(ctx.realExpr()));
    }

    @Override
    public FormattedFormula visitRealExprConst(ExprLangParser.RealExprConstContext ctx) {
        return visit(ctx.realConst());
    }

    @Override
    public FormattedFormula visitRealExprCond(ExprLangParser.RealExprCondContext ctx) {
        return formatIfThenElse(true, visit(ctx.boolExpr()), visit(ctx.realExpr(0)), visit(ctx.realExpr(1)));
    }

    @Override
    public FormattedFormula visitRealExprCond2(ExprLangParser.RealExprCond2Context ctx) {
        return formatIfThenElse(false, visit(ctx.boolExpr()), visit(ctx.realExpr(0)), visit(ctx.realExpr(1)));
    }

    @Override
    public FormattedFormula visitRealExprPdfXVar(ExprLangParser.RealExprPdfXVarContext ctx) {
        return new FormattedFormula(lang, true, ctx.getText());
    }

    @Override
    public FormattedFormula visitRealExprClockVar(ExprLangParser.RealExprClockVarContext ctx) {
        return visit(ctx.clockVar());
    }

    @Override
    public FormattedFormula visitRealExprUnknownId(ExprLangParser.RealExprUnknownIdContext ctx) {
        return unknownId(ctx.ID());
    }

    @Override
    public FormattedFormula visitRealExprFromList(ExprLangParser.RealExprFromListContext ctx) {
        switch (lang) {
            case LATEX:
                return format(true, "\\mathbf{FromList}[", ctx.STRING_LITERAL(), ", ", visit(ctx.intExpr()), "]");
            case PNPRO:
                return format(true, "FromList[", ctx.STRING_LITERAL(), ", ", visit(ctx.intExpr()), "]");
            case GREATSPN:
                return format(true, "FromList[", ctx.STRING_LITERAL(), ", ", visit(ctx.intExpr()), "]");
            case CPP:
                String file_name = ctx.STRING_LITERAL().toString();
                String define_name = file_name.substring(1, file_name.length() - 1).replace(".", "_");
                return format(true, "class_files[", define_name, "].getConstantFromList(", visit(ctx.intExpr()), ")");
            default:
                throw new UnsupportedOperationException();
        }
    }

    @Override
    public FormattedFormula visitRealExprFromTable(ExprLangParser.RealExprFromTableContext ctx) {
        switch (lang) {
            case LATEX:
                return format(true, "\\mathbf{FromTable}[", ctx.STRING_LITERAL(), ", ", visit(ctx.intExpr(0)), ", ", visit(ctx.intExpr(1)), "]");
            case PNPRO:
                return format(true, "FromTable[", ctx.STRING_LITERAL(), ", ", visit(ctx.intExpr(0)), ", ", visit(ctx.intExpr(1)), "]");
            case GREATSPN:
                return format(true, "FromTable[", ctx.STRING_LITERAL(), ", ", visit(ctx.intExpr(0)), ", ", visit(ctx.intExpr(1)), "]");
            case CPP:
                String file_name = ctx.STRING_LITERAL().toString();
                String define_name = file_name.substring(1, file_name.length() - 1).replace(".", "_");
                return format(true, "class_files[", define_name, "].getConstantFromTable(", visit(ctx.intExpr(0)), ",", visit(ctx.intExpr(1)), ")");
            default:
                throw new UnsupportedOperationException();
        }
    }

    @Override
    public FormattedFormula visitRealExprFromTimeTable(ExprLangParser.RealExprFromTimeTableContext ctx) {
        switch (lang) {
            case LATEX:
                return format(true, "\\mathbf{FromTimeTable}[", ctx.STRING_LITERAL(), ", ", visit(ctx.intExpr(0)), ", ", visit(ctx.intExpr(1)), "]");
            case PNPRO:
                return format(true, "FromTimeTable[", ctx.STRING_LITERAL(), ", ", visit(ctx.intExpr(0)), ", ", visit(ctx.intExpr(1)), "]");
            case GREATSPN:
                return format(true, "FromTimeTable[", ctx.STRING_LITERAL(), ", ", visit(ctx.intExpr(0)), ", ", visit(ctx.intExpr(1)), "]");
            case CPP:
                String file_name = ctx.STRING_LITERAL().toString();
                String define_name = file_name.substring(1, file_name.length() - 1).replace(".", "_");
                return format(true, "class_files[", define_name, "].getConstantFromTimeTable(", visit(ctx.intExpr(0)), ",", visit(ctx.intExpr(1)), ")");
            default:
                throw new UnsupportedOperationException();
        }
    }

    @Override
    public FormattedFormula visitRealExprCall(ExprLangParser.RealExprCallContext ctx) {
        switch (lang) {
            case LATEX:
                return format(true, "\\mathbf{Call}[", ctx.STRING_LITERAL(), visit(ctx.intOrRealList()), "]");
            case PNPRO:
                return format(true, "Call[", ctx.STRING_LITERAL(), visit(ctx.intOrRealList()), "]");
            case GREATSPN:
                return format(true, "Call[", ctx.STRING_LITERAL(), visit(ctx.intOrRealList()), "]");
            case CPP:
                String call_name = ctx.STRING_LITERAL().toString();
                String function_name = call_name.substring(1, call_name.length() - 1);
                String arguments = visit(ctx.intOrRealList()).getFormula();
                String arguments_defined;
                if (arguments.isEmpty()) {
                    if (context.cppForFluxBalance) {
                        arguments_defined = "Value, vec_fluxb, NumTrans, NumPlaces, NameTrans, Trans, T, time";

                    } else {
                        arguments_defined = "Value, NumTrans, NumPlaces, NameTrans, Trans, T, time";
                    }
                } else {
                    arguments_defined = arguments.substring(2);
                }
                return format(true, function_name, "(", arguments_defined, ")");
            default:
                throw new UnsupportedOperationException();
        }
    }

    @Override
    public FormattedFormula visitIntOrRealListEmptyList(ExprLangParser.IntOrRealListEmptyListContext ctx) {
        return format(true, "");
    }

    @Override
    public FormattedFormula visitIntOrRealListInt(ExprLangParser.IntOrRealListIntContext ctx) {
        return format(false, ", ", visit(ctx.intExpr()), visit(ctx.intOrRealList()));
    }

    @Override
    public FormattedFormula visitIntOrRealListReal(ExprLangParser.IntOrRealListRealContext ctx) {
        return format(false, ", ", visit(ctx.realExpr()), visit(ctx.intOrRealList()));
    }

    //==========================================================================
    //  Boolean expressions:
    //==========================================================================    
    @Override
    public FormattedFormula visitBoolConst(ExprLangParser.BoolConstContext ctx) {
        boolean value;

        if (ctx.TRUE() != null) {
            value = true;
        } else if (ctx.FALSE() != null) {
            value = false;
        } else {
            throw new UnsupportedOperationException();
        }
        switch (lang) {
            case LATEX:
                return format(true, value ? "\\mathrm{true}" : "\\mathrm{false}");
            case GREATSPN:
                return format(true, value ? "true" : "false");
            case PNPRO:
                return format(true, value ? "True" : "False");
            case APNN:
                return format(true, value ? "true" : "false");
            case GRML:
                return format(true, "<attribute name=\"boolValue\">" + (value ? "true" : "false") + "</attribute>");
            case PNML:
                return format(true, "<numberconstant value=\"" + (value ? "true" : "false") + "\">");
            case NETLOGO:
                return format(true, value ? "TRUE" : "FALSE");
            default:
                throw new UnsupportedOperationException("visitBoolConst");
        }
    }

    @Override
    public FormattedFormula visitBoolExprStatePropId(ExprLangParser.BoolExprStatePropIdContext ctx) {
        TemplateVariable stateProp = (TemplateVariable) context.getNodeByUniqueName(ctx.STATEPROP_ID().getText());
        return format(true, stateProp);
    }

    @Override
    public FormattedFormula visitBoolExprAnd(ExprLangParser.BoolExprAndContext ctx) {
        return formatBinaryFn(ExprLangParser.AND, visit(ctx.boolExpr(0)), visit(ctx.boolExpr(1)));
    }

    @Override
    public FormattedFormula visitBoolExprOr(ExprLangParser.BoolExprOrContext ctx) {
        return formatBinaryFn(ExprLangParser.OR, visit(ctx.boolExpr(0)), visit(ctx.boolExpr(1)));
    }

    @Override
    public FormattedFormula visitBoolExprImply(ExprLangParser.BoolExprImplyContext ctx) {
        return formatBinaryFn(ExprLangParser.IMPLY, visit(ctx.boolExpr(0)), visit(ctx.boolExpr(1)));
    }

    @Override
    public FormattedFormula visitBoolExprBiimply(ExprLangParser.BoolExprBiimplyContext ctx) {
        return formatBinaryFn(ExprLangParser.BIIMPLY, visit(ctx.boolExpr(0)), visit(ctx.boolExpr(1)));
    }

    @Override
    public FormattedFormula visitBoolExprNot(ExprLangParser.BoolExprNotContext ctx) {
        return formatUnaryFn(ExprLangParser.NOT, visit(ctx.boolExpr()));
    }

    @Override
    public FormattedFormula visitBoolExprParen(ExprLangParser.BoolExprParenContext ctx) {
        return formatUnaryFn(ExprLangParser.OP_PAREN, visit(ctx.boolExpr()));
    }

    @Override
    public FormattedFormula visitBoolExprIntComp(ExprLangParser.BoolExprIntCompContext ctx) {
        return formatBinaryFn(ctx.op.getType(), visit(ctx.intExpr(0)), visit(ctx.intExpr(1)));
    }

    @Override
    public FormattedFormula visitBoolExprRealComp(ExprLangParser.BoolExprRealCompContext ctx) {
        return formatBinaryFn(ctx.op.getType(), visit(ctx.realExpr(0)), visit(ctx.realExpr(1)));
    }

    @Override
    public FormattedFormula visitBoolExprConst(ExprLangParser.BoolExprConstContext ctx) {
        return visit(ctx.boolConst());
    }

    @Override
    public FormattedFormula visitBoolExprUnknownId(ExprLangParser.BoolExprUnknownIdContext ctx) {
        return unknownId(ctx.ID());
    }

    @Override
    public FormattedFormula visitBoolExprColorTermComp(ExprLangParser.BoolExprColorTermCompContext ctx) {
        FormattedFormula f0 = visit(ctx.colorTerm(0));
        FormattedFormula f1 = visit(ctx.colorTerm(1));
        MultiSetElemType m0 = (MultiSetElemType) f0.getPayload();
        MultiSetElemType m1 = (MultiSetElemType) f1.getPayload();
        if (m0.colorClass != null && m1.colorClass != null) { // avoid syntactic errors
            boolean sameColorClass = m0.colorClass.equals(m1.colorClass);
            if (!sameColorClass) {
                context.addNewError("Could not compare expression " + ctx.colorTerm(0).getText()
                        + " of color " + m0.colorClass.getUniqueName() + " with expression "
                        + ctx.colorTerm(1).getText() + " of color " + m1.colorClass.getUniqueName() + ".");
            }
            if (sameColorClass) {
                boolean opIsEq = (ctx.op.getType() == ExprLangParser.EQUAL)
                        || (ctx.op.getType() == ExprLangParser.NOT_EQUAL);
                // comparison operators <, <=, >, >= can only be used on enumerative classes
                if (m0.colorClass.isCircular() && !opIsEq && strictColorExpressionChecks) {
                    context.addNewError("Ordered color comparison '" + ctx.getText()
                            + "' cannot be done on terms of a circular class.");
                }
            }
        }
        return formatBinaryFn(ctx.op.getType(), f0, f1);
    }

    @Override
    public FormattedFormula visitBoolExprColorTermIn(ExprLangParser.BoolExprColorTermInContext ctx) {
        FormattedFormula term = visit(ctx.colorTerm());
        FormattedFormula set = visit(ctx.colorSet());
        MultiSetElemType mTerm = (MultiSetElemType) term.getPayload();
        MultiSetElemType mSet = (MultiSetElemType) set.getPayload();
        if (mTerm.colorClass != null && mSet.colorClass != null) { // avoid syntactic errors
            if (!mTerm.colorClass.equals(mSet.colorClass)) {
                context.addNewError("Could not compare expression " + ctx.colorTerm().getText()
                        + " of color " + mTerm.colorClass.getUniqueName() + " with expression "
                        + ctx.colorSet().getText() + " of color " + mSet.colorClass.getUniqueName() + ".");
            }
        }
        boolean contains;
        if (ctx.CONTAINS() != null) {
            contains = true;
        } else if (ctx.DONT_CONTAINS() != null) {
            contains = false;
        } else {
            throw new UnsupportedOperationException();
        }
        return formatBinaryFn(contains ? ExprLangParser.CONTAINS : ExprLangParser.DONT_CONTAINS, term, set);
    }

    // Boolean CTL formulas
    @Override
    public FormattedFormula visitBoolExprCTL(ExprLangParser.BoolExprCTLContext ctx) {
        return format(false, visit(ctx.temporal_op()), " ", visit(ctx.boolExpr()));
    }

    private FormattedFormula formatCTLUntil(boolean isE, FormattedFormula f1, FormattedFormula f2) {
        switch (lang) {
            case LATEX:
                return format(true, (isE ? "\\exists" : "\\forall"),
                        "(", f1, "~\\mathrm{U}~", f2, ")");
            case GREATSPN:
            case PNPRO:
                return format(true, (isE ? "E" : "A"),
                        "[", f1, " U ", f2, "]");
            default:
                throw new UnsupportedOperationException();
        }
    }

    @Override
    public FormattedFormula visitBoolExprCTLUntil(ExprLangParser.BoolExprCTLUntilContext ctx) {
        return formatCTLUntil(ctx.EXISTS() != null, visit(ctx.boolExpr(0)), visit(ctx.boolExpr(1)));
    }

    @Override
    public FormattedFormula visitBoolExprCTLUntil2(ExprLangParser.BoolExprCTLUntil2Context ctx) {
        return formatCTLUntil(ctx.EXISTS() != null, visit(ctx.boolExpr(0)), visit(ctx.boolExpr(1)));
    }

    @Override
    public FormattedFormula visitBoolExprCTLenabled(ExprLangParser.BoolExprCTLenabledContext ctx) {
        switch (lang) {
            case LATEX:
                return format(true, "\\mathrm{en}(", visit(ctx.TRANSITION_ID()), ")");
            case GREATSPN:
            case PNPRO:
                return format(true, "en(", visit(ctx.TRANSITION_ID()), ")");
            default:
                throw new UnsupportedOperationException();
        }
    }

    @Override
    public FormattedFormula visitBoolExprCTLpin(ExprLangParser.BoolExprCTLpinContext ctx) {
        switch (lang) {
            case LATEX:
                return format(false, "\\mathrm{", ctx.pin.getText(), "} ", visit(ctx.boolExpr()).getAsSimpleTerm());
            case GREATSPN:
            case PNPRO:
                return format(false, ctx.pin.getText(), " ", visit(ctx.boolExpr()).getAsSimpleTerm());
            default:
                throw new UnsupportedOperationException();
        }
    }

    @Override
    public FormattedFormula visitBoolExprCTLdeadlocks(ExprLangParser.BoolExprCTLdeadlocksContext ctx) {
        switch (lang) {
            case LATEX:
                return format(true, ctx.DEADLOCK() != null ? "\\mathrm{deadlock}" : "\\mathrm{ndeadlock}");
            case GREATSPN:
            case PNPRO:
                return format(true, ctx.DEADLOCK() != null ? "deadlock" : "ndeadlock");
            default:
                throw new UnsupportedOperationException();
        }
    }

    @Override
    public FormattedFormula visitBoolExprCTLinitState(ExprLangParser.BoolExprCTLinitStateContext ctx) {
        switch (lang) {
            case LATEX:
                return format(true, "\\mathrm{initial}");
            case GREATSPN:
            case PNPRO:
                return format(true, "initial");
            default:
                throw new UnsupportedOperationException();
        }
    }

    //==========================================================================
    //  CTLSTAR terms:
    //==========================================================================
    @Override
    public FormattedFormula visitBoolExprCTLStarQuantif(ExprLangParser.BoolExprCTLStarQuantifContext ctx) {
        return format(true, ctx.q.getText(), " ", visit(ctx.boolExpr()));
    }

    @Override
    public FormattedFormula visitBoolExprCTLStarUntil2(ExprLangParser.BoolExprCTLStarUntil2Context ctx) {
        switch (lang) {
            case LATEX:
                return format(true, "(", visit(ctx.boolExpr(0)), "~\\mathrm{U}~", visit(ctx.boolExpr(1)), ")");
            case GREATSPN:
            case PNPRO:
                return format(true, "[", visit(ctx.boolExpr(0)), " U ", visit(ctx.boolExpr(1)), "]");
            default:
                throw new UnsupportedOperationException();
        }
    }

    @Override
    public FormattedFormula visitBoolExprCTLStarUntil(ExprLangParser.BoolExprCTLStarUntilContext ctx) {
        switch (lang) {
            case LATEX:
                return format(true, "(", visit(ctx.boolExpr(0)), "~\\mathrm{U}~", visit(ctx.boolExpr(1)), ")");
            case GREATSPN:
            case PNPRO:
                return format(true, "[", visit(ctx.boolExpr(0)), " U ", visit(ctx.boolExpr(1)), "]");
            default:
                throw new UnsupportedOperationException();
        }
    }

    @Override
    public FormattedFormula visitBoolExprCTLStar(ExprLangParser.BoolExprCTLStarContext ctx) {
        return format(true, ctx.op.getText(), " ", visit(ctx.boolExpr()));
    }

    @Override
    public FormattedFormula visitBoolExprCTLStar2(ExprLangParser.BoolExprCTLStar2Context ctx) {
        return format(true, visit(ctx.composite_temporal_op_ctlstar()), " ", visit(ctx.boolExpr()));
    }

    //==========================================================================
    //  Multiset terms:
    //==========================================================================
    @Override
    public FormattedFormula visitColorVarIdentifier(ExprLangParser.ColorVarIdentifierContext ctx) {
        ruleNotAvailableForConstExpr("color variable"); // color vars cannot appear in constant multiset expr.
        ColorVar var = (ColorVar) context.getNodeByUniqueName(ctx.COLORVAR_ID().getText());
        // Color variables are recognized only if there is the colorVarsInUse[] set in the context.
        if (var == null/* || context.colorVarsInUse == null*/) {
            return unknownId(ctx.COLORVAR_ID());
        }
        if (context.colorVarsInUse != null) {
            context.colorVarsInUse.add(var);
        }

        return formatPayload(true, new MultiSetElemType(var, context), var);
    }

    @Override
    public FormattedFormula visitColorSubclassIdentifier(ExprLangParser.ColorSubclassIdentifierContext ctx) {
        String id = ctx.COLORSUBCLASS_ID().getText();
        Iterator<ColorClass> ccIt = context.colorClassIterator();
        while (ccIt.hasNext()) {
            ColorClass cc = ccIt.next();
//                System.out.println(cc.getUniqueName()+".testHasStaticSubclassNamed("+id+") = "
//                            +cc.testHasStaticSubclassNamed(id));
            if (cc.isSimpleClass() && cc.testHasStaticSubclassNamed(id)) {
                String altId;
                switch (lang) {
                    case LATEX:
                        altId = AlternateNameFunction.NUMBERS_AS_SUBSCRIPTS
                                .prepareLatexText(id, null, NetObject.STYLE_ROMAN);
                        break;
                    case PNML:
                        throw new UnsupportedOperationException("Static subclasses are not supporeted in PNML.");
                    case NETLOGO:
                        altId = id;
                        break;
                    default:
                        throw new UnsupportedOperationException();
                }
                return formatPayload(true, new MultiSetElemType(cc), altId);
            }
        }
        return unknownId(ctx.COLORSUBCLASS_ID()); // unknown static subclass id
    }

//    @Override
//    public FormattedFormula visitMSetTermColorVar(ExprLangParser.MSetTermColorVarContext ctx) {
//        return visit(ctx.colorVar());
//    }
//
//    @Override
//    public FormattedFormula visitMSetTermColorVarNext(ExprLangParser.MSetTermColorVarNextContext ctx) {
//        FormattedFormula var = visit(ctx.colorVar());
//        return formatUnaryFn(ExprLangParser.POSTINCR, var).addPayload(var.getPayload());
//    }
//
//    @Override
//    public FormattedFormula visitMSetTermColorVarPrev(ExprLangParser.MSetTermColorVarPrevContext ctx) {
//        FormattedFormula var = visit(ctx.colorVar());
//        return formatUnaryFn(ExprLangParser.POSTDECR, var).addPayload(var.getPayload());
//    }
//    @Override
//    public FormattedFormula visitMSetTermColorClass(ExprLangParser.MSetTermColorClassContext ctx) {
//        ColorClass cc = (ColorClass)context.getNodeByUniqueName(ctx.SIMPLECOLORCLASS_ID().getText());
//        return formatPayload(true, new MultiSetElemType(cc)/*payload*/, cc);
//    }
//    @Override
//    public FormattedFormula visitMSetTermSubclass(ExprLangParser.MSetTermSubclassContext ctx) {
//        return visit(ctx.colorSubclass());
//    }
//    @Override
//    public FormattedFormula visitMSetTermColorName(ExprLangParser.MSetTermColorNameContext ctx) {
//        String id = ctx.COLOR_ID().getText();
//        for (Node node : context.page.nodes) {
//            if (node instanceof ColorClass) {
//                ColorClass cc = (ColorClass)node;
//                if (!cc.isSimpleClass())
//                    continue;
//                if (cc.testHasColorNamed(context.page, id)) {
//                    String altId = AlternateNameFunction.NUMBERS_AS_SUBSCRIPTS
//                                    .prepareLatexText(id, null, NetObject.STYLE_ITALIC);
//                    return new FormattedFormula(true, id, altId, new MultiSetElemType(cc));
//                }
//            }
//        }
//        return unknownId(ctx.COLOR_ID()); // unknown color name - could this happen?
//    }
//    @Override
//    public FormattedFormula visitMSetTermSubclass(ExprLangParser.MSetTermSubclassContext ctx) {
////        for (Node node : context.page.nodes) {
////            if (node instanceof ColorClass) {
////                ColorClass cc = (ColorClass)node;
////                if (cc.getValueExpr().isFormattedAndCorrect()) {
////                    ParsedColorClass pcc = cc.getParsedColorClass();if (pcc.isSimpleClass()) {
////                        ParsedColorClassDef def = (ParsedColorClassDef)pcc;
////                        for (ParsedColorSubclass subclass : def.subclasses) {
////                            if (subclass.isNamed() && subclass.name.equals(ctx.COLORSUBCLASS_ID().getText())) {
////                                
////                            }
////                        }
////                    }
////                }
////            }
////        }
//        String text = ctx.COLORSUBCLASS_ID().getText();
//        String altText = AlternateNameFunction.NUMBERS_AS_SUBSCRIPTS.prepareLatexText(text, null, NetObject.STYLE_ROMAN);
//        return new FormattedFormula(true, text, altText);
//    }
//    @Override
//    public FormattedFormula visitMSetTermAll(ExprLangParser.MSetTermAllContext ctx) {
//        return new FormattedFormula(true, "All", "\\text{All}", ALL_ELEM_TYPE);
//    }
//    @Override
//    public FormattedFormula visitMSetElemTerm(ExprLangParser.MSetElemTermContext ctx) {
//        return visit(ctx.multiSetTerm());
//    }
    @Override
    public FormattedFormula visitMSetElemAddSub(ExprLangParser.MSetElemAddSubContext ctx) {
        FormattedFormula e0 = visit(ctx.multiSetElem(0));
        FormattedFormula e1 = visit(ctx.multiSetElem(1));
        MultiSetElemType payload0 = (MultiSetElemType) e0.getPayload();
        MultiSetElemType payload1 = (MultiSetElemType) e1.getPayload();
        MultiSetElemType outPayload = payload0.join(payload1, context);
        if (lang == GRML) {
            if (ctx.op.getType() == ExprLangParser.SUB) {
                throw new UnsupportedOperationException("Subtraction of multiset elements is not supported in GRML format.");
            }
            return formatPayload(true, payload1, e0, e1); // concatenate e0 with e1, with no '+' operator
        }
        return formatBinaryFn(ctx.op.getType(), e0, e1).addPayload(outPayload);
    }

    @Override
    public FormattedFormula visitMSetElemColorTerm(ExprLangParser.MSetElemColorTermContext ctx) {
        return visit(ctx.colorSet());
    }

    //==========================================================================
    //  Color Sets:
    //==========================================================================
    @Override
    public FormattedFormula visitColorSetTerm(ExprLangParser.ColorSetTermContext ctx) {
        return visit(ctx.colorTerm());
    }

    @Override
    public FormattedFormula visitColorSetAll(ExprLangParser.ColorSetAllContext ctx) {
        switch (lang) {
            case LATEX:
                return formatPayload(true, ALL_ELEM_TYPE, "\\text{All}");
            case PNML:
            case GRML:
                return formatPayload(true, ALL_ELEM_TYPE, "ALL_TERM"/* will be changed in visitMultiSetDef */);
            default:
                throw new UnsupportedOperationException("visitColorSetAll");
        }
    }

    @Override
    public FormattedFormula visitColorSetSubclassOf(ExprLangParser.ColorSetSubclassOfContext ctx) {
        FormattedFormula term = visit(ctx.colorTerm());
        return formatUnaryFn(ExprLangParser.MULTISET_SUBCLASS, term).addPayload(term.getPayload());
    }

    @Override
    public FormattedFormula visitColorSetSubclassOf2(ExprLangParser.ColorSetSubclassOf2Context ctx) {
        FormattedFormula term = visit(ctx.colorTerm());
        return formatUnaryFn(ExprLangParser.MULTISET_SUBCLASS, term).addPayload(term.getPayload());
    }

    @Override
    public FormattedFormula visitColorSetSubClass(ExprLangParser.ColorSetSubClassContext ctx) {
        return visit(ctx.colorSubclass());
    }

    @Override
    public FormattedFormula visitColorSetSubClass2(ExprLangParser.ColorSetSubClass2Context ctx) {
        FormattedFormula subCl = visit(ctx.colorSubclass());
        return formatPayload(true, subCl.getPayload(), "\\text{S} ", subCl);
    }

    @Override
    public FormattedFormula visitColorSetClass(ExprLangParser.ColorSetClassContext ctx) {
        ColorClass cc = (ColorClass) context.getNodeByUniqueName(ctx.SIMPLECOLORCLASS_ID().getText());
        switch (lang) {
            case GREATSPN:
            case LATEX:
                return formatPayload(true, new MultiSetElemType(cc)/*payload*/, cc);
            case PNML:
                return formatPayload(true, new MultiSetElemType(cc)/*payload*/,
                        "<all><usersort declaration=\"" + cc.getUniqueName() + "\"/></all>");
            case GRML:
                return formatPayload(true, new MultiSetElemType(cc)/*payload*/,
                        "<attribute name=\"function\"><attribute name=\"all\"><attribute name=\"type\">"
                        + cc.getUniqueName() + "</attribute></attribute></attribute>");
            default:
                throw new UnsupportedOperationException();
        }
    }

    @Override
    public FormattedFormula visitColorSetClass2(ExprLangParser.ColorSetClass2Context ctx) {
        ColorClass cc = (ColorClass) context.getNodeByUniqueName(ctx.SIMPLECOLORCLASS_ID().getText());
        return formatPayload(true, new MultiSetElemType(cc)/*payload*/, "\\text{S} ", cc);
    }

    //==========================================================================
    //  Color Terms:
    //==========================================================================
    @Override
    public FormattedFormula visitColorTermColor(ExprLangParser.ColorTermColorContext ctx) {
        String id = ctx.COLOR_ID().getText();
        Iterator<ColorClass> ccIt = context.colorClassIterator();
        while (ccIt.hasNext()) {
            ColorClass cc = ccIt.next();
            if (!cc.isSimpleClass()) {
                continue;
            }
            if (cc.testHasColorNamed(context, id)) {
                String clrId;
                switch (lang) {
                    case LATEX:
                        clrId = AlternateNameFunction.NUMBERS_AS_SUBSCRIPTS
                                .prepareLatexText(id, null, NetObject.STYLE_ITALIC);
                        break;
                    case PNML:
                        if (cc.isSimpleFiniteIntRange()) {
                            // convert the color name into an index
                            ParsedColorSubclass pcs = cc.getSubclass(0);
                            id = id.substring(pcs.getIntervalPrefix().length());
                            clrId = " <finiteintrangeconstant value=\"" + id + "\">"
                                    + "<finiteintrange start=\"" + pcs.getStartRangeExpr() + "\" "
                                    + "end=\"" + pcs.getEndRangeExpr() + "\"/>"
                                    + "</finiteintrangeconstant>";
                        } else {
                            clrId = "<useroperator declaration=\"" + id + "\"/>";
                        }
                        break;
                    case GRML:
                        clrId = "<attribute name=\"expr\"><attribute name=\"enumConst\"><attribute name=\"type\">"
                                + cc.getUniqueName() + "</attribute><attribute name=\"enumValue\">" + id
                                + "</attribute></attribute></attribute>";
                        break;
                    case NETLOGO:
                        clrId = id;
                        break;
                    default:
                        throw new UnsupportedOperationException();
                }
                return formatPayload(true, new MultiSetElemType(cc), clrId);
            }
        }
        return unknownId(ctx.COLOR_ID()); // unknown color name - could this happen?        
    }

    @Override
    public FormattedFormula visitColorTermVar(ExprLangParser.ColorTermVarContext ctx) {
        return visit(ctx.colorVar());
    }

    @Override
    public FormattedFormula visitColorTermNextPrev(ExprLangParser.ColorTermNextPrevContext ctx) {
        return colorTermNextPrev(visit(ctx.colorTerm()), ctx.colorTerm().getText(), ctx.op.getType());
    }

//    @Override
//    public FormattedFormula visitColorTermNextPrev2(ExprLangParser.ColorTermNextPrev2Context ctx) {
//        int op = (ctx.op.getType() == ExprLangParser.NOT) ? ExprLangParser.POSTINCR 
//                                                          : ExprLangParser.POSTDECR;
//        return colorTermNextPrev(visit(ctx.colorTerm()), ctx.colorTerm().getText(), op);
//    }
    private FormattedFormula colorTermNextPrev(FormattedFormula term, String termText, int op) {
        if (term.getPayload() != null && term.getPayload() instanceof MultiSetElemType) {
            // Check that the class of this term is circular
            MultiSetElemType mset = (MultiSetElemType) term.getPayload();
            if (!mset.colorClass.isCircular() && strictColorExpressionChecks) {
                context.addNewError("Cannot ask next/previous color of term "
                        + termText + " of unordered color class "
                        + mset.colorClass.getUniqueName() + ". Use a circular color class.");
            }
        }
        return formatUnaryFn(op, term).addPayload(term.getPayload());
    }

    @Override
    public FormattedFormula visitColorTermFilterThis(ExprLangParser.ColorTermFilterThisContext ctx) {
        switch (lang) {
            case PNPRO:
            case LATEX:
            case GREATSPN: {
                int index = (ctx.INT() != null ? Integer.parseInt(ctx.INT().getText()) : 0);
                String className = (ctx.SIMPLECOLORCLASS_ID() != null ? ctx.SIMPLECOLORCLASS_ID().getText() : null);
                MultiSetElemType ty = null;

                if (className != null && ctx.INT() == null) {
                    // if only the color class is provided and not an index, the color name
                    // should appear only once in the color domain
                    int colorCount = 0;
                    for (int cc = 0; cc < context.colorDomainOfExpr.getNumClassesInDomain(); cc++) {
                        if (className.equals(context.colorDomainOfExpr.getColorClassName(cc))) {
                            ++colorCount;
                            if (colorCount > 1) {
                                context.addNewError("Color domain " + context.colorDomainOfExpr.getUniqueName()
                                        + " have multiple instances of " + className + ": an index must be specified.");
                                break;
                            }
                        }
                    }
                }
                if (className == null && ctx.INT() == null) {
                    context.addNewError("'@': either a class name or an index in the color domain tuple (or both) must be specified");
                }

                // find the element index in the color tuple
                int found = -1;
                for (int cc = 0, index2 = index; cc < context.colorDomainOfExpr.getNumClassesInDomain(); cc++) {
                    if (className != null && !className.equals(context.colorDomainOfExpr.getColorClassName(cc))) {
                        continue; // not the same class
                    }
                    if (index2 > 0) {
                        --index2;
                        continue; // not the i-th instance
                    }
                    found = cc;
                    break;
                }
                if (found == -1) {
                    if (className != null) {
                        context.addNewError("Color domain " + context.colorDomainOfExpr.getUniqueName()
                                + " does not have " + index + " of color class " + className);
                    } else {
                        context.addNewError("Color domain " + context.colorDomainOfExpr.getUniqueName()
                                + " does not have " + index + " color classes.");
                    }
//                    context.addNewError("Domain index "+index+" must be in range [0, "
//                                        +context.colorDomainOfExpr.getNumClassesInDomain()+").");
                    ty = new MultiSetElemType(ALL_COLORS);
                } else {
                    ty = new MultiSetElemType(context.colorDomainOfExpr.getColorClass(found));
                }
//                if (index < 0 || index >= context.colorDomainOfExpr.getNumClassesInDomain()) {
//                    context.addNewError("Domain index "+index+" must be in range [0, "
//                                        +context.colorDomainOfExpr.getNumClassesInDomain()+").");
//                    ty = new MultiSetElemType(ALL_COLORS);
//                }
//                else {
//                    ty = new MultiSetElemType(context.colorDomainOfExpr.getColorClass(index));
//                }

                FormattedFormula f = format(true, '@');
                if (ctx.SIMPLECOLORCLASS_ID() != null) {
                    f = format(true, f, className);
                }
                if (ctx.INT() != null) {
                    f = format(true, f, "[", index, "]");
                }
                return f.addPayload(ty);
            }
            default:
                throw new UnsupportedOperationException("Multiset Filter Predicates are not supported");
        }
    }

    //==========================================================================
    //  Payloads of multiset expressions:
    //==========================================================================
    private static class AllColorsObj {
    }
    private static final AllColorsObj ALL_COLORS = new AllColorsObj();

    static class MultiSetElemType implements FormulaPayload {

        public final ColorClass colorClass;

        public MultiSetElemType(AllColorsObj a) {
            this.colorClass = null;
        }

        public MultiSetElemType(ColorClass colorClass) {
            this.colorClass = colorClass;
        }

        public MultiSetElemType(ColorVar colorVar, NodeNamespace nspace) {
            this.colorClass = colorVar.findColorClass(nspace);
        }

        public MultiSetElemType join(MultiSetElemType elemType, ParserContext context) {
            if (this == ALL_ELEM_TYPE) {
                return elemType;
            }
            if (elemType == ALL_ELEM_TYPE) {
                return this;
            }

            if (colorClass != elemType.colorClass) {
                context.addNewError("Color class mismatch: could not add/subtract "
                        + "colors of two separate color classes " + colorClass.getUniqueName()
                        + " and " + elemType.colorClass.getUniqueName());
                return this;
            }
            return new MultiSetElemType(colorClass);
        }
    }
    private static final MultiSetElemType ALL_ELEM_TYPE = new MultiSetElemType(ALL_COLORS);

    //==========================================================================
    //  Multiset boolean predicates:
    //==========================================================================
    @Override
    public FormattedFormula visitMSetBoolPredicate(ExprLangParser.MSetBoolPredicateContext ctx) {
        switch (lang) {
            case PNPRO:
            case LATEX:
            case GREATSPN:
                return format(true, "[", visit(ctx.boolExpr()), "]");

            default:
                throw new UnsupportedOperationException("MultiSet predicates are not supported.");
        }
    }

    @Override
    public FormattedFormula visitMSetElemBoolPredicate(ExprLangParser.MSetElemBoolPredicateContext ctx) {
        switch (lang) {
            case PNPRO:
            case LATEX:
            case GREATSPN:
                return format(true, "\\bigl[", visit(ctx.boolExpr()), "\\bigr]");

            default:
                throw new UnsupportedOperationException("MultiSet filter predicates are not supported.");
        }
    }

    //==========================================================================
    //  Integer multiset expression:
    //==========================================================================
    public FormattedFormula visitMultiSetDef(ParseTree mult, ParseTree pred,
            List<ExprLangParser.MultiSetElemContext> msetElemList,
            ParseTree filterPred) {
        final ColorClass fixedDomain = context.colorDomainOfExpr;
        StringBuilder buffer = new StringBuilder();

        // Multiplicity & beginning of tuple
        switch (lang) {
            case LATEX:
                if (mult != null) {
                    FormattedFormula multFF = visit(mult);
                    buffer.append(multFF.getFormula()).append(" \\cdot ");
                }
                if (pred != null) {
                    FormattedFormula predFF = visit(pred);
                    buffer.append(predFF.getFormula());
                }
                buffer.append("\\langle ");
                break;
            case PNML:
                if (mult != null) {
                    buffer.append("<numberof><subterm>");
                    buffer.append(visit(mult).getFormula());
                    buffer.append("</subterm><subterm>");
                }
                if (pred != null) {
                    context.addNewError("Multiset boolean predicates are not supported in PNML.");
                }
                buffer.append("<tuple>");
                break;
            case GRML:
                buffer.append("<attribute name=\"token\">");
                buffer.append("<attribute name=\"occurs\">");
                if (mult != null) {
                    buffer.append(visit(mult).getFormula());
                } else {
                    buffer.append("<attribute name=\"intValue\">1</attribute>");
                }
                buffer.append("</attribute><attribute name=\"tokenProfile\">");
                break;
            case NETLOGO:
                if (mult != null) {
                    throw new UnsupportedOperationException("Tuple multiplicity is not supported!");
                }
                buffer.append("<");
                break;
            default:
                throw new UnsupportedOperationException("visitMultiSetDef");
        }
        int count = 0;
        MultiSetElemType[] elemTypes = new MultiSetElemType[msetElemList.size()];
        for (ParserRuleContext node : msetElemList) {
            FormattedFormula msetElem = visit(node);
            MultiSetElemType elemType = (MultiSetElemType) msetElem.getPayload();
            elemTypes[count] = elemType;

            ColorClass expectedClass = null;
            if (fixedDomain != null) {
                // Verify that we are respecting the fixed domain
                if (count >= fixedDomain.getNumClassesInDomain()) {
                    context.addNewError("Unexpected entry " + msetElem.getFormula() + ", domain "
                            + fixedDomain.getUniqueName()
                            + " has " + fixedDomain.getNumClassesInDomain() + " color classes.");
                } else {
                    if (elemType != null) {
                        /*could be null in case of unknown id */
                        if (fixedDomain.isParseDataOk()) {
                            if (fixedDomain.isSimpleClass()) {
                                expectedClass = fixedDomain;
                            } else if (fixedDomain.getColorClassName(count) != null) {
                                Node ccNode = context.getNodeByUniqueName(fixedDomain.getColorClassName(count));
                                if (ccNode != null && ccNode instanceof ColorClass) {
                                    expectedClass = (ColorClass) ccNode;
                                }
                            }
                        }
                        if (elemType != ALL_ELEM_TYPE
                                && /* ALL term does not declare its type */ elemType.colorClass != null
                                && /* Could be a color var with a wrong color domain */ expectedClass != null
                                && /* wrong domain definition */ elemType.colorClass != expectedClass) {
                            context.addNewError("Entry " + msetElem.getFormula() + " should be of color "
                                    + fixedDomain.getColorClassName(count)
                                    + " instead of " + elemType.colorClass.getUniqueName());
                        }
                    }
                }
            }
            switch (lang) {
                case LATEX:
                case NETLOGO:
                    if (count > 0) {
                        buffer.append(", ");
                    }
                    buffer.append(msetElem.getFormula());
                    break;
                case PNML: {
                    String subterm = msetElem.getFormula();
                    subterm = subterm.replaceAll("ALL_TERM", "<all><usersort declaration=\""
                            + expectedClass.getUniqueName() + "\"/></all>");
                    buffer.append("<subterm>").append(subterm).append("</subterm>");
//                        if (elemType == ALL_ELEM_TYPE) {
//                            if (expectedClass == null)
//                                throw new UnsupportedOperationException("Cannot determine class of ALL term.");
//                            buffer.append("<subterm><all><usersort declaration=\"")
//                                    .append(expectedClass.getUniqueName())
//                                    .append("\"/></all></subterm>");
//                        }
//                        else {
//                            buffer.append("<subterm>").append(msetElem.getFormula()).append("</subterm>");
//                        }
                }
                break;
                case GRML: {
                    String subterm = msetElem.getFormula();
                    subterm = subterm.replaceAll("ALL_TERM",
                            "<attribute name=\"function\"><attribute name=\"all\"><attribute name=\"type\">"
                            + expectedClass.getUniqueName() + "</attribute></attribute></attribute>");
                    buffer.append("<attribute name=\"expr\">").append(subterm).append("</attribute>");
                }
                break;
                default:
                    throw new UnsupportedOperationException();
            }
            count++;
        }
        // End of tuple
        switch (lang) {
            case LATEX:
                buffer.append("\\rangle ");
                if (filterPred != null) {
                    FormattedFormula predFF = visit(filterPred);
                    buffer.append(predFF.getFormula());
                }
                break;
            case PNML:
                buffer.append("</tuple>");
                if (mult != null) {
                    buffer.append("</subterm></numberof>");
                }
                if (filterPred != null) {
                    context.addNewError("Multiset filter predicates are not supported in PNML.");
                }
                break;
            case GRML:
                buffer.append("</attribute></attribute>"); // tokenProfile, token
                break;
            case NETLOGO:
                buffer.append(">");
                break;
            default:
                throw new UnsupportedOperationException();
        }

        ColorClass resultDomain = null;
        if (fixedDomain != null) {
            if (count != fixedDomain.getNumClassesInDomain()) {
                context.addNewError("Multiset element of domain " + fixedDomain.getUniqueName()
                        + " should have " + fixedDomain.getNumClassesInDomain() + " entry(s), found " + count);
            }
            resultDomain = fixedDomain;
        } else {
            // Search for a valid color domain.
            Iterator<ColorClass> ccIt = context.colorClassIterator();
            while (ccIt.hasNext()) {
                ColorClass cc = ccIt.next();
                if (cc.isParseDataOk() && cc.getNumClassesInDomain() == elemTypes.length) {
                    boolean matches = true;
                    for (int i = 0; i < elemTypes.length; i++) {
                        if (elemTypes[i].colorClass != cc.getColorClass(i)) {
                            matches = false;
                            break;
                        }
                    }
                    if (matches) {
                        resultDomain = cc;
                        break;
                    }
                }
            }
            if (resultDomain == null) {
                context.addNewError("Multiset expression " + buffer.toString() + " is not in a defined color domain.");
            }
        }
        return formatPayload(false, resultDomain, buffer.toString());
    }

    @Override
    public FormattedFormula visitIntMSetExprParen(ExprLangParser.IntMSetExprParenContext ctx) {
        return formatUnaryFn(ExprLangParser.OP_PAREN, visit(ctx.intMSetExpr()));
    }

    @Override
    public FormattedFormula visitIntMSetExprAddSub(ExprLangParser.IntMSetExprAddSubContext ctx) {
        FormattedFormula e0 = visit(ctx.intMSetExpr(0)), e1 = visit(ctx.intMSetExpr(1));
        ensureSameDomain(e0, e1);
        if (lang == GRML) {
            if (ctx.op.getType() == ExprLangParser.SUB) {
                throw new UnsupportedOperationException("Subtraction of multisets is not supported in GRML format.");
            }
            return format(true, e0, e1); // concatenate e0 with e1, with no '+' operator
        }
        return formatBinaryFn(ctx.op.getType(), e0, e1);
    }

    @Override
    public FormattedFormula visitIntMSetExprElemProduct(ExprLangParser.IntMSetExprElemProductContext ctx) {
        return visitMultiSetDef(ctx.intExpr(), ctx.mSetPredicate(), ctx.multiSetElem(), ctx.mSetElemPredicate());
    }

    @Override
    public FormattedFormula visitIntMsetExprPlaceMarking(ExprLangParser.IntMsetExprPlaceMarkingContext ctx) {
        ruleNotAvailableForConstExpr("place marking");
        final ColorClass fixedDomain = context.colorDomainOfExpr;
        Place place = (Place) context.getNodeByUniqueName(ctx.INT_MSET_PLACE_ID().getText());
        if (fixedDomain != null && !place.getColorDomainName().equals(fixedDomain.getUniqueName())) {
            context.addNewError("Place " + place.getUniqueName() + " is in domain "
                    + place.getColorDomainName()
                    + ", expected expression in domain " + fixedDomain.getUniqueName());
        }
        switch (lang) { // same for visitRealMsetExprPlaceMarking
            case LATEX:
                return format(true, "\\#", place.getUniqueNameDecor().getVisualizedValue());

            case GREATSPN:
            case PNPRO:
                return format(true, "#" + ctx.INT_MSET_PLACE_ID().getText());

//            case APNN:
//                return format(true, ctx.INT_MSET_PLACE_ID().getText());
            default:
                throw new UnsupportedOperationException("visitIntMsetExprPlaceMarking");
        }
    }

    @Override
    public FormattedFormula visitIntMSetExprConst(ExprLangParser.IntMSetExprConstContext ctx) {
        final ColorClass fixedDomain = context.colorDomainOfExpr;
        ConstantID intConstId = (ConstantID) context.getNodeByUniqueName(ctx.INT_MSET_CONST_ID().getText());
        if (fixedDomain != null && !intConstId.getColorDomainName().equals(fixedDomain.getUniqueName())) {
            context.addNewError("Constant " + intConstId.getUniqueName() + " is in domain "
                    + intConstId.getColorDomainName()
                    + ", expected expression in domain " + fixedDomain.getUniqueName());
        }
        return format(true, intConstId);
    }

    //==========================================================================
    //  Real multiset expression:
    //==========================================================================
    @Override
    public FormattedFormula visitRealMSetExprParen(ExprLangParser.RealMSetExprParenContext ctx) {
        return formatUnaryFn(ExprLangParser.OP_PAREN, visit(ctx.realMSetExpr()));
    }

    @Override
    public FormattedFormula visitRealMSetExprAddSub(ExprLangParser.RealMSetExprAddSubContext ctx) {
        FormattedFormula e0 = visit(ctx.realMSetExpr(0)), e1 = visit(ctx.realMSetExpr(1));
        ensureSameDomain(e0, e1);
        return formatBinaryFn(ctx.op.getType(), e0, e1);
    }

    @Override
    public FormattedFormula visitRealMSetExprElemProduct(ExprLangParser.RealMSetExprElemProductContext ctx) {
        return visitMultiSetDef(ctx.realExpr(), ctx.mSetPredicate(), ctx.multiSetElem(), ctx.mSetElemPredicate());
    }

    @Override
    public FormattedFormula visitRealMsetExprPlaceMarking(ExprLangParser.RealMsetExprPlaceMarkingContext ctx) {
        ruleNotAvailableForConstExpr("place marking");
        requireLatexLanguage();
        final ColorClass fixedDomain = context.colorDomainOfExpr;
        Place place = (Place) context.getNodeByUniqueName(ctx.REAL_MSET_PLACE_ID().getText());
        if (fixedDomain != null && !place.getColorDomainName().equals(fixedDomain.getUniqueName())) {
            context.addNewError("Place " + place.getUniqueName() + " is in domain "
                    + place.getColorDomainName()
                    + ", expected expression in domain " + fixedDomain.getUniqueName());
        }
        switch (lang) { // same for visitRealMsetExprPlaceMarking
            case LATEX:
                return format(true, "\\#", place.getUniqueNameDecor().getVisualizedValue());

            case GREATSPN:
            case PNPRO:
                return format(true, "#" + ctx.REAL_MSET_PLACE_ID().getText());

//            case APNN:
//                return format(true, ctx.REAL_MSET_PLACE_ID().getText());
            default:
                throw new UnsupportedOperationException("visitRealMsetExprPlaceMarking");
        }
    }

    @Override
    public FormattedFormula visitRealMsetExprConst(ExprLangParser.RealMsetExprConstContext ctx) {
        final ColorClass fixedDomain = context.colorDomainOfExpr;
        ConstantID intConstId = (ConstantID) context.getNodeByUniqueName(ctx.REAL_MSET_CONST_ID().getText());
        if (fixedDomain != null && !intConstId.getColorDomainName().equals(fixedDomain.getUniqueName())) {
            context.addNewError("Constant " + intConstId.getUniqueName() + " is in domain "
                    + intConstId.getColorDomainName()
                    + ", expected expression in domain " + fixedDomain.getUniqueName());
        }
        return format(true, intConstId);
    }

    //==========================================================================
    //  Color Class definitions:
    //==========================================================================
    private static class SubclassName implements FormulaPayload {

        public SubclassName(String name) {
            this.name = name;
        }
        public String name;
    }

    @Override
    public FormattedFormula visitColorSubclassNameDef(ExprLangParser.ColorSubclassNameDefContext ctx) {
        requireLatexLanguage();
        String text = ctx.ID().getText();
        String altText = AlternateNameFunction.NUMBERS_AS_SUBSCRIPTS.prepareLatexText(text, null, NetObject.STYLE_ROMAN);
        return formatPayload(false, new SubclassName(ctx.ID().getText()),
                "\\text{\\bf is }", altText);
    }

    @Override
    public FormattedFormula visitColorListIDs(ExprLangParser.ColorListIDsContext ctx) {
        requireLatexLanguage();
        StringBuilder buffer = new StringBuilder("\\{");
        List<String> colorList = new LinkedList<>();

        List<TerminalNode> idList = ctx.ID();
        int count = 0;
        for (TerminalNode node : idList) {
            if (count > 0) {
                buffer.append(", ");
            }
            colorList.add(node.getText());
            buffer.append(AlternateNameFunction.NUMBERS_AS_SUBSCRIPTS.prepareLatexText(node.getText(), null, NetObject.STYLE_ROMAN));
            count++;
        }
        buffer.append("\\}");
        String name = null;
        if (ctx.colorSubclassName() != null) {
            FormattedFormula ffName = visit(ctx.colorSubclassName());
            name = ((SubclassName) ffName.getPayload()).name;
            buffer.append(ffName.getFormula());
        }
        return formatPayload(true, new ParsedColorSubclass(name, colorList), buffer.toString());
    }

    @Override
    public FormattedFormula visitColorListInterval(ExprLangParser.ColorListIntervalContext ctx) {
        requireLatexLanguage();
        FormattedFormula prefix = (ctx.anyID() != null ? visit(ctx.anyID()) : FormattedFormula.EMPTY_LATEX);
        FormattedFormula i0 = visit(ctx.intConst(0)), i1 = visit(ctx.intConst(1));
        String start = ctx.intConst(0).getText(), end = ctx.intConst(1).getText();
        String name = null;
        FormattedFormula ffName = FormattedFormula.EMPTY_LATEX;
        if (ctx.colorSubclassName() != null) {
            ffName = visit(ctx.colorSubclassName());
            name = ((SubclassName) ffName.getPayload()).name;
        }
        return formatPayload(false, new ParsedColorSubclass(name, prefix.getFormula(), start, end),
                prefix, "\\{", i0, " .. ", i1, "\\}", ffName);
    }

//    @Override
//    public FormattedFormula visitColorClassDefSet(ExprLangParser.ColorClassDefSetContext ctx) {
//        return visit(ctx.colorList());
//    }
    @Override
    public FormattedFormula visitColorClassDefNamedSet(ExprLangParser.ColorClassDefNamedSetContext ctx) {
        requireLatexLanguage();
        StringBuilder buffer = new StringBuilder();

        if (ctx.colorClassOrd() != null) {
            for (ExprLangParser.ColorClassOrdContext keyword : ctx.colorClassOrd()) {
                buffer.append("\\mathbf{").append(keyword.getText()).append("}\\hspace{0.4em}");
            }
        }
        List<ParsedColorSubclass> subClassList = new LinkedList<>();

        List<ExprLangParser.ColorListContext> classNodesList = ctx.colorList();
        int count = 0;
        for (ExprLangParser.ColorListContext node : classNodesList) {
            if (count > 0) {
                buffer.append(" + ");
            }
            FormattedFormula subClass = visit(node);
            subClassList.add((ParsedColorSubclass) subClass.getPayload());
            buffer.append(subClass.getFormula());
            count++;
        }
        ParsedColorSubclass[] subClasses = new ParsedColorSubclass[subClassList.size()];
        return format(false, buffer.toString());
    }

    @Override
    public FormattedFormula visitColorClassDefProduct(ExprLangParser.ColorClassDefProductContext ctx) {
        requireLatexLanguage();
        StringBuilder buffer = new StringBuilder();
        List<ColorClass> domainList = new LinkedList<>();

        List<TerminalNode> domainNodesList = ctx.SIMPLECOLORCLASS_ID();
        int count = 0;
        for (TerminalNode node : domainNodesList) {
            ColorClass cc = (ColorClass) context.getNodeByUniqueName(node.getText());
            if (count > 0) {
                buffer.append(" \\times ");
            }
            domainList.add(cc);
            buffer.append(cc.getVisualizedUniqueName());
            count++;
        }
        ColorClass[] domain = new ColorClass[domainList.size()];
        return format(false, buffer.toString());
    }

    @Override
    public FormattedFormula visitColorVarDef(ExprLangParser.ColorVarDefContext ctx) {
        ColorClass cc = (ColorClass) context.getNodeByUniqueName(ctx.SIMPLECOLORCLASS_ID().getText());
        if (cc == null) {
            return unknownId(ctx.SIMPLECOLORCLASS_ID());
        }
        return formatPayload(true, cc/*payload*/, cc);
    }

    @Override
    public FormattedFormula visitPlaceDomainColorClass(ExprLangParser.PlaceDomainColorClassContext ctx) {
        ColorClass cc = (ColorClass) context.getNodeByUniqueName(ctx.SIMPLECOLORCLASS_ID().getText());
        if (cc == null) {
            return unknownId(ctx.SIMPLECOLORCLASS_ID());
        }
        return formatPayload(true, cc/*payload*/, cc);
    }

    @Override
    public FormattedFormula visitPlaceDomainColorDomain(ExprLangParser.PlaceDomainColorDomainContext ctx) {
        ColorClass cc = (ColorClass) context.getNodeByUniqueName(ctx.COLORDOMAIN_ID().getText());
        if (cc == null) {
            return unknownId(ctx.COLORDOMAIN_ID());
        }
        return formatPayload(true, cc/*payload*/, cc);
    }
//    @Override
//    public FormattedFormula visitErrorNode(ErrorNode node) {
//        System.out.println("<<"+node.getText()+">>");
//        return null;
//    }    

    //==========================================================================
    //  Action lists and sets:
    //==========================================================================
    @Override
    public FormattedFormula visitActListEmpty(ExprLangParser.ActListEmptyContext ctx) {
        requireLatexLanguage();
        return format(true, "\\{ \\}");
    }

    @Override
    public FormattedFormula visitActListList(ExprLangParser.ActListListContext ctx) {
        requireLatexLanguage();
        StringBuilder buffer = new StringBuilder("\\{");

        List<TerminalNode> actList = ctx.ACTION_ID();
        int count = 0;
        for (TerminalNode node : actList) {
            if (count > 0) {
                buffer.append(", ");
            }
            TemplateVariable actName = (TemplateVariable) context.getNodeByUniqueName(node.getText());
            buffer.append(actName.getVisualizedUniqueName());
            count++;
        }
        buffer.append("\\}");
        return format(true, buffer.toString());
    }

    @Override
    public FormattedFormula visitActSetAllExceptList(ExprLangParser.ActSetAllExceptListContext ctx) {
        requireLatexLanguage();
        return format(false, "\\mathit{Act} \\setminus ", visit(ctx.actList()));
    }

    @Override
    public FormattedFormula visitActSetList(ExprLangParser.ActSetListContext ctx) {
        return visit(ctx.actList());
    }

    @Override
    public FormattedFormula visitActSetAll(ExprLangParser.ActSetAllContext ctx) {
        requireLatexLanguage();
        return format(true, "\\mathit{Act}");
    }

    @Override
    public FormattedFormula visitActBindingTransition(ExprLangParser.ActBindingTransitionContext ctx) {
        Transition trn = (Transition) context.getNodeByUniqueName(ctx.getText());
        return format(true, trn);
    }

//    @Override
//    public FormattedFormula visitActSetBoundary(ExprLangParser.ActSetBoundaryContext ctx) {
//        return new FormattedFormula(true, "\\sharp");
//    }
    //==========================================================================
    //  Language of clock constraints for DTA edges:
    //==========================================================================
    @Override
    public FormattedFormula visitClockVarId(ExprLangParser.ClockVarIdContext ctx) {
        ClockVar clockVar = (ClockVar) context.getNodeByUniqueName(ctx.CLOCK_ID().getText());
        return format(true, clockVar);
    }

    @Override
    public FormattedFormula visitClockConstrEquals(ExprLangParser.ClockConstrEqualsContext ctx) {
        return format(false, visit(ctx.clockVar()), " = ", visit(ctx.realExpr()));
    }

    @Override
    public FormattedFormula visitClockConstrBetween(ExprLangParser.ClockConstrBetweenContext ctx) {
        return format(false, visit(ctx.realExpr(0)), " < ", visit(ctx.clockVar()), " < ", visit(ctx.realExpr(1)));
    }

    @Override
    public FormattedFormula visitClockConstrIdCmpReal(ExprLangParser.ClockConstrIdCmpRealContext ctx) {
        return format(false, visit(ctx.clockVar()), ctx.op.getText(), visit(ctx.realExpr()));
    }

    @Override
    public FormattedFormula visitClockConstrRealCmpId(ExprLangParser.ClockConstrRealCmpIdContext ctx) {
        return format(false, visit(ctx.realExpr()), ctx.op.getText(), visit(ctx.clockVar()));
    }

    @Override
    public FormattedFormula visitClockConstrAnd(ExprLangParser.ClockConstrAndContext ctx) {
        return formatBinaryFn(ExprLangParser.AND, visit(ctx.clockConstr(0)), visit(ctx.clockConstr(1)));
    }

    //==========================================================================
    //  LHA edge reset expression:
    //==========================================================================
    @Override
    public FormattedFormula visitVarUpdateValue(ExprLangParser.VarUpdateValueContext ctx) {
        requireLatexLanguage();
        return format(true, visit(ctx.clockVar()), " = ", visit(ctx.realExpr()));
    }

    @Override
    public FormattedFormula visitVarUpdateListList(ExprLangParser.VarUpdateListListContext ctx) {
        requireLatexLanguage();
        FormattedFormula ff = new FormattedFormula(lang, true, "");
        for (int i = 0; i < ctx.varUpdate().size(); i++) {
            ff = format(true, ff, (i > 0 ? ", " : ""), visit(ctx.varUpdate(i)));
        }
        return ff;
    }

    @Override
    public FormattedFormula visitVarUpdateListEmpty(ExprLangParser.VarUpdateListEmptyContext ctx) {
        requireLatexLanguage();
        return format(true, "");
    }

    //==========================================================================
    //  Flow indicators for LHA:
    //==========================================================================
    @Override
    public FormattedFormula visitVarFlowListEmpty(ExprLangParser.VarFlowListEmptyContext ctx) {
        requireLatexLanguage();
        return format(true, "");
    }

    @Override
    public FormattedFormula visitVarFlowDefinition(ExprLangParser.VarFlowDefinitionContext ctx) {
        requireLatexLanguage();
        return format(true, "\\dot{", visit(ctx.clockVar()), "\\hspace{3pt}}\\hspace{-3tp} = ", visit(ctx.realExpr()));
    }

    @Override
    public FormattedFormula visitVarFlowListList(ExprLangParser.VarFlowListListContext ctx) {
        requireLatexLanguage();
        int numVars = ctx.varFlow().size();
        if (numVars == 1) {
            return visit(ctx.varFlow(0));
        }
        FormattedFormula ff = new FormattedFormula(ExpressionLanguage.LATEX, true, "\\begin{array}{c}");
        for (int i = 0; i < numVars; i++) {
            ff = format(true, ff, visit(ctx.varFlow(i)), (i == numVars - 1 ? "\\end{array}" : "\\\\"));
        }
        return ff;
    }

    //==========================================================================
    //  Performance measures:
    //==========================================================================
    private FormattedFormula formatMeasure(char PEX, Object f1, ParserRuleContext cond,
            ParserRuleContext mult) {
        FormattedFormula ff;
        switch (lang) {
            case LATEX:
                if (cond == null) {
                    ff = format(true, "\\mathrm{" + PEX + "}\\left\\{", f1, "\\right\\}");
                } else {
                    ff = format(true, "\\mathrm{" + PEX + "}\\left\\{", f1, "/", cond, "\\right\\}");
                }

                if (mult != null) {
                    ff = format(false, mult, " \\cdot ", ff);
                }
                return ff;

            case GREATSPN:
            case PNPRO:
                if (cond == null) {
                    ff = format(true, PEX + "{ ", f1, " }");
                } else {
                    ff = format(true, PEX + "{ ", f1, " / ", cond, " }");
                }

                if (mult != null) {
                    ff = format(true/* still simple term */, mult, " ", ff);
                }
                return ff;

            default:
                throw new UnsupportedOperationException();
        }
    }

    @Override
    public FormattedFormula visitMeasureP(ExprLangParser.MeasurePContext ctx) {
        return formatMeasure('P', ctx.boolExpr(), null, ctx.realConst());
    }

    @Override
    public FormattedFormula visitMeasureE(ExprLangParser.MeasureEContext ctx) {
        return formatMeasure('E', ctx.intExpr(), ctx.boolExpr(), ctx.realConst());
    }

    @Override
    public FormattedFormula visitMeasureX(ExprLangParser.MeasureXContext ctx) {
        return formatMeasure('X', format(true, ctx.TRANSITION_ID().getText()),
                ctx.boolExpr(), ctx.realConst());
    }

    @Override
    public FormattedFormula visitMeasureAddSub(ExprLangParser.MeasureAddSubContext ctx) {
        return format(false, visit(ctx.measure(0)),
                (ctx.op.getType() == ExprLangLexer.ADD ? " + " : " - "),
                visit(ctx.measure(1)));
    }

    //==========================================================================
    //  CSL^TA state formula language:
    //==========================================================================
    @Override
    public FormattedFormula visitCsltaExprNot(ExprLangParser.CsltaExprNotContext ctx) {
        return format(false, "\\neg ", visit(ctx.csltaExpr()));
    }

    @Override
    public FormattedFormula visitCsltaExprOr(ExprLangParser.CsltaExprOrContext ctx) {
        return format(false, visit(ctx.csltaExpr(0)), " \\vee ", visit(ctx.csltaExpr(1)));
    }

    @Override
    public FormattedFormula visitCsltaExprAnd(ExprLangParser.CsltaExprAndContext ctx) {
        return format(false, visit(ctx.csltaExpr(0)), " \\wedge ", visit(ctx.csltaExpr(1)));
    }

    @Override
    public FormattedFormula visitCsltaExprParen(ExprLangParser.CsltaExprParenContext ctx) {
        return format(true, "\\left( ", visit(ctx.csltaExpr()), " \\right) ");
    }

    @Override
    public FormattedFormula visitCsltaExprPlace(ExprLangParser.CsltaExprPlaceContext ctx) {
        Place place = (Place) context.getNodeByUniqueName(ctx.INT_PLACE_ID().getText());
        return format(true, place);
    }

    @Override
    public FormattedFormula visitCsltaExprBool(ExprLangParser.CsltaExprBoolContext ctx) {
        return visit(ctx.boolExpr());
    }

    @Override
    public FormattedFormula visitCsltaExprProbTA(ExprLangParser.CsltaExprProbTAContext ctx) {
        FormattedFormula ff = format(false, "\\mathcal{P}_{", ctx.op, visit(ctx.q), "}\\text{", ctx.dtaName, "}");
        ff = format(false, ff, "(");

        DtaSignature sig = null;
        if (context.knownDtas != null) {
            for (DtaSignature ds : context.knownDtas) {
                if (ds.dta.getPageName().equals(ctx.dtaName.getText())) {
                    sig = ds;
                    sig.isUsed = true;
                    break;
                }
            }
        }
        if (sig == null) {
            context.addNewError("Unknown DTA with name \"" + ctx.dtaName.getText() + "\".");
        }

        // Push the signature onto the stack (push even if it is null)
        if (context.dtaSigStack == null) {
            context.dtaSigStack = new Stack<>();
        }
        context.dtaSigStack.push(sig);

        try {
            int numClkExpr = 0, numActs = 0, numSubExpr = 0;
            for (ExprLangParser.Real_assignContext clockAssign : ctx.real_assign()) {
                ff = format(false, ff, (numClkExpr++ > 0 ? "," : ""), visit(clockAssign));
            }
            ff = format(false, ff, "~|~");
            for (ExprLangParser.Act_assignContext actAssign : ctx.act_assign()) {
                ff = format(false, ff, (numActs++ > 0 ? "," : ""), visit(actAssign));
            }
            ff = format(false, ff, "~|~");
            for (ExprLangParser.Sp_assignContext spAssign : ctx.sp_assign()) {
                ff = format(false, ff, (numSubExpr++ > 0 ? "," : ""), visit(spAssign));
            }
        } finally {
            context.dtaSigStack.pop();
        }
        return format(false, ff, ")");
    }

    @Override
    public FormattedFormula visitSp_assign(ExprLangParser.Sp_assignContext ctx) {
        DtaSignature sig = context.dtaSigStack.lastElement();

        String id = ctx.label.getText();
        FormattedFormula ff = visit(ctx.csltaExpr());

        if (sig != null && !sig.stateProps.contains(id)) {
            context.addNewError("DTA \"" + sig.dta.getPageName() + "\". has no state proposition named \"" + id + "\".");
        }

        return format(false, id, "=", ff);
    }

    @Override
    public FormattedFormula visitReal_assign(ExprLangParser.Real_assignContext ctx) {
        DtaSignature sig = context.dtaSigStack.lastElement();

        String id = ctx.label.getText();
        FormattedFormula ff = visit(ctx.realConst());

        if (sig != null && !sig.clockVals.contains(id)) {
            context.addNewError("DTA \"" + sig.dta.getPageName() + "\". has no clock value named \"" + id + "\".");
        }

        return format(false, id, "=", ff);
    }

    @Override
    public FormattedFormula visitAct_assign(ExprLangParser.Act_assignContext ctx) {
        DtaSignature sig = context.dtaSigStack.lastElement();

        String id = ctx.label.getText();
        String trn = ctx.trn.getText();

        if (sig != null && !sig.actNames.contains(id)) {
            context.addNewError("DTA \"" + sig.dta.getPageName() + "\". has no action named \"" + id + "\".");
        }

        return format(false, id, "=", trn);
    }

    //==========================================================================
    //  CTL formulas
    //==========================================================================
    private FormattedFormula temporalOpCTL(char quant, char path) {
        switch (lang) {
            case LATEX:
                return format(true, (quant == 'E' ? "\\exists" : "\\forall"),
                        " \\mathrm{" + path + "}");
            case GREATSPN:
                return format(true, quant, " ", path);
            case PNPRO:
                return format(true, quant, path);

            default:
                throw new UnsupportedOperationException("CTL temporal operator.");
        }
    }

    @Override
    public FormattedFormula visitTemporalOp2T(ExprLangParser.TemporalOp2TContext ctx) {
        if (ctx.EXISTS() != null) {
            if (ctx.NEXT() != null) {
                return temporalOpCTL('E', 'X');
            }
            if (ctx.FUTURE() != null) {
                return temporalOpCTL('E', 'F');
            }
            if (ctx.GLOBALLY() != null) {
                return temporalOpCTL('E', 'G');
            }
        } else {
            assert ctx.FORALL() != null;
            if (ctx.NEXT() != null) {
                return temporalOpCTL('A', 'X');
            }
            if (ctx.FUTURE() != null) {
                return temporalOpCTL('A', 'F');
            }
            if (ctx.GLOBALLY() != null) {
                return temporalOpCTL('A', 'G');
            }
        }
        throw new IllegalStateException();
    }

    @Override
    public FormattedFormula visitTemporalOpExistX(ExprLangParser.TemporalOpExistXContext ctx) {
        if (ctx.EXISTS_NEXT() != null) {
            return temporalOpCTL('E', 'X');
        }
        if (ctx.EXISTS_FUTURE() != null) {
            return temporalOpCTL('E', 'F');
        }
        if (ctx.EXISTS_GLOBALLY() != null) {
            return temporalOpCTL('E', 'G');
        }
        throw new IllegalStateException();
    }

    @Override
    public FormattedFormula visitTemporalOpForallX(ExprLangParser.TemporalOpForallXContext ctx) {
        if (ctx.FORALL_NEXT() != null) {
            return temporalOpCTL('A', 'X');
        }
        if (ctx.FORALL_FUTURE() != null) {
            return temporalOpCTL('A', 'F');
        }
        if (ctx.FORALL_GLOBALLY() != null) {
            return temporalOpCTL('A', 'G');
        }
        throw new IllegalStateException();
    }

    //==========================================================================
    //  Tag rewriting rules:
    //==========================================================================
    @Override
    public FormattedFormula visitTagDefinition(ExprLangParser.TagDefinitionContext ctx) {
        return new FormattedFormula(lang, true, ctx.getText());
    }

    @Override
    public FormattedFormula visitTagComplDefinition(ExprLangParser.TagComplDefinitionContext ctx) {
        return new FormattedFormula(lang, true, ctx.getText() + "?");
    }

    @Override
    public FormattedFormula visitTagRewriteRule(ExprLangParser.TagRewriteRuleContext ctx) {
        return format(false, visit(ctx.tag(0)), " -> ", visit(ctx.tag(1)));
    }

    @Override
    public FormattedFormula visitTagRewriteListEmpty(ExprLangParser.TagRewriteListEmptyContext ctx) {
        requireLatexLanguage();
        return format(false, "");
    }

    @Override
    public FormattedFormula visitTagRewriteListList(ExprLangParser.TagRewriteListListContext ctx) {
        requireLatexLanguage();
        FormattedFormula ff = new FormattedFormula(lang, true, "");
        for (int i = 0; i < ctx.tagRewrite().size(); i++) {
            ff = format(true, ff, (i > 0 ? ", " : ""), visit(ctx.tagRewrite(i)));
        }
        return ff;
    }

    //==========================================================================
    //  Miscellaneous:
    //==========================================================================
    @Override
    public FormattedFormula visitAnyIdentifier(ExprLangParser.AnyIdentifierContext ctx) {
        return format(true, ctx.getText());
    }
}
