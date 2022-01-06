// Generated from JavaGUI/Editor/src/editor/domain/grammar/ExprLang.g4 by ANTLR 4.2.1
package editor.domain.grammar;

import editor.domain.*;
import editor.domain.elements.*;

import org.antlr.v4.runtime.Lexer;
import org.antlr.v4.runtime.CharStream;
import org.antlr.v4.runtime.Token;
import org.antlr.v4.runtime.TokenStream;
import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.atn.*;
import org.antlr.v4.runtime.dfa.DFA;
import org.antlr.v4.runtime.misc.*;

@SuppressWarnings({"all", "warnings", "unchecked", "unused", "cast"})
public class ExprLangLexer extends Lexer {
	protected static final DFA[] _decisionToDFA;
	protected static final PredictionContextCache _sharedContextCache =
		new PredictionContextCache();
	public static final int
		T__15=1, T__14=2, T__13=3, T__12=4, T__11=5, T__10=6, T__9=7, T__8=8, 
		T__7=9, T__6=10, T__5=11, T__4=12, T__3=13, T__2=14, T__1=15, T__0=16, 
		MUL=17, DIV=18, ADD=19, SUB=20, AND=21, OR=22, POSTINCR=23, POSTDECR=24, 
		HAT=25, NOT=26, IMPLY=27, BIIMPLY=28, COMMA=29, COLON=30, SEMICOLON=31, 
		OP_PAREN=32, CL_PAREN=33, OP_SQPAR=34, CL_SQPAR=35, OP_BRACK=36, CL_BRACK=37, 
		TWODOTS=38, EQUAL=39, NOT_EQUAL=40, LESS=41, GREATER=42, LESS_EQ=43, GREATER_EQ=44, 
		SHARP=45, TRUE=46, FALSE=47, INFINITY=48, CLOCK=49, ACT=50, PROB_TA=51, 
		CONTAINS=52, DONT_CONTAINS=53, COLOR_ORDINAL=54, MULTISET_CARD=55, MULTISET_SUBCLASS=56, 
		MIN_FN=57, MAX_FN=58, MOD_FN=59, POW_FN=60, FRACT_FN=61, FACTORIAL_FN=62, 
		ABS_FN=63, SIN_FN=64, COS_FN=65, TAN_FN=66, ARCSIN_FN=67, ARCCOS_FN=68, 
		ARCTAN_FN=69, EXP_FN=70, LOG_FN=71, SQRT_FN=72, CEIL_FN=73, FLOOR_FN=74, 
		ROUND_FN=75, BINOMIAL_FN=76, IF_FN=77, WHITESPACES=78, ID=79, INT=80, 
		REAL=81;
	public static String[] modeNames = {
		"DEFAULT_MODE"
	};

	public static final String[] tokenNames = {
		"<INVALID>",
		"'ordered'", "'circular'", "'unordered'", "'\\'", "'='", "'?'", "'enum'", 
		"'when'", "'All'", "'is'", "'E{'", "'|'", "'ever'", "'X{'", "'@'", "'P{'", 
		"'*'", "'/'", "'+'", "'-'", "'&&'", "'||'", "'++'", "'--'", "'^'", "'!'", 
		"'->'", "'<->'", "','", "':'", "';'", "'('", "')'", "'['", "']'", "'{'", 
		"'}'", "'..'", "'=='", "'!='", "'<'", "'>'", "'<='", "'>='", "'#'", "'True'", 
		"'False'", "'Infinite'", "'clock'", "'Act'", "'PROB_TA'", "'in'", "'!in'", 
		"'CN'", "'Card'", "'Subclass'", "'Min'", "'Max'", "'Mod'", "'Pow'", "'Fract'", 
		"'Factorial'", "'Abs'", "'Sin'", "'Cos'", "'Tan'", "'Asin'", "'Acos'", 
		"'Atan'", "'Exp'", "'Log'", "'Sqrt'", "'Ceil'", "'Floor'", "'Round'", 
		"'Binomial'", "'If'", "WHITESPACES", "ID", "INT", "REAL"
	};
	public static final String[] ruleNames = {
		"T__15", "T__14", "T__13", "T__12", "T__11", "T__10", "T__9", "T__8", 
		"T__7", "T__6", "T__5", "T__4", "T__3", "T__2", "T__1", "T__0", "MUL", 
		"DIV", "ADD", "SUB", "AND", "OR", "POSTINCR", "POSTDECR", "HAT", "NOT", 
		"IMPLY", "BIIMPLY", "COMMA", "COLON", "SEMICOLON", "OP_PAREN", "CL_PAREN", 
		"OP_SQPAR", "CL_SQPAR", "OP_BRACK", "CL_BRACK", "TWODOTS", "EQUAL", "NOT_EQUAL", 
		"LESS", "GREATER", "LESS_EQ", "GREATER_EQ", "SHARP", "TRUE", "FALSE", 
		"INFINITY", "CLOCK", "ACT", "PROB_TA", "CONTAINS", "DONT_CONTAINS", "COLOR_ORDINAL", 
		"MULTISET_CARD", "MULTISET_SUBCLASS", "MIN_FN", "MAX_FN", "MOD_FN", "POW_FN", 
		"FRACT_FN", "FACTORIAL_FN", "ABS_FN", "SIN_FN", "COS_FN", "TAN_FN", "ARCSIN_FN", 
		"ARCCOS_FN", "ARCTAN_FN", "EXP_FN", "LOG_FN", "SQRT_FN", "CEIL_FN", "FLOOR_FN", 
		"ROUND_FN", "BINOMIAL_FN", "IF_FN", "WHITESPACES", "ID_LETTER", "DIGIT", 
		"SignPart", "ExponentPart", "ID", "INT", "REAL"
	};


