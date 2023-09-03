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
		T__20=1, T__19=2, T__18=3, T__17=4, T__16=5, T__15=6, T__14=7, T__13=8, 
		T__12=9, T__11=10, T__10=11, T__9=12, T__8=13, T__7=14, T__6=15, T__5=16, 
		T__4=17, T__3=18, T__2=19, T__1=20, T__0=21, MUL=22, DIV=23, ADD=24, SUB=25, 
		AND=26, OR=27, POSTINCR=28, POSTDECR=29, HAT=30, NOT=31, IMPLY=32, BIIMPLY=33, 
		COMMA=34, COLON=35, SEMICOLON=36, OP_PAREN=37, CL_PAREN=38, OP_SQPAR=39, 
		CL_SQPAR=40, OP_BRACK=41, CL_BRACK=42, TWODOTS=43, EQUAL=44, NOT_EQUAL=45, 
		LESS=46, GREATER=47, LESS_EQ=48, GREATER_EQ=49, SHARP=50, TRUE=51, FALSE=52, 
		INFINITY=53, CLOCK=54, ACT=55, PROB_TA=56, CONTAINS=57, DONT_CONTAINS=58, 
		COLOR_ORDINAL=59, MULTISET_CARD=60, MULTISET_SUBCLASS=61, MIN_FN=62, MAX_FN=63, 
		MOD_FN=64, POW_FN=65, FRACT_FN=66, FACTORIAL_FN=67, ABS_FN=68, SIN_FN=69, 
		COS_FN=70, TAN_FN=71, ARCSIN_FN=72, ARCCOS_FN=73, ARCTAN_FN=74, EXP_FN=75, 
		LOG_FN=76, SQRT_FN=77, CEIL_FN=78, FLOOR_FN=79, ROUND_FN=80, BINOMIAL_FN=81, 
		IF_FN=82, WHITESPACES=83, ID=84, INT=85, REAL=86, STRING_LITERAL=87;
	public static String[] modeNames = {
		"DEFAULT_MODE"
	};

	public static final String[] tokenNames = {
		"<INVALID>",
		"'ordered'", "'='", "'agent'", "'ever'", "'X{'", "'@'", "'Call'", "'P{'", 
		"'circular'", "'unordered'", "'FromList'", "'\\'", "'?'", "'enum'", "'when'", 
		"'All'", "'is'", "'FromTimeTable'", "'E{'", "'|'", "'FromTable'", "'*'", 
		"'/'", "'+'", "'-'", "'&&'", "'||'", "'++'", "'--'", "'^'", "'!'", "'->'", 
		"'<->'", "','", "':'", "';'", "'('", "')'", "'['", "']'", "'{'", "'}'", 
		"'..'", "'=='", "'!='", "'<'", "'>'", "'<='", "'>='", "'#'", "'True'", 
		"'False'", "'Infinite'", "'clock'", "'Act'", "'PROB_TA'", "'in'", "'!in'", 
		"'CN'", "'Card'", "'Subclass'", "'Min'", "'Max'", "'Mod'", "'Pow'", "'Fract'", 
		"'Factorial'", "'Abs'", "'Sin'", "'Cos'", "'Tan'", "'Asin'", "'Acos'", 
		"'Atan'", "'Exp'", "'Log'", "'Sqrt'", "'Ceil'", "'Floor'", "'Round'", 
		"'Binomial'", "'If'", "WHITESPACES", "ID", "INT", "REAL", "STRING_LITERAL"
	};
	public static final String[] ruleNames = {
		"T__20", "T__19", "T__18", "T__17", "T__16", "T__15", "T__14", "T__13", 
		"T__12", "T__11", "T__10", "T__9", "T__8", "T__7", "T__6", "T__5", "T__4", 
		"T__3", "T__2", "T__1", "T__0", "MUL", "DIV", "ADD", "SUB", "AND", "OR", 
		"POSTINCR", "POSTDECR", "HAT", "NOT", "IMPLY", "BIIMPLY", "COMMA", "COLON", 
		"SEMICOLON", "OP_PAREN", "CL_PAREN", "OP_SQPAR", "CL_SQPAR", "OP_BRACK", 
		"CL_BRACK", "TWODOTS", "EQUAL", "NOT_EQUAL", "LESS", "GREATER", "LESS_EQ", 
		"GREATER_EQ", "SHARP", "TRUE", "FALSE", "INFINITY", "CLOCK", "ACT", "PROB_TA", 
		"CONTAINS", "DONT_CONTAINS", "COLOR_ORDINAL", "MULTISET_CARD", "MULTISET_SUBCLASS", 
		"MIN_FN", "MAX_FN", "MOD_FN", "POW_FN", "FRACT_FN", "FACTORIAL_FN", "ABS_FN", 
		"SIN_FN", "COS_FN", "TAN_FN", "ARCSIN_FN", "ARCCOS_FN", "ARCTAN_FN", "EXP_FN", 
		"LOG_FN", "SQRT_FN", "CEIL_FN", "FLOOR_FN", "ROUND_FN", "BINOMIAL_FN", 
		"IF_FN", "WHITESPACES", "ID_LETTER", "DIGIT", "SignPart", "ExponentPart", 
		"ID", "INT", "REAL", "STRING_LITERAL", "SCharSequence", "SChar"
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
		case 87: ID_action((RuleContext)_localctx, actionIndex); break;
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
		            case "possibly":       setType(ExprLangParser.POSSIBLY);       return;
		            case "impossibly":     setType(ExprLangParser.IMPOSSIBLY);     return;
		            case "invariantly":    setType(ExprLangParser.INVARIANTLY);    return;
		            case "en":             setType(ExprLangParser.ENABLED);        return;
		            case "bounds":         setType(ExprLangParser.BOUNDS);         return;
		            case "deadlock":       setType(ExprLangParser.DEADLOCK);       return;
		            case "ndeadlock":      setType(ExprLangParser.NO_DEADLOCK);    return;
		            case "initial":        setType(ExprLangParser.INITIAL_STATE);  return;
		            case "has_deadlock":   setType(ExprLangParser.HAS_DEADLOCK);   return;
		            case "quasi_liveness": setType(ExprLangParser.QUASI_LIVENESS); return;
		            case "stable_marking": setType(ExprLangParser.STABLE_MARKING); return;
		            case "liveness":       setType(ExprLangParser.LIVENESS);       return;
		            case "onesafe":        setType(ExprLangParser.ONESAFE);        return;
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
		"\3\u0430\ud6d1\u8206\uad2d\u4417\uaef1\u8d80\uaadd\2Y\u0270\b\1\4\2\t"+
		"\2\4\3\t\3\4\4\t\4\4\5\t\5\4\6\t\6\4\7\t\7\4\b\t\b\4\t\t\t\4\n\t\n\4\13"+
		"\t\13\4\f\t\f\4\r\t\r\4\16\t\16\4\17\t\17\4\20\t\20\4\21\t\21\4\22\t\22"+
		"\4\23\t\23\4\24\t\24\4\25\t\25\4\26\t\26\4\27\t\27\4\30\t\30\4\31\t\31"+
		"\4\32\t\32\4\33\t\33\4\34\t\34\4\35\t\35\4\36\t\36\4\37\t\37\4 \t \4!"+
		"\t!\4\"\t\"\4#\t#\4$\t$\4%\t%\4&\t&\4\'\t\'\4(\t(\4)\t)\4*\t*\4+\t+\4"+
		",\t,\4-\t-\4.\t.\4/\t/\4\60\t\60\4\61\t\61\4\62\t\62\4\63\t\63\4\64\t"+
		"\64\4\65\t\65\4\66\t\66\4\67\t\67\48\t8\49\t9\4:\t:\4;\t;\4<\t<\4=\t="+
		"\4>\t>\4?\t?\4@\t@\4A\tA\4B\tB\4C\tC\4D\tD\4E\tE\4F\tF\4G\tG\4H\tH\4I"+
		"\tI\4J\tJ\4K\tK\4L\tL\4M\tM\4N\tN\4O\tO\4P\tP\4Q\tQ\4R\tR\4S\tS\4T\tT"+
		"\4U\tU\4V\tV\4W\tW\4X\tX\4Y\tY\4Z\tZ\4[\t[\4\\\t\\\4]\t]\4^\t^\3\2\3\2"+
		"\3\2\3\2\3\2\3\2\3\2\3\2\3\3\3\3\3\4\3\4\3\4\3\4\3\4\3\4\3\5\3\5\3\5\3"+
		"\5\3\5\3\6\3\6\3\6\3\7\3\7\3\b\3\b\3\b\3\b\3\b\3\t\3\t\3\t\3\n\3\n\3\n"+
		"\3\n\3\n\3\n\3\n\3\n\3\n\3\13\3\13\3\13\3\13\3\13\3\13\3\13\3\13\3\13"+
		"\3\13\3\f\3\f\3\f\3\f\3\f\3\f\3\f\3\f\3\f\3\r\3\r\3\16\3\16\3\17\3\17"+
		"\3\17\3\17\3\17\3\20\3\20\3\20\3\20\3\20\3\21\3\21\3\21\3\21\3\22\3\22"+
		"\3\22\3\23\3\23\3\23\3\23\3\23\3\23\3\23\3\23\3\23\3\23\3\23\3\23\3\23"+
		"\3\23\3\24\3\24\3\24\3\25\3\25\3\26\3\26\3\26\3\26\3\26\3\26\3\26\3\26"+
		"\3\26\3\26\3\27\3\27\3\30\3\30\3\31\3\31\3\32\3\32\3\33\3\33\3\33\3\34"+
		"\3\34\3\34\3\35\3\35\3\35\3\36\3\36\3\36\3\37\3\37\3 \3 \3!\3!\3!\3\""+
		"\3\"\3\"\3\"\3#\3#\3$\3$\3%\3%\3&\3&\3\'\3\'\3(\3(\3)\3)\3*\3*\3+\3+\3"+
		",\3,\3,\3-\3-\3-\3.\3.\3.\3/\3/\3\60\3\60\3\61\3\61\3\61\3\62\3\62\3\62"+
		"\3\63\3\63\3\64\3\64\3\64\3\64\3\64\3\65\3\65\3\65\3\65\3\65\3\65\3\66"+
		"\3\66\3\66\3\66\3\66\3\66\3\66\3\66\3\66\3\67\3\67\3\67\3\67\3\67\3\67"+
		"\38\38\38\38\39\39\39\39\39\39\39\39\3:\3:\3:\3;\3;\3;\3;\3<\3<\3<\3="+
		"\3=\3=\3=\3=\3>\3>\3>\3>\3>\3>\3>\3>\3>\3?\3?\3?\3?\3@\3@\3@\3@\3A\3A"+
		"\3A\3A\3B\3B\3B\3B\3C\3C\3C\3C\3C\3C\3D\3D\3D\3D\3D\3D\3D\3D\3D\3D\3E"+
		"\3E\3E\3E\3F\3F\3F\3F\3G\3G\3G\3G\3H\3H\3H\3H\3I\3I\3I\3I\3I\3J\3J\3J"+
		"\3J\3J\3K\3K\3K\3K\3K\3L\3L\3L\3L\3M\3M\3M\3M\3N\3N\3N\3N\3N\3O\3O\3O"+
		"\3O\3O\3P\3P\3P\3P\3P\3P\3Q\3Q\3Q\3Q\3Q\3Q\3R\3R\3R\3R\3R\3R\3R\3R\3R"+
		"\3S\3S\3S\3T\6T\u021c\nT\rT\16T\u021d\3T\3T\3U\3U\3V\3V\3W\3W\3X\3X\5"+
		"X\u022a\nX\3X\6X\u022d\nX\rX\16X\u022e\3X\3X\5X\u0233\nX\3X\6X\u0236\n"+
		"X\rX\16X\u0237\5X\u023a\nX\3Y\3Y\3Y\7Y\u023f\nY\fY\16Y\u0242\13Y\3Y\3"+
		"Y\3Z\6Z\u0247\nZ\rZ\16Z\u0248\3[\6[\u024c\n[\r[\16[\u024d\3[\3[\6[\u0252"+
		"\n[\r[\16[\u0253\3[\5[\u0257\n[\3[\3[\6[\u025b\n[\r[\16[\u025c\3[\5[\u0260"+
		"\n[\5[\u0262\n[\3\\\3\\\5\\\u0266\n\\\3\\\3\\\3]\6]\u026b\n]\r]\16]\u026c"+
		"\3^\3^\2\2_\3\3\5\4\7\5\t\6\13\7\r\b\17\t\21\n\23\13\25\f\27\r\31\16\33"+
		"\17\35\20\37\21!\22#\23%\24\'\25)\26+\27-\30/\31\61\32\63\33\65\34\67"+
		"\359\36;\37= ?!A\"C#E$G%I&K\'M(O)Q*S+U,W-Y.[/]\60_\61a\62c\63e\64g\65"+
		"i\66k\67m8o9q:s;u<w=y>{?}@\177A\u0081B\u0083C\u0085D\u0087E\u0089F\u008b"+
		"G\u008dH\u008fI\u0091J\u0093K\u0095L\u0097M\u0099N\u009bO\u009dP\u009f"+
		"Q\u00a1R\u00a3S\u00a5T\u00a7U\u00a9\2\u00ab\2\u00ad\2\u00af\2\u00b1V\u00b3"+
		"W\u00b5X\u00b7Y\u00b9\2\u00bb\2\3\2\6\5\2\13\f\17\17\"\"\5\2C\\aac|\4"+
		"\2--//\6\2\f\f\17\17$$^^\u027a\2\3\3\2\2\2\2\5\3\2\2\2\2\7\3\2\2\2\2\t"+
		"\3\2\2\2\2\13\3\2\2\2\2\r\3\2\2\2\2\17\3\2\2\2\2\21\3\2\2\2\2\23\3\2\2"+
		"\2\2\25\3\2\2\2\2\27\3\2\2\2\2\31\3\2\2\2\2\33\3\2\2\2\2\35\3\2\2\2\2"+
		"\37\3\2\2\2\2!\3\2\2\2\2#\3\2\2\2\2%\3\2\2\2\2\'\3\2\2\2\2)\3\2\2\2\2"+
		"+\3\2\2\2\2-\3\2\2\2\2/\3\2\2\2\2\61\3\2\2\2\2\63\3\2\2\2\2\65\3\2\2\2"+
		"\2\67\3\2\2\2\29\3\2\2\2\2;\3\2\2\2\2=\3\2\2\2\2?\3\2\2\2\2A\3\2\2\2\2"+
		"C\3\2\2\2\2E\3\2\2\2\2G\3\2\2\2\2I\3\2\2\2\2K\3\2\2\2\2M\3\2\2\2\2O\3"+
		"\2\2\2\2Q\3\2\2\2\2S\3\2\2\2\2U\3\2\2\2\2W\3\2\2\2\2Y\3\2\2\2\2[\3\2\2"+
		"\2\2]\3\2\2\2\2_\3\2\2\2\2a\3\2\2\2\2c\3\2\2\2\2e\3\2\2\2\2g\3\2\2\2\2"+
		"i\3\2\2\2\2k\3\2\2\2\2m\3\2\2\2\2o\3\2\2\2\2q\3\2\2\2\2s\3\2\2\2\2u\3"+
		"\2\2\2\2w\3\2\2\2\2y\3\2\2\2\2{\3\2\2\2\2}\3\2\2\2\2\177\3\2\2\2\2\u0081"+
		"\3\2\2\2\2\u0083\3\2\2\2\2\u0085\3\2\2\2\2\u0087\3\2\2\2\2\u0089\3\2\2"+
		"\2\2\u008b\3\2\2\2\2\u008d\3\2\2\2\2\u008f\3\2\2\2\2\u0091\3\2\2\2\2\u0093"+
		"\3\2\2\2\2\u0095\3\2\2\2\2\u0097\3\2\2\2\2\u0099\3\2\2\2\2\u009b\3\2\2"+
		"\2\2\u009d\3\2\2\2\2\u009f\3\2\2\2\2\u00a1\3\2\2\2\2\u00a3\3\2\2\2\2\u00a5"+
		"\3\2\2\2\2\u00a7\3\2\2\2\2\u00b1\3\2\2\2\2\u00b3\3\2\2\2\2\u00b5\3\2\2"+
		"\2\2\u00b7\3\2\2\2\3\u00bd\3\2\2\2\5\u00c5\3\2\2\2\7\u00c7\3\2\2\2\t\u00cd"+
		"\3\2\2\2\13\u00d2\3\2\2\2\r\u00d5\3\2\2\2\17\u00d7\3\2\2\2\21\u00dc\3"+
		"\2\2\2\23\u00df\3\2\2\2\25\u00e8\3\2\2\2\27\u00f2\3\2\2\2\31\u00fb\3\2"+
		"\2\2\33\u00fd\3\2\2\2\35\u00ff\3\2\2\2\37\u0104\3\2\2\2!\u0109\3\2\2\2"+
		"#\u010d\3\2\2\2%\u0110\3\2\2\2\'\u011e\3\2\2\2)\u0121\3\2\2\2+\u0123\3"+
		"\2\2\2-\u012d\3\2\2\2/\u012f\3\2\2\2\61\u0131\3\2\2\2\63\u0133\3\2\2\2"+
		"\65\u0135\3\2\2\2\67\u0138\3\2\2\29\u013b\3\2\2\2;\u013e\3\2\2\2=\u0141"+
		"\3\2\2\2?\u0143\3\2\2\2A\u0145\3\2\2\2C\u0148\3\2\2\2E\u014c\3\2\2\2G"+
		"\u014e\3\2\2\2I\u0150\3\2\2\2K\u0152\3\2\2\2M\u0154\3\2\2\2O\u0156\3\2"+
		"\2\2Q\u0158\3\2\2\2S\u015a\3\2\2\2U\u015c\3\2\2\2W\u015e\3\2\2\2Y\u0161"+
		"\3\2\2\2[\u0164\3\2\2\2]\u0167\3\2\2\2_\u0169\3\2\2\2a\u016b\3\2\2\2c"+
		"\u016e\3\2\2\2e\u0171\3\2\2\2g\u0173\3\2\2\2i\u0178\3\2\2\2k\u017e\3\2"+
		"\2\2m\u0187\3\2\2\2o\u018d\3\2\2\2q\u0191\3\2\2\2s\u0199\3\2\2\2u\u019c"+
		"\3\2\2\2w\u01a0\3\2\2\2y\u01a3\3\2\2\2{\u01a8\3\2\2\2}\u01b1\3\2\2\2\177"+
		"\u01b5\3\2\2\2\u0081\u01b9\3\2\2\2\u0083\u01bd\3\2\2\2\u0085\u01c1\3\2"+
		"\2\2\u0087\u01c7\3\2\2\2\u0089\u01d1\3\2\2\2\u008b\u01d5\3\2\2\2\u008d"+
		"\u01d9\3\2\2\2\u008f\u01dd\3\2\2\2\u0091\u01e1\3\2\2\2\u0093\u01e6\3\2"+
		"\2\2\u0095\u01eb\3\2\2\2\u0097\u01f0\3\2\2\2\u0099\u01f4\3\2\2\2\u009b"+
		"\u01f8\3\2\2\2\u009d\u01fd\3\2\2\2\u009f\u0202\3\2\2\2\u00a1\u0208\3\2"+
		"\2\2\u00a3\u020e\3\2\2\2\u00a5\u0217\3\2\2\2\u00a7\u021b\3\2\2\2\u00a9"+
		"\u0221\3\2\2\2\u00ab\u0223\3\2\2\2\u00ad\u0225\3\2\2\2\u00af\u0239\3\2"+
		"\2\2\u00b1\u023b\3\2\2\2\u00b3\u0246\3\2\2\2\u00b5\u0261\3\2\2\2\u00b7"+
		"\u0263\3\2\2\2\u00b9\u026a\3\2\2\2\u00bb\u026e\3\2\2\2\u00bd\u00be\7q"+
		"\2\2\u00be\u00bf\7t\2\2\u00bf\u00c0\7f\2\2\u00c0\u00c1\7g\2\2\u00c1\u00c2"+
		"\7t\2\2\u00c2\u00c3\7g\2\2\u00c3\u00c4\7f\2\2\u00c4\4\3\2\2\2\u00c5\u00c6"+
		"\7?\2\2\u00c6\6\3\2\2\2\u00c7\u00c8\7c\2\2\u00c8\u00c9\7i\2\2\u00c9\u00ca"+
		"\7g\2\2\u00ca\u00cb\7p\2\2\u00cb\u00cc\7v\2\2\u00cc\b\3\2\2\2\u00cd\u00ce"+
		"\7g\2\2\u00ce\u00cf\7x\2\2\u00cf\u00d0\7g\2\2\u00d0\u00d1\7t\2\2\u00d1"+
		"\n\3\2\2\2\u00d2\u00d3\7Z\2\2\u00d3\u00d4\7}\2\2\u00d4\f\3\2\2\2\u00d5"+
		"\u00d6\7B\2\2\u00d6\16\3\2\2\2\u00d7\u00d8\7E\2\2\u00d8\u00d9\7c\2\2\u00d9"+
		"\u00da\7n\2\2\u00da\u00db\7n\2\2\u00db\20\3\2\2\2\u00dc\u00dd\7R\2\2\u00dd"+
		"\u00de\7}\2\2\u00de\22\3\2\2\2\u00df\u00e0\7e\2\2\u00e0\u00e1\7k\2\2\u00e1"+
		"\u00e2\7t\2\2\u00e2\u00e3\7e\2\2\u00e3\u00e4\7w\2\2\u00e4\u00e5\7n\2\2"+
		"\u00e5\u00e6\7c\2\2\u00e6\u00e7\7t\2\2\u00e7\24\3\2\2\2\u00e8\u00e9\7"+
		"w\2\2\u00e9\u00ea\7p\2\2\u00ea\u00eb\7q\2\2\u00eb\u00ec\7t\2\2\u00ec\u00ed"+
		"\7f\2\2\u00ed\u00ee\7g\2\2\u00ee\u00ef\7t\2\2\u00ef\u00f0\7g\2\2\u00f0"+
		"\u00f1\7f\2\2\u00f1\26\3\2\2\2\u00f2\u00f3\7H\2\2\u00f3\u00f4\7t\2\2\u00f4"+
		"\u00f5\7q\2\2\u00f5\u00f6\7o\2\2\u00f6\u00f7\7N\2\2\u00f7\u00f8\7k\2\2"+
		"\u00f8\u00f9\7u\2\2\u00f9\u00fa\7v\2\2\u00fa\30\3\2\2\2\u00fb\u00fc\7"+
		"^\2\2\u00fc\32\3\2\2\2\u00fd\u00fe\7A\2\2\u00fe\34\3\2\2\2\u00ff\u0100"+
		"\7g\2\2\u0100\u0101\7p\2\2\u0101\u0102\7w\2\2\u0102\u0103\7o\2\2\u0103"+
		"\36\3\2\2\2\u0104\u0105\7y\2\2\u0105\u0106\7j\2\2\u0106\u0107\7g\2\2\u0107"+
		"\u0108\7p\2\2\u0108 \3\2\2\2\u0109\u010a\7C\2\2\u010a\u010b\7n\2\2\u010b"+
		"\u010c\7n\2\2\u010c\"\3\2\2\2\u010d\u010e\7k\2\2\u010e\u010f\7u\2\2\u010f"+
		"$\3\2\2\2\u0110\u0111\7H\2\2\u0111\u0112\7t\2\2\u0112\u0113\7q\2\2\u0113"+
		"\u0114\7o\2\2\u0114\u0115\7V\2\2\u0115\u0116\7k\2\2\u0116\u0117\7o\2\2"+
		"\u0117\u0118\7g\2\2\u0118\u0119\7V\2\2\u0119\u011a\7c\2\2\u011a\u011b"+
		"\7d\2\2\u011b\u011c\7n\2\2\u011c\u011d\7g\2\2\u011d&\3\2\2\2\u011e\u011f"+
		"\7G\2\2\u011f\u0120\7}\2\2\u0120(\3\2\2\2\u0121\u0122\7~\2\2\u0122*\3"+
		"\2\2\2\u0123\u0124\7H\2\2\u0124\u0125\7t\2\2\u0125\u0126\7q\2\2\u0126"+
		"\u0127\7o\2\2\u0127\u0128\7V\2\2\u0128\u0129\7c\2\2\u0129\u012a\7d\2\2"+
		"\u012a\u012b\7n\2\2\u012b\u012c\7g\2\2\u012c,\3\2\2\2\u012d\u012e\7,\2"+
		"\2\u012e.\3\2\2\2\u012f\u0130\7\61\2\2\u0130\60\3\2\2\2\u0131\u0132\7"+
		"-\2\2\u0132\62\3\2\2\2\u0133\u0134\7/\2\2\u0134\64\3\2\2\2\u0135\u0136"+
		"\7(\2\2\u0136\u0137\7(\2\2\u0137\66\3\2\2\2\u0138\u0139\7~\2\2\u0139\u013a"+
		"\7~\2\2\u013a8\3\2\2\2\u013b\u013c\7-\2\2\u013c\u013d\7-\2\2\u013d:\3"+
		"\2\2\2\u013e\u013f\7/\2\2\u013f\u0140\7/\2\2\u0140<\3\2\2\2\u0141\u0142"+
		"\7`\2\2\u0142>\3\2\2\2\u0143\u0144\7#\2\2\u0144@\3\2\2\2\u0145\u0146\7"+
		"/\2\2\u0146\u0147\7@\2\2\u0147B\3\2\2\2\u0148\u0149\7>\2\2\u0149\u014a"+
		"\7/\2\2\u014a\u014b\7@\2\2\u014bD\3\2\2\2\u014c\u014d\7.\2\2\u014dF\3"+
		"\2\2\2\u014e\u014f\7<\2\2\u014fH\3\2\2\2\u0150\u0151\7=\2\2\u0151J\3\2"+
		"\2\2\u0152\u0153\7*\2\2\u0153L\3\2\2\2\u0154\u0155\7+\2\2\u0155N\3\2\2"+
		"\2\u0156\u0157\7]\2\2\u0157P\3\2\2\2\u0158\u0159\7_\2\2\u0159R\3\2\2\2"+
		"\u015a\u015b\7}\2\2\u015bT\3\2\2\2\u015c\u015d\7\177\2\2\u015dV\3\2\2"+
		"\2\u015e\u015f\7\60\2\2\u015f\u0160\7\60\2\2\u0160X\3\2\2\2\u0161\u0162"+
		"\7?\2\2\u0162\u0163\7?\2\2\u0163Z\3\2\2\2\u0164\u0165\7#\2\2\u0165\u0166"+
		"\7?\2\2\u0166\\\3\2\2\2\u0167\u0168\7>\2\2\u0168^\3\2\2\2\u0169\u016a"+
		"\7@\2\2\u016a`\3\2\2\2\u016b\u016c\7>\2\2\u016c\u016d\7?\2\2\u016db\3"+
		"\2\2\2\u016e\u016f\7@\2\2\u016f\u0170\7?\2\2\u0170d\3\2\2\2\u0171\u0172"+
		"\7%\2\2\u0172f\3\2\2\2\u0173\u0174\7V\2\2\u0174\u0175\7t\2\2\u0175\u0176"+
		"\7w\2\2\u0176\u0177\7g\2\2\u0177h\3\2\2\2\u0178\u0179\7H\2\2\u0179\u017a"+
		"\7c\2\2\u017a\u017b\7n\2\2\u017b\u017c\7u\2\2\u017c\u017d\7g\2\2\u017d"+
		"j\3\2\2\2\u017e\u017f\7K\2\2\u017f\u0180\7p\2\2\u0180\u0181\7h\2\2\u0181"+
		"\u0182\7k\2\2\u0182\u0183\7p\2\2\u0183\u0184\7k\2\2\u0184\u0185\7v\2\2"+
		"\u0185\u0186\7g\2\2\u0186l\3\2\2\2\u0187\u0188\7e\2\2\u0188\u0189\7n\2"+
		"\2\u0189\u018a\7q\2\2\u018a\u018b\7e\2\2\u018b\u018c\7m\2\2\u018cn\3\2"+
		"\2\2\u018d\u018e\7C\2\2\u018e\u018f\7e\2\2\u018f\u0190\7v\2\2\u0190p\3"+
		"\2\2\2\u0191\u0192\7R\2\2\u0192\u0193\7T\2\2\u0193\u0194\7Q\2\2\u0194"+
		"\u0195\7D\2\2\u0195\u0196\7a\2\2\u0196\u0197\7V\2\2\u0197\u0198\7C\2\2"+
		"\u0198r\3\2\2\2\u0199\u019a\7k\2\2\u019a\u019b\7p\2\2\u019bt\3\2\2\2\u019c"+
		"\u019d\7#\2\2\u019d\u019e\7k\2\2\u019e\u019f\7p\2\2\u019fv\3\2\2\2\u01a0"+
		"\u01a1\7E\2\2\u01a1\u01a2\7P\2\2\u01a2x\3\2\2\2\u01a3\u01a4\7E\2\2\u01a4"+
		"\u01a5\7c\2\2\u01a5\u01a6\7t\2\2\u01a6\u01a7\7f\2\2\u01a7z\3\2\2\2\u01a8"+
		"\u01a9\7U\2\2\u01a9\u01aa\7w\2\2\u01aa\u01ab\7d\2\2\u01ab\u01ac\7e\2\2"+
		"\u01ac\u01ad\7n\2\2\u01ad\u01ae\7c\2\2\u01ae\u01af\7u\2\2\u01af\u01b0"+
		"\7u\2\2\u01b0|\3\2\2\2\u01b1\u01b2\7O\2\2\u01b2\u01b3\7k\2\2\u01b3\u01b4"+
		"\7p\2\2\u01b4~\3\2\2\2\u01b5\u01b6\7O\2\2\u01b6\u01b7\7c\2\2\u01b7\u01b8"+
		"\7z\2\2\u01b8\u0080\3\2\2\2\u01b9\u01ba\7O\2\2\u01ba\u01bb\7q\2\2\u01bb"+
		"\u01bc\7f\2\2\u01bc\u0082\3\2\2\2\u01bd\u01be\7R\2\2\u01be\u01bf\7q\2"+
		"\2\u01bf\u01c0\7y\2\2\u01c0\u0084\3\2\2\2\u01c1\u01c2\7H\2\2\u01c2\u01c3"+
		"\7t\2\2\u01c3\u01c4\7c\2\2\u01c4\u01c5\7e\2\2\u01c5\u01c6\7v\2\2\u01c6"+
		"\u0086\3\2\2\2\u01c7\u01c8\7H\2\2\u01c8\u01c9\7c\2\2\u01c9\u01ca\7e\2"+
		"\2\u01ca\u01cb\7v\2\2\u01cb\u01cc\7q\2\2\u01cc\u01cd\7t\2\2\u01cd\u01ce"+
		"\7k\2\2\u01ce\u01cf\7c\2\2\u01cf\u01d0\7n\2\2\u01d0\u0088\3\2\2\2\u01d1"+
		"\u01d2\7C\2\2\u01d2\u01d3\7d\2\2\u01d3\u01d4\7u\2\2\u01d4\u008a\3\2\2"+
		"\2\u01d5\u01d6\7U\2\2\u01d6\u01d7\7k\2\2\u01d7\u01d8\7p\2\2\u01d8\u008c"+
		"\3\2\2\2\u01d9\u01da\7E\2\2\u01da\u01db\7q\2\2\u01db\u01dc\7u\2\2\u01dc"+
		"\u008e\3\2\2\2\u01dd\u01de\7V\2\2\u01de\u01df\7c\2\2\u01df\u01e0\7p\2"+
		"\2\u01e0\u0090\3\2\2\2\u01e1\u01e2\7C\2\2\u01e2\u01e3\7u\2\2\u01e3\u01e4"+
		"\7k\2\2\u01e4\u01e5\7p\2\2\u01e5\u0092\3\2\2\2\u01e6\u01e7\7C\2\2\u01e7"+
		"\u01e8\7e\2\2\u01e8\u01e9\7q\2\2\u01e9\u01ea\7u\2\2\u01ea\u0094\3\2\2"+
		"\2\u01eb\u01ec\7C\2\2\u01ec\u01ed\7v\2\2\u01ed\u01ee\7c\2\2\u01ee\u01ef"+
		"\7p\2\2\u01ef\u0096\3\2\2\2\u01f0\u01f1\7G\2\2\u01f1\u01f2\7z\2\2\u01f2"+
		"\u01f3\7r\2\2\u01f3\u0098\3\2\2\2\u01f4\u01f5\7N\2\2\u01f5\u01f6\7q\2"+
		"\2\u01f6\u01f7\7i\2\2\u01f7\u009a\3\2\2\2\u01f8\u01f9\7U\2\2\u01f9\u01fa"+
		"\7s\2\2\u01fa\u01fb\7t\2\2\u01fb\u01fc\7v\2\2\u01fc\u009c\3\2\2\2\u01fd"+
		"\u01fe\7E\2\2\u01fe\u01ff\7g\2\2\u01ff\u0200\7k\2\2\u0200\u0201\7n\2\2"+
		"\u0201\u009e\3\2\2\2\u0202\u0203\7H\2\2\u0203\u0204\7n\2\2\u0204\u0205"+
		"\7q\2\2\u0205\u0206\7q\2\2\u0206\u0207\7t\2\2\u0207\u00a0\3\2\2\2\u0208"+
		"\u0209\7T\2\2\u0209\u020a\7q\2\2\u020a\u020b\7w\2\2\u020b\u020c\7p\2\2"+
		"\u020c\u020d\7f\2\2\u020d\u00a2\3\2\2\2\u020e\u020f\7D\2\2\u020f\u0210"+
		"\7k\2\2\u0210\u0211\7p\2\2\u0211\u0212\7q\2\2\u0212\u0213\7o\2\2\u0213"+
		"\u0214\7k\2\2\u0214\u0215\7c\2\2\u0215\u0216\7n\2\2\u0216\u00a4\3\2\2"+
		"\2\u0217\u0218\7K\2\2\u0218\u0219\7h\2\2\u0219\u00a6\3\2\2\2\u021a\u021c"+
		"\t\2\2\2\u021b\u021a\3\2\2\2\u021c\u021d\3\2\2\2\u021d\u021b\3\2\2\2\u021d"+
		"\u021e\3\2\2\2\u021e\u021f\3\2\2\2\u021f\u0220\bT\2\2\u0220\u00a8\3\2"+
		"\2\2\u0221\u0222\t\3\2\2\u0222\u00aa\3\2\2\2\u0223\u0224\4\62;\2\u0224"+
		"\u00ac\3\2\2\2\u0225\u0226\t\4\2\2\u0226\u00ae\3\2\2\2\u0227\u0229\7g"+
		"\2\2\u0228\u022a\5\u00adW\2\u0229\u0228\3\2\2\2\u0229\u022a\3\2\2\2\u022a"+
		"\u022c\3\2\2\2\u022b\u022d\5\u00abV\2\u022c\u022b\3\2\2\2\u022d\u022e"+
		"\3\2\2\2\u022e\u022c\3\2\2\2\u022e\u022f\3\2\2\2\u022f\u023a\3\2\2\2\u0230"+
		"\u0232\7G\2\2\u0231\u0233\5\u00adW\2\u0232\u0231\3\2\2\2\u0232\u0233\3"+
		"\2\2\2\u0233\u0235\3\2\2\2\u0234\u0236\5\u00abV\2\u0235\u0234\3\2\2\2"+
		"\u0236\u0237\3\2\2\2\u0237\u0235\3\2\2\2\u0237\u0238\3\2\2\2\u0238\u023a"+
		"\3\2\2\2\u0239\u0227\3\2\2\2\u0239\u0230\3\2\2\2\u023a\u00b0\3\2\2\2\u023b"+
		"\u0240\5\u00a9U\2\u023c\u023f\5\u00a9U\2\u023d\u023f\5\u00abV\2\u023e"+
		"\u023c\3\2\2\2\u023e\u023d\3\2\2\2\u023f\u0242\3\2\2\2\u0240\u023e\3\2"+
		"\2\2\u0240\u0241\3\2\2\2\u0241\u0243\3\2\2\2\u0242\u0240\3\2\2\2\u0243"+
		"\u0244\bY\3\2\u0244\u00b2\3\2\2\2\u0245\u0247\5\u00abV\2\u0246\u0245\3"+
		"\2\2\2\u0247\u0248\3\2\2\2\u0248\u0246\3\2\2\2\u0248\u0249\3\2\2\2\u0249"+
		"\u00b4\3\2\2\2\u024a\u024c\5\u00abV\2\u024b\u024a\3\2\2\2\u024c\u024d"+
		"\3\2\2\2\u024d\u024b\3\2\2\2\u024d\u024e\3\2\2\2\u024e\u024f\3\2\2\2\u024f"+
		"\u0251\7\60\2\2\u0250\u0252\5\u00abV\2\u0251\u0250\3\2\2\2\u0252\u0253"+
		"\3\2\2\2\u0253\u0251\3\2\2\2\u0253\u0254\3\2\2\2\u0254\u0256\3\2\2\2\u0255"+
		"\u0257\5\u00afX\2\u0256\u0255\3\2\2\2\u0256\u0257\3\2\2\2\u0257\u0262"+
		"\3\2\2\2\u0258\u025a\7\60\2\2\u0259\u025b\5\u00abV\2\u025a\u0259\3\2\2"+
		"\2\u025b\u025c\3\2\2\2\u025c\u025a\3\2\2\2\u025c\u025d\3\2\2\2\u025d\u025f"+
		"\3\2\2\2\u025e\u0260\5\u00afX\2\u025f\u025e\3\2\2\2\u025f\u0260\3\2\2"+
		"\2\u0260\u0262\3\2\2\2\u0261\u024b\3\2\2\2\u0261\u0258\3\2\2\2\u0262\u00b6"+
		"\3\2\2\2\u0263\u0265\7$\2\2\u0264\u0266\5\u00b9]\2\u0265\u0264\3\2\2\2"+
		"\u0265\u0266\3\2\2\2\u0266\u0267\3\2\2\2\u0267\u0268\7$\2\2\u0268\u00b8"+
		"\3\2\2\2\u0269\u026b\5\u00bb^\2\u026a\u0269\3\2\2\2\u026b\u026c\3\2\2"+
		"\2\u026c\u026a\3\2\2\2\u026c\u026d\3\2\2\2\u026d\u00ba\3\2\2\2\u026e\u026f"+
		"\n\5\2\2\u026f\u00bc\3\2\2\2\24\2\u021d\u0229\u022e\u0232\u0237\u0239"+
		"\u023e\u0240\u0248\u024d\u0253\u0256\u025c\u025f\u0261\u0265\u026c\4\b"+
		"\2\2\3Y\2";
	public static final ATN _ATN =
		new ATNDeserializer().deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
		for (int i = 0; i < _ATN.getNumberOfDecisions(); i++) {
			_decisionToDFA[i] = new DFA(_ATN.getDecisionState(i), i);
		}
	}
}