	public ParserContext context = null;
	public int parseFlags;
	public boolean clockVarInExpr = true;


	public ExprLangLexer(CharStream input) {
		super(input);
		_interp = new LexerATNSimulator(this,_ATN,_decisionToDFA,_sharedContextCache);
	}

	@Override
	public String getGrammarFileName() { return "ExprLang.g4"; }

	@Override
	public String[] getTokenNames() { return tokenNames; }

	@Override
	public String[] getRuleNames() { return ruleNames; }

	@Override
	public String getSerializedATN() { return _serializedATN; }

	@Override
	public String[] getModeNames() { return modeNames; }

	@Override
	public ATN getATN() { return _ATN; }

	@Override
	public void action(RuleContext _localctx, int ruleIndex, int actionIndex) {
		switch (ruleIndex) {
		case 82: ID_action((RuleContext)_localctx, actionIndex); break;
		}
	}
	private void ID_action(RuleContext _localctx, int actionIndex) {
		switch (actionIndex) {
		case 0: 
		    // Classify context-sensitive identifiers
		    if (context != null) {
		        if (context.hasNamespacePage()) {
		            boolean isMarkDepExpr = 0==(parseFlags & ParserContext.PF_CONST_EXPR);
		            boolean haveStaticSubclass = 0==(parseFlags & ParserContext.PF_NO_STATIC_SUBCLASS_ID);

		            Node node = context.getNodeByUniqueName(getText());
		            if (node != null) { // We have a node with the same name
		                if (node instanceof Place && isMarkDepExpr) {
		                    Place plc = (Place)node;
		                    if (plc.isInNeutralDomain())
		                        setType(plc.isDiscrete() ? ExprLangParser.INT_PLACE_ID 
		                                                 : ExprLangParser.REAL_PLACE_ID);
		                    else // in color domain
		                        setType(plc.isDiscrete() ? ExprLangParser.INT_MSET_PLACE_ID 
		                                                 : ExprLangParser.REAL_MSET_PLACE_ID);
		                }
		                else if (node instanceof editor.domain.elements.Transition/* && !isConstExpr*/)
		                    setType(ExprLangParser.TRANSITION_ID);
		                else if (node instanceof ConstantID) {
		                    ConstantID con = (ConstantID)node;
		                    if (con.isInNeutralDomain())
		                        setType(con.isIntConst() ? ExprLangParser.INT_CONST_ID
		                                                 : ExprLangParser.REAL_CONST_ID);
		                    else // in color domain
		                        setType(con.isIntConst() ? ExprLangParser.INT_MSET_CONST_ID
		                                                 : ExprLangParser.REAL_MSET_CONST_ID);
		                }
		                else if (node instanceof ClockVar) {
		                    setType(ExprLangParser.CLOCK_ID);
		                }
		                else if (node instanceof editor.domain.elements.ColorClass) {
		                    editor.domain.elements.ColorClass cc = (editor.domain.elements.ColorClass)node;
		                    if (cc.isCrossDomain())
		                        setType(ExprLangParser.COLORDOMAIN_ID);
		                    else
		                        setType(ExprLangParser.SIMPLECOLORCLASS_ID);
		                }
		                else if (node instanceof editor.domain.elements.ColorVar) {
		                    setType(ExprLangParser.COLORVAR_ID);
		                }
		                else if (node instanceof TemplateVariable) {
		                    switch (((TemplateVariable)node).getType()) {
		                        case INTEGER:
		                            setType(ExprLangParser.INT_TEMPLATE_ID); 
		                            break;
		                        case REAL:      
		                            setType(ExprLangParser.REAL_TEMPLATE_ID); 
		                            break;
		                        case ACTION:    
		                            setType(ExprLangParser.ACTION_ID); 
		                            break;
		                        case STATEPROP: 
		                            if (isMarkDepExpr)
		                                setType(ExprLangParser.STATEPROP_ID); 
		                            break;
		                        default: throw new IllegalStateException("Unknown template var type.");
		                    }
		                }
		                else return; // Unknown ID type
		            }
		            // Otherwise, try each color class to see if getText() is a sub-id
		            if (haveStaticSubclass) {
		                java.util.Iterator<ColorClass> ccIt = context.colorClassIterator();
		                while (ccIt != null && ccIt.hasNext()) {
		                    ColorClass cc = ccIt.next();
		                    // Test if it is the name of a color subclass
		                    if (cc.isSimpleClass()) {
		                        if (cc.testHasStaticSubclassNamed(getText())) {
		                            setType(ExprLangParser.COLORSUBCLASS_ID);
		                            return;
		                        }
		                        if (cc.testHasColorNamed(context, getText())) {
		                            setType(ExprLangParser.COLOR_ID);
		                            return;
		                        }
		                    }
		                }
		            }
		        }
		    }
		    // Language-dependent tokens
		    boolean isCTL = 0!=(parseFlags & ParserContext.PF_CTL_KEYWORDS);
		    boolean isLTL = 0!=(parseFlags & ParserContext.PF_LTL_KEYWORDS);
		    boolean isCTLSTAR = 0!=(parseFlags & ParserContext.PF_CTLSTAR_KEYWORDS);
		    if (isCTL) {
		        switch (getText()) {
		            case "E":   setType(ExprLangParser.EXISTS);           return;
		            case "A":   setType(ExprLangParser.FORALL);           return;
		            case "X":   setType(ExprLangParser.NEXT);             return;
		            case "F":   setType(ExprLangParser.FUTURE);           return;
		            case "G":   setType(ExprLangParser.GLOBALLY);         return;
		            case "U":   setType(ExprLangParser.UNTIL);            return;
		            case "EX":  setType(ExprLangParser.EXISTS_NEXT);      return;
		            case "EF":  setType(ExprLangParser.EXISTS_FUTURE);    return;
		            case "EG":  setType(ExprLangParser.EXISTS_GLOBALLY);  return;
		            case "AX":  setType(ExprLangParser.FORALL_NEXT);      return;
		            case "AF":  setType(ExprLangParser.FORALL_FUTURE);    return;
		            case "AG":  setType(ExprLangParser.FORALL_GLOBALLY);  return;
		            // If new context-dependent keywords are added to this list,
		            // they must also be added in NetObject.extraKeyWords[].
		        }
		    }
		    if (isLTL || isCTLSTAR) {
		        switch (getText()) {
		            case "X":   setType(ExprLangParser.CTLSTAR_NEXT);             return;
		            case "F":   setType(ExprLangParser.CTLSTAR_FUTURE);           return;
		            case "G":   setType(ExprLangParser.CTLSTAR_GLOBALLY);         return;
		            case "U":   setType(ExprLangParser.CTLSTAR_UNTIL);            return;
		        }
		    }
		    if (isCTLSTAR) {
		        switch (getText()) {
		            case "E":   setType(ExprLangParser.CTLSTAR_EXISTS);           return;
		            case "A":   setType(ExprLangParser.CTLSTAR_FORALL);           return;
		            case "EX":  setType(ExprLangParser.CTLSTAR_EXISTS_NEXT);      return;
		            case "EF":  setType(ExprLangParser.CTLSTAR_EXISTS_FUTURE);    return;
		            case "EG":  setType(ExprLangParser.CTLSTAR_EXISTS_GLOBALLY);  return;
		            case "AX":  setType(ExprLangParser.CTLSTAR_FORALL_NEXT);      return;
		            case "AF":  setType(ExprLangParser.CTLSTAR_FORALL_FUTURE);    return;
		            case "AG":  setType(ExprLangParser.CTLSTAR_FORALL_GLOBALLY);  return;
		        }
		    }
		    if (isCTL || isLTL || isCTLSTAR) {
		        switch (getText()) {
		            case "possibly":     setType(ExprLangParser.POSSIBLY);    return;
		            case "impossibly":   setType(ExprLangParser.IMPOSSIBLY);  return;
		            case "invariantly":  setType(ExprLangParser.INVARIANTLY); return;
		            case "en":           setType(ExprLangParser.ENABLED);     return;
		            case "bounds":       setType(ExprLangParser.BOUNDS);      return;
		            case "deadlock":     setType(ExprLangParser.DEADLOCK);    return;
		            case "ndeadlock":    setType(ExprLangParser.NO_DEADLOCK); return;
		            case "initial":      setType(ExprLangParser.INITIAL_STATE);    return;
		            // If new context-dependent keywords are added to this list,
		            // they must also be added in NetObject.extraKeyWords[].
		        }
		    }

		    boolean isGenFn = 0!=(parseFlags & ParserContext.PF_GENERAL_FUNCTION_KEYWORDS);
		    if (isGenFn) {
		        switch (getText()) {
		            case "x":             setType(ExprLangParser.PDF_X_VAR);        return;
		            case "I":             setType(ExprLangParser.DIRAC_DELTA_FN);   return;
		            case "R":             setType(ExprLangParser.RECT_FN);          return;
		            case "Uniform":       setType(ExprLangParser.UNIFORM_FN);       return;
		            case "Triangular":    setType(ExprLangParser.TRIANGULAR_FN);    return;
		            case "Erlang":        setType(ExprLangParser.ERLANG_FN);        return;
		            case "TruncatedExp":  setType(ExprLangParser.TRUNCATED_EXP_FN); return;
		            case "Pareto":        setType(ExprLangParser.PARETO_FN);        return;
		        }
		    }
		 break;
		}
	}

	public static final String _serializedATN =
		"\3\u0430\ud6d1\u8206\uad2d\u4417\uaef1\u8d80\uaadd\2S\u0227\b\1\4\2\t"+
		"\2\4\3\t\3\4\4\t\4\4\5\t\5\4\6\t\6\4\7\t\7\4\b\t\b\4\t\t\t\4\n\t\n\4\13"+
		"\t\13\4\f\t\f\4\r\t\r\4\16\t\16\4\17\t\17\4\20\t\20\4\21\t\21\4\22\t\22"+
		"\4\23\t\23\4\24\t\24\4\25\t\25\4\26\t\26\4\27\t\27\4\30\t\30\4\31\t\31"+
		"\4\32\t\32\4\33\t\33\4\34\t\34\4\35\t\35\4\36\t\36\4\37\t\37\4 \t \4!"+
		"\t!\4\"\t\"\4#\t#\4$\t$\4%\t%\4&\t&\4\'\t\'\4(\t(\4)\t)\4*\t*\4+\t+\4"+
		",\t,\4-\t-\4.\t.\4/\t/\4\60\t\60\4\61\t\61\4\62\t\62\4\63\t\63\4\64\t"+
		"\64\4\65\t\65\4\66\t\66\4\67\t\67\48\t8\49\t9\4:\t:\4;\t;\4<\t<\4=\t="+
		"\4>\t>\4?\t?\4@\t@\4A\tA\4B\tB\4C\tC\4D\tD\4E\tE\4F\tF\4G\tG\4H\tH\4I"+
		"\tI\4J\tJ\4K\tK\4L\tL\4M\tM\4N\tN\4O\tO\4P\tP\4Q\tQ\4R\tR\4S\tS\4T\tT"+
		"\4U\tU\4V\tV\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\3\3\3\3\3\3\3\3\3\3\3\3\3"+
		"\3\3\3\3\3\3\4\3\4\3\4\3\4\3\4\3\4\3\4\3\4\3\4\3\4\3\5\3\5\3\6\3\6\3\7"+
		"\3\7\3\b\3\b\3\b\3\b\3\b\3\t\3\t\3\t\3\t\3\t\3\n\3\n\3\n\3\n\3\13\3\13"+
		"\3\13\3\f\3\f\3\f\3\r\3\r\3\16\3\16\3\16\3\16\3\16\3\17\3\17\3\17\3\20"+
		"\3\20\3\21\3\21\3\21\3\22\3\22\3\23\3\23\3\24\3\24\3\25\3\25\3\26\3\26"+
		"\3\26\3\27\3\27\3\27\3\30\3\30\3\30\3\31\3\31\3\31\3\32\3\32\3\33\3\33"+
		"\3\34\3\34\3\34\3\35\3\35\3\35\3\35\3\36\3\36\3\37\3\37\3 \3 \3!\3!\3"+
		"\"\3\"\3#\3#\3$\3$\3%\3%\3&\3&\3\'\3\'\3\'\3(\3(\3(\3)\3)\3)\3*\3*\3+"+
		"\3+\3,\3,\3,\3-\3-\3-\3.\3.\3/\3/\3/\3/\3/\3\60\3\60\3\60\3\60\3\60\3"+
		"\60\3\61\3\61\3\61\3\61\3\61\3\61\3\61\3\61\3\61\3\62\3\62\3\62\3\62\3"+
		"\62\3\62\3\63\3\63\3\63\3\63\3\64\3\64\3\64\3\64\3\64\3\64\3\64\3\64\3"+
		"\65\3\65\3\65\3\66\3\66\3\66\3\66\3\67\3\67\3\67\38\38\38\38\38\39\39"+
		"\39\39\39\39\39\39\39\3:\3:\3:\3:\3;\3;\3;\3;\3<\3<\3<\3<\3=\3=\3=\3="+
		"\3>\3>\3>\3>\3>\3>\3?\3?\3?\3?\3?\3?\3?\3?\3?\3?\3@\3@\3@\3@\3A\3A\3A"+
		"\3A\3B\3B\3B\3B\3C\3C\3C\3C\3D\3D\3D\3D\3D\3E\3E\3E\3E\3E\3F\3F\3F\3F"+
		"\3F\3G\3G\3G\3G\3H\3H\3H\3H\3I\3I\3I\3I\3I\3J\3J\3J\3J\3J\3K\3K\3K\3K"+
		"\3K\3K\3L\3L\3L\3L\3L\3L\3M\3M\3M\3M\3M\3M\3M\3M\3M\3N\3N\3N\3O\6O\u01e0"+
		"\nO\rO\16O\u01e1\3O\3O\3P\3P\3Q\3Q\3R\3R\3S\3S\5S\u01ee\nS\3S\6S\u01f1"+
		"\nS\rS\16S\u01f2\3S\3S\5S\u01f7\nS\3S\6S\u01fa\nS\rS\16S\u01fb\5S\u01fe"+
		"\nS\3T\3T\3T\7T\u0203\nT\fT\16T\u0206\13T\3T\3T\3U\6U\u020b\nU\rU\16U"+
		"\u020c\3V\6V\u0210\nV\rV\16V\u0211\3V\3V\6V\u0216\nV\rV\16V\u0217\3V\5"+
		"V\u021b\nV\3V\3V\6V\u021f\nV\rV\16V\u0220\3V\5V\u0224\nV\5V\u0226\nV\2"+
		"\2W\3\3\5\4\7\5\t\6\13\7\r\b\17\t\21\n\23\13\25\f\27\r\31\16\33\17\35"+
		"\20\37\21!\22#\23%\24\'\25)\26+\27-\30/\31\61\32\63\33\65\34\67\359\36"+
		";\37= ?!A\"C#E$G%I&K\'M(O)Q*S+U,W-Y.[/]\60_\61a\62c\63e\64g\65i\66k\67"+
		"m8o9q:s;u<w=y>{?}@\177A\u0081B\u0083C\u0085D\u0087E\u0089F\u008bG\u008d"+
		"H\u008fI\u0091J\u0093K\u0095L\u0097M\u0099N\u009bO\u009dP\u009f\2\u00a1"+
		"\2\u00a3\2\u00a5\2\u00a7Q\u00a9R\u00abS\3\2\5\5\2\13\f\17\17\"\"\5\2C"+
		"\\aac|\4\2--//\u0231\2\3\3\2\2\2\2\5\3\2\2\2\2\7\3\2\2\2\2\t\3\2\2\2\2"+
		"\13\3\2\2\2\2\r\3\2\2\2\2\17\3\2\2\2\2\21\3\2\2\2\2\23\3\2\2\2\2\25\3"+
		"\2\2\2\2\27\3\2\2\2\2\31\3\2\2\2\2\33\3\2\2\2\2\35\3\2\2\2\2\37\3\2\2"+
		"\2\2!\3\2\2\2\2#\3\2\2\2\2%\3\2\2\2\2\'\3\2\2\2\2)\3\2\2\2\2+\3\2\2\2"+
		"\2-\3\2\2\2\2/\3\2\2\2\2\61\3\2\2\2\2\63\3\2\2\2\2\65\3\2\2\2\2\67\3\2"+
		"\2\2\29\3\2\2\2\2;\3\2\2\2\2=\3\2\2\2\2?\3\2\2\2\2A\3\2\2\2\2C\3\2\2\2"+
		"\2E\3\2\2\2\2G\3\2\2\2\2I\3\2\2\2\2K\3\2\2\2\2M\3\2\2\2\2O\3\2\2\2\2Q"+
		"\3\2\2\2\2S\3\2\2\2\2U\3\2\2\2\2W\3\2\2\2\2Y\3\2\2\2\2[\3\2\2\2\2]\3\2"+
		"\2\2\2_\3\2\2\2\2a\3\2\2\2\2c\3\2\2\2\2e\3\2\2\2\2g\3\2\2\2\2i\3\2\2\2"+
		"\2k\3\2\2\2\2m\3\2\2\2\2o\3\2\2\2\2q\3\2\2\2\2s\3\2\2\2\2u\3\2\2\2\2w"+
		"\3\2\2\2\2y\3\2\2\2\2{\3\2\2\2\2}\3\2\2\2\2\177\3\2\2\2\2\u0081\3\2\2"+
		"\2\2\u0083\3\2\2\2\2\u0085\3\2\2\2\2\u0087\3\2\2\2\2\u0089\3\2\2\2\2\u008b"+
		"\3\2\2\2\2\u008d\3\2\2\2\2\u008f\3\2\2\2\2\u0091\3\2\2\2\2\u0093\3\2\2"+
		"\2\2\u0095\3\2\2\2\2\u0097\3\2\2\2\2\u0099\3\2\2\2\2\u009b\3\2\2\2\2\u009d"+
		"\3\2\2\2\2\u00a7\3\2\2\2\2\u00a9\3\2\2\2\2\u00ab\3\2\2\2\3\u00ad\3\2\2"+
		"\2\5\u00b5\3\2\2\2\7\u00be\3\2\2\2\t\u00c8\3\2\2\2\13\u00ca\3\2\2\2\r"+
		"\u00cc\3\2\2\2\17\u00ce\3\2\2\2\21\u00d3\3\2\2\2\23\u00d8\3\2\2\2\25\u00dc"+
		"\3\2\2\2\27\u00df\3\2\2\2\31\u00e2\3\2\2\2\33\u00e4\3\2\2\2\35\u00e9\3"+
		"\2\2\2\37\u00ec\3\2\2\2!\u00ee\3\2\2\2#\u00f1\3\2\2\2%\u00f3\3\2\2\2\'"+
		"\u00f5\3\2\2\2)\u00f7\3\2\2\2+\u00f9\3\2\2\2-\u00fc\3\2\2\2/\u00ff\3\2"+
		"\2\2\61\u0102\3\2\2\2\63\u0105\3\2\2\2\65\u0107\3\2\2\2\67\u0109\3\2\2"+
		"\29\u010c\3\2\2\2;\u0110\3\2\2\2=\u0112\3\2\2\2?\u0114\3\2\2\2A\u0116"+
		"\3\2\2\2C\u0118\3\2\2\2E\u011a\3\2\2\2G\u011c\3\2\2\2I\u011e\3\2\2\2K"+
		"\u0120\3\2\2\2M\u0122\3\2\2\2O\u0125\3\2\2\2Q\u0128\3\2\2\2S\u012b\3\2"+
		"\2\2U\u012d\3\2\2\2W\u012f\3\2\2\2Y\u0132\3\2\2\2[\u0135\3\2\2\2]\u0137"+
		"\3\2\2\2_\u013c\3\2\2\2a\u0142\3\2\2\2c\u014b\3\2\2\2e\u0151\3\2\2\2g"+
		"\u0155\3\2\2\2i\u015d\3\2\2\2k\u0160\3\2\2\2m\u0164\3\2\2\2o\u0167\3\2"+
		"\2\2q\u016c\3\2\2\2s\u0175\3\2\2\2u\u0179\3\2\2\2w\u017d\3\2\2\2y\u0181"+
		"\3\2\2\2{\u0185\3\2\2\2}\u018b\3\2\2\2\177\u0195\3\2\2\2\u0081\u0199\3"+
		"\2\2\2\u0083\u019d\3\2\2\2\u0085\u01a1\3\2\2\2\u0087\u01a5\3\2\2\2\u0089"+
		"\u01aa\3\2\2\2\u008b\u01af\3\2\2\2\u008d\u01b4\3\2\2\2\u008f\u01b8\3\2"+
		"\2\2\u0091\u01bc\3\2\2\2\u0093\u01c1\3\2\2\2\u0095\u01c6\3\2\2\2\u0097"+
		"\u01cc\3\2\2\2\u0099\u01d2\3\2\2\2\u009b\u01db\3\2\2\2\u009d\u01df\3\2"+
		"\2\2\u009f\u01e5\3\2\2\2\u00a1\u01e7\3\2\2\2\u00a3\u01e9\3\2\2\2\u00a5"+
		"\u01fd\3\2\2\2\u00a7\u01ff\3\2\2\2\u00a9\u020a\3\2\2\2\u00ab\u0225\3\2"+
		"\2\2\u00ad\u00ae\7q\2\2\u00ae\u00af\7t\2\2\u00af\u00b0\7f\2\2\u00b0\u00b1"+
		"\7g\2\2\u00b1\u00b2\7t\2\2\u00b2\u00b3\7g\2\2\u00b3\u00b4\7f\2\2\u00b4"+
		"\4\3\2\2\2\u00b5\u00b6\7e\2\2\u00b6\u00b7\7k\2\2\u00b7\u00b8\7t\2\2\u00b8"+
		"\u00b9\7e\2\2\u00b9\u00ba\7w\2\2\u00ba\u00bb\7n\2\2\u00bb\u00bc\7c\2\2"+
		"\u00bc\u00bd\7t\2\2\u00bd\6\3\2\2\2\u00be\u00bf\7w\2\2\u00bf\u00c0\7p"+
		"\2\2\u00c0\u00c1\7q\2\2\u00c1\u00c2\7t\2\2\u00c2\u00c3\7f\2\2\u00c3\u00c4"+
		"\7g\2\2\u00c4\u00c5\7t\2\2\u00c5\u00c6\7g\2\2\u00c6\u00c7\7f\2\2\u00c7"+
		"\b\3\2\2\2\u00c8\u00c9\7^\2\2\u00c9\n\3\2\2\2\u00ca\u00cb\7?\2\2\u00cb"+
		"\f\3\2\2\2\u00cc\u00cd\7A\2\2\u00cd\16\3\2\2\2\u00ce\u00cf\7g\2\2\u00cf"+
		"\u00d0\7p\2\2\u00d0\u00d1\7w\2\2\u00d1\u00d2\7o\2\2\u00d2\20\3\2\2\2\u00d3"+
		"\u00d4\7y\2\2\u00d4\u00d5\7j\2\2\u00d5\u00d6\7g\2\2\u00d6\u00d7\7p\2\2"+
		"\u00d7\22\3\2\2\2\u00d8\u00d9\7C\2\2\u00d9\u00da\7n\2\2\u00da\u00db\7"+
		"n\2\2\u00db\24\3\2\2\2\u00dc\u00dd\7k\2\2\u00dd\u00de\7u\2\2\u00de\26"+
		"\3\2\2\2\u00df\u00e0\7G\2\2\u00e0\u00e1\7}\2\2\u00e1\30\3\2\2\2\u00e2"+
		"\u00e3\7~\2\2\u00e3\32\3\2\2\2\u00e4\u00e5\7g\2\2\u00e5\u00e6\7x\2\2\u00e6"+
		"\u00e7\7g\2\2\u00e7\u00e8\7t\2\2\u00e8\34\3\2\2\2\u00e9\u00ea\7Z\2\2\u00ea"+
		"\u00eb\7}\2\2\u00eb\36\3\2\2\2\u00ec\u00ed\7B\2\2\u00ed \3\2\2\2\u00ee"+
		"\u00ef\7R\2\2\u00ef\u00f0\7}\2\2\u00f0\"\3\2\2\2\u00f1\u00f2\7,\2\2\u00f2"+
		"$\3\2\2\2\u00f3\u00f4\7\61\2\2\u00f4&\3\2\2\2\u00f5\u00f6\7-\2\2\u00f6"+
		"(\3\2\2\2\u00f7\u00f8\7/\2\2\u00f8*\3\2\2\2\u00f9\u00fa\7(\2\2\u00fa\u00fb"+
		"\7(\2\2\u00fb,\3\2\2\2\u00fc\u00fd\7~\2\2\u00fd\u00fe\7~\2\2\u00fe.\3"+
		"\2\2\2\u00ff\u0100\7-\2\2\u0100\u0101\7-\2\2\u0101\60\3\2\2\2\u0102\u0103"+
		"\7/\2\2\u0103\u0104\7/\2\2\u0104\62\3\2\2\2\u0105\u0106\7`\2\2\u0106\64"+
		"\3\2\2\2\u0107\u0108\7#\2\2\u0108\66\3\2\2\2\u0109\u010a\7/\2\2\u010a"+
		"\u010b\7@\2\2\u010b8\3\2\2\2\u010c\u010d\7>\2\2\u010d\u010e\7/\2\2\u010e"+
		"\u010f\7@\2\2\u010f:\3\2\2\2\u0110\u0111\7.\2\2\u0111<\3\2\2\2\u0112\u0113"+
		"\7<\2\2\u0113>\3\2\2\2\u0114\u0115\7=\2\2\u0115@\3\2\2\2\u0116\u0117\7"+
		"*\2\2\u0117B\3\2\2\2\u0118\u0119\7+\2\2\u0119D\3\2\2\2\u011a\u011b\7]"+
		"\2\2\u011bF\3\2\2\2\u011c\u011d\7_\2\2\u011dH\3\2\2\2\u011e\u011f\7}\2"+
		"\2\u011fJ\3\2\2\2\u0120\u0121\7\177\2\2\u0121L\3\2\2\2\u0122\u0123\7\60"+
		"\2\2\u0123\u0124\7\60\2\2\u0124N\3\2\2\2\u0125\u0126\7?\2\2\u0126\u0127"+
		"\7?\2\2\u0127P\3\2\2\2\u0128\u0129\7#\2\2\u0129\u012a\7?\2\2\u012aR\3"+
		"\2\2\2\u012b\u012c\7>\2\2\u012cT\3\2\2\2\u012d\u012e\7@\2\2\u012eV\3\2"+
		"\2\2\u012f\u0130\7>\2\2\u0130\u0131\7?\2\2\u0131X\3\2\2\2\u0132\u0133"+
		"\7@\2\2\u0133\u0134\7?\2\2\u0134Z\3\2\2\2\u0135\u0136\7%\2\2\u0136\\\3"+
		"\2\2\2\u0137\u0138\7V\2\2\u0138\u0139\7t\2\2\u0139\u013a\7w\2\2\u013a"+
		"\u013b\7g\2\2\u013b^\3\2\2\2\u013c\u013d\7H\2\2\u013d\u013e\7c\2\2\u013e"+
		"\u013f\7n\2\2\u013f\u0140\7u\2\2\u0140\u0141\7g\2\2\u0141`\3\2\2\2\u0142"+
		"\u0143\7K\2\2\u0143\u0144\7p\2\2\u0144\u0145\7h\2\2\u0145\u0146\7k\2\2"+
		"\u0146\u0147\7p\2\2\u0147\u0148\7k\2\2\u0148\u0149\7v\2\2\u0149\u014a"+
		"\7g\2\2\u014ab\3\2\2\2\u014b\u014c\7e\2\2\u014c\u014d\7n\2\2\u014d\u014e"+
		"\7q\2\2\u014e\u014f\7e\2\2\u014f\u0150\7m\2\2\u0150d\3\2\2\2\u0151\u0152"+
		"\7C\2\2\u0152\u0153\7e\2\2\u0153\u0154\7v\2\2\u0154f\3\2\2\2\u0155\u0156"+
		"\7R\2\2\u0156\u0157\7T\2\2\u0157\u0158\7Q\2\2\u0158\u0159\7D\2\2\u0159"+
		"\u015a\7a\2\2\u015a\u015b\7V\2\2\u015b\u015c\7C\2\2\u015ch\3\2\2\2\u015d"+
		"\u015e\7k\2\2\u015e\u015f\7p\2\2\u015fj\3\2\2\2\u0160\u0161\7#\2\2\u0161"+
		"\u0162\7k\2\2\u0162\u0163\7p\2\2\u0163l\3\2\2\2\u0164\u0165\7E\2\2\u0165"+
		"\u0166\7P\2\2\u0166n\3\2\2\2\u0167\u0168\7E\2\2\u0168\u0169\7c\2\2\u0169"+
		"\u016a\7t\2\2\u016a\u016b\7f\2\2\u016bp\3\2\2\2\u016c\u016d\7U\2\2\u016d"+
		"\u016e\7w\2\2\u016e\u016f\7d\2\2\u016f\u0170\7e\2\2\u0170\u0171\7n\2\2"+
		"\u0171\u0172\7c\2\2\u0172\u0173\7u\2\2\u0173\u0174\7u\2\2\u0174r\3\2\2"+
		"\2\u0175\u0176\7O\2\2\u0176\u0177\7k\2\2\u0177\u0178\7p\2\2\u0178t\3\2"+
		"\2\2\u0179\u017a\7O\2\2\u017a\u017b\7c\2\2\u017b\u017c\7z\2\2\u017cv\3"+
		"\2\2\2\u017d\u017e\7O\2\2\u017e\u017f\7q\2\2\u017f\u0180\7f\2\2\u0180"+
		"x\3\2\2\2\u0181\u0182\7R\2\2\u0182\u0183\7q\2\2\u0183\u0184\7y\2\2\u0184"+
		"z\3\2\2\2\u0185\u0186\7H\2\2\u0186\u0187\7t\2\2\u0187\u0188\7c\2\2\u0188"+
		"\u0189\7e\2\2\u0189\u018a\7v\2\2\u018a|\3\2\2\2\u018b\u018c\7H\2\2\u018c"+
		"\u018d\7c\2\2\u018d\u018e\7e\2\2\u018e\u018f\7v\2\2\u018f\u0190\7q\2\2"+
		"\u0190\u0191\7t\2\2\u0191\u0192\7k\2\2\u0192\u0193\7c\2\2\u0193\u0194"+
		"\7n\2\2\u0194~\3\2\2\2\u0195\u0196\7C\2\2\u0196\u0197\7d\2\2\u0197\u0198"+
		"\7u\2\2\u0198\u0080\3\2\2\2\u0199\u019a\7U\2\2\u019a\u019b\7k\2\2\u019b"+
		"\u019c\7p\2\2\u019c\u0082\3\2\2\2\u019d\u019e\7E\2\2\u019e\u019f\7q\2"+
		"\2\u019f\u01a0\7u\2\2\u01a0\u0084\3\2\2\2\u01a1\u01a2\7V\2\2\u01a2\u01a3"+
		"\7c\2\2\u01a3\u01a4\7p\2\2\u01a4\u0086\3\2\2\2\u01a5\u01a6\7C\2\2\u01a6"+
		"\u01a7\7u\2\2\u01a7\u01a8\7k\2\2\u01a8\u01a9\7p\2\2\u01a9\u0088\3\2\2"+
		"\2\u01aa\u01ab\7C\2\2\u01ab\u01ac\7e\2\2\u01ac\u01ad\7q\2\2\u01ad\u01ae"+
		"\7u\2\2\u01ae\u008a\3\2\2\2\u01af\u01b0\7C\2\2\u01b0\u01b1\7v\2\2\u01b1"+
		"\u01b2\7c\2\2\u01b2\u01b3\7p\2\2\u01b3\u008c\3\2\2\2\u01b4\u01b5\7G\2"+
		"\2\u01b5\u01b6\7z\2\2\u01b6\u01b7\7r\2\2\u01b7\u008e\3\2\2\2\u01b8\u01b9"+
		"\7N\2\2\u01b9\u01ba\7q\2\2\u01ba\u01bb\7i\2\2\u01bb\u0090\3\2\2\2\u01bc"+
		"\u01bd\7U\2\2\u01bd\u01be\7s\2\2\u01be\u01bf\7t\2\2\u01bf\u01c0\7v\2\2"+
		"\u01c0\u0092\3\2\2\2\u01c1\u01c2\7E\2\2\u01c2\u01c3\7g\2\2\u01c3\u01c4"+
		"\7k\2\2\u01c4\u01c5\7n\2\2\u01c5\u0094\3\2\2\2\u01c6\u01c7\7H\2\2\u01c7"+
		"\u01c8\7n\2\2\u01c8\u01c9\7q\2\2\u01c9\u01ca\7q\2\2\u01ca\u01cb\7t\2\2"+
		"\u01cb\u0096\3\2\2\2\u01cc\u01cd\7T\2\2\u01cd\u01ce\7q\2\2\u01ce\u01cf"+
		"\7w\2\2\u01cf\u01d0\7p\2\2\u01d0\u01d1\7f\2\2\u01d1\u0098\3\2\2\2\u01d2"+
		"\u01d3\7D\2\2\u01d3\u01d4\7k\2\2\u01d4\u01d5\7p\2\2\u01d5\u01d6\7q\2\2"+
		"\u01d6\u01d7\7o\2\2\u01d7\u01d8\7k\2\2\u01d8\u01d9\7c\2\2\u01d9\u01da"+
		"\7n\2\2\u01da\u009a\3\2\2\2\u01db\u01dc\7K\2\2\u01dc\u01dd\7h\2\2\u01dd"+
		"\u009c\3\2\2\2\u01de\u01e0\t\2\2\2\u01df\u01de\3\2\2\2\u01e0\u01e1\3\2"+
		"\2\2\u01e1\u01df\3\2\2\2\u01e1\u01e2\3\2\2\2\u01e2\u01e3\3\2\2\2\u01e3"+
		"\u01e4\bO\2\2\u01e4\u009e\3\2\2\2\u01e5\u01e6\t\3\2\2\u01e6\u00a0\3\2"+
		"\2\2\u01e7\u01e8\4\62;\2\u01e8\u00a2\3\2\2\2\u01e9\u01ea\t\4\2\2\u01ea"+
		"\u00a4\3\2\2\2\u01eb\u01ed\7g\2\2\u01ec\u01ee\5\u00a3R\2\u01ed\u01ec\3"+
		"\2\2\2\u01ed\u01ee\3\2\2\2\u01ee\u01f0\3\2\2\2\u01ef\u01f1\5\u00a1Q\2"+
		"\u01f0\u01ef\3\2\2\2\u01f1\u01f2\3\2\2\2\u01f2\u01f0\3\2\2\2\u01f2\u01f3"+
		"\3\2\2\2\u01f3\u01fe\3\2\2\2\u01f4\u01f6\7G\2\2\u01f5\u01f7\5\u00a3R\2"+
		"\u01f6\u01f5\3\2\2\2\u01f6\u01f7\3\2\2\2\u01f7\u01f9\3\2\2\2\u01f8\u01fa"+
		"\5\u00a1Q\2\u01f9\u01f8\3\2\2\2\u01fa\u01fb\3\2\2\2\u01fb\u01f9\3\2\2"+
		"\2\u01fb\u01fc\3\2\2\2\u01fc\u01fe\3\2\2\2\u01fd\u01eb\3\2\2\2\u01fd\u01f4"+
		"\3\2\2\2\u01fe\u00a6\3\2\2\2\u01ff\u0204\5\u009fP\2\u0200\u0203\5\u009f"+
		"P\2\u0201\u0203\5\u00a1Q\2\u0202\u0200\3\2\2\2\u0202\u0201\3\2\2\2\u0203"+
		"\u0206\3\2\2\2\u0204\u0202\3\2\2\2\u0204\u0205\3\2\2\2\u0205\u0207\3\2"+
		"\2\2\u0206\u0204\3\2\2\2\u0207\u0208\bT\3\2\u0208\u00a8\3\2\2\2\u0209"+
		"\u020b\5\u00a1Q\2\u020a\u0209\3\2\2\2\u020b\u020c\3\2\2\2\u020c\u020a"+
		"\3\2\2\2\u020c\u020d\3\2\2\2\u020d\u00aa\3\2\2\2\u020e\u0210\5\u00a1Q"+
		"\2\u020f\u020e\3\2\2\2\u0210\u0211\3\2\2\2\u0211\u020f\3\2\2\2\u0211\u0212"+
		"\3\2\2\2\u0212\u0213\3\2\2\2\u0213\u0215\7\60\2\2\u0214\u0216\5\u00a1"+
		"Q\2\u0215\u0214\3\2\2\2\u0216\u0217\3\2\2\2\u0217\u0215\3\2\2\2\u0217"+
		"\u0218\3\2\2\2\u0218\u021a\3\2\2\2\u0219\u021b\5\u00a5S\2\u021a\u0219"+
		"\3\2\2\2\u021a\u021b\3\2\2\2\u021b\u0226\3\2\2\2\u021c\u021e\7\60\2\2"+
		"\u021d\u021f\5\u00a1Q\2\u021e\u021d\3\2\2\2\u021f\u0220\3\2\2\2\u0220"+
		"\u021e\3\2\2\2\u0220\u0221\3\2\2\2\u0221\u0223\3\2\2\2\u0222\u0224\5\u00a5"+
		"S\2\u0223\u0222\3\2\2\2\u0223\u0224\3\2\2\2\u0224\u0226\3\2\2\2\u0225"+
		"\u020f\3\2\2\2\u0225\u021c\3\2\2\2\u0226\u00ac\3\2\2\2\22\2\u01e1\u01ed"+
		"\u01f2\u01f6\u01fb\u01fd\u0202\u0204\u020c\u0211\u0217\u021a\u0220\u0223"+
		"\u0225\4\b\2\2\3T\2";
	public static final ATN _ATN =
		new ATNDeserializer().deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
		for (int i = 0; i < _ATN.getNumberOfDecisions(); i++) {
			_decisionToDFA[i] = new DFA(_ATN.getDecisionState(i), i);
		}
	}
}