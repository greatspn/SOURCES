// Generated from JavaGUI/Editor/src/editor/domain/grammar/ExprLang.g4 by ANTLR 4.2.1
package editor.domain.grammar;

import editor.domain.*;
import editor.domain.elements.*;

import org.antlr.v4.runtime.atn.*;
import org.antlr.v4.runtime.dfa.DFA;
import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.misc.*;
import org.antlr.v4.runtime.tree.*;
import java.util.List;
import java.util.Iterator;
import java.util.ArrayList;

@SuppressWarnings({"all", "warnings", "unchecked", "unused", "cast"})
public class ExprLangParser extends Parser {
	protected static final DFA[] _decisionToDFA;
	protected static final PredictionContextCache _sharedContextCache =
		new PredictionContextCache();
	public static final int
		T__14=1, T__13=2, T__12=3, T__11=4, T__10=5, T__9=6, T__8=7, T__7=8, T__6=9, 
		T__5=10, T__4=11, T__3=12, T__2=13, T__1=14, T__0=15, MUL=16, DIV=17, 
		ADD=18, SUB=19, AND=20, OR=21, POSTINCR=22, POSTDECR=23, HAT=24, NOT=25, 
		IMPLY=26, BIIMPLY=27, COMMA=28, COLON=29, SEMICOLON=30, OP_PAREN=31, CL_PAREN=32, 
		OP_SQPAR=33, CL_SQPAR=34, OP_BRACK=35, CL_BRACK=36, TWODOTS=37, EQUAL=38, 
		NOT_EQUAL=39, LESS=40, GREATER=41, LESS_EQ=42, GREATER_EQ=43, SHARP=44, 
		TRUE=45, FALSE=46, INFINITY=47, CLOCK=48, ACT=49, PROB_TA=50, CONTAINS=51, 
		DONT_CONTAINS=52, COLOR_ORDINAL=53, MULTISET_CARD=54, MULTISET_SUBCLASS=55, 
		MIN_FN=56, MAX_FN=57, MOD_FN=58, POW_FN=59, FRACT_FN=60, FACTORIAL_FN=61, 
		ABS_FN=62, SIN_FN=63, COS_FN=64, TAN_FN=65, ARCSIN_FN=66, ARCCOS_FN=67, 
		ARCTAN_FN=68, EXP_FN=69, LOG_FN=70, SQRT_FN=71, CEIL_FN=72, FLOOR_FN=73, 
		ROUND_FN=74, BINOMIAL_FN=75, IF_FN=76, WHITESPACES=77, ID=78, INT=79, 
		REAL=80, INT_TO_REAL=81, UNION=82, SETMINUS=83, SAFE_DIV=84, LESS_ZEROINF=85, 
		MULTISET_MIN=86, MULTISET_MAX=87, MULTISET_FILTER_NEGATIVES=88, INT_PLACE_ID=89, 
		REAL_PLACE_ID=90, INT_MSET_PLACE_ID=91, REAL_MSET_PLACE_ID=92, TRANSITION_ID=93, 
		INT_CONST_ID=94, REAL_CONST_ID=95, INT_MSET_CONST_ID=96, REAL_MSET_CONST_ID=97, 
		INT_TEMPLATE_ID=98, REAL_TEMPLATE_ID=99, ACTION_ID=100, STATEPROP_ID=101, 
		CLOCK_ID=102, SIMPLECOLORCLASS_ID=103, COLORDOMAIN_ID=104, COLORVAR_ID=105, 
		COLORSUBCLASS_ID=106, COLOR_ID=107, GREATSPN_ALL=108, GREATSPN_SUBCLASS=109, 
		EXISTS=110, FORALL=111, NEXT=112, FUTURE=113, GLOBALLY=114, UNTIL=115, 
		EXISTS_NEXT=116, EXISTS_FUTURE=117, EXISTS_GLOBALLY=118, FORALL_NEXT=119, 
		FORALL_FUTURE=120, FORALL_GLOBALLY=121, POSSIBLY=122, IMPOSSIBLY=123, 
		INVARIANTLY=124, ENABLED=125, BOUNDS=126, DEADLOCK=127, NO_DEADLOCK=128, 
		INITIAL_STATE=129, CTLSTAR_NEXT=130, CTLSTAR_FUTURE=131, CTLSTAR_GLOBALLY=132, 
		CTLSTAR_UNTIL=133, CTLSTAR_EXISTS=134, CTLSTAR_FORALL=135, CTLSTAR_EXISTS_NEXT=136, 
		CTLSTAR_EXISTS_FUTURE=137, CTLSTAR_EXISTS_GLOBALLY=138, CTLSTAR_FORALL_NEXT=139, 
		CTLSTAR_FORALL_FUTURE=140, CTLSTAR_FORALL_GLOBALLY=141, PDF_X_VAR=142, 
		RECT_FN=143, UNIFORM_FN=144, TRIANGULAR_FN=145, ERLANG_FN=146, TRUNCATED_EXP_FN=147, 
		PARETO_FN=148, DIRAC_DELTA_FN=149;
	public static final String[] tokenNames = {
		"<INVALID>", "'ordered'", "'circular'", "'unordered'", "'\\'", "'='", 
		"'enum'", "'when'", "'All'", "'is'", "'E{'", "'|'", "'ever'", "'X{'", 
		"'@'", "'P{'", "'*'", "'/'", "'+'", "'-'", "'&&'", "'||'", "'++'", "'--'", 
		"'^'", "'!'", "'->'", "'<->'", "','", "':'", "';'", "'('", "')'", "'['", 
		"']'", "'{'", "'}'", "'..'", "'=='", "'!='", "'<'", "'>'", "'<='", "'>='", 
		"'#'", "'True'", "'False'", "'Infinite'", "'clock'", "'Act'", "'PROB_TA'", 
		"'in'", "'!in'", "'CN'", "'Card'", "'Subclass'", "'Min'", "'Max'", "'Mod'", 
		"'Pow'", "'Fract'", "'Factorial'", "'Abs'", "'Sin'", "'Cos'", "'Tan'", 
		"'Asin'", "'Acos'", "'Atan'", "'Exp'", "'Log'", "'Sqrt'", "'Ceil'", "'Floor'", 
		"'Round'", "'Binomial'", "'If'", "WHITESPACES", "ID", "INT", "REAL", "INT_TO_REAL", 
		"UNION", "SETMINUS", "SAFE_DIV", "LESS_ZEROINF", "MULTISET_MIN", "MULTISET_MAX", 
		"MULTISET_FILTER_NEGATIVES", "INT_PLACE_ID", "REAL_PLACE_ID", "INT_MSET_PLACE_ID", 
		"REAL_MSET_PLACE_ID", "TRANSITION_ID", "INT_CONST_ID", "REAL_CONST_ID", 
		"INT_MSET_CONST_ID", "REAL_MSET_CONST_ID", "INT_TEMPLATE_ID", "REAL_TEMPLATE_ID", 
		"ACTION_ID", "STATEPROP_ID", "CLOCK_ID", "SIMPLECOLORCLASS_ID", "COLORDOMAIN_ID", 
		"COLORVAR_ID", "COLORSUBCLASS_ID", "COLOR_ID", "GREATSPN_ALL", "GREATSPN_SUBCLASS", 
		"EXISTS", "FORALL", "NEXT", "FUTURE", "GLOBALLY", "UNTIL", "EXISTS_NEXT", 
		"EXISTS_FUTURE", "EXISTS_GLOBALLY", "FORALL_NEXT", "FORALL_FUTURE", "FORALL_GLOBALLY", 
		"POSSIBLY", "IMPOSSIBLY", "INVARIANTLY", "ENABLED", "BOUNDS", "DEADLOCK", 
		"NO_DEADLOCK", "INITIAL_STATE", "CTLSTAR_NEXT", "CTLSTAR_FUTURE", "CTLSTAR_GLOBALLY", 
		"CTLSTAR_UNTIL", "CTLSTAR_EXISTS", "CTLSTAR_FORALL", "CTLSTAR_EXISTS_NEXT", 
		"CTLSTAR_EXISTS_FUTURE", "CTLSTAR_EXISTS_GLOBALLY", "CTLSTAR_FORALL_NEXT", 
		"CTLSTAR_FORALL_FUTURE", "CTLSTAR_FORALL_GLOBALLY", "PDF_X_VAR", "RECT_FN", 
		"UNIFORM_FN", "TRIANGULAR_FN", "ERLANG_FN", "TRUNCATED_EXP_FN", "PARETO_FN", 
		"DIRAC_DELTA_FN"
	};
	public static final int
		RULE_mainIntExpr = 0, RULE_mainRealExpr = 1, RULE_mainBoolExpr = 2, RULE_mainIntOrInfiniteExpr = 3, 
		RULE_mainIntExprList = 4, RULE_mainRealExprList = 5, RULE_mainClockConstr = 6, 
		RULE_mainActSet = 7, RULE_mainStatePropExpr = 8, RULE_mainActBinding = 9, 
		RULE_mainVarFlow = 10, RULE_mainClockDef = 11, RULE_mainMeasure = 12, 
		RULE_mainCsltaExpr = 13, RULE_mainColorClassDef = 14, RULE_mainColorVarDef = 15, 
		RULE_mainPlaceColorDomainDef = 16, RULE_mainRealMSetExpr = 17, RULE_mainIntMSetExpr = 18, 
		RULE_intConst = 19, RULE_realConst = 20, RULE_boolConst = 21, RULE_intExpr = 22, 
		RULE_realExpr = 23, RULE_boolExpr = 24, RULE_unaryIntFn = 25, RULE_unaryIntRealFn = 26, 
		RULE_binaryIntFn = 27, RULE_binaryRealFn = 28, RULE_unaryRealFn = 29, 
		RULE_mSetPredicate = 30, RULE_mSetElemPredicate = 31, RULE_realMSetExpr = 32, 
		RULE_intMSetExpr = 33, RULE_multiSetElem = 34, RULE_colorVar = 35, RULE_colorSubclass = 36, 
		RULE_colorTerm = 37, RULE_colorSet = 38, RULE_colorClassDef = 39, RULE_colorClassOrd = 40, 
		RULE_colorList = 41, RULE_colorSubclassName = 42, RULE_colorVarDef = 43, 
		RULE_placeColorDomainDef = 44, RULE_actList = 45, RULE_actSet = 46, RULE_actBinding = 47, 
		RULE_clockVar = 48, RULE_clockConstr = 49, RULE_varUpdate = 50, RULE_varUpdateList = 51, 
		RULE_varFlowList = 52, RULE_varFlow = 53, RULE_measure = 54, RULE_csltaExpr = 55, 
		RULE_real_assign = 56, RULE_act_assign = 57, RULE_sp_assign = 58, RULE_temporal_op = 59, 
		RULE_composite_temporal_op_ctlstar = 60, RULE_anyID = 61;
	public static final String[] ruleNames = {
		"mainIntExpr", "mainRealExpr", "mainBoolExpr", "mainIntOrInfiniteExpr", 
		"mainIntExprList", "mainRealExprList", "mainClockConstr", "mainActSet", 
		"mainStatePropExpr", "mainActBinding", "mainVarFlow", "mainClockDef", 
		"mainMeasure", "mainCsltaExpr", "mainColorClassDef", "mainColorVarDef", 
		"mainPlaceColorDomainDef", "mainRealMSetExpr", "mainIntMSetExpr", "intConst", 
		"realConst", "boolConst", "intExpr", "realExpr", "boolExpr", "unaryIntFn", 
		"unaryIntRealFn", "binaryIntFn", "binaryRealFn", "unaryRealFn", "mSetPredicate", 
		"mSetElemPredicate", "realMSetExpr", "intMSetExpr", "multiSetElem", "colorVar", 
		"colorSubclass", "colorTerm", "colorSet", "colorClassDef", "colorClassOrd", 
		"colorList", "colorSubclassName", "colorVarDef", "placeColorDomainDef", 
		"actList", "actSet", "actBinding", "clockVar", "clockConstr", "varUpdate", 
		"varUpdateList", "varFlowList", "varFlow", "measure", "csltaExpr", "real_assign", 
		"act_assign", "sp_assign", "temporal_op", "composite_temporal_op_ctlstar", 
		"anyID"
	};

	@Override
	public String getGrammarFileName() { return "ExprLang.g4"; }

	@Override
	public String[] getTokenNames() { return tokenNames; }

	@Override
	public String[] getRuleNames() { return ruleNames; }

	@Override
	public String getSerializedATN() { return _serializedATN; }

	@Override
	public ATN getATN() { return _ATN; }


	public ParserContext context = null;
	public int parseFlags;
	public boolean clockVarInExpr = true;

	public ExprLangParser(TokenStream input) {
		super(input);
		_interp = new ParserATNSimulator(this,_ATN,_decisionToDFA,_sharedContextCache);
	}
	public static class MainIntExprContext extends ParserRuleContext {
		public TerminalNode EOF() { return getToken(ExprLangParser.EOF, 0); }
		public IntExprContext intExpr() {
			return getRuleContext(IntExprContext.class,0);
		}
		public MainIntExprContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_mainIntExpr; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMainIntExpr(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MainIntExprContext mainIntExpr() throws RecognitionException {
		MainIntExprContext _localctx = new MainIntExprContext(_ctx, getState());
		enterRule(_localctx, 0, RULE_mainIntExpr);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(124); intExpr(0);
			setState(125); match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class MainRealExprContext extends ParserRuleContext {
		public RealExprContext realExpr() {
			return getRuleContext(RealExprContext.class,0);
		}
		public TerminalNode EOF() { return getToken(ExprLangParser.EOF, 0); }
		public MainRealExprContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_mainRealExpr; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMainRealExpr(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MainRealExprContext mainRealExpr() throws RecognitionException {
		MainRealExprContext _localctx = new MainRealExprContext(_ctx, getState());
		enterRule(_localctx, 2, RULE_mainRealExpr);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(127); realExpr(0);
			setState(128); match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class MainBoolExprContext extends ParserRuleContext {
		public BoolExprContext boolExpr() {
			return getRuleContext(BoolExprContext.class,0);
		}
		public TerminalNode EOF() { return getToken(ExprLangParser.EOF, 0); }
		public MainBoolExprContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_mainBoolExpr; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMainBoolExpr(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MainBoolExprContext mainBoolExpr() throws RecognitionException {
		MainBoolExprContext _localctx = new MainBoolExprContext(_ctx, getState());
		enterRule(_localctx, 4, RULE_mainBoolExpr);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(130); boolExpr(0);
			setState(131); match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class MainIntOrInfiniteExprContext extends ParserRuleContext {
		public TerminalNode EOF() { return getToken(ExprLangParser.EOF, 0); }
		public IntExprContext intExpr() {
			return getRuleContext(IntExprContext.class,0);
		}
		public TerminalNode INFINITY() { return getToken(ExprLangParser.INFINITY, 0); }
		public MainIntOrInfiniteExprContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_mainIntOrInfiniteExpr; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMainIntOrInfiniteExpr(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MainIntOrInfiniteExprContext mainIntOrInfiniteExpr() throws RecognitionException {
		MainIntOrInfiniteExprContext _localctx = new MainIntOrInfiniteExprContext(_ctx, getState());
		enterRule(_localctx, 6, RULE_mainIntOrInfiniteExpr);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(135);
			switch (_input.LA(1)) {
			case 7:
			case SUB:
			case OP_PAREN:
			case SHARP:
			case COLOR_ORDINAL:
			case MULTISET_CARD:
			case MIN_FN:
			case MAX_FN:
			case MOD_FN:
			case POW_FN:
			case FRACT_FN:
			case FACTORIAL_FN:
			case ABS_FN:
			case CEIL_FN:
			case FLOOR_FN:
			case ROUND_FN:
			case BINOMIAL_FN:
			case IF_FN:
			case ID:
			case INT:
			case INT_CONST_ID:
			case INT_TEMPLATE_ID:
			case BOUNDS:
				{
				setState(133); intExpr(0);
				}
				break;
			case INFINITY:
				{
				setState(134); match(INFINITY);
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
			setState(137); match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class MainIntExprListContext extends ParserRuleContext {
		public TerminalNode EOF() { return getToken(ExprLangParser.EOF, 0); }
		public List<IntExprContext> intExpr() {
			return getRuleContexts(IntExprContext.class);
		}
		public IntExprContext intExpr(int i) {
			return getRuleContext(IntExprContext.class,i);
		}
		public MainIntExprListContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_mainIntExprList; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMainIntExprList(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MainIntExprListContext mainIntExprList() throws RecognitionException {
		MainIntExprListContext _localctx = new MainIntExprListContext(_ctx, getState());
		enterRule(_localctx, 8, RULE_mainIntExprList);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(139); intExpr(0);
			setState(144);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==COMMA) {
				{
				{
				setState(140); match(COMMA);
				setState(141); intExpr(0);
				}
				}
				setState(146);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			setState(147); match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class MainRealExprListContext extends ParserRuleContext {
		public List<RealExprContext> realExpr() {
			return getRuleContexts(RealExprContext.class);
		}
		public TerminalNode EOF() { return getToken(ExprLangParser.EOF, 0); }
		public RealExprContext realExpr(int i) {
			return getRuleContext(RealExprContext.class,i);
		}
		public MainRealExprListContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_mainRealExprList; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMainRealExprList(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MainRealExprListContext mainRealExprList() throws RecognitionException {
		MainRealExprListContext _localctx = new MainRealExprListContext(_ctx, getState());
		enterRule(_localctx, 10, RULE_mainRealExprList);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(149); realExpr(0);
			setState(154);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==COMMA) {
				{
				{
				setState(150); match(COMMA);
				setState(151); realExpr(0);
				}
				}
				setState(156);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			setState(157); match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class MainClockConstrContext extends ParserRuleContext {
		public ClockConstrContext clockConstr() {
			return getRuleContext(ClockConstrContext.class,0);
		}
		public TerminalNode EOF() { return getToken(ExprLangParser.EOF, 0); }
		public VarUpdateListContext varUpdateList() {
			return getRuleContext(VarUpdateListContext.class,0);
		}
		public MainClockConstrContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_mainClockConstr; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMainClockConstr(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MainClockConstrContext mainClockConstr() throws RecognitionException {
		MainClockConstrContext _localctx = new MainClockConstrContext(_ctx, getState());
		enterRule(_localctx, 12, RULE_mainClockConstr);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(159); clockConstr(0);
			setState(162);
			_la = _input.LA(1);
			if (_la==SEMICOLON) {
				{
				setState(160); match(SEMICOLON);
				setState(161); varUpdateList();
				}
			}

			setState(164); match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class MainActSetContext extends ParserRuleContext {
		public TerminalNode EOF() { return getToken(ExprLangParser.EOF, 0); }
		public ActSetContext actSet() {
			return getRuleContext(ActSetContext.class,0);
		}
		public MainActSetContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_mainActSet; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMainActSet(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MainActSetContext mainActSet() throws RecognitionException {
		MainActSetContext _localctx = new MainActSetContext(_ctx, getState());
		enterRule(_localctx, 14, RULE_mainActSet);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(166); actSet();
			setState(167); match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class MainStatePropExprContext extends ParserRuleContext {
		public BoolExprContext boolExpr() {
			return getRuleContext(BoolExprContext.class,0);
		}
		public TerminalNode EOF() { return getToken(ExprLangParser.EOF, 0); }
		public MainStatePropExprContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_mainStatePropExpr; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMainStatePropExpr(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MainStatePropExprContext mainStatePropExpr() throws RecognitionException {
		MainStatePropExprContext _localctx = new MainStatePropExprContext(_ctx, getState());
		enterRule(_localctx, 16, RULE_mainStatePropExpr);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(169); boolExpr(0);
			setState(170); match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class MainActBindingContext extends ParserRuleContext {
		public TerminalNode EOF() { return getToken(ExprLangParser.EOF, 0); }
		public ActBindingContext actBinding() {
			return getRuleContext(ActBindingContext.class,0);
		}
		public MainActBindingContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_mainActBinding; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMainActBinding(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MainActBindingContext mainActBinding() throws RecognitionException {
		MainActBindingContext _localctx = new MainActBindingContext(_ctx, getState());
		enterRule(_localctx, 18, RULE_mainActBinding);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(172); actBinding();
			setState(173); match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class MainVarFlowContext extends ParserRuleContext {
		public TerminalNode EOF() { return getToken(ExprLangParser.EOF, 0); }
		public VarFlowListContext varFlowList() {
			return getRuleContext(VarFlowListContext.class,0);
		}
		public MainVarFlowContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_mainVarFlow; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMainVarFlow(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MainVarFlowContext mainVarFlow() throws RecognitionException {
		MainVarFlowContext _localctx = new MainVarFlowContext(_ctx, getState());
		enterRule(_localctx, 20, RULE_mainVarFlow);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(175); varFlowList();
			setState(176); match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class MainClockDefContext extends ParserRuleContext {
		public TerminalNode EOF() { return getToken(ExprLangParser.EOF, 0); }
		public TerminalNode CLOCK() { return getToken(ExprLangParser.CLOCK, 0); }
		public MainClockDefContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_mainClockDef; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMainClockDef(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MainClockDefContext mainClockDef() throws RecognitionException {
		MainClockDefContext _localctx = new MainClockDefContext(_ctx, getState());
		enterRule(_localctx, 22, RULE_mainClockDef);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(178); match(CLOCK);
			setState(179); match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class MainMeasureContext extends ParserRuleContext {
		public TerminalNode EOF() { return getToken(ExprLangParser.EOF, 0); }
		public MeasureContext measure() {
			return getRuleContext(MeasureContext.class,0);
		}
		public MainMeasureContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_mainMeasure; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMainMeasure(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MainMeasureContext mainMeasure() throws RecognitionException {
		MainMeasureContext _localctx = new MainMeasureContext(_ctx, getState());
		enterRule(_localctx, 24, RULE_mainMeasure);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(181); measure(0);
			setState(182); match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class MainCsltaExprContext extends ParserRuleContext {
		public TerminalNode EOF() { return getToken(ExprLangParser.EOF, 0); }
		public CsltaExprContext csltaExpr() {
			return getRuleContext(CsltaExprContext.class,0);
		}
		public MainCsltaExprContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_mainCsltaExpr; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMainCsltaExpr(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MainCsltaExprContext mainCsltaExpr() throws RecognitionException {
		MainCsltaExprContext _localctx = new MainCsltaExprContext(_ctx, getState());
		enterRule(_localctx, 26, RULE_mainCsltaExpr);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(184); csltaExpr(0);
			setState(185); match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class MainColorClassDefContext extends ParserRuleContext {
		public TerminalNode EOF() { return getToken(ExprLangParser.EOF, 0); }
		public ColorClassDefContext colorClassDef() {
			return getRuleContext(ColorClassDefContext.class,0);
		}
		public MainColorClassDefContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_mainColorClassDef; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMainColorClassDef(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MainColorClassDefContext mainColorClassDef() throws RecognitionException {
		MainColorClassDefContext _localctx = new MainColorClassDefContext(_ctx, getState());
		enterRule(_localctx, 28, RULE_mainColorClassDef);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(187); colorClassDef();
			setState(188); match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class MainColorVarDefContext extends ParserRuleContext {
		public TerminalNode EOF() { return getToken(ExprLangParser.EOF, 0); }
		public ColorVarDefContext colorVarDef() {
			return getRuleContext(ColorVarDefContext.class,0);
		}
		public MainColorVarDefContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_mainColorVarDef; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMainColorVarDef(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MainColorVarDefContext mainColorVarDef() throws RecognitionException {
		MainColorVarDefContext _localctx = new MainColorVarDefContext(_ctx, getState());
		enterRule(_localctx, 30, RULE_mainColorVarDef);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(190); colorVarDef();
			setState(191); match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class MainPlaceColorDomainDefContext extends ParserRuleContext {
		public TerminalNode EOF() { return getToken(ExprLangParser.EOF, 0); }
		public PlaceColorDomainDefContext placeColorDomainDef() {
			return getRuleContext(PlaceColorDomainDefContext.class,0);
		}
		public MainPlaceColorDomainDefContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_mainPlaceColorDomainDef; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMainPlaceColorDomainDef(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MainPlaceColorDomainDefContext mainPlaceColorDomainDef() throws RecognitionException {
		MainPlaceColorDomainDefContext _localctx = new MainPlaceColorDomainDefContext(_ctx, getState());
		enterRule(_localctx, 32, RULE_mainPlaceColorDomainDef);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(193); placeColorDomainDef();
			setState(194); match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class MainRealMSetExprContext extends ParserRuleContext {
		public RealMSetExprContext realMSetExpr() {
			return getRuleContext(RealMSetExprContext.class,0);
		}
		public TerminalNode EOF() { return getToken(ExprLangParser.EOF, 0); }
		public MainRealMSetExprContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_mainRealMSetExpr; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMainRealMSetExpr(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MainRealMSetExprContext mainRealMSetExpr() throws RecognitionException {
		MainRealMSetExprContext _localctx = new MainRealMSetExprContext(_ctx, getState());
		enterRule(_localctx, 34, RULE_mainRealMSetExpr);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(196); realMSetExpr(0);
			setState(197); match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class MainIntMSetExprContext extends ParserRuleContext {
		public IntMSetExprContext intMSetExpr() {
			return getRuleContext(IntMSetExprContext.class,0);
		}
		public TerminalNode EOF() { return getToken(ExprLangParser.EOF, 0); }
		public MainIntMSetExprContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_mainIntMSetExpr; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMainIntMSetExpr(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MainIntMSetExprContext mainIntMSetExpr() throws RecognitionException {
		MainIntMSetExprContext _localctx = new MainIntMSetExprContext(_ctx, getState());
		enterRule(_localctx, 36, RULE_mainIntMSetExpr);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(199); intMSetExpr(0);
			setState(200); match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class IntConstContext extends ParserRuleContext {
		public IntConstContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_intConst; }
	 
		public IntConstContext() { }
		public void copyFrom(IntConstContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class IntConstLiteralContext extends IntConstContext {
		public TerminalNode INT() { return getToken(ExprLangParser.INT, 0); }
		public IntConstLiteralContext(IntConstContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntConstLiteral(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class IntTemplateIdContext extends IntConstContext {
		public TerminalNode INT_TEMPLATE_ID() { return getToken(ExprLangParser.INT_TEMPLATE_ID, 0); }
		public IntTemplateIdContext(IntConstContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntTemplateId(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class IntConstIdContext extends IntConstContext {
		public TerminalNode INT_CONST_ID() { return getToken(ExprLangParser.INT_CONST_ID, 0); }
		public IntConstIdContext(IntConstContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntConstId(this);
			else return visitor.visitChildren(this);
		}
	}

	public final IntConstContext intConst() throws RecognitionException {
		IntConstContext _localctx = new IntConstContext(_ctx, getState());
		enterRule(_localctx, 38, RULE_intConst);
		try {
			setState(205);
			switch (_input.LA(1)) {
			case INT:
				_localctx = new IntConstLiteralContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(202); match(INT);
				}
				break;
			case INT_CONST_ID:
				_localctx = new IntConstIdContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(203); match(INT_CONST_ID);
				}
				break;
			case INT_TEMPLATE_ID:
				_localctx = new IntTemplateIdContext(_localctx);
				enterOuterAlt(_localctx, 3);
				{
				setState(204); match(INT_TEMPLATE_ID);
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class RealConstContext extends ParserRuleContext {
		public RealConstContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_realConst; }
	 
		public RealConstContext() { }
		public void copyFrom(RealConstContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class RealTemplateIdContext extends RealConstContext {
		public TerminalNode REAL_TEMPLATE_ID() { return getToken(ExprLangParser.REAL_TEMPLATE_ID, 0); }
		public RealTemplateIdContext(RealConstContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealTemplateId(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class RealFromIntConstContext extends RealConstContext {
		public IntConstContext intConst() {
			return getRuleContext(IntConstContext.class,0);
		}
		public RealFromIntConstContext(RealConstContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealFromIntConst(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class RealConstLiteralContext extends RealConstContext {
		public TerminalNode REAL() { return getToken(ExprLangParser.REAL, 0); }
		public RealConstLiteralContext(RealConstContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealConstLiteral(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class RealConstIdContext extends RealConstContext {
		public TerminalNode REAL_CONST_ID() { return getToken(ExprLangParser.REAL_CONST_ID, 0); }
		public RealConstIdContext(RealConstContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealConstId(this);
			else return visitor.visitChildren(this);
		}
	}

	public final RealConstContext realConst() throws RecognitionException {
		RealConstContext _localctx = new RealConstContext(_ctx, getState());
		enterRule(_localctx, 40, RULE_realConst);
		try {
			setState(211);
			switch (_input.LA(1)) {
			case INT:
			case INT_CONST_ID:
			case INT_TEMPLATE_ID:
				_localctx = new RealFromIntConstContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(207); intConst();
				}
				break;
			case REAL:
				_localctx = new RealConstLiteralContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(208); match(REAL);
				}
				break;
			case REAL_CONST_ID:
				_localctx = new RealConstIdContext(_localctx);
				enterOuterAlt(_localctx, 3);
				{
				setState(209); match(REAL_CONST_ID);
				}
				break;
			case REAL_TEMPLATE_ID:
				_localctx = new RealTemplateIdContext(_localctx);
				enterOuterAlt(_localctx, 4);
				{
				setState(210); match(REAL_TEMPLATE_ID);
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class BoolConstContext extends ParserRuleContext {
		public TerminalNode FALSE() { return getToken(ExprLangParser.FALSE, 0); }
		public TerminalNode TRUE() { return getToken(ExprLangParser.TRUE, 0); }
		public BoolConstContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_boolConst; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolConst(this);
			else return visitor.visitChildren(this);
		}
	}

	public final BoolConstContext boolConst() throws RecognitionException {
		BoolConstContext _localctx = new BoolConstContext(_ctx, getState());
		enterRule(_localctx, 42, RULE_boolConst);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(213);
			_la = _input.LA(1);
			if ( !(_la==TRUE || _la==FALSE) ) {
			_errHandler.recoverInline(this);
			}
			consume();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class IntExprContext extends ParserRuleContext {
		public IntExprContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_intExpr; }
	 
		public IntExprContext() { }
		public void copyFrom(IntExprContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class IntExprConstContext extends IntExprContext {
		public IntConstContext intConst() {
			return getRuleContext(IntConstContext.class,0);
		}
		public IntExprConstContext(IntExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntExprConst(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class IntExprBinaryFnContext extends IntExprContext {
		public List<IntExprContext> intExpr() {
			return getRuleContexts(IntExprContext.class);
		}
		public BinaryIntFnContext binaryIntFn() {
			return getRuleContext(BinaryIntFnContext.class,0);
		}
		public IntExprContext intExpr(int i) {
			return getRuleContext(IntExprContext.class,i);
		}
		public IntExprBinaryFnContext(IntExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntExprBinaryFn(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class IntExprCTLBoundContext extends IntExprContext {
		public List<TerminalNode> INT_PLACE_ID() { return getTokens(ExprLangParser.INT_PLACE_ID); }
		public TerminalNode INT_PLACE_ID(int i) {
			return getToken(ExprLangParser.INT_PLACE_ID, i);
		}
		public TerminalNode BOUNDS() { return getToken(ExprLangParser.BOUNDS, 0); }
		public IntExprCTLBoundContext(IntExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntExprCTLBound(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class IntExprUnknownPlaceIdContext extends IntExprContext {
		public TerminalNode ID() { return getToken(ExprLangParser.ID, 0); }
		public IntExprUnknownPlaceIdContext(IntExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntExprUnknownPlaceId(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class IntExprColorNumContext extends IntExprContext {
		public ColorTermContext colorTerm() {
			return getRuleContext(ColorTermContext.class,0);
		}
		public IntExprColorNumContext(IntExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntExprColorNum(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class IntExprParenContext extends IntExprContext {
		public IntExprContext intExpr() {
			return getRuleContext(IntExprContext.class,0);
		}
		public IntExprParenContext(IntExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntExprParen(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class IntExprCondContext extends IntExprContext {
		public BoolExprContext boolExpr() {
			return getRuleContext(BoolExprContext.class,0);
		}
		public List<IntExprContext> intExpr() {
			return getRuleContexts(IntExprContext.class);
		}
		public IntExprContext intExpr(int i) {
			return getRuleContext(IntExprContext.class,i);
		}
		public IntExprCondContext(IntExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntExprCond(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class IntExprCardMSetContext extends IntExprContext {
		public IntMSetExprContext intMSetExpr() {
			return getRuleContext(IntMSetExprContext.class,0);
		}
		public IntExprCardMSetContext(IntExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntExprCardMSet(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class IntExprNegateContext extends IntExprContext {
		public IntExprContext intExpr() {
			return getRuleContext(IntExprContext.class,0);
		}
		public IntExprNegateContext(IntExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntExprNegate(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class IntExprAddSubContext extends IntExprContext {
		public Token op;
		public List<IntExprContext> intExpr() {
			return getRuleContexts(IntExprContext.class);
		}
		public IntExprContext intExpr(int i) {
			return getRuleContext(IntExprContext.class,i);
		}
		public IntExprAddSubContext(IntExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntExprAddSub(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class IntExprCond2Context extends IntExprContext {
		public BoolExprContext boolExpr() {
			return getRuleContext(BoolExprContext.class,0);
		}
		public List<IntExprContext> intExpr() {
			return getRuleContexts(IntExprContext.class);
		}
		public IntExprContext intExpr(int i) {
			return getRuleContext(IntExprContext.class,i);
		}
		public IntExprCond2Context(IntExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntExprCond2(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class IntExprMulDivContext extends IntExprContext {
		public Token op;
		public List<IntExprContext> intExpr() {
			return getRuleContexts(IntExprContext.class);
		}
		public IntExprContext intExpr(int i) {
			return getRuleContext(IntExprContext.class,i);
		}
		public IntExprMulDivContext(IntExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntExprMulDiv(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class IntExprUnaryFnContext extends IntExprContext {
		public UnaryIntFnContext unaryIntFn() {
			return getRuleContext(UnaryIntFnContext.class,0);
		}
		public IntExprContext intExpr() {
			return getRuleContext(IntExprContext.class,0);
		}
		public IntExprUnaryFnContext(IntExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntExprUnaryFn(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class IntExprUnaryRealFnContext extends IntExprContext {
		public RealExprContext realExpr() {
			return getRuleContext(RealExprContext.class,0);
		}
		public UnaryIntRealFnContext unaryIntRealFn() {
			return getRuleContext(UnaryIntRealFnContext.class,0);
		}
		public IntExprUnaryRealFnContext(IntExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntExprUnaryRealFn(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class IntExprUnknownIdContext extends IntExprContext {
		public TerminalNode ID() { return getToken(ExprLangParser.ID, 0); }
		public IntExprUnknownIdContext(IntExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntExprUnknownId(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class IntExprPlaceMarkingContext extends IntExprContext {
		public TerminalNode INT_PLACE_ID() { return getToken(ExprLangParser.INT_PLACE_ID, 0); }
		public IntExprPlaceMarkingContext(IntExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntExprPlaceMarking(this);
			else return visitor.visitChildren(this);
		}
	}

	public final IntExprContext intExpr() throws RecognitionException {
		return intExpr(0);
	}

	private IntExprContext intExpr(int _p) throws RecognitionException {
		ParserRuleContext _parentctx = _ctx;
		int _parentState = getState();
		IntExprContext _localctx = new IntExprContext(_ctx, _parentState);
		IntExprContext _prevctx = _localctx;
		int _startState = 44;
		enterRecursionRule(_localctx, 44, RULE_intExpr, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(284);
			switch ( getInterpreter().adaptivePredict(_input,7,_ctx) ) {
			case 1:
				{
				_localctx = new IntExprNegateContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(216); match(SUB);
				setState(217); intExpr(14);
				}
				break;

			case 2:
				{
				_localctx = new IntExprConstContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(218); intConst();
				}
				break;

			case 3:
				{
				_localctx = new IntExprPlaceMarkingContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(219); match(SHARP);
				setState(220); match(INT_PLACE_ID);
				}
				break;

			case 4:
				{
				_localctx = new IntExprParenContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(221); match(OP_PAREN);
				setState(222); intExpr(0);
				setState(223); match(CL_PAREN);
				}
				break;

			case 5:
				{
				_localctx = new IntExprUnaryFnContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(225); unaryIntFn();
				setState(226); match(OP_SQPAR);
				setState(227); intExpr(0);
				setState(228); match(CL_SQPAR);
				}
				break;

			case 6:
				{
				_localctx = new IntExprUnaryRealFnContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(230); unaryIntRealFn();
				setState(231); match(OP_SQPAR);
				setState(232); realExpr(0);
				setState(233); match(CL_SQPAR);
				}
				break;

			case 7:
				{
				_localctx = new IntExprBinaryFnContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(235); binaryIntFn();
				setState(236); match(OP_SQPAR);
				setState(237); intExpr(0);
				setState(238); match(COMMA);
				setState(239); intExpr(0);
				setState(240); match(CL_SQPAR);
				}
				break;

			case 8:
				{
				_localctx = new IntExprCondContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(242); match(IF_FN);
				setState(243); match(OP_SQPAR);
				setState(244); boolExpr(0);
				setState(245); match(COMMA);
				setState(246); intExpr(0);
				setState(247); match(COMMA);
				setState(248); intExpr(0);
				setState(249); match(CL_SQPAR);
				}
				break;

			case 9:
				{
				_localctx = new IntExprCond2Context(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(251); match(7);
				setState(252); boolExpr(0);
				setState(253); match(COLON);
				setState(254); intExpr(0);
				setState(255); match(SEMICOLON);
				setState(256); match(12);
				setState(257); intExpr(0);
				setState(258); match(SEMICOLON);
				}
				break;

			case 10:
				{
				_localctx = new IntExprCardMSetContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(260); match(MULTISET_CARD);
				setState(261); match(OP_SQPAR);
				setState(262); intMSetExpr(0);
				setState(263); match(CL_SQPAR);
				}
				break;

			case 11:
				{
				_localctx = new IntExprColorNumContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(265); match(COLOR_ORDINAL);
				setState(266); match(OP_SQPAR);
				setState(267); colorTerm(0);
				setState(268); match(CL_SQPAR);
				}
				break;

			case 12:
				{
				_localctx = new IntExprCTLBoundContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(270); match(BOUNDS);
				setState(271); match(OP_PAREN);
				setState(272); match(INT_PLACE_ID);
				setState(277);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==COMMA) {
					{
					{
					setState(273); match(COMMA);
					setState(274); match(INT_PLACE_ID);
					}
					}
					setState(279);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				setState(280); match(CL_PAREN);
				}
				break;

			case 13:
				{
				_localctx = new IntExprUnknownIdContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(281); match(ID);
				}
				break;

			case 14:
				{
				_localctx = new IntExprUnknownPlaceIdContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(282); match(SHARP);
				setState(283); match(ID);
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(294);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,9,_ctx);
			while ( _alt!=2 && _alt!=-1 ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					setState(292);
					switch ( getInterpreter().adaptivePredict(_input,8,_ctx) ) {
					case 1:
						{
						_localctx = new IntExprMulDivContext(new IntExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_intExpr);
						setState(286);
						if (!(precpred(_ctx, 16))) throw new FailedPredicateException(this, "precpred(_ctx, 16)");
						setState(287);
						((IntExprMulDivContext)_localctx).op = _input.LT(1);
						_la = _input.LA(1);
						if ( !(_la==MUL || _la==DIV) ) {
							((IntExprMulDivContext)_localctx).op = (Token)_errHandler.recoverInline(this);
						}
						consume();
						setState(288); intExpr(17);
						}
						break;

					case 2:
						{
						_localctx = new IntExprAddSubContext(new IntExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_intExpr);
						setState(289);
						if (!(precpred(_ctx, 15))) throw new FailedPredicateException(this, "precpred(_ctx, 15)");
						setState(290);
						((IntExprAddSubContext)_localctx).op = _input.LT(1);
						_la = _input.LA(1);
						if ( !(_la==ADD || _la==SUB) ) {
							((IntExprAddSubContext)_localctx).op = (Token)_errHandler.recoverInline(this);
						}
						consume();
						setState(291); intExpr(16);
						}
						break;
					}
					} 
				}
				setState(296);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,9,_ctx);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			unrollRecursionContexts(_parentctx);
		}
		return _localctx;
	}

	public static class RealExprContext extends ParserRuleContext {
		public RealExprContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_realExpr; }
	 
		public RealExprContext() { }
		public void copyFrom(RealExprContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class RealExprCondContext extends RealExprContext {
		public List<RealExprContext> realExpr() {
			return getRuleContexts(RealExprContext.class);
		}
		public BoolExprContext boolExpr() {
			return getRuleContext(BoolExprContext.class,0);
		}
		public RealExprContext realExpr(int i) {
			return getRuleContext(RealExprContext.class,i);
		}
		public RealExprCondContext(RealExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealExprCond(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class RealExprClockVarContext extends RealExprContext {
		public ClockVarContext clockVar() {
			return getRuleContext(ClockVarContext.class,0);
		}
		public RealExprClockVarContext(RealExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealExprClockVar(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class RealExprIntContext extends RealExprContext {
		public IntExprContext intExpr() {
			return getRuleContext(IntExprContext.class,0);
		}
		public RealExprIntContext(RealExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealExprInt(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class RealExprConstContext extends RealExprContext {
		public RealConstContext realConst() {
			return getRuleContext(RealConstContext.class,0);
		}
		public RealExprConstContext(RealExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealExprConst(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class RealExprCond2Context extends RealExprContext {
		public List<RealExprContext> realExpr() {
			return getRuleContexts(RealExprContext.class);
		}
		public BoolExprContext boolExpr() {
			return getRuleContext(BoolExprContext.class,0);
		}
		public RealExprContext realExpr(int i) {
			return getRuleContext(RealExprContext.class,i);
		}
		public RealExprCond2Context(RealExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealExprCond2(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class RealExprPlaceMarkingContext extends RealExprContext {
		public TerminalNode REAL_PLACE_ID() { return getToken(ExprLangParser.REAL_PLACE_ID, 0); }
		public RealExprPlaceMarkingContext(RealExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealExprPlaceMarking(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class RealExprPdfXVarContext extends RealExprContext {
		public TerminalNode PDF_X_VAR() { return getToken(ExprLangParser.PDF_X_VAR, 0); }
		public RealExprPdfXVarContext(RealExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealExprPdfXVar(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class RealExprUnknownIdContext extends RealExprContext {
		public TerminalNode ID() { return getToken(ExprLangParser.ID, 0); }
		public RealExprUnknownIdContext(RealExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealExprUnknownId(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class RealExprMulDivContext extends RealExprContext {
		public Token op;
		public List<RealExprContext> realExpr() {
			return getRuleContexts(RealExprContext.class);
		}
		public RealExprContext realExpr(int i) {
			return getRuleContext(RealExprContext.class,i);
		}
		public RealExprMulDivContext(RealExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealExprMulDiv(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class RealExprAddSubContext extends RealExprContext {
		public Token op;
		public List<RealExprContext> realExpr() {
			return getRuleContexts(RealExprContext.class);
		}
		public RealExprContext realExpr(int i) {
			return getRuleContext(RealExprContext.class,i);
		}
		public RealExprAddSubContext(RealExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealExprAddSub(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class RealExprBinaryFnContext extends RealExprContext {
		public List<RealExprContext> realExpr() {
			return getRuleContexts(RealExprContext.class);
		}
		public BinaryRealFnContext binaryRealFn() {
			return getRuleContext(BinaryRealFnContext.class,0);
		}
		public RealExprContext realExpr(int i) {
			return getRuleContext(RealExprContext.class,i);
		}
		public BinaryIntFnContext binaryIntFn() {
			return getRuleContext(BinaryIntFnContext.class,0);
		}
		public RealExprBinaryFnContext(RealExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealExprBinaryFn(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class RealExprUnaryFnContext extends RealExprContext {
		public RealExprContext realExpr() {
			return getRuleContext(RealExprContext.class,0);
		}
		public UnaryRealFnContext unaryRealFn() {
			return getRuleContext(UnaryRealFnContext.class,0);
		}
		public UnaryIntFnContext unaryIntFn() {
			return getRuleContext(UnaryIntFnContext.class,0);
		}
		public RealExprUnaryFnContext(RealExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealExprUnaryFn(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class RealExprNegateContext extends RealExprContext {
		public RealExprContext realExpr() {
			return getRuleContext(RealExprContext.class,0);
		}
		public RealExprNegateContext(RealExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealExprNegate(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class RealExprParenContext extends RealExprContext {
		public RealExprContext realExpr() {
			return getRuleContext(RealExprContext.class,0);
		}
		public RealExprParenContext(RealExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealExprParen(this);
			else return visitor.visitChildren(this);
		}
	}

	public final RealExprContext realExpr() throws RecognitionException {
		return realExpr(0);
	}

	private RealExprContext realExpr(int _p) throws RecognitionException {
		ParserRuleContext _parentctx = _ctx;
		int _parentState = getState();
		RealExprContext _localctx = new RealExprContext(_ctx, _parentState);
		RealExprContext _prevctx = _localctx;
		int _startState = 46;
		enterRecursionRule(_localctx, 46, RULE_realExpr, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(348);
			switch ( getInterpreter().adaptivePredict(_input,12,_ctx) ) {
			case 1:
				{
				_localctx = new RealExprNegateContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(298); match(SUB);
				setState(299); realExpr(12);
				}
				break;

			case 2:
				{
				_localctx = new RealExprConstContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(300); realConst();
				}
				break;

			case 3:
				{
				_localctx = new RealExprPlaceMarkingContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(301); match(SHARP);
				setState(302); match(REAL_PLACE_ID);
				}
				break;

			case 4:
				{
				_localctx = new RealExprIntContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(303); intExpr(0);
				}
				break;

			case 5:
				{
				_localctx = new RealExprParenContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(304); match(OP_PAREN);
				setState(305); realExpr(0);
				setState(306); match(CL_PAREN);
				}
				break;

			case 6:
				{
				_localctx = new RealExprUnaryFnContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(310);
				switch (_input.LA(1)) {
				case SIN_FN:
				case COS_FN:
				case TAN_FN:
				case ARCSIN_FN:
				case ARCCOS_FN:
				case ARCTAN_FN:
				case EXP_FN:
				case SQRT_FN:
				case DIRAC_DELTA_FN:
					{
					setState(308); unaryRealFn();
					}
					break;
				case FACTORIAL_FN:
				case ABS_FN:
					{
					setState(309); unaryIntFn();
					}
					break;
				default:
					throw new NoViableAltException(this);
				}
				setState(312); match(OP_SQPAR);
				setState(313); realExpr(0);
				setState(314); match(CL_SQPAR);
				}
				break;

			case 7:
				{
				_localctx = new RealExprBinaryFnContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(318);
				switch (_input.LA(1)) {
				case MIN_FN:
				case MAX_FN:
				case MOD_FN:
				case POW_FN:
				case FRACT_FN:
				case BINOMIAL_FN:
					{
					setState(316); binaryIntFn();
					}
					break;
				case RECT_FN:
				case UNIFORM_FN:
				case TRIANGULAR_FN:
				case ERLANG_FN:
				case TRUNCATED_EXP_FN:
				case PARETO_FN:
					{
					setState(317); binaryRealFn();
					}
					break;
				default:
					throw new NoViableAltException(this);
				}
				setState(320); match(OP_SQPAR);
				setState(321); realExpr(0);
				setState(322); match(COMMA);
				setState(323); realExpr(0);
				setState(324); match(CL_SQPAR);
				}
				break;

			case 8:
				{
				_localctx = new RealExprCondContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(326); match(IF_FN);
				setState(327); match(OP_SQPAR);
				setState(328); boolExpr(0);
				setState(329); match(COMMA);
				setState(330); realExpr(0);
				setState(331); match(COMMA);
				setState(332); realExpr(0);
				setState(333); match(CL_SQPAR);
				}
				break;

			case 9:
				{
				_localctx = new RealExprCond2Context(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(335); match(7);
				setState(336); boolExpr(0);
				setState(337); match(COLON);
				setState(338); realExpr(0);
				setState(339); match(SEMICOLON);
				setState(340); match(12);
				setState(341); realExpr(0);
				setState(342); match(SEMICOLON);
				}
				break;

			case 10:
				{
				_localctx = new RealExprPdfXVarContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(344); match(PDF_X_VAR);
				}
				break;

			case 11:
				{
				_localctx = new RealExprClockVarContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(345);
				if (!(clockVarInExpr)) throw new FailedPredicateException(this, "clockVarInExpr");
				setState(346); clockVar();
				}
				break;

			case 12:
				{
				_localctx = new RealExprUnknownIdContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(347); match(ID);
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(358);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,14,_ctx);
			while ( _alt!=2 && _alt!=-1 ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					setState(356);
					switch ( getInterpreter().adaptivePredict(_input,13,_ctx) ) {
					case 1:
						{
						_localctx = new RealExprMulDivContext(new RealExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_realExpr);
						setState(350);
						if (!(precpred(_ctx, 14))) throw new FailedPredicateException(this, "precpred(_ctx, 14)");
						setState(351);
						((RealExprMulDivContext)_localctx).op = _input.LT(1);
						_la = _input.LA(1);
						if ( !(_la==MUL || _la==DIV) ) {
							((RealExprMulDivContext)_localctx).op = (Token)_errHandler.recoverInline(this);
						}
						consume();
						setState(352); realExpr(15);
						}
						break;

					case 2:
						{
						_localctx = new RealExprAddSubContext(new RealExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_realExpr);
						setState(353);
						if (!(precpred(_ctx, 13))) throw new FailedPredicateException(this, "precpred(_ctx, 13)");
						setState(354);
						((RealExprAddSubContext)_localctx).op = _input.LT(1);
						_la = _input.LA(1);
						if ( !(_la==ADD || _la==SUB) ) {
							((RealExprAddSubContext)_localctx).op = (Token)_errHandler.recoverInline(this);
						}
						consume();
						setState(355); realExpr(14);
						}
						break;
					}
					} 
				}
				setState(360);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,14,_ctx);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			unrollRecursionContexts(_parentctx);
		}
		return _localctx;
	}

	public static class BoolExprContext extends ParserRuleContext {
		public BoolExprContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_boolExpr; }
	 
		public BoolExprContext() { }
		public void copyFrom(BoolExprContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class BoolExprCTLinitStateContext extends BoolExprContext {
		public TerminalNode INITIAL_STATE() { return getToken(ExprLangParser.INITIAL_STATE, 0); }
		public BoolExprCTLinitStateContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprCTLinitState(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprCTLStarUntil2Context extends BoolExprContext {
		public List<BoolExprContext> boolExpr() {
			return getRuleContexts(BoolExprContext.class);
		}
		public BoolExprContext boolExpr(int i) {
			return getRuleContext(BoolExprContext.class,i);
		}
		public TerminalNode CTLSTAR_UNTIL() { return getToken(ExprLangParser.CTLSTAR_UNTIL, 0); }
		public BoolExprCTLStarUntil2Context(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprCTLStarUntil2(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprColorTermCompContext extends BoolExprContext {
		public Token op;
		public ColorTermContext colorTerm(int i) {
			return getRuleContext(ColorTermContext.class,i);
		}
		public List<ColorTermContext> colorTerm() {
			return getRuleContexts(ColorTermContext.class);
		}
		public BoolExprColorTermCompContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprColorTermComp(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprOrContext extends BoolExprContext {
		public List<BoolExprContext> boolExpr() {
			return getRuleContexts(BoolExprContext.class);
		}
		public BoolExprContext boolExpr(int i) {
			return getRuleContext(BoolExprContext.class,i);
		}
		public BoolExprOrContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprOr(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprAndContext extends BoolExprContext {
		public List<BoolExprContext> boolExpr() {
			return getRuleContexts(BoolExprContext.class);
		}
		public BoolExprContext boolExpr(int i) {
			return getRuleContext(BoolExprContext.class,i);
		}
		public BoolExprAndContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprAnd(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprCTLStarUntilContext extends BoolExprContext {
		public List<BoolExprContext> boolExpr() {
			return getRuleContexts(BoolExprContext.class);
		}
		public BoolExprContext boolExpr(int i) {
			return getRuleContext(BoolExprContext.class,i);
		}
		public TerminalNode CTLSTAR_UNTIL() { return getToken(ExprLangParser.CTLSTAR_UNTIL, 0); }
		public BoolExprCTLStarUntilContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprCTLStarUntil(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprCTLStarQuantifContext extends BoolExprContext {
		public Token q;
		public TerminalNode CTLSTAR_EXISTS() { return getToken(ExprLangParser.CTLSTAR_EXISTS, 0); }
		public BoolExprContext boolExpr() {
			return getRuleContext(BoolExprContext.class,0);
		}
		public TerminalNode CTLSTAR_FORALL() { return getToken(ExprLangParser.CTLSTAR_FORALL, 0); }
		public BoolExprCTLStarQuantifContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprCTLStarQuantif(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprCTLStar2Context extends BoolExprContext {
		public BoolExprContext boolExpr() {
			return getRuleContext(BoolExprContext.class,0);
		}
		public Composite_temporal_op_ctlstarContext composite_temporal_op_ctlstar() {
			return getRuleContext(Composite_temporal_op_ctlstarContext.class,0);
		}
		public BoolExprCTLStar2Context(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprCTLStar2(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprImplyContext extends BoolExprContext {
		public List<BoolExprContext> boolExpr() {
			return getRuleContexts(BoolExprContext.class);
		}
		public BoolExprContext boolExpr(int i) {
			return getRuleContext(BoolExprContext.class,i);
		}
		public BoolExprImplyContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprImply(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprCTLContext extends BoolExprContext {
		public BoolExprContext boolExpr() {
			return getRuleContext(BoolExprContext.class,0);
		}
		public Temporal_opContext temporal_op() {
			return getRuleContext(Temporal_opContext.class,0);
		}
		public BoolExprCTLContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprCTL(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprNotContext extends BoolExprContext {
		public BoolExprContext boolExpr() {
			return getRuleContext(BoolExprContext.class,0);
		}
		public BoolExprNotContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprNot(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprCTLStarContext extends BoolExprContext {
		public Token op;
		public TerminalNode CTLSTAR_GLOBALLY() { return getToken(ExprLangParser.CTLSTAR_GLOBALLY, 0); }
		public BoolExprContext boolExpr() {
			return getRuleContext(BoolExprContext.class,0);
		}
		public TerminalNode CTLSTAR_FUTURE() { return getToken(ExprLangParser.CTLSTAR_FUTURE, 0); }
		public TerminalNode CTLSTAR_NEXT() { return getToken(ExprLangParser.CTLSTAR_NEXT, 0); }
		public BoolExprCTLStarContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprCTLStar(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprCTLpinContext extends BoolExprContext {
		public Token pin;
		public BoolExprContext boolExpr() {
			return getRuleContext(BoolExprContext.class,0);
		}
		public TerminalNode IMPOSSIBLY() { return getToken(ExprLangParser.IMPOSSIBLY, 0); }
		public TerminalNode INVARIANTLY() { return getToken(ExprLangParser.INVARIANTLY, 0); }
		public TerminalNode POSSIBLY() { return getToken(ExprLangParser.POSSIBLY, 0); }
		public BoolExprCTLpinContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprCTLpin(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprBiimplyContext extends BoolExprContext {
		public List<BoolExprContext> boolExpr() {
			return getRuleContexts(BoolExprContext.class);
		}
		public BoolExprContext boolExpr(int i) {
			return getRuleContext(BoolExprContext.class,i);
		}
		public BoolExprBiimplyContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprBiimply(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprIntCompContext extends BoolExprContext {
		public Token op;
		public List<IntExprContext> intExpr() {
			return getRuleContexts(IntExprContext.class);
		}
		public IntExprContext intExpr(int i) {
			return getRuleContext(IntExprContext.class,i);
		}
		public BoolExprIntCompContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprIntComp(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprConstContext extends BoolExprContext {
		public BoolConstContext boolConst() {
			return getRuleContext(BoolConstContext.class,0);
		}
		public BoolExprConstContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprConst(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprRealCompContext extends BoolExprContext {
		public Token op;
		public List<RealExprContext> realExpr() {
			return getRuleContexts(RealExprContext.class);
		}
		public RealExprContext realExpr(int i) {
			return getRuleContext(RealExprContext.class,i);
		}
		public BoolExprRealCompContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprRealComp(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprParenContext extends BoolExprContext {
		public BoolExprContext boolExpr() {
			return getRuleContext(BoolExprContext.class,0);
		}
		public BoolExprParenContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprParen(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprCTLUntilContext extends BoolExprContext {
		public TerminalNode EXISTS() { return getToken(ExprLangParser.EXISTS, 0); }
		public List<BoolExprContext> boolExpr() {
			return getRuleContexts(BoolExprContext.class);
		}
		public BoolExprContext boolExpr(int i) {
			return getRuleContext(BoolExprContext.class,i);
		}
		public TerminalNode UNTIL() { return getToken(ExprLangParser.UNTIL, 0); }
		public TerminalNode FORALL() { return getToken(ExprLangParser.FORALL, 0); }
		public BoolExprCTLUntilContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprCTLUntil(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprColorTermInContext extends BoolExprContext {
		public TerminalNode CONTAINS() { return getToken(ExprLangParser.CONTAINS, 0); }
		public TerminalNode DONT_CONTAINS() { return getToken(ExprLangParser.DONT_CONTAINS, 0); }
		public ColorTermContext colorTerm() {
			return getRuleContext(ColorTermContext.class,0);
		}
		public ColorSetContext colorSet() {
			return getRuleContext(ColorSetContext.class,0);
		}
		public BoolExprColorTermInContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprColorTermIn(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprCTLenabledContext extends BoolExprContext {
		public TerminalNode TRANSITION_ID() { return getToken(ExprLangParser.TRANSITION_ID, 0); }
		public TerminalNode ENABLED() { return getToken(ExprLangParser.ENABLED, 0); }
		public BoolExprCTLenabledContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprCTLenabled(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprStatePropIdContext extends BoolExprContext {
		public TerminalNode STATEPROP_ID() { return getToken(ExprLangParser.STATEPROP_ID, 0); }
		public BoolExprStatePropIdContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprStatePropId(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprUnknownIdContext extends BoolExprContext {
		public TerminalNode ID() { return getToken(ExprLangParser.ID, 0); }
		public BoolExprUnknownIdContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprUnknownId(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprCTLdeadlocksContext extends BoolExprContext {
		public TerminalNode DEADLOCK() { return getToken(ExprLangParser.DEADLOCK, 0); }
		public TerminalNode NO_DEADLOCK() { return getToken(ExprLangParser.NO_DEADLOCK, 0); }
		public BoolExprCTLdeadlocksContext(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprCTLdeadlocks(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class BoolExprCTLUntil2Context extends BoolExprContext {
		public TerminalNode EXISTS() { return getToken(ExprLangParser.EXISTS, 0); }
		public List<BoolExprContext> boolExpr() {
			return getRuleContexts(BoolExprContext.class);
		}
		public BoolExprContext boolExpr(int i) {
			return getRuleContext(BoolExprContext.class,i);
		}
		public TerminalNode UNTIL() { return getToken(ExprLangParser.UNTIL, 0); }
		public TerminalNode FORALL() { return getToken(ExprLangParser.FORALL, 0); }
		public BoolExprCTLUntil2Context(BoolExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBoolExprCTLUntil2(this);
			else return visitor.visitChildren(this);
		}
	}

	public final BoolExprContext boolExpr() throws RecognitionException {
		return boolExpr(0);
	}

	private BoolExprContext boolExpr(int _p) throws RecognitionException {
		ParserRuleContext _parentctx = _ctx;
		int _parentState = getState();
		BoolExprContext _localctx = new BoolExprContext(_ctx, _parentState);
		BoolExprContext _prevctx = _localctx;
		int _startState = 48;
		enterRecursionRule(_localctx, 48, RULE_boolExpr, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(431);
			switch ( getInterpreter().adaptivePredict(_input,15,_ctx) ) {
			case 1:
				{
				_localctx = new BoolExprNotContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(362); match(NOT);
				setState(363); boolExpr(25);
				}
				break;

			case 2:
				{
				_localctx = new BoolExprCTLContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(364); temporal_op();
				setState(365); boolExpr(12);
				}
				break;

			case 3:
				{
				_localctx = new BoolExprCTLpinContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(367);
				((BoolExprCTLpinContext)_localctx).pin = _input.LT(1);
				_la = _input.LA(1);
				if ( !(((((_la - 122)) & ~0x3f) == 0 && ((1L << (_la - 122)) & ((1L << (POSSIBLY - 122)) | (1L << (IMPOSSIBLY - 122)) | (1L << (INVARIANTLY - 122)))) != 0)) ) {
					((BoolExprCTLpinContext)_localctx).pin = (Token)_errHandler.recoverInline(this);
				}
				consume();
				setState(368); boolExpr(9);
				}
				break;

			case 4:
				{
				_localctx = new BoolExprCTLStarContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(369);
				((BoolExprCTLStarContext)_localctx).op = _input.LT(1);
				_la = _input.LA(1);
				if ( !(((((_la - 130)) & ~0x3f) == 0 && ((1L << (_la - 130)) & ((1L << (CTLSTAR_NEXT - 130)) | (1L << (CTLSTAR_FUTURE - 130)) | (1L << (CTLSTAR_GLOBALLY - 130)))) != 0)) ) {
					((BoolExprCTLStarContext)_localctx).op = (Token)_errHandler.recoverInline(this);
				}
				consume();
				setState(370); boolExpr(5);
				}
				break;

			case 5:
				{
				_localctx = new BoolExprCTLStarQuantifContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(371);
				((BoolExprCTLStarQuantifContext)_localctx).q = _input.LT(1);
				_la = _input.LA(1);
				if ( !(_la==CTLSTAR_EXISTS || _la==CTLSTAR_FORALL) ) {
					((BoolExprCTLStarQuantifContext)_localctx).q = (Token)_errHandler.recoverInline(this);
				}
				consume();
				setState(372); boolExpr(2);
				}
				break;

			case 6:
				{
				_localctx = new BoolExprCTLStar2Context(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(373); composite_temporal_op_ctlstar();
				setState(374); boolExpr(1);
				}
				break;

			case 7:
				{
				_localctx = new BoolExprConstContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(376); boolConst();
				}
				break;

			case 8:
				{
				_localctx = new BoolExprIntCompContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(377); intExpr(0);
				setState(378);
				((BoolExprIntCompContext)_localctx).op = _input.LT(1);
				_la = _input.LA(1);
				if ( !((((_la) & ~0x3f) == 0 && ((1L << _la) & ((1L << EQUAL) | (1L << NOT_EQUAL) | (1L << LESS) | (1L << GREATER) | (1L << LESS_EQ) | (1L << GREATER_EQ))) != 0)) ) {
					((BoolExprIntCompContext)_localctx).op = (Token)_errHandler.recoverInline(this);
				}
				consume();
				setState(379); intExpr(0);
				}
				break;

			case 9:
				{
				_localctx = new BoolExprRealCompContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(381); realExpr(0);
				setState(382);
				((BoolExprRealCompContext)_localctx).op = _input.LT(1);
				_la = _input.LA(1);
				if ( !((((_la) & ~0x3f) == 0 && ((1L << _la) & ((1L << EQUAL) | (1L << NOT_EQUAL) | (1L << LESS) | (1L << GREATER) | (1L << LESS_EQ) | (1L << GREATER_EQ))) != 0)) ) {
					((BoolExprRealCompContext)_localctx).op = (Token)_errHandler.recoverInline(this);
				}
				consume();
				setState(383); realExpr(0);
				}
				break;

			case 10:
				{
				_localctx = new BoolExprParenContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(385); match(OP_PAREN);
				setState(386); boolExpr(0);
				setState(387); match(CL_PAREN);
				}
				break;

			case 11:
				{
				_localctx = new BoolExprUnknownIdContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(389); match(ID);
				}
				break;

			case 12:
				{
				_localctx = new BoolExprStatePropIdContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(390); match(STATEPROP_ID);
				}
				break;

			case 13:
				{
				_localctx = new BoolExprColorTermCompContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(391); colorTerm(0);
				setState(392);
				((BoolExprColorTermCompContext)_localctx).op = _input.LT(1);
				_la = _input.LA(1);
				if ( !((((_la) & ~0x3f) == 0 && ((1L << _la) & ((1L << EQUAL) | (1L << NOT_EQUAL) | (1L << LESS) | (1L << GREATER) | (1L << LESS_EQ) | (1L << GREATER_EQ))) != 0)) ) {
					((BoolExprColorTermCompContext)_localctx).op = (Token)_errHandler.recoverInline(this);
				}
				consume();
				setState(393); colorTerm(0);
				}
				break;

			case 14:
				{
				_localctx = new BoolExprColorTermInContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(395); colorTerm(0);
				setState(396);
				_la = _input.LA(1);
				if ( !(_la==CONTAINS || _la==DONT_CONTAINS) ) {
				_errHandler.recoverInline(this);
				}
				consume();
				setState(397); colorSet();
				}
				break;

			case 15:
				{
				_localctx = new BoolExprCTLUntilContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(399);
				_la = _input.LA(1);
				if ( !(_la==EXISTS || _la==FORALL) ) {
				_errHandler.recoverInline(this);
				}
				consume();
				setState(400); match(OP_PAREN);
				setState(401); boolExpr(0);
				setState(402); match(UNTIL);
				setState(403); boolExpr(0);
				setState(404); match(CL_PAREN);
				}
				break;

			case 16:
				{
				_localctx = new BoolExprCTLUntil2Context(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(406);
				_la = _input.LA(1);
				if ( !(_la==EXISTS || _la==FORALL) ) {
				_errHandler.recoverInline(this);
				}
				consume();
				setState(407); match(OP_SQPAR);
				setState(408); boolExpr(0);
				setState(409); match(UNTIL);
				setState(410); boolExpr(0);
				setState(411); match(CL_SQPAR);
				}
				break;

			case 17:
				{
				_localctx = new BoolExprCTLenabledContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(413); match(ENABLED);
				setState(414); match(OP_PAREN);
				setState(415); match(TRANSITION_ID);
				setState(416); match(CL_PAREN);
				}
				break;

			case 18:
				{
				_localctx = new BoolExprCTLdeadlocksContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(417);
				_la = _input.LA(1);
				if ( !(_la==DEADLOCK || _la==NO_DEADLOCK) ) {
				_errHandler.recoverInline(this);
				}
				consume();
				}
				break;

			case 19:
				{
				_localctx = new BoolExprCTLinitStateContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(418); match(INITIAL_STATE);
				}
				break;

			case 20:
				{
				_localctx = new BoolExprCTLStarUntilContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(419); match(OP_PAREN);
				setState(420); boolExpr(0);
				setState(421); match(CTLSTAR_UNTIL);
				setState(422); boolExpr(0);
				setState(423); match(CL_PAREN);
				}
				break;

			case 21:
				{
				_localctx = new BoolExprCTLStarUntil2Context(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(425); match(OP_SQPAR);
				setState(426); boolExpr(0);
				setState(427); match(CTLSTAR_UNTIL);
				setState(428); boolExpr(0);
				setState(429); match(CL_SQPAR);
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(447);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,17,_ctx);
			while ( _alt!=2 && _alt!=-1 ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					setState(445);
					switch ( getInterpreter().adaptivePredict(_input,16,_ctx) ) {
					case 1:
						{
						_localctx = new BoolExprAndContext(new BoolExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_boolExpr);
						setState(433);
						if (!(precpred(_ctx, 24))) throw new FailedPredicateException(this, "precpred(_ctx, 24)");
						setState(434); match(AND);
						setState(435); boolExpr(25);
						}
						break;

					case 2:
						{
						_localctx = new BoolExprOrContext(new BoolExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_boolExpr);
						setState(436);
						if (!(precpred(_ctx, 23))) throw new FailedPredicateException(this, "precpred(_ctx, 23)");
						setState(437); match(OR);
						setState(438); boolExpr(24);
						}
						break;

					case 3:
						{
						_localctx = new BoolExprImplyContext(new BoolExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_boolExpr);
						setState(439);
						if (!(precpred(_ctx, 22))) throw new FailedPredicateException(this, "precpred(_ctx, 22)");
						setState(440); match(IMPLY);
						setState(441); boolExpr(23);
						}
						break;

					case 4:
						{
						_localctx = new BoolExprBiimplyContext(new BoolExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_boolExpr);
						setState(442);
						if (!(precpred(_ctx, 21))) throw new FailedPredicateException(this, "precpred(_ctx, 21)");
						setState(443); match(BIIMPLY);
						setState(444); boolExpr(22);
						}
						break;
					}
					} 
				}
				setState(449);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,17,_ctx);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			unrollRecursionContexts(_parentctx);
		}
		return _localctx;
	}

	public static class UnaryIntFnContext extends ParserRuleContext {
		public Token fn;
		public TerminalNode FACTORIAL_FN() { return getToken(ExprLangParser.FACTORIAL_FN, 0); }
		public TerminalNode ABS_FN() { return getToken(ExprLangParser.ABS_FN, 0); }
		public UnaryIntFnContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_unaryIntFn; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitUnaryIntFn(this);
			else return visitor.visitChildren(this);
		}
	}

	public final UnaryIntFnContext unaryIntFn() throws RecognitionException {
		UnaryIntFnContext _localctx = new UnaryIntFnContext(_ctx, getState());
		enterRule(_localctx, 50, RULE_unaryIntFn);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(450);
			((UnaryIntFnContext)_localctx).fn = _input.LT(1);
			_la = _input.LA(1);
			if ( !(_la==FACTORIAL_FN || _la==ABS_FN) ) {
				((UnaryIntFnContext)_localctx).fn = (Token)_errHandler.recoverInline(this);
			}
			consume();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class UnaryIntRealFnContext extends ParserRuleContext {
		public Token fn;
		public TerminalNode CEIL_FN() { return getToken(ExprLangParser.CEIL_FN, 0); }
		public TerminalNode ROUND_FN() { return getToken(ExprLangParser.ROUND_FN, 0); }
		public TerminalNode FLOOR_FN() { return getToken(ExprLangParser.FLOOR_FN, 0); }
		public UnaryIntRealFnContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_unaryIntRealFn; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitUnaryIntRealFn(this);
			else return visitor.visitChildren(this);
		}
	}

	public final UnaryIntRealFnContext unaryIntRealFn() throws RecognitionException {
		UnaryIntRealFnContext _localctx = new UnaryIntRealFnContext(_ctx, getState());
		enterRule(_localctx, 52, RULE_unaryIntRealFn);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(452);
			((UnaryIntRealFnContext)_localctx).fn = _input.LT(1);
			_la = _input.LA(1);
			if ( !(((((_la - 72)) & ~0x3f) == 0 && ((1L << (_la - 72)) & ((1L << (CEIL_FN - 72)) | (1L << (FLOOR_FN - 72)) | (1L << (ROUND_FN - 72)))) != 0)) ) {
				((UnaryIntRealFnContext)_localctx).fn = (Token)_errHandler.recoverInline(this);
			}
			consume();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class BinaryIntFnContext extends ParserRuleContext {
		public Token fn;
		public TerminalNode MOD_FN() { return getToken(ExprLangParser.MOD_FN, 0); }
		public TerminalNode BINOMIAL_FN() { return getToken(ExprLangParser.BINOMIAL_FN, 0); }
		public TerminalNode FRACT_FN() { return getToken(ExprLangParser.FRACT_FN, 0); }
		public TerminalNode MAX_FN() { return getToken(ExprLangParser.MAX_FN, 0); }
		public TerminalNode POW_FN() { return getToken(ExprLangParser.POW_FN, 0); }
		public TerminalNode MIN_FN() { return getToken(ExprLangParser.MIN_FN, 0); }
		public BinaryIntFnContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_binaryIntFn; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBinaryIntFn(this);
			else return visitor.visitChildren(this);
		}
	}

	public final BinaryIntFnContext binaryIntFn() throws RecognitionException {
		BinaryIntFnContext _localctx = new BinaryIntFnContext(_ctx, getState());
		enterRule(_localctx, 54, RULE_binaryIntFn);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(454);
			((BinaryIntFnContext)_localctx).fn = _input.LT(1);
			_la = _input.LA(1);
			if ( !(((((_la - 56)) & ~0x3f) == 0 && ((1L << (_la - 56)) & ((1L << (MIN_FN - 56)) | (1L << (MAX_FN - 56)) | (1L << (MOD_FN - 56)) | (1L << (POW_FN - 56)) | (1L << (FRACT_FN - 56)) | (1L << (BINOMIAL_FN - 56)))) != 0)) ) {
				((BinaryIntFnContext)_localctx).fn = (Token)_errHandler.recoverInline(this);
			}
			consume();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class BinaryRealFnContext extends ParserRuleContext {
		public Token fn;
		public TerminalNode TRIANGULAR_FN() { return getToken(ExprLangParser.TRIANGULAR_FN, 0); }
		public TerminalNode ERLANG_FN() { return getToken(ExprLangParser.ERLANG_FN, 0); }
		public TerminalNode RECT_FN() { return getToken(ExprLangParser.RECT_FN, 0); }
		public TerminalNode UNIFORM_FN() { return getToken(ExprLangParser.UNIFORM_FN, 0); }
		public TerminalNode TRUNCATED_EXP_FN() { return getToken(ExprLangParser.TRUNCATED_EXP_FN, 0); }
		public TerminalNode PARETO_FN() { return getToken(ExprLangParser.PARETO_FN, 0); }
		public BinaryRealFnContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_binaryRealFn; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitBinaryRealFn(this);
			else return visitor.visitChildren(this);
		}
	}

	public final BinaryRealFnContext binaryRealFn() throws RecognitionException {
		BinaryRealFnContext _localctx = new BinaryRealFnContext(_ctx, getState());
		enterRule(_localctx, 56, RULE_binaryRealFn);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(456);
			((BinaryRealFnContext)_localctx).fn = _input.LT(1);
			_la = _input.LA(1);
			if ( !(((((_la - 143)) & ~0x3f) == 0 && ((1L << (_la - 143)) & ((1L << (RECT_FN - 143)) | (1L << (UNIFORM_FN - 143)) | (1L << (TRIANGULAR_FN - 143)) | (1L << (ERLANG_FN - 143)) | (1L << (TRUNCATED_EXP_FN - 143)) | (1L << (PARETO_FN - 143)))) != 0)) ) {
				((BinaryRealFnContext)_localctx).fn = (Token)_errHandler.recoverInline(this);
			}
			consume();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class UnaryRealFnContext extends ParserRuleContext {
		public Token fn;
		public TerminalNode ARCSIN_FN() { return getToken(ExprLangParser.ARCSIN_FN, 0); }
		public TerminalNode EXP_FN() { return getToken(ExprLangParser.EXP_FN, 0); }
		public TerminalNode DIRAC_DELTA_FN() { return getToken(ExprLangParser.DIRAC_DELTA_FN, 0); }
		public TerminalNode COS_FN() { return getToken(ExprLangParser.COS_FN, 0); }
		public TerminalNode SIN_FN() { return getToken(ExprLangParser.SIN_FN, 0); }
		public TerminalNode ARCCOS_FN() { return getToken(ExprLangParser.ARCCOS_FN, 0); }
		public TerminalNode ARCTAN_FN() { return getToken(ExprLangParser.ARCTAN_FN, 0); }
		public TerminalNode TAN_FN() { return getToken(ExprLangParser.TAN_FN, 0); }
		public TerminalNode SQRT_FN() { return getToken(ExprLangParser.SQRT_FN, 0); }
		public UnaryRealFnContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_unaryRealFn; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitUnaryRealFn(this);
			else return visitor.visitChildren(this);
		}
	}

	public final UnaryRealFnContext unaryRealFn() throws RecognitionException {
		UnaryRealFnContext _localctx = new UnaryRealFnContext(_ctx, getState());
		enterRule(_localctx, 58, RULE_unaryRealFn);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(458);
			((UnaryRealFnContext)_localctx).fn = _input.LT(1);
			_la = _input.LA(1);
			if ( !(((((_la - 63)) & ~0x3f) == 0 && ((1L << (_la - 63)) & ((1L << (SIN_FN - 63)) | (1L << (COS_FN - 63)) | (1L << (TAN_FN - 63)) | (1L << (ARCSIN_FN - 63)) | (1L << (ARCCOS_FN - 63)) | (1L << (ARCTAN_FN - 63)) | (1L << (EXP_FN - 63)) | (1L << (SQRT_FN - 63)))) != 0) || _la==DIRAC_DELTA_FN) ) {
				((UnaryRealFnContext)_localctx).fn = (Token)_errHandler.recoverInline(this);
			}
			consume();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class MSetPredicateContext extends ParserRuleContext {
		public MSetPredicateContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_mSetPredicate; }
	 
		public MSetPredicateContext() { }
		public void copyFrom(MSetPredicateContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class MSetBoolPredicateContext extends MSetPredicateContext {
		public BoolExprContext boolExpr() {
			return getRuleContext(BoolExprContext.class,0);
		}
		public MSetBoolPredicateContext(MSetPredicateContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMSetBoolPredicate(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MSetPredicateContext mSetPredicate() throws RecognitionException {
		MSetPredicateContext _localctx = new MSetPredicateContext(_ctx, getState());
		enterRule(_localctx, 60, RULE_mSetPredicate);
		try {
			_localctx = new MSetBoolPredicateContext(_localctx);
			enterOuterAlt(_localctx, 1);
			{
			setState(460); match(OP_SQPAR);
			setState(461); boolExpr(0);
			setState(462); match(CL_SQPAR);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class MSetElemPredicateContext extends ParserRuleContext {
		public MSetElemPredicateContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_mSetElemPredicate; }
	 
		public MSetElemPredicateContext() { }
		public void copyFrom(MSetElemPredicateContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class MSetElemBoolPredicateContext extends MSetElemPredicateContext {
		public BoolExprContext boolExpr() {
			return getRuleContext(BoolExprContext.class,0);
		}
		public MSetElemBoolPredicateContext(MSetElemPredicateContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMSetElemBoolPredicate(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MSetElemPredicateContext mSetElemPredicate() throws RecognitionException {
		MSetElemPredicateContext _localctx = new MSetElemPredicateContext(_ctx, getState());
		enterRule(_localctx, 62, RULE_mSetElemPredicate);
		try {
			_localctx = new MSetElemBoolPredicateContext(_localctx);
			enterOuterAlt(_localctx, 1);
			{
			setState(464); match(OP_SQPAR);
			setState(465); boolExpr(0);
			setState(466); match(CL_SQPAR);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class RealMSetExprContext extends ParserRuleContext {
		public RealMSetExprContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_realMSetExpr; }
	 
		public RealMSetExprContext() { }
		public void copyFrom(RealMSetExprContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class RealMSetExprElemProductContext extends RealMSetExprContext {
		public RealExprContext realExpr() {
			return getRuleContext(RealExprContext.class,0);
		}
		public MSetElemPredicateContext mSetElemPredicate() {
			return getRuleContext(MSetElemPredicateContext.class,0);
		}
		public MSetPredicateContext mSetPredicate() {
			return getRuleContext(MSetPredicateContext.class,0);
		}
		public MultiSetElemContext multiSetElem(int i) {
			return getRuleContext(MultiSetElemContext.class,i);
		}
		public List<MultiSetElemContext> multiSetElem() {
			return getRuleContexts(MultiSetElemContext.class);
		}
		public RealMSetExprElemProductContext(RealMSetExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealMSetExprElemProduct(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class RealMSetExprParenContext extends RealMSetExprContext {
		public RealMSetExprContext realMSetExpr() {
			return getRuleContext(RealMSetExprContext.class,0);
		}
		public RealMSetExprParenContext(RealMSetExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealMSetExprParen(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class RealMSetExprAddSubContext extends RealMSetExprContext {
		public Token op;
		public List<RealMSetExprContext> realMSetExpr() {
			return getRuleContexts(RealMSetExprContext.class);
		}
		public RealMSetExprContext realMSetExpr(int i) {
			return getRuleContext(RealMSetExprContext.class,i);
		}
		public RealMSetExprAddSubContext(RealMSetExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealMSetExprAddSub(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class RealMsetExprPlaceMarkingContext extends RealMSetExprContext {
		public TerminalNode REAL_MSET_PLACE_ID() { return getToken(ExprLangParser.REAL_MSET_PLACE_ID, 0); }
		public RealMsetExprPlaceMarkingContext(RealMSetExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealMsetExprPlaceMarking(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class RealMsetExprConstContext extends RealMSetExprContext {
		public TerminalNode REAL_MSET_CONST_ID() { return getToken(ExprLangParser.REAL_MSET_CONST_ID, 0); }
		public RealMsetExprConstContext(RealMSetExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitRealMsetExprConst(this);
			else return visitor.visitChildren(this);
		}
	}

	public final RealMSetExprContext realMSetExpr() throws RecognitionException {
		return realMSetExpr(0);
	}

	private RealMSetExprContext realMSetExpr(int _p) throws RecognitionException {
		ParserRuleContext _parentctx = _ctx;
		int _parentState = getState();
		RealMSetExprContext _localctx = new RealMSetExprContext(_ctx, _parentState);
		RealMSetExprContext _prevctx = _localctx;
		int _startState = 64;
		enterRecursionRule(_localctx, 64, RULE_realMSetExpr, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(495);
			switch ( getInterpreter().adaptivePredict(_input,22,_ctx) ) {
			case 1:
				{
				_localctx = new RealMSetExprParenContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(469); match(OP_PAREN);
				setState(470); realMSetExpr(0);
				setState(471); match(CL_PAREN);
				}
				break;

			case 2:
				{
				_localctx = new RealMSetExprElemProductContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(474);
				switch ( getInterpreter().adaptivePredict(_input,18,_ctx) ) {
				case 1:
					{
					setState(473); realExpr(0);
					}
					break;
				}
				setState(477);
				_la = _input.LA(1);
				if (_la==OP_SQPAR) {
					{
					setState(476); mSetPredicate();
					}
				}

				setState(479); match(LESS);
				setState(480); multiSetElem(0);
				setState(485);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==COMMA) {
					{
					{
					setState(481); match(COMMA);
					setState(482); multiSetElem(0);
					}
					}
					setState(487);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				setState(488); match(GREATER);
				setState(490);
				switch ( getInterpreter().adaptivePredict(_input,21,_ctx) ) {
				case 1:
					{
					setState(489); mSetElemPredicate();
					}
					break;
				}
				}
				break;

			case 3:
				{
				_localctx = new RealMsetExprPlaceMarkingContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(492); match(SHARP);
				setState(493); match(REAL_MSET_PLACE_ID);
				}
				break;

			case 4:
				{
				_localctx = new RealMsetExprConstContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(494); match(REAL_MSET_CONST_ID);
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(502);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,23,_ctx);
			while ( _alt!=2 && _alt!=-1 ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					{
					_localctx = new RealMSetExprAddSubContext(new RealMSetExprContext(_parentctx, _parentState));
					pushNewRecursionContext(_localctx, _startState, RULE_realMSetExpr);
					setState(497);
					if (!(precpred(_ctx, 4))) throw new FailedPredicateException(this, "precpred(_ctx, 4)");
					setState(498);
					((RealMSetExprAddSubContext)_localctx).op = _input.LT(1);
					_la = _input.LA(1);
					if ( !(_la==ADD || _la==SUB) ) {
						((RealMSetExprAddSubContext)_localctx).op = (Token)_errHandler.recoverInline(this);
					}
					consume();
					setState(499); realMSetExpr(5);
					}
					} 
				}
				setState(504);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,23,_ctx);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			unrollRecursionContexts(_parentctx);
		}
		return _localctx;
	}

	public static class IntMSetExprContext extends ParserRuleContext {
		public IntMSetExprContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_intMSetExpr; }
	 
		public IntMSetExprContext() { }
		public void copyFrom(IntMSetExprContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class IntMSetExprConstContext extends IntMSetExprContext {
		public TerminalNode INT_MSET_CONST_ID() { return getToken(ExprLangParser.INT_MSET_CONST_ID, 0); }
		public IntMSetExprConstContext(IntMSetExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntMSetExprConst(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class IntMsetExprPlaceMarkingContext extends IntMSetExprContext {
		public TerminalNode INT_MSET_PLACE_ID() { return getToken(ExprLangParser.INT_MSET_PLACE_ID, 0); }
		public IntMsetExprPlaceMarkingContext(IntMSetExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntMsetExprPlaceMarking(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class IntMSetExprParenContext extends IntMSetExprContext {
		public IntMSetExprContext intMSetExpr() {
			return getRuleContext(IntMSetExprContext.class,0);
		}
		public IntMSetExprParenContext(IntMSetExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntMSetExprParen(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class IntMSetExprAddSubContext extends IntMSetExprContext {
		public Token op;
		public List<IntMSetExprContext> intMSetExpr() {
			return getRuleContexts(IntMSetExprContext.class);
		}
		public IntMSetExprContext intMSetExpr(int i) {
			return getRuleContext(IntMSetExprContext.class,i);
		}
		public IntMSetExprAddSubContext(IntMSetExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntMSetExprAddSub(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class IntMSetExprElemProductContext extends IntMSetExprContext {
		public MSetElemPredicateContext mSetElemPredicate() {
			return getRuleContext(MSetElemPredicateContext.class,0);
		}
		public IntExprContext intExpr() {
			return getRuleContext(IntExprContext.class,0);
		}
		public MSetPredicateContext mSetPredicate() {
			return getRuleContext(MSetPredicateContext.class,0);
		}
		public MultiSetElemContext multiSetElem(int i) {
			return getRuleContext(MultiSetElemContext.class,i);
		}
		public List<MultiSetElemContext> multiSetElem() {
			return getRuleContexts(MultiSetElemContext.class);
		}
		public IntMSetExprElemProductContext(IntMSetExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitIntMSetExprElemProduct(this);
			else return visitor.visitChildren(this);
		}
	}

	public final IntMSetExprContext intMSetExpr() throws RecognitionException {
		return intMSetExpr(0);
	}

	private IntMSetExprContext intMSetExpr(int _p) throws RecognitionException {
		ParserRuleContext _parentctx = _ctx;
		int _parentState = getState();
		IntMSetExprContext _localctx = new IntMSetExprContext(_ctx, _parentState);
		IntMSetExprContext _prevctx = _localctx;
		int _startState = 66;
		enterRecursionRule(_localctx, 66, RULE_intMSetExpr, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(532);
			switch ( getInterpreter().adaptivePredict(_input,28,_ctx) ) {
			case 1:
				{
				_localctx = new IntMSetExprParenContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(506); match(OP_PAREN);
				setState(507); intMSetExpr(0);
				setState(508); match(CL_PAREN);
				}
				break;

			case 2:
				{
				_localctx = new IntMSetExprElemProductContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(511);
				_la = _input.LA(1);
				if ((((_la) & ~0x3f) == 0 && ((1L << _la) & ((1L << 7) | (1L << SUB) | (1L << OP_PAREN) | (1L << SHARP) | (1L << COLOR_ORDINAL) | (1L << MULTISET_CARD) | (1L << MIN_FN) | (1L << MAX_FN) | (1L << MOD_FN) | (1L << POW_FN) | (1L << FRACT_FN) | (1L << FACTORIAL_FN) | (1L << ABS_FN))) != 0) || ((((_la - 72)) & ~0x3f) == 0 && ((1L << (_la - 72)) & ((1L << (CEIL_FN - 72)) | (1L << (FLOOR_FN - 72)) | (1L << (ROUND_FN - 72)) | (1L << (BINOMIAL_FN - 72)) | (1L << (IF_FN - 72)) | (1L << (ID - 72)) | (1L << (INT - 72)) | (1L << (INT_CONST_ID - 72)) | (1L << (INT_TEMPLATE_ID - 72)) | (1L << (BOUNDS - 72)))) != 0)) {
					{
					setState(510); intExpr(0);
					}
				}

				setState(514);
				_la = _input.LA(1);
				if (_la==OP_SQPAR) {
					{
					setState(513); mSetPredicate();
					}
				}

				setState(516); match(LESS);
				setState(517); multiSetElem(0);
				setState(522);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==COMMA) {
					{
					{
					setState(518); match(COMMA);
					setState(519); multiSetElem(0);
					}
					}
					setState(524);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				setState(525); match(GREATER);
				setState(527);
				switch ( getInterpreter().adaptivePredict(_input,27,_ctx) ) {
				case 1:
					{
					setState(526); mSetElemPredicate();
					}
					break;
				}
				}
				break;

			case 3:
				{
				_localctx = new IntMsetExprPlaceMarkingContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(529); match(SHARP);
				setState(530); match(INT_MSET_PLACE_ID);
				}
				break;

			case 4:
				{
				_localctx = new IntMSetExprConstContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(531); match(INT_MSET_CONST_ID);
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(539);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,29,_ctx);
			while ( _alt!=2 && _alt!=-1 ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					{
					_localctx = new IntMSetExprAddSubContext(new IntMSetExprContext(_parentctx, _parentState));
					pushNewRecursionContext(_localctx, _startState, RULE_intMSetExpr);
					setState(534);
					if (!(precpred(_ctx, 4))) throw new FailedPredicateException(this, "precpred(_ctx, 4)");
					setState(535);
					((IntMSetExprAddSubContext)_localctx).op = _input.LT(1);
					_la = _input.LA(1);
					if ( !(_la==ADD || _la==SUB) ) {
						((IntMSetExprAddSubContext)_localctx).op = (Token)_errHandler.recoverInline(this);
					}
					consume();
					setState(536); intMSetExpr(5);
					}
					} 
				}
				setState(541);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,29,_ctx);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			unrollRecursionContexts(_parentctx);
		}
		return _localctx;
	}

	public static class MultiSetElemContext extends ParserRuleContext {
		public MultiSetElemContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_multiSetElem; }
	 
		public MultiSetElemContext() { }
		public void copyFrom(MultiSetElemContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class MSetElemColorTermContext extends MultiSetElemContext {
		public ColorSetContext colorSet() {
			return getRuleContext(ColorSetContext.class,0);
		}
		public MSetElemColorTermContext(MultiSetElemContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMSetElemColorTerm(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class MSetElemAddSubContext extends MultiSetElemContext {
		public Token op;
		public MultiSetElemContext multiSetElem(int i) {
			return getRuleContext(MultiSetElemContext.class,i);
		}
		public List<MultiSetElemContext> multiSetElem() {
			return getRuleContexts(MultiSetElemContext.class);
		}
		public MSetElemAddSubContext(MultiSetElemContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMSetElemAddSub(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MultiSetElemContext multiSetElem() throws RecognitionException {
		return multiSetElem(0);
	}

	private MultiSetElemContext multiSetElem(int _p) throws RecognitionException {
		ParserRuleContext _parentctx = _ctx;
		int _parentState = getState();
		MultiSetElemContext _localctx = new MultiSetElemContext(_ctx, _parentState);
		MultiSetElemContext _prevctx = _localctx;
		int _startState = 68;
		enterRecursionRule(_localctx, 68, RULE_multiSetElem, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			{
			_localctx = new MSetElemColorTermContext(_localctx);
			_ctx = _localctx;
			_prevctx = _localctx;

			setState(543); colorSet();
			}
			_ctx.stop = _input.LT(-1);
			setState(550);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,30,_ctx);
			while ( _alt!=2 && _alt!=-1 ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					{
					_localctx = new MSetElemAddSubContext(new MultiSetElemContext(_parentctx, _parentState));
					pushNewRecursionContext(_localctx, _startState, RULE_multiSetElem);
					setState(545);
					if (!(precpred(_ctx, 2))) throw new FailedPredicateException(this, "precpred(_ctx, 2)");
					setState(546);
					((MSetElemAddSubContext)_localctx).op = _input.LT(1);
					_la = _input.LA(1);
					if ( !(_la==ADD || _la==SUB) ) {
						((MSetElemAddSubContext)_localctx).op = (Token)_errHandler.recoverInline(this);
					}
					consume();
					setState(547); multiSetElem(3);
					}
					} 
				}
				setState(552);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,30,_ctx);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			unrollRecursionContexts(_parentctx);
		}
		return _localctx;
	}

	public static class ColorVarContext extends ParserRuleContext {
		public ColorVarContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_colorVar; }
	 
		public ColorVarContext() { }
		public void copyFrom(ColorVarContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class ColorVarIdentifierContext extends ColorVarContext {
		public TerminalNode COLORVAR_ID() { return getToken(ExprLangParser.COLORVAR_ID, 0); }
		public ColorVarIdentifierContext(ColorVarContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitColorVarIdentifier(this);
			else return visitor.visitChildren(this);
		}
	}

	public final ColorVarContext colorVar() throws RecognitionException {
		ColorVarContext _localctx = new ColorVarContext(_ctx, getState());
		enterRule(_localctx, 70, RULE_colorVar);
		try {
			_localctx = new ColorVarIdentifierContext(_localctx);
			enterOuterAlt(_localctx, 1);
			{
			setState(553); match(COLORVAR_ID);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class ColorSubclassContext extends ParserRuleContext {
		public ColorSubclassContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_colorSubclass; }
	 
		public ColorSubclassContext() { }
		public void copyFrom(ColorSubclassContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class ColorSubclassIdentifierContext extends ColorSubclassContext {
		public TerminalNode COLORSUBCLASS_ID() { return getToken(ExprLangParser.COLORSUBCLASS_ID, 0); }
		public ColorSubclassIdentifierContext(ColorSubclassContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitColorSubclassIdentifier(this);
			else return visitor.visitChildren(this);
		}
	}

	public final ColorSubclassContext colorSubclass() throws RecognitionException {
		ColorSubclassContext _localctx = new ColorSubclassContext(_ctx, getState());
		enterRule(_localctx, 72, RULE_colorSubclass);
		try {
			_localctx = new ColorSubclassIdentifierContext(_localctx);
			enterOuterAlt(_localctx, 1);
			{
			setState(555); match(COLORSUBCLASS_ID);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class ColorTermContext extends ParserRuleContext {
		public ColorTermContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_colorTerm; }
	 
		public ColorTermContext() { }
		public void copyFrom(ColorTermContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class ColorTermNextPrevContext extends ColorTermContext {
		public Token op;
		public ColorTermContext colorTerm() {
			return getRuleContext(ColorTermContext.class,0);
		}
		public ColorTermNextPrevContext(ColorTermContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitColorTermNextPrev(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ColorTermColorContext extends ColorTermContext {
		public TerminalNode COLOR_ID() { return getToken(ExprLangParser.COLOR_ID, 0); }
		public ColorTermColorContext(ColorTermContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitColorTermColor(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ColorTermVarContext extends ColorTermContext {
		public ColorVarContext colorVar() {
			return getRuleContext(ColorVarContext.class,0);
		}
		public ColorTermVarContext(ColorTermContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitColorTermVar(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ColorTermFilterThisContext extends ColorTermContext {
		public TerminalNode SIMPLECOLORCLASS_ID() { return getToken(ExprLangParser.SIMPLECOLORCLASS_ID, 0); }
		public TerminalNode INT() { return getToken(ExprLangParser.INT, 0); }
		public ColorTermFilterThisContext(ColorTermContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitColorTermFilterThis(this);
			else return visitor.visitChildren(this);
		}
	}

	public final ColorTermContext colorTerm() throws RecognitionException {
		return colorTerm(0);
	}

	private ColorTermContext colorTerm(int _p) throws RecognitionException {
		ParserRuleContext _parentctx = _ctx;
		int _parentState = getState();
		ColorTermContext _localctx = new ColorTermContext(_ctx, _parentState);
		ColorTermContext _prevctx = _localctx;
		int _startState = 74;
		enterRecursionRule(_localctx, 74, RULE_colorTerm, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(569);
			switch (_input.LA(1)) {
			case COLORVAR_ID:
				{
				_localctx = new ColorTermVarContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(558); colorVar();
				}
				break;
			case COLOR_ID:
				{
				_localctx = new ColorTermColorContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(559); match(COLOR_ID);
				}
				break;
			case 14:
				{
				_localctx = new ColorTermFilterThisContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(560); match(14);
				setState(562);
				switch ( getInterpreter().adaptivePredict(_input,31,_ctx) ) {
				case 1:
					{
					setState(561); match(SIMPLECOLORCLASS_ID);
					}
					break;
				}
				setState(567);
				switch ( getInterpreter().adaptivePredict(_input,32,_ctx) ) {
				case 1:
					{
					setState(564); match(OP_SQPAR);
					setState(565); match(INT);
					setState(566); match(CL_SQPAR);
					}
					break;
				}
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
			_ctx.stop = _input.LT(-1);
			setState(575);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,34,_ctx);
			while ( _alt!=2 && _alt!=-1 ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					{
					_localctx = new ColorTermNextPrevContext(new ColorTermContext(_parentctx, _parentState));
					pushNewRecursionContext(_localctx, _startState, RULE_colorTerm);
					setState(571);
					if (!(precpred(_ctx, 4))) throw new FailedPredicateException(this, "precpred(_ctx, 4)");
					setState(572);
					((ColorTermNextPrevContext)_localctx).op = _input.LT(1);
					_la = _input.LA(1);
					if ( !(_la==POSTINCR || _la==POSTDECR) ) {
						((ColorTermNextPrevContext)_localctx).op = (Token)_errHandler.recoverInline(this);
					}
					consume();
					}
					} 
				}
				setState(577);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,34,_ctx);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			unrollRecursionContexts(_parentctx);
		}
		return _localctx;
	}

	public static class ColorSetContext extends ParserRuleContext {
		public ColorSetContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_colorSet; }
	 
		public ColorSetContext() { }
		public void copyFrom(ColorSetContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class ColorSetClass2Context extends ColorSetContext {
		public TerminalNode SIMPLECOLORCLASS_ID() { return getToken(ExprLangParser.SIMPLECOLORCLASS_ID, 0); }
		public TerminalNode GREATSPN_ALL() { return getToken(ExprLangParser.GREATSPN_ALL, 0); }
		public ColorSetClass2Context(ColorSetContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitColorSetClass2(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ColorSetSubClassContext extends ColorSetContext {
		public ColorSubclassContext colorSubclass() {
			return getRuleContext(ColorSubclassContext.class,0);
		}
		public ColorSetSubClassContext(ColorSetContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitColorSetSubClass(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ColorSetAllContext extends ColorSetContext {
		public ColorSetAllContext(ColorSetContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitColorSetAll(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ColorSetClassContext extends ColorSetContext {
		public TerminalNode SIMPLECOLORCLASS_ID() { return getToken(ExprLangParser.SIMPLECOLORCLASS_ID, 0); }
		public ColorSetClassContext(ColorSetContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitColorSetClass(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ColorSetTermContext extends ColorSetContext {
		public ColorTermContext colorTerm() {
			return getRuleContext(ColorTermContext.class,0);
		}
		public ColorSetTermContext(ColorSetContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitColorSetTerm(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ColorSetSubClass2Context extends ColorSetContext {
		public ColorSubclassContext colorSubclass() {
			return getRuleContext(ColorSubclassContext.class,0);
		}
		public TerminalNode GREATSPN_ALL() { return getToken(ExprLangParser.GREATSPN_ALL, 0); }
		public ColorSetSubClass2Context(ColorSetContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitColorSetSubClass2(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ColorSetSubclassOf2Context extends ColorSetContext {
		public ColorTermContext colorTerm() {
			return getRuleContext(ColorTermContext.class,0);
		}
		public TerminalNode GREATSPN_SUBCLASS() { return getToken(ExprLangParser.GREATSPN_SUBCLASS, 0); }
		public ColorSetSubclassOf2Context(ColorSetContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitColorSetSubclassOf2(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ColorSetSubclassOfContext extends ColorSetContext {
		public ColorTermContext colorTerm() {
			return getRuleContext(ColorTermContext.class,0);
		}
		public ColorSetSubclassOfContext(ColorSetContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitColorSetSubclassOf(this);
			else return visitor.visitChildren(this);
		}
	}

	public final ColorSetContext colorSet() throws RecognitionException {
		ColorSetContext _localctx = new ColorSetContext(_ctx, getState());
		enterRule(_localctx, 76, RULE_colorSet);
		try {
			setState(596);
			switch ( getInterpreter().adaptivePredict(_input,35,_ctx) ) {
			case 1:
				_localctx = new ColorSetTermContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(578); colorTerm(0);
				}
				break;

			case 2:
				_localctx = new ColorSetAllContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(579); match(8);
				}
				break;

			case 3:
				_localctx = new ColorSetClassContext(_localctx);
				enterOuterAlt(_localctx, 3);
				{
				setState(580); match(SIMPLECOLORCLASS_ID);
				}
				break;

			case 4:
				_localctx = new ColorSetClass2Context(_localctx);
				enterOuterAlt(_localctx, 4);
				{
				setState(581); match(GREATSPN_ALL);
				setState(582); match(SIMPLECOLORCLASS_ID);
				}
				break;

			case 5:
				_localctx = new ColorSetSubClassContext(_localctx);
				enterOuterAlt(_localctx, 5);
				{
				setState(583); colorSubclass();
				}
				break;

			case 6:
				_localctx = new ColorSetSubClass2Context(_localctx);
				enterOuterAlt(_localctx, 6);
				{
				setState(584); match(GREATSPN_ALL);
				setState(585); colorSubclass();
				}
				break;

			case 7:
				_localctx = new ColorSetSubclassOfContext(_localctx);
				enterOuterAlt(_localctx, 7);
				{
				setState(586); match(MULTISET_SUBCLASS);
				setState(587); match(OP_SQPAR);
				setState(588); colorTerm(0);
				setState(589); match(CL_SQPAR);
				}
				break;

			case 8:
				_localctx = new ColorSetSubclassOf2Context(_localctx);
				enterOuterAlt(_localctx, 8);
				{
				setState(591); match(GREATSPN_SUBCLASS);
				setState(592); match(OP_PAREN);
				setState(593); colorTerm(0);
				setState(594); match(CL_PAREN);
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class ColorClassDefContext extends ParserRuleContext {
		public ColorClassDefContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_colorClassDef; }
	 
		public ColorClassDefContext() { }
		public void copyFrom(ColorClassDefContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class ColorClassDefProductContext extends ColorClassDefContext {
		public List<TerminalNode> SIMPLECOLORCLASS_ID() { return getTokens(ExprLangParser.SIMPLECOLORCLASS_ID); }
		public TerminalNode SIMPLECOLORCLASS_ID(int i) {
			return getToken(ExprLangParser.SIMPLECOLORCLASS_ID, i);
		}
		public ColorClassDefProductContext(ColorClassDefContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitColorClassDefProduct(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ColorClassDefNamedSetContext extends ColorClassDefContext {
		public List<ColorListContext> colorList() {
			return getRuleContexts(ColorListContext.class);
		}
		public ColorClassOrdContext colorClassOrd() {
			return getRuleContext(ColorClassOrdContext.class,0);
		}
		public ColorListContext colorList(int i) {
			return getRuleContext(ColorListContext.class,i);
		}
		public ColorClassDefNamedSetContext(ColorClassDefContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitColorClassDefNamedSet(this);
			else return visitor.visitChildren(this);
		}
	}

	public final ColorClassDefContext colorClassDef() throws RecognitionException {
		ColorClassDefContext _localctx = new ColorClassDefContext(_ctx, getState());
		enterRule(_localctx, 78, RULE_colorClassDef);
		int _la;
		try {
			setState(616);
			switch ( getInterpreter().adaptivePredict(_input,39,_ctx) ) {
			case 1:
				_localctx = new ColorClassDefNamedSetContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(599);
				_la = _input.LA(1);
				if ((((_la) & ~0x3f) == 0 && ((1L << _la) & ((1L << 1) | (1L << 2) | (1L << 3) | (1L << 6))) != 0)) {
					{
					setState(598); colorClassOrd();
					}
				}

				setState(601); colorList();
				setState(606);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==ADD) {
					{
					{
					setState(602); match(ADD);
					setState(603); colorList();
					}
					}
					setState(608);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				}
				break;

			case 2:
				_localctx = new ColorClassDefProductContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(609); match(SIMPLECOLORCLASS_ID);
				setState(612); 
				_errHandler.sync(this);
				_la = _input.LA(1);
				do {
					{
					{
					setState(610); match(MUL);
					setState(611); match(SIMPLECOLORCLASS_ID);
					}
					}
					setState(614); 
					_errHandler.sync(this);
					_la = _input.LA(1);
				} while ( _la==MUL );
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class ColorClassOrdContext extends ParserRuleContext {
		public ColorClassOrdContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_colorClassOrd; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitColorClassOrd(this);
			else return visitor.visitChildren(this);
		}
	}

	public final ColorClassOrdContext colorClassOrd() throws RecognitionException {
		ColorClassOrdContext _localctx = new ColorClassOrdContext(_ctx, getState());
		enterRule(_localctx, 80, RULE_colorClassOrd);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(618);
			_la = _input.LA(1);
			if ( !((((_la) & ~0x3f) == 0 && ((1L << _la) & ((1L << 1) | (1L << 2) | (1L << 3) | (1L << 6))) != 0)) ) {
			_errHandler.recoverInline(this);
			}
			consume();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class ColorListContext extends ParserRuleContext {
		public ColorListContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_colorList; }
	 
		public ColorListContext() { }
		public void copyFrom(ColorListContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class ColorListIDsContext extends ColorListContext {
		public List<TerminalNode> ID() { return getTokens(ExprLangParser.ID); }
		public ColorSubclassNameContext colorSubclassName() {
			return getRuleContext(ColorSubclassNameContext.class,0);
		}
		public TerminalNode ID(int i) {
			return getToken(ExprLangParser.ID, i);
		}
		public ColorListIDsContext(ColorListContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitColorListIDs(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ColorListIntervalContext extends ColorListContext {
		public AnyIDContext anyID() {
			return getRuleContext(AnyIDContext.class,0);
		}
		public ColorSubclassNameContext colorSubclassName() {
			return getRuleContext(ColorSubclassNameContext.class,0);
		}
		public IntConstContext intConst(int i) {
			return getRuleContext(IntConstContext.class,i);
		}
		public List<IntConstContext> intConst() {
			return getRuleContexts(IntConstContext.class);
		}
		public ColorListIntervalContext(ColorListContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitColorListInterval(this);
			else return visitor.visitChildren(this);
		}
	}

	public final ColorListContext colorList() throws RecognitionException {
		ColorListContext _localctx = new ColorListContext(_ctx, getState());
		enterRule(_localctx, 82, RULE_colorList);
		int _la;
		try {
			setState(644);
			switch ( getInterpreter().adaptivePredict(_input,44,_ctx) ) {
			case 1:
				_localctx = new ColorListIDsContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(620); match(OP_BRACK);
				setState(621); match(ID);
				setState(626);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==COMMA) {
					{
					{
					setState(622); match(COMMA);
					setState(623); match(ID);
					}
					}
					setState(628);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				setState(629); match(CL_BRACK);
				setState(631);
				_la = _input.LA(1);
				if (_la==9) {
					{
					setState(630); colorSubclassName();
					}
				}

				}
				break;

			case 2:
				_localctx = new ColorListIntervalContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(634);
				_la = _input.LA(1);
				if (((((_la - 78)) & ~0x3f) == 0 && ((1L << (_la - 78)) & ((1L << (ID - 78)) | (1L << (INT_PLACE_ID - 78)) | (1L << (REAL_PLACE_ID - 78)) | (1L << (INT_MSET_PLACE_ID - 78)) | (1L << (REAL_MSET_PLACE_ID - 78)) | (1L << (TRANSITION_ID - 78)) | (1L << (INT_CONST_ID - 78)) | (1L << (REAL_CONST_ID - 78)) | (1L << (INT_MSET_CONST_ID - 78)) | (1L << (REAL_MSET_CONST_ID - 78)) | (1L << (INT_TEMPLATE_ID - 78)) | (1L << (REAL_TEMPLATE_ID - 78)) | (1L << (ACTION_ID - 78)) | (1L << (STATEPROP_ID - 78)) | (1L << (CLOCK_ID - 78)) | (1L << (SIMPLECOLORCLASS_ID - 78)) | (1L << (COLORDOMAIN_ID - 78)) | (1L << (COLORVAR_ID - 78)) | (1L << (COLORSUBCLASS_ID - 78)) | (1L << (COLOR_ID - 78)))) != 0)) {
					{
					setState(633); anyID();
					}
				}

				setState(636); match(OP_BRACK);
				setState(637); intConst();
				setState(638); match(TWODOTS);
				setState(639); intConst();
				setState(640); match(CL_BRACK);
				setState(642);
				_la = _input.LA(1);
				if (_la==9) {
					{
					setState(641); colorSubclassName();
					}
				}

				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class ColorSubclassNameContext extends ParserRuleContext {
		public ColorSubclassNameContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_colorSubclassName; }
	 
		public ColorSubclassNameContext() { }
		public void copyFrom(ColorSubclassNameContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class ColorSubclassNameDefContext extends ColorSubclassNameContext {
		public TerminalNode ID() { return getToken(ExprLangParser.ID, 0); }
		public ColorSubclassNameDefContext(ColorSubclassNameContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitColorSubclassNameDef(this);
			else return visitor.visitChildren(this);
		}
	}

	public final ColorSubclassNameContext colorSubclassName() throws RecognitionException {
		ColorSubclassNameContext _localctx = new ColorSubclassNameContext(_ctx, getState());
		enterRule(_localctx, 84, RULE_colorSubclassName);
		try {
			_localctx = new ColorSubclassNameDefContext(_localctx);
			enterOuterAlt(_localctx, 1);
			{
			setState(646); match(9);
			setState(647); match(ID);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class ColorVarDefContext extends ParserRuleContext {
		public TerminalNode SIMPLECOLORCLASS_ID() { return getToken(ExprLangParser.SIMPLECOLORCLASS_ID, 0); }
		public ColorVarDefContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_colorVarDef; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitColorVarDef(this);
			else return visitor.visitChildren(this);
		}
	}

	public final ColorVarDefContext colorVarDef() throws RecognitionException {
		ColorVarDefContext _localctx = new ColorVarDefContext(_ctx, getState());
		enterRule(_localctx, 86, RULE_colorVarDef);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(649); match(SIMPLECOLORCLASS_ID);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class PlaceColorDomainDefContext extends ParserRuleContext {
		public PlaceColorDomainDefContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_placeColorDomainDef; }
	 
		public PlaceColorDomainDefContext() { }
		public void copyFrom(PlaceColorDomainDefContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class PlaceDomainColorDomainContext extends PlaceColorDomainDefContext {
		public TerminalNode COLORDOMAIN_ID() { return getToken(ExprLangParser.COLORDOMAIN_ID, 0); }
		public PlaceDomainColorDomainContext(PlaceColorDomainDefContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitPlaceDomainColorDomain(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class PlaceDomainColorClassContext extends PlaceColorDomainDefContext {
		public TerminalNode SIMPLECOLORCLASS_ID() { return getToken(ExprLangParser.SIMPLECOLORCLASS_ID, 0); }
		public PlaceDomainColorClassContext(PlaceColorDomainDefContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitPlaceDomainColorClass(this);
			else return visitor.visitChildren(this);
		}
	}

	public final PlaceColorDomainDefContext placeColorDomainDef() throws RecognitionException {
		PlaceColorDomainDefContext _localctx = new PlaceColorDomainDefContext(_ctx, getState());
		enterRule(_localctx, 88, RULE_placeColorDomainDef);
		try {
			setState(653);
			switch (_input.LA(1)) {
			case SIMPLECOLORCLASS_ID:
				_localctx = new PlaceDomainColorClassContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(651); match(SIMPLECOLORCLASS_ID);
				}
				break;
			case COLORDOMAIN_ID:
				_localctx = new PlaceDomainColorDomainContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(652); match(COLORDOMAIN_ID);
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class ActListContext extends ParserRuleContext {
		public ActListContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_actList; }
	 
		public ActListContext() { }
		public void copyFrom(ActListContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class ActListEmptyContext extends ActListContext {
		public ActListEmptyContext(ActListContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitActListEmpty(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ActListListContext extends ActListContext {
		public List<TerminalNode> ACTION_ID() { return getTokens(ExprLangParser.ACTION_ID); }
		public TerminalNode ACTION_ID(int i) {
			return getToken(ExprLangParser.ACTION_ID, i);
		}
		public ActListListContext(ActListContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitActListList(this);
			else return visitor.visitChildren(this);
		}
	}

	public final ActListContext actList() throws RecognitionException {
		ActListContext _localctx = new ActListContext(_ctx, getState());
		enterRule(_localctx, 90, RULE_actList);
		int _la;
		try {
			setState(667);
			switch ( getInterpreter().adaptivePredict(_input,47,_ctx) ) {
			case 1:
				_localctx = new ActListEmptyContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(655); match(OP_BRACK);
				setState(656); match(CL_BRACK);
				}
				break;

			case 2:
				_localctx = new ActListListContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(657); match(OP_BRACK);
				setState(658); match(ACTION_ID);
				setState(663);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==COMMA) {
					{
					{
					setState(659); match(COMMA);
					setState(660); match(ACTION_ID);
					}
					}
					setState(665);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				setState(666); match(CL_BRACK);
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class ActSetContext extends ParserRuleContext {
		public ActSetContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_actSet; }
	 
		public ActSetContext() { }
		public void copyFrom(ActSetContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class ActSetListContext extends ActSetContext {
		public ActListContext actList() {
			return getRuleContext(ActListContext.class,0);
		}
		public ActSetListContext(ActSetContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitActSetList(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ActSetAllExceptListContext extends ActSetContext {
		public ActListContext actList() {
			return getRuleContext(ActListContext.class,0);
		}
		public TerminalNode ACT() { return getToken(ExprLangParser.ACT, 0); }
		public ActSetAllExceptListContext(ActSetContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitActSetAllExceptList(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ActSetAllContext extends ActSetContext {
		public TerminalNode ACT() { return getToken(ExprLangParser.ACT, 0); }
		public ActSetAllContext(ActSetContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitActSetAll(this);
			else return visitor.visitChildren(this);
		}
	}

	public final ActSetContext actSet() throws RecognitionException {
		ActSetContext _localctx = new ActSetContext(_ctx, getState());
		enterRule(_localctx, 92, RULE_actSet);
		try {
			setState(674);
			switch ( getInterpreter().adaptivePredict(_input,48,_ctx) ) {
			case 1:
				_localctx = new ActSetAllContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(669); match(ACT);
				}
				break;

			case 2:
				_localctx = new ActSetAllExceptListContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(670); match(ACT);
				setState(671); match(4);
				setState(672); actList();
				}
				break;

			case 3:
				_localctx = new ActSetListContext(_localctx);
				enterOuterAlt(_localctx, 3);
				{
				setState(673); actList();
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class ActBindingContext extends ParserRuleContext {
		public ActBindingContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_actBinding; }
	 
		public ActBindingContext() { }
		public void copyFrom(ActBindingContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class ActBindingTransitionContext extends ActBindingContext {
		public TerminalNode TRANSITION_ID() { return getToken(ExprLangParser.TRANSITION_ID, 0); }
		public ActBindingTransitionContext(ActBindingContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitActBindingTransition(this);
			else return visitor.visitChildren(this);
		}
	}

	public final ActBindingContext actBinding() throws RecognitionException {
		ActBindingContext _localctx = new ActBindingContext(_ctx, getState());
		enterRule(_localctx, 94, RULE_actBinding);
		try {
			_localctx = new ActBindingTransitionContext(_localctx);
			enterOuterAlt(_localctx, 1);
			{
			setState(676); match(TRANSITION_ID);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class ClockVarContext extends ParserRuleContext {
		public ClockVarContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_clockVar; }
	 
		public ClockVarContext() { }
		public void copyFrom(ClockVarContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class ClockVarIdContext extends ClockVarContext {
		public TerminalNode CLOCK_ID() { return getToken(ExprLangParser.CLOCK_ID, 0); }
		public ClockVarIdContext(ClockVarContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitClockVarId(this);
			else return visitor.visitChildren(this);
		}
	}

	public final ClockVarContext clockVar() throws RecognitionException {
		ClockVarContext _localctx = new ClockVarContext(_ctx, getState());
		enterRule(_localctx, 96, RULE_clockVar);
		try {
			_localctx = new ClockVarIdContext(_localctx);
			enterOuterAlt(_localctx, 1);
			{
			setState(678); match(CLOCK_ID);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class ClockConstrContext extends ParserRuleContext {
		public ClockConstrContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_clockConstr; }
	 
		public ClockConstrContext() { }
		public void copyFrom(ClockConstrContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class ClockConstrAndContext extends ClockConstrContext {
		public List<ClockConstrContext> clockConstr() {
			return getRuleContexts(ClockConstrContext.class);
		}
		public ClockConstrContext clockConstr(int i) {
			return getRuleContext(ClockConstrContext.class,i);
		}
		public ClockConstrAndContext(ClockConstrContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitClockConstrAnd(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ClockConstrBetweenContext extends ClockConstrContext {
		public List<RealExprContext> realExpr() {
			return getRuleContexts(RealExprContext.class);
		}
		public ClockVarContext clockVar() {
			return getRuleContext(ClockVarContext.class,0);
		}
		public RealExprContext realExpr(int i) {
			return getRuleContext(RealExprContext.class,i);
		}
		public ClockConstrBetweenContext(ClockConstrContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitClockConstrBetween(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ClockConstrIdCmpRealContext extends ClockConstrContext {
		public Token op;
		public RealExprContext realExpr() {
			return getRuleContext(RealExprContext.class,0);
		}
		public ClockVarContext clockVar() {
			return getRuleContext(ClockVarContext.class,0);
		}
		public ClockConstrIdCmpRealContext(ClockConstrContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitClockConstrIdCmpReal(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ClockConstrEqualsContext extends ClockConstrContext {
		public RealExprContext realExpr() {
			return getRuleContext(RealExprContext.class,0);
		}
		public ClockVarContext clockVar() {
			return getRuleContext(ClockVarContext.class,0);
		}
		public ClockConstrEqualsContext(ClockConstrContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitClockConstrEquals(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ClockConstrRealCmpIdContext extends ClockConstrContext {
		public Token op;
		public RealExprContext realExpr() {
			return getRuleContext(RealExprContext.class,0);
		}
		public ClockVarContext clockVar() {
			return getRuleContext(ClockVarContext.class,0);
		}
		public ClockConstrRealCmpIdContext(ClockConstrContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitClockConstrRealCmpId(this);
			else return visitor.visitChildren(this);
		}
	}

	public final ClockConstrContext clockConstr() throws RecognitionException {
		return clockConstr(0);
	}

	private ClockConstrContext clockConstr(int _p) throws RecognitionException {
		ParserRuleContext _parentctx = _ctx;
		int _parentState = getState();
		ClockConstrContext _localctx = new ClockConstrContext(_ctx, _parentState);
		ClockConstrContext _prevctx = _localctx;
		int _startState = 98;
		enterRecursionRule(_localctx, 98, RULE_clockConstr, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(699);
			switch ( getInterpreter().adaptivePredict(_input,49,_ctx) ) {
			case 1:
				{
				_localctx = new ClockConstrBetweenContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(681); realExpr(0);
				setState(682); match(LESS);
				setState(683); clockVar();
				setState(684); match(LESS);
				setState(685); realExpr(0);
				}
				break;

			case 2:
				{
				_localctx = new ClockConstrIdCmpRealContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(687); clockVar();
				setState(688);
				((ClockConstrIdCmpRealContext)_localctx).op = _input.LT(1);
				_la = _input.LA(1);
				if ( !(_la==LESS || _la==GREATER) ) {
					((ClockConstrIdCmpRealContext)_localctx).op = (Token)_errHandler.recoverInline(this);
				}
				consume();
				setState(689); realExpr(0);
				}
				break;

			case 3:
				{
				_localctx = new ClockConstrRealCmpIdContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(691); realExpr(0);
				setState(692);
				((ClockConstrRealCmpIdContext)_localctx).op = _input.LT(1);
				_la = _input.LA(1);
				if ( !(_la==LESS || _la==GREATER) ) {
					((ClockConstrRealCmpIdContext)_localctx).op = (Token)_errHandler.recoverInline(this);
				}
				consume();
				setState(693); clockVar();
				}
				break;

			case 4:
				{
				_localctx = new ClockConstrEqualsContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(695); clockVar();
				setState(696); match(5);
				setState(697); realExpr(0);
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(706);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,50,_ctx);
			while ( _alt!=2 && _alt!=-1 ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					{
					_localctx = new ClockConstrAndContext(new ClockConstrContext(_parentctx, _parentState));
					pushNewRecursionContext(_localctx, _startState, RULE_clockConstr);
					setState(701);
					if (!(precpred(_ctx, 1))) throw new FailedPredicateException(this, "precpred(_ctx, 1)");
					setState(702); match(AND);
					setState(703); clockConstr(2);
					}
					} 
				}
				setState(708);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,50,_ctx);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			unrollRecursionContexts(_parentctx);
		}
		return _localctx;
	}

	public static class VarUpdateContext extends ParserRuleContext {
		public VarUpdateContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_varUpdate; }
	 
		public VarUpdateContext() { }
		public void copyFrom(VarUpdateContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class VarUpdateValueContext extends VarUpdateContext {
		public RealExprContext realExpr() {
			return getRuleContext(RealExprContext.class,0);
		}
		public ClockVarContext clockVar() {
			return getRuleContext(ClockVarContext.class,0);
		}
		public VarUpdateValueContext(VarUpdateContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitVarUpdateValue(this);
			else return visitor.visitChildren(this);
		}
	}

	public final VarUpdateContext varUpdate() throws RecognitionException {
		VarUpdateContext _localctx = new VarUpdateContext(_ctx, getState());
		enterRule(_localctx, 100, RULE_varUpdate);
		try {
			_localctx = new VarUpdateValueContext(_localctx);
			enterOuterAlt(_localctx, 1);
			{
			setState(709); clockVar();
			setState(710); match(5);
			setState(711); realExpr(0);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class VarUpdateListContext extends ParserRuleContext {
		public VarUpdateListContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_varUpdateList; }
	 
		public VarUpdateListContext() { }
		public void copyFrom(VarUpdateListContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class VarUpdateListEmptyContext extends VarUpdateListContext {
		public VarUpdateListEmptyContext(VarUpdateListContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitVarUpdateListEmpty(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class VarUpdateListListContext extends VarUpdateListContext {
		public VarUpdateContext varUpdate(int i) {
			return getRuleContext(VarUpdateContext.class,i);
		}
		public List<VarUpdateContext> varUpdate() {
			return getRuleContexts(VarUpdateContext.class);
		}
		public VarUpdateListListContext(VarUpdateListContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitVarUpdateListList(this);
			else return visitor.visitChildren(this);
		}
	}

	public final VarUpdateListContext varUpdateList() throws RecognitionException {
		VarUpdateListContext _localctx = new VarUpdateListContext(_ctx, getState());
		enterRule(_localctx, 102, RULE_varUpdateList);
		int _la;
		try {
			setState(722);
			switch (_input.LA(1)) {
			case EOF:
				_localctx = new VarUpdateListEmptyContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				}
				break;
			case CLOCK_ID:
				_localctx = new VarUpdateListListContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(714); varUpdate();
				setState(719);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==COMMA) {
					{
					{
					setState(715); match(COMMA);
					setState(716); varUpdate();
					}
					}
					setState(721);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class VarFlowListContext extends ParserRuleContext {
		public VarFlowListContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_varFlowList; }
	 
		public VarFlowListContext() { }
		public void copyFrom(VarFlowListContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class VarFlowListListContext extends VarFlowListContext {
		public List<VarFlowContext> varFlow() {
			return getRuleContexts(VarFlowContext.class);
		}
		public VarFlowContext varFlow(int i) {
			return getRuleContext(VarFlowContext.class,i);
		}
		public VarFlowListListContext(VarFlowListContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitVarFlowListList(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class VarFlowListEmptyContext extends VarFlowListContext {
		public VarFlowListEmptyContext(VarFlowListContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitVarFlowListEmpty(this);
			else return visitor.visitChildren(this);
		}
	}

	public final VarFlowListContext varFlowList() throws RecognitionException {
		VarFlowListContext _localctx = new VarFlowListContext(_ctx, getState());
		enterRule(_localctx, 104, RULE_varFlowList);
		int _la;
		try {
			setState(733);
			switch (_input.LA(1)) {
			case EOF:
				_localctx = new VarFlowListEmptyContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				}
				break;
			case CLOCK_ID:
				_localctx = new VarFlowListListContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(725); varFlow();
				setState(730);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==COMMA) {
					{
					{
					setState(726); match(COMMA);
					setState(727); varFlow();
					}
					}
					setState(732);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class VarFlowContext extends ParserRuleContext {
		public VarFlowContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_varFlow; }
	 
		public VarFlowContext() { }
		public void copyFrom(VarFlowContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class VarFlowDefinitionContext extends VarFlowContext {
		public RealExprContext realExpr() {
			return getRuleContext(RealExprContext.class,0);
		}
		public ClockVarContext clockVar() {
			return getRuleContext(ClockVarContext.class,0);
		}
		public VarFlowDefinitionContext(VarFlowContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitVarFlowDefinition(this);
			else return visitor.visitChildren(this);
		}
	}

	public final VarFlowContext varFlow() throws RecognitionException {
		VarFlowContext _localctx = new VarFlowContext(_ctx, getState());
		enterRule(_localctx, 106, RULE_varFlow);
		try {
			_localctx = new VarFlowDefinitionContext(_localctx);
			enterOuterAlt(_localctx, 1);
			{
			setState(735); clockVar();
			setState(736); match(5);
			setState(737); realExpr(0);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class MeasureContext extends ParserRuleContext {
		public MeasureContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_measure; }
	 
		public MeasureContext() { }
		public void copyFrom(MeasureContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class MeasurePContext extends MeasureContext {
		public BoolExprContext boolExpr() {
			return getRuleContext(BoolExprContext.class,0);
		}
		public RealConstContext realConst() {
			return getRuleContext(RealConstContext.class,0);
		}
		public MeasurePContext(MeasureContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMeasureP(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class MeasureEContext extends MeasureContext {
		public BoolExprContext boolExpr() {
			return getRuleContext(BoolExprContext.class,0);
		}
		public IntExprContext intExpr() {
			return getRuleContext(IntExprContext.class,0);
		}
		public TerminalNode DIV() { return getToken(ExprLangParser.DIV, 0); }
		public RealConstContext realConst() {
			return getRuleContext(RealConstContext.class,0);
		}
		public MeasureEContext(MeasureContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMeasureE(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class MeasureXContext extends MeasureContext {
		public TerminalNode TRANSITION_ID() { return getToken(ExprLangParser.TRANSITION_ID, 0); }
		public BoolExprContext boolExpr() {
			return getRuleContext(BoolExprContext.class,0);
		}
		public TerminalNode DIV() { return getToken(ExprLangParser.DIV, 0); }
		public RealConstContext realConst() {
			return getRuleContext(RealConstContext.class,0);
		}
		public MeasureXContext(MeasureContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMeasureX(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class MeasureAddSubContext extends MeasureContext {
		public Token op;
		public MeasureContext measure(int i) {
			return getRuleContext(MeasureContext.class,i);
		}
		public List<MeasureContext> measure() {
			return getRuleContexts(MeasureContext.class);
		}
		public MeasureAddSubContext(MeasureContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitMeasureAddSub(this);
			else return visitor.visitChildren(this);
		}
	}

	public final MeasureContext measure() throws RecognitionException {
		return measure(0);
	}

	private MeasureContext measure(int _p) throws RecognitionException {
		ParserRuleContext _parentctx = _ctx;
		int _parentState = getState();
		MeasureContext _localctx = new MeasureContext(_ctx, _parentState);
		MeasureContext _prevctx = _localctx;
		int _startState = 108;
		enterRecursionRule(_localctx, 108, RULE_measure, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(768);
			switch ( getInterpreter().adaptivePredict(_input,60,_ctx) ) {
			case 1:
				{
				_localctx = new MeasurePContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(741);
				_la = _input.LA(1);
				if (((((_la - 79)) & ~0x3f) == 0 && ((1L << (_la - 79)) & ((1L << (INT - 79)) | (1L << (REAL - 79)) | (1L << (INT_CONST_ID - 79)) | (1L << (REAL_CONST_ID - 79)) | (1L << (INT_TEMPLATE_ID - 79)) | (1L << (REAL_TEMPLATE_ID - 79)))) != 0)) {
					{
					setState(740); realConst();
					}
				}

				setState(743); match(15);
				setState(744); boolExpr(0);
				setState(745); match(CL_BRACK);
				}
				break;

			case 2:
				{
				_localctx = new MeasureEContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(748);
				_la = _input.LA(1);
				if (((((_la - 79)) & ~0x3f) == 0 && ((1L << (_la - 79)) & ((1L << (INT - 79)) | (1L << (REAL - 79)) | (1L << (INT_CONST_ID - 79)) | (1L << (REAL_CONST_ID - 79)) | (1L << (INT_TEMPLATE_ID - 79)) | (1L << (REAL_TEMPLATE_ID - 79)))) != 0)) {
					{
					setState(747); realConst();
					}
				}

				setState(750); match(10);
				setState(751); intExpr(0);
				setState(754);
				_la = _input.LA(1);
				if (_la==DIV) {
					{
					setState(752); match(DIV);
					setState(753); boolExpr(0);
					}
				}

				setState(756); match(CL_BRACK);
				}
				break;

			case 3:
				{
				_localctx = new MeasureXContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(759);
				_la = _input.LA(1);
				if (((((_la - 79)) & ~0x3f) == 0 && ((1L << (_la - 79)) & ((1L << (INT - 79)) | (1L << (REAL - 79)) | (1L << (INT_CONST_ID - 79)) | (1L << (REAL_CONST_ID - 79)) | (1L << (INT_TEMPLATE_ID - 79)) | (1L << (REAL_TEMPLATE_ID - 79)))) != 0)) {
					{
					setState(758); realConst();
					}
				}

				setState(761); match(13);
				setState(762); match(TRANSITION_ID);
				setState(765);
				_la = _input.LA(1);
				if (_la==DIV) {
					{
					setState(763); match(DIV);
					setState(764); boolExpr(0);
					}
				}

				setState(767); match(CL_BRACK);
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(775);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,61,_ctx);
			while ( _alt!=2 && _alt!=-1 ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					{
					_localctx = new MeasureAddSubContext(new MeasureContext(_parentctx, _parentState));
					pushNewRecursionContext(_localctx, _startState, RULE_measure);
					setState(770);
					if (!(precpred(_ctx, 4))) throw new FailedPredicateException(this, "precpred(_ctx, 4)");
					setState(771);
					((MeasureAddSubContext)_localctx).op = _input.LT(1);
					_la = _input.LA(1);
					if ( !(_la==ADD || _la==SUB) ) {
						((MeasureAddSubContext)_localctx).op = (Token)_errHandler.recoverInline(this);
					}
					consume();
					setState(772); measure(5);
					}
					} 
				}
				setState(777);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,61,_ctx);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			unrollRecursionContexts(_parentctx);
		}
		return _localctx;
	}

	public static class CsltaExprContext extends ParserRuleContext {
		public CsltaExprContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_csltaExpr; }
	 
		public CsltaExprContext() { }
		public void copyFrom(CsltaExprContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class CsltaExprPlaceContext extends CsltaExprContext {
		public TerminalNode INT_PLACE_ID() { return getToken(ExprLangParser.INT_PLACE_ID, 0); }
		public CsltaExprPlaceContext(CsltaExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitCsltaExprPlace(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class CsltaExprBoolContext extends CsltaExprContext {
		public BoolExprContext boolExpr() {
			return getRuleContext(BoolExprContext.class,0);
		}
		public CsltaExprBoolContext(CsltaExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitCsltaExprBool(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class CsltaExprProbTAContext extends CsltaExprContext {
		public Token op;
		public RealConstContext q;
		public Token dtaName;
		public List<Sp_assignContext> sp_assign() {
			return getRuleContexts(Sp_assignContext.class);
		}
		public TerminalNode ID() { return getToken(ExprLangParser.ID, 0); }
		public List<Real_assignContext> real_assign() {
			return getRuleContexts(Real_assignContext.class);
		}
		public Real_assignContext real_assign(int i) {
			return getRuleContext(Real_assignContext.class,i);
		}
		public List<Act_assignContext> act_assign() {
			return getRuleContexts(Act_assignContext.class);
		}
		public TerminalNode PROB_TA() { return getToken(ExprLangParser.PROB_TA, 0); }
		public Sp_assignContext sp_assign(int i) {
			return getRuleContext(Sp_assignContext.class,i);
		}
		public RealConstContext realConst() {
			return getRuleContext(RealConstContext.class,0);
		}
		public Act_assignContext act_assign(int i) {
			return getRuleContext(Act_assignContext.class,i);
		}
		public CsltaExprProbTAContext(CsltaExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitCsltaExprProbTA(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class CsltaExprAndContext extends CsltaExprContext {
		public CsltaExprContext csltaExpr(int i) {
			return getRuleContext(CsltaExprContext.class,i);
		}
		public TerminalNode AND() { return getToken(ExprLangParser.AND, 0); }
		public List<CsltaExprContext> csltaExpr() {
			return getRuleContexts(CsltaExprContext.class);
		}
		public CsltaExprAndContext(CsltaExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitCsltaExprAnd(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class CsltaExprNotContext extends CsltaExprContext {
		public TerminalNode NOT() { return getToken(ExprLangParser.NOT, 0); }
		public CsltaExprContext csltaExpr() {
			return getRuleContext(CsltaExprContext.class,0);
		}
		public CsltaExprNotContext(CsltaExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitCsltaExprNot(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class CsltaExprParenContext extends CsltaExprContext {
		public CsltaExprContext csltaExpr() {
			return getRuleContext(CsltaExprContext.class,0);
		}
		public CsltaExprParenContext(CsltaExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitCsltaExprParen(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class CsltaExprOrContext extends CsltaExprContext {
		public CsltaExprContext csltaExpr(int i) {
			return getRuleContext(CsltaExprContext.class,i);
		}
		public TerminalNode OR() { return getToken(ExprLangParser.OR, 0); }
		public List<CsltaExprContext> csltaExpr() {
			return getRuleContexts(CsltaExprContext.class);
		}
		public CsltaExprOrContext(CsltaExprContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitCsltaExprOr(this);
			else return visitor.visitChildren(this);
		}
	}

	public final CsltaExprContext csltaExpr() throws RecognitionException {
		return csltaExpr(0);
	}

	private CsltaExprContext csltaExpr(int _p) throws RecognitionException {
		ParserRuleContext _parentctx = _ctx;
		int _parentState = getState();
		CsltaExprContext _localctx = new CsltaExprContext(_ctx, _parentState);
		CsltaExprContext _prevctx = _localctx;
		int _startState = 110;
		enterRecursionRule(_localctx, 110, RULE_csltaExpr, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(826);
			switch ( getInterpreter().adaptivePredict(_input,68,_ctx) ) {
			case 1:
				{
				_localctx = new CsltaExprNotContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(779); match(NOT);
				setState(780); csltaExpr(4);
				}
				break;

			case 2:
				{
				_localctx = new CsltaExprBoolContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(781); boolExpr(0);
				}
				break;

			case 3:
				{
				_localctx = new CsltaExprPlaceContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(782); match(INT_PLACE_ID);
				}
				break;

			case 4:
				{
				_localctx = new CsltaExprParenContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(783); match(OP_PAREN);
				setState(784); csltaExpr(0);
				setState(785); match(CL_PAREN);
				}
				break;

			case 5:
				{
				_localctx = new CsltaExprProbTAContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(787); match(PROB_TA);
				setState(788);
				((CsltaExprProbTAContext)_localctx).op = _input.LT(1);
				_la = _input.LA(1);
				if ( !((((_la) & ~0x3f) == 0 && ((1L << _la) & ((1L << EQUAL) | (1L << NOT_EQUAL) | (1L << LESS) | (1L << GREATER) | (1L << LESS_EQ) | (1L << GREATER_EQ))) != 0)) ) {
					((CsltaExprProbTAContext)_localctx).op = (Token)_errHandler.recoverInline(this);
				}
				consume();
				setState(789); ((CsltaExprProbTAContext)_localctx).q = realConst();
				setState(790); ((CsltaExprProbTAContext)_localctx).dtaName = match(ID);
				setState(791); match(OP_PAREN);
				setState(800);
				_la = _input.LA(1);
				if (((((_la - 78)) & ~0x3f) == 0 && ((1L << (_la - 78)) & ((1L << (ID - 78)) | (1L << (REAL_CONST_ID - 78)) | (1L << (REAL_TEMPLATE_ID - 78)))) != 0)) {
					{
					setState(792); real_assign();
					setState(797);
					_errHandler.sync(this);
					_la = _input.LA(1);
					while (_la==COMMA) {
						{
						{
						setState(793); match(COMMA);
						setState(794); real_assign();
						}
						}
						setState(799);
						_errHandler.sync(this);
						_la = _input.LA(1);
					}
					}
				}

				setState(802); match(11);
				setState(811);
				_la = _input.LA(1);
				if (((((_la - 78)) & ~0x3f) == 0 && ((1L << (_la - 78)) & ((1L << (ID - 78)) | (1L << (TRANSITION_ID - 78)) | (1L << (ACTION_ID - 78)))) != 0)) {
					{
					setState(803); act_assign();
					setState(808);
					_errHandler.sync(this);
					_la = _input.LA(1);
					while (_la==COMMA) {
						{
						{
						setState(804); match(COMMA);
						setState(805); act_assign();
						}
						}
						setState(810);
						_errHandler.sync(this);
						_la = _input.LA(1);
					}
					}
				}

				setState(813); match(11);
				setState(822);
				_la = _input.LA(1);
				if (_la==ID || _la==STATEPROP_ID) {
					{
					setState(814); sp_assign();
					setState(819);
					_errHandler.sync(this);
					_la = _input.LA(1);
					while (_la==COMMA) {
						{
						{
						setState(815); match(COMMA);
						setState(816); sp_assign();
						}
						}
						setState(821);
						_errHandler.sync(this);
						_la = _input.LA(1);
					}
					}
				}

				setState(824); match(CL_PAREN);
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(836);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,70,_ctx);
			while ( _alt!=2 && _alt!=-1 ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					setState(834);
					switch ( getInterpreter().adaptivePredict(_input,69,_ctx) ) {
					case 1:
						{
						_localctx = new CsltaExprAndContext(new CsltaExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_csltaExpr);
						setState(828);
						if (!(precpred(_ctx, 3))) throw new FailedPredicateException(this, "precpred(_ctx, 3)");
						setState(829); match(AND);
						setState(830); csltaExpr(4);
						}
						break;

					case 2:
						{
						_localctx = new CsltaExprOrContext(new CsltaExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_csltaExpr);
						setState(831);
						if (!(precpred(_ctx, 2))) throw new FailedPredicateException(this, "precpred(_ctx, 2)");
						setState(832); match(OR);
						setState(833); csltaExpr(3);
						}
						break;
					}
					} 
				}
				setState(838);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,70,_ctx);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			unrollRecursionContexts(_parentctx);
		}
		return _localctx;
	}

	public static class Real_assignContext extends ParserRuleContext {
		public Token label;
		public TerminalNode ID() { return getToken(ExprLangParser.ID, 0); }
		public TerminalNode REAL_TEMPLATE_ID() { return getToken(ExprLangParser.REAL_TEMPLATE_ID, 0); }
		public TerminalNode REAL_CONST_ID() { return getToken(ExprLangParser.REAL_CONST_ID, 0); }
		public RealConstContext realConst() {
			return getRuleContext(RealConstContext.class,0);
		}
		public Real_assignContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_real_assign; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitReal_assign(this);
			else return visitor.visitChildren(this);
		}
	}

	public final Real_assignContext real_assign() throws RecognitionException {
		Real_assignContext _localctx = new Real_assignContext(_ctx, getState());
		enterRule(_localctx, 112, RULE_real_assign);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(839);
			((Real_assignContext)_localctx).label = _input.LT(1);
			_la = _input.LA(1);
			if ( !(((((_la - 78)) & ~0x3f) == 0 && ((1L << (_la - 78)) & ((1L << (ID - 78)) | (1L << (REAL_CONST_ID - 78)) | (1L << (REAL_TEMPLATE_ID - 78)))) != 0)) ) {
				((Real_assignContext)_localctx).label = (Token)_errHandler.recoverInline(this);
			}
			consume();
			setState(840); match(5);
			setState(841); realConst();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class Act_assignContext extends ParserRuleContext {
		public Token label;
		public Token trn;
		public List<TerminalNode> TRANSITION_ID() { return getTokens(ExprLangParser.TRANSITION_ID); }
		public TerminalNode ID() { return getToken(ExprLangParser.ID, 0); }
		public TerminalNode ACTION_ID() { return getToken(ExprLangParser.ACTION_ID, 0); }
		public TerminalNode TRANSITION_ID(int i) {
			return getToken(ExprLangParser.TRANSITION_ID, i);
		}
		public Act_assignContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_act_assign; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitAct_assign(this);
			else return visitor.visitChildren(this);
		}
	}

	public final Act_assignContext act_assign() throws RecognitionException {
		Act_assignContext _localctx = new Act_assignContext(_ctx, getState());
		enterRule(_localctx, 114, RULE_act_assign);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(843);
			((Act_assignContext)_localctx).label = _input.LT(1);
			_la = _input.LA(1);
			if ( !(((((_la - 78)) & ~0x3f) == 0 && ((1L << (_la - 78)) & ((1L << (ID - 78)) | (1L << (TRANSITION_ID - 78)) | (1L << (ACTION_ID - 78)))) != 0)) ) {
				((Act_assignContext)_localctx).label = (Token)_errHandler.recoverInline(this);
			}
			consume();
			setState(844); match(5);
			setState(845); ((Act_assignContext)_localctx).trn = match(TRANSITION_ID);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class Sp_assignContext extends ParserRuleContext {
		public Token label;
		public TerminalNode ID() { return getToken(ExprLangParser.ID, 0); }
		public TerminalNode STATEPROP_ID() { return getToken(ExprLangParser.STATEPROP_ID, 0); }
		public CsltaExprContext csltaExpr() {
			return getRuleContext(CsltaExprContext.class,0);
		}
		public Sp_assignContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_sp_assign; }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitSp_assign(this);
			else return visitor.visitChildren(this);
		}
	}

	public final Sp_assignContext sp_assign() throws RecognitionException {
		Sp_assignContext _localctx = new Sp_assignContext(_ctx, getState());
		enterRule(_localctx, 116, RULE_sp_assign);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(847);
			((Sp_assignContext)_localctx).label = _input.LT(1);
			_la = _input.LA(1);
			if ( !(_la==ID || _la==STATEPROP_ID) ) {
				((Sp_assignContext)_localctx).label = (Token)_errHandler.recoverInline(this);
			}
			consume();
			setState(848); match(5);
			setState(849); csltaExpr(0);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class Temporal_opContext extends ParserRuleContext {
		public Temporal_opContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_temporal_op; }
	 
		public Temporal_opContext() { }
		public void copyFrom(Temporal_opContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class TemporalOp2TContext extends Temporal_opContext {
		public TerminalNode EXISTS() { return getToken(ExprLangParser.EXISTS, 0); }
		public TerminalNode NEXT() { return getToken(ExprLangParser.NEXT, 0); }
		public TerminalNode FUTURE() { return getToken(ExprLangParser.FUTURE, 0); }
		public TerminalNode GLOBALLY() { return getToken(ExprLangParser.GLOBALLY, 0); }
		public TerminalNode FORALL() { return getToken(ExprLangParser.FORALL, 0); }
		public TemporalOp2TContext(Temporal_opContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitTemporalOp2T(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class TemporalOpExistXContext extends Temporal_opContext {
		public TerminalNode EXISTS_NEXT() { return getToken(ExprLangParser.EXISTS_NEXT, 0); }
		public TerminalNode EXISTS_FUTURE() { return getToken(ExprLangParser.EXISTS_FUTURE, 0); }
		public TerminalNode EXISTS_GLOBALLY() { return getToken(ExprLangParser.EXISTS_GLOBALLY, 0); }
		public TemporalOpExistXContext(Temporal_opContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitTemporalOpExistX(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class TemporalOpForallXContext extends Temporal_opContext {
		public TerminalNode FORALL_FUTURE() { return getToken(ExprLangParser.FORALL_FUTURE, 0); }
		public TerminalNode FORALL_NEXT() { return getToken(ExprLangParser.FORALL_NEXT, 0); }
		public TerminalNode FORALL_GLOBALLY() { return getToken(ExprLangParser.FORALL_GLOBALLY, 0); }
		public TemporalOpForallXContext(Temporal_opContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitTemporalOpForallX(this);
			else return visitor.visitChildren(this);
		}
	}

	public final Temporal_opContext temporal_op() throws RecognitionException {
		Temporal_opContext _localctx = new Temporal_opContext(_ctx, getState());
		enterRule(_localctx, 118, RULE_temporal_op);
		int _la;
		try {
			setState(855);
			switch (_input.LA(1)) {
			case EXISTS:
			case FORALL:
				_localctx = new TemporalOp2TContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(851);
				_la = _input.LA(1);
				if ( !(_la==EXISTS || _la==FORALL) ) {
				_errHandler.recoverInline(this);
				}
				consume();
				setState(852);
				_la = _input.LA(1);
				if ( !(((((_la - 112)) & ~0x3f) == 0 && ((1L << (_la - 112)) & ((1L << (NEXT - 112)) | (1L << (FUTURE - 112)) | (1L << (GLOBALLY - 112)))) != 0)) ) {
				_errHandler.recoverInline(this);
				}
				consume();
				}
				break;
			case EXISTS_NEXT:
			case EXISTS_FUTURE:
			case EXISTS_GLOBALLY:
				_localctx = new TemporalOpExistXContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(853);
				_la = _input.LA(1);
				if ( !(((((_la - 116)) & ~0x3f) == 0 && ((1L << (_la - 116)) & ((1L << (EXISTS_NEXT - 116)) | (1L << (EXISTS_FUTURE - 116)) | (1L << (EXISTS_GLOBALLY - 116)))) != 0)) ) {
				_errHandler.recoverInline(this);
				}
				consume();
				}
				break;
			case FORALL_NEXT:
			case FORALL_FUTURE:
			case FORALL_GLOBALLY:
				_localctx = new TemporalOpForallXContext(_localctx);
				enterOuterAlt(_localctx, 3);
				{
				setState(854);
				_la = _input.LA(1);
				if ( !(((((_la - 119)) & ~0x3f) == 0 && ((1L << (_la - 119)) & ((1L << (FORALL_NEXT - 119)) | (1L << (FORALL_FUTURE - 119)) | (1L << (FORALL_GLOBALLY - 119)))) != 0)) ) {
				_errHandler.recoverInline(this);
				}
				consume();
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class Composite_temporal_op_ctlstarContext extends ParserRuleContext {
		public Composite_temporal_op_ctlstarContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_composite_temporal_op_ctlstar; }
	 
		public Composite_temporal_op_ctlstarContext() { }
		public void copyFrom(Composite_temporal_op_ctlstarContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class ComposTempOpEXContext extends Composite_temporal_op_ctlstarContext {
		public TerminalNode CTLSTAR_EXISTS_NEXT() { return getToken(ExprLangParser.CTLSTAR_EXISTS_NEXT, 0); }
		public ComposTempOpEXContext(Composite_temporal_op_ctlstarContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitComposTempOpEX(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ComposTempOpEGContext extends Composite_temporal_op_ctlstarContext {
		public TerminalNode CTLSTAR_EXISTS_GLOBALLY() { return getToken(ExprLangParser.CTLSTAR_EXISTS_GLOBALLY, 0); }
		public ComposTempOpEGContext(Composite_temporal_op_ctlstarContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitComposTempOpEG(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ComposTempOpFContext extends Composite_temporal_op_ctlstarContext {
		public TerminalNode CTLSTAR_FORALL_FUTURE() { return getToken(ExprLangParser.CTLSTAR_FORALL_FUTURE, 0); }
		public ComposTempOpFContext(Composite_temporal_op_ctlstarContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitComposTempOpF(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ComposTempOpEFContext extends Composite_temporal_op_ctlstarContext {
		public TerminalNode CTLSTAR_EXISTS_FUTURE() { return getToken(ExprLangParser.CTLSTAR_EXISTS_FUTURE, 0); }
		public ComposTempOpEFContext(Composite_temporal_op_ctlstarContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitComposTempOpEF(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ComposTempOpAXContext extends Composite_temporal_op_ctlstarContext {
		public TerminalNode CTLSTAR_FORALL_NEXT() { return getToken(ExprLangParser.CTLSTAR_FORALL_NEXT, 0); }
		public ComposTempOpAXContext(Composite_temporal_op_ctlstarContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitComposTempOpAX(this);
			else return visitor.visitChildren(this);
		}
	}
	public static class ComposTempOpAGContext extends Composite_temporal_op_ctlstarContext {
		public TerminalNode CTLSTAR_FORALL_GLOBALLY() { return getToken(ExprLangParser.CTLSTAR_FORALL_GLOBALLY, 0); }
		public ComposTempOpAGContext(Composite_temporal_op_ctlstarContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitComposTempOpAG(this);
			else return visitor.visitChildren(this);
		}
	}

	public final Composite_temporal_op_ctlstarContext composite_temporal_op_ctlstar() throws RecognitionException {
		Composite_temporal_op_ctlstarContext _localctx = new Composite_temporal_op_ctlstarContext(_ctx, getState());
		enterRule(_localctx, 120, RULE_composite_temporal_op_ctlstar);
		try {
			setState(863);
			switch (_input.LA(1)) {
			case CTLSTAR_EXISTS_NEXT:
				_localctx = new ComposTempOpEXContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(857); match(CTLSTAR_EXISTS_NEXT);
				}
				break;
			case CTLSTAR_EXISTS_FUTURE:
				_localctx = new ComposTempOpEFContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(858); match(CTLSTAR_EXISTS_FUTURE);
				}
				break;
			case CTLSTAR_EXISTS_GLOBALLY:
				_localctx = new ComposTempOpEGContext(_localctx);
				enterOuterAlt(_localctx, 3);
				{
				setState(859); match(CTLSTAR_EXISTS_GLOBALLY);
				}
				break;
			case CTLSTAR_FORALL_NEXT:
				_localctx = new ComposTempOpAXContext(_localctx);
				enterOuterAlt(_localctx, 4);
				{
				setState(860); match(CTLSTAR_FORALL_NEXT);
				}
				break;
			case CTLSTAR_FORALL_FUTURE:
				_localctx = new ComposTempOpFContext(_localctx);
				enterOuterAlt(_localctx, 5);
				{
				setState(861); match(CTLSTAR_FORALL_FUTURE);
				}
				break;
			case CTLSTAR_FORALL_GLOBALLY:
				_localctx = new ComposTempOpAGContext(_localctx);
				enterOuterAlt(_localctx, 6);
				{
				setState(862); match(CTLSTAR_FORALL_GLOBALLY);
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static class AnyIDContext extends ParserRuleContext {
		public AnyIDContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_anyID; }
	 
		public AnyIDContext() { }
		public void copyFrom(AnyIDContext ctx) {
			super.copyFrom(ctx);
		}
	}
	public static class AnyIdentifierContext extends AnyIDContext {
		public TerminalNode SIMPLECOLORCLASS_ID() { return getToken(ExprLangParser.SIMPLECOLORCLASS_ID, 0); }
		public TerminalNode REAL_MSET_CONST_ID() { return getToken(ExprLangParser.REAL_MSET_CONST_ID, 0); }
		public TerminalNode COLOR_ID() { return getToken(ExprLangParser.COLOR_ID, 0); }
		public TerminalNode COLORSUBCLASS_ID() { return getToken(ExprLangParser.COLORSUBCLASS_ID, 0); }
		public TerminalNode STATEPROP_ID() { return getToken(ExprLangParser.STATEPROP_ID, 0); }
		public TerminalNode INT_MSET_CONST_ID() { return getToken(ExprLangParser.INT_MSET_CONST_ID, 0); }
		public TerminalNode REAL_CONST_ID() { return getToken(ExprLangParser.REAL_CONST_ID, 0); }
		public TerminalNode INT_CONST_ID() { return getToken(ExprLangParser.INT_CONST_ID, 0); }
		public TerminalNode TRANSITION_ID() { return getToken(ExprLangParser.TRANSITION_ID, 0); }
		public TerminalNode COLORVAR_ID() { return getToken(ExprLangParser.COLORVAR_ID, 0); }
		public TerminalNode ID() { return getToken(ExprLangParser.ID, 0); }
		public TerminalNode INT_PLACE_ID() { return getToken(ExprLangParser.INT_PLACE_ID, 0); }
		public TerminalNode REAL_MSET_PLACE_ID() { return getToken(ExprLangParser.REAL_MSET_PLACE_ID, 0); }
		public TerminalNode REAL_PLACE_ID() { return getToken(ExprLangParser.REAL_PLACE_ID, 0); }
		public TerminalNode ACTION_ID() { return getToken(ExprLangParser.ACTION_ID, 0); }
		public TerminalNode INT_TEMPLATE_ID() { return getToken(ExprLangParser.INT_TEMPLATE_ID, 0); }
		public TerminalNode REAL_TEMPLATE_ID() { return getToken(ExprLangParser.REAL_TEMPLATE_ID, 0); }
		public TerminalNode CLOCK_ID() { return getToken(ExprLangParser.CLOCK_ID, 0); }
		public TerminalNode INT_MSET_PLACE_ID() { return getToken(ExprLangParser.INT_MSET_PLACE_ID, 0); }
		public TerminalNode COLORDOMAIN_ID() { return getToken(ExprLangParser.COLORDOMAIN_ID, 0); }
		public AnyIdentifierContext(AnyIDContext ctx) { copyFrom(ctx); }
		@Override
		public <T> T accept(ParseTreeVisitor<? extends T> visitor) {
			if ( visitor instanceof ExprLangVisitor ) return ((ExprLangVisitor<? extends T>)visitor).visitAnyIdentifier(this);
			else return visitor.visitChildren(this);
		}
	}

	public final AnyIDContext anyID() throws RecognitionException {
		AnyIDContext _localctx = new AnyIDContext(_ctx, getState());
		enterRule(_localctx, 122, RULE_anyID);
		int _la;
		try {
			_localctx = new AnyIdentifierContext(_localctx);
			enterOuterAlt(_localctx, 1);
			{
			setState(865);
			_la = _input.LA(1);
			if ( !(((((_la - 78)) & ~0x3f) == 0 && ((1L << (_la - 78)) & ((1L << (ID - 78)) | (1L << (INT_PLACE_ID - 78)) | (1L << (REAL_PLACE_ID - 78)) | (1L << (INT_MSET_PLACE_ID - 78)) | (1L << (REAL_MSET_PLACE_ID - 78)) | (1L << (TRANSITION_ID - 78)) | (1L << (INT_CONST_ID - 78)) | (1L << (REAL_CONST_ID - 78)) | (1L << (INT_MSET_CONST_ID - 78)) | (1L << (REAL_MSET_CONST_ID - 78)) | (1L << (INT_TEMPLATE_ID - 78)) | (1L << (REAL_TEMPLATE_ID - 78)) | (1L << (ACTION_ID - 78)) | (1L << (STATEPROP_ID - 78)) | (1L << (CLOCK_ID - 78)) | (1L << (SIMPLECOLORCLASS_ID - 78)) | (1L << (COLORDOMAIN_ID - 78)) | (1L << (COLORVAR_ID - 78)) | (1L << (COLORSUBCLASS_ID - 78)) | (1L << (COLOR_ID - 78)))) != 0)) ) {
			_errHandler.recoverInline(this);
			}
			consume();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public boolean sempred(RuleContext _localctx, int ruleIndex, int predIndex) {
		switch (ruleIndex) {
		case 22: return intExpr_sempred((IntExprContext)_localctx, predIndex);

		case 23: return realExpr_sempred((RealExprContext)_localctx, predIndex);

		case 24: return boolExpr_sempred((BoolExprContext)_localctx, predIndex);

		case 32: return realMSetExpr_sempred((RealMSetExprContext)_localctx, predIndex);

		case 33: return intMSetExpr_sempred((IntMSetExprContext)_localctx, predIndex);

		case 34: return multiSetElem_sempred((MultiSetElemContext)_localctx, predIndex);

		case 37: return colorTerm_sempred((ColorTermContext)_localctx, predIndex);

		case 49: return clockConstr_sempred((ClockConstrContext)_localctx, predIndex);

		case 54: return measure_sempred((MeasureContext)_localctx, predIndex);

		case 55: return csltaExpr_sempred((CsltaExprContext)_localctx, predIndex);
		}
		return true;
	}
	private boolean clockConstr_sempred(ClockConstrContext _localctx, int predIndex) {
		switch (predIndex) {
		case 13: return precpred(_ctx, 1);
		}
		return true;
	}
	private boolean intMSetExpr_sempred(IntMSetExprContext _localctx, int predIndex) {
		switch (predIndex) {
		case 10: return precpred(_ctx, 4);
		}
		return true;
	}
	private boolean intExpr_sempred(IntExprContext _localctx, int predIndex) {
		switch (predIndex) {
		case 0: return precpred(_ctx, 16);

		case 1: return precpred(_ctx, 15);
		}
		return true;
	}
	private boolean measure_sempred(MeasureContext _localctx, int predIndex) {
		switch (predIndex) {
		case 14: return precpred(_ctx, 4);
		}
		return true;
	}
	private boolean realExpr_sempred(RealExprContext _localctx, int predIndex) {
		switch (predIndex) {
		case 2: return clockVarInExpr;

		case 3: return precpred(_ctx, 14);

		case 4: return precpred(_ctx, 13);
		}
		return true;
	}
	private boolean csltaExpr_sempred(CsltaExprContext _localctx, int predIndex) {
		switch (predIndex) {
		case 16: return precpred(_ctx, 2);

		case 15: return precpred(_ctx, 3);
		}
		return true;
	}
	private boolean boolExpr_sempred(BoolExprContext _localctx, int predIndex) {
		switch (predIndex) {
		case 5: return precpred(_ctx, 24);

		case 6: return precpred(_ctx, 23);

		case 7: return precpred(_ctx, 22);

		case 8: return precpred(_ctx, 21);
		}
		return true;
	}
	private boolean multiSetElem_sempred(MultiSetElemContext _localctx, int predIndex) {
		switch (predIndex) {
		case 11: return precpred(_ctx, 2);
		}
		return true;
	}
	private boolean realMSetExpr_sempred(RealMSetExprContext _localctx, int predIndex) {
		switch (predIndex) {
		case 9: return precpred(_ctx, 4);
		}
		return true;
	}
	private boolean colorTerm_sempred(ColorTermContext _localctx, int predIndex) {
		switch (predIndex) {
		case 12: return precpred(_ctx, 4);
		}
		return true;
	}

	public static final String _serializedATN =
		"\3\u0430\ud6d1\u8206\uad2d\u4417\uaef1\u8d80\uaadd\3\u0097\u0366\4\2\t"+
		"\2\4\3\t\3\4\4\t\4\4\5\t\5\4\6\t\6\4\7\t\7\4\b\t\b\4\t\t\t\4\n\t\n\4\13"+
		"\t\13\4\f\t\f\4\r\t\r\4\16\t\16\4\17\t\17\4\20\t\20\4\21\t\21\4\22\t\22"+
		"\4\23\t\23\4\24\t\24\4\25\t\25\4\26\t\26\4\27\t\27\4\30\t\30\4\31\t\31"+
		"\4\32\t\32\4\33\t\33\4\34\t\34\4\35\t\35\4\36\t\36\4\37\t\37\4 \t \4!"+
		"\t!\4\"\t\"\4#\t#\4$\t$\4%\t%\4&\t&\4\'\t\'\4(\t(\4)\t)\4*\t*\4+\t+\4"+
		",\t,\4-\t-\4.\t.\4/\t/\4\60\t\60\4\61\t\61\4\62\t\62\4\63\t\63\4\64\t"+
		"\64\4\65\t\65\4\66\t\66\4\67\t\67\48\t8\49\t9\4:\t:\4;\t;\4<\t<\4=\t="+
		"\4>\t>\4?\t?\3\2\3\2\3\2\3\3\3\3\3\3\3\4\3\4\3\4\3\5\3\5\5\5\u008a\n\5"+
		"\3\5\3\5\3\6\3\6\3\6\7\6\u0091\n\6\f\6\16\6\u0094\13\6\3\6\3\6\3\7\3\7"+
		"\3\7\7\7\u009b\n\7\f\7\16\7\u009e\13\7\3\7\3\7\3\b\3\b\3\b\5\b\u00a5\n"+
		"\b\3\b\3\b\3\t\3\t\3\t\3\n\3\n\3\n\3\13\3\13\3\13\3\f\3\f\3\f\3\r\3\r"+
		"\3\r\3\16\3\16\3\16\3\17\3\17\3\17\3\20\3\20\3\20\3\21\3\21\3\21\3\22"+
		"\3\22\3\22\3\23\3\23\3\23\3\24\3\24\3\24\3\25\3\25\3\25\5\25\u00d0\n\25"+
		"\3\26\3\26\3\26\3\26\5\26\u00d6\n\26\3\27\3\27\3\30\3\30\3\30\3\30\3\30"+
		"\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30"+
		"\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30"+
		"\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30"+
		"\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\7\30"+
		"\u0116\n\30\f\30\16\30\u0119\13\30\3\30\3\30\3\30\3\30\5\30\u011f\n\30"+
		"\3\30\3\30\3\30\3\30\3\30\3\30\7\30\u0127\n\30\f\30\16\30\u012a\13\30"+
		"\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\5\31"+
		"\u0139\n\31\3\31\3\31\3\31\3\31\3\31\3\31\5\31\u0141\n\31\3\31\3\31\3"+
		"\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3"+
		"\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\5\31\u015f"+
		"\n\31\3\31\3\31\3\31\3\31\3\31\3\31\7\31\u0167\n\31\f\31\16\31\u016a\13"+
		"\31\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3"+
		"\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3"+
		"\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3"+
		"\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3"+
		"\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3"+
		"\32\5\32\u01b2\n\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32"+
		"\3\32\3\32\7\32\u01c0\n\32\f\32\16\32\u01c3\13\32\3\33\3\33\3\34\3\34"+
		"\3\35\3\35\3\36\3\36\3\37\3\37\3 \3 \3 \3 \3!\3!\3!\3!\3\"\3\"\3\"\3\""+
		"\3\"\3\"\5\"\u01dd\n\"\3\"\5\"\u01e0\n\"\3\"\3\"\3\"\3\"\7\"\u01e6\n\""+
		"\f\"\16\"\u01e9\13\"\3\"\3\"\5\"\u01ed\n\"\3\"\3\"\3\"\5\"\u01f2\n\"\3"+
		"\"\3\"\3\"\7\"\u01f7\n\"\f\"\16\"\u01fa\13\"\3#\3#\3#\3#\3#\3#\5#\u0202"+
		"\n#\3#\5#\u0205\n#\3#\3#\3#\3#\7#\u020b\n#\f#\16#\u020e\13#\3#\3#\5#\u0212"+
		"\n#\3#\3#\3#\5#\u0217\n#\3#\3#\3#\7#\u021c\n#\f#\16#\u021f\13#\3$\3$\3"+
		"$\3$\3$\3$\7$\u0227\n$\f$\16$\u022a\13$\3%\3%\3&\3&\3\'\3\'\3\'\3\'\3"+
		"\'\5\'\u0235\n\'\3\'\3\'\3\'\5\'\u023a\n\'\5\'\u023c\n\'\3\'\3\'\7\'\u0240"+
		"\n\'\f\'\16\'\u0243\13\'\3(\3(\3(\3(\3(\3(\3(\3(\3(\3(\3(\3(\3(\3(\3("+
		"\3(\3(\3(\5(\u0257\n(\3)\5)\u025a\n)\3)\3)\3)\7)\u025f\n)\f)\16)\u0262"+
		"\13)\3)\3)\3)\6)\u0267\n)\r)\16)\u0268\5)\u026b\n)\3*\3*\3+\3+\3+\3+\7"+
		"+\u0273\n+\f+\16+\u0276\13+\3+\3+\5+\u027a\n+\3+\5+\u027d\n+\3+\3+\3+"+
		"\3+\3+\3+\5+\u0285\n+\5+\u0287\n+\3,\3,\3,\3-\3-\3.\3.\5.\u0290\n.\3/"+
		"\3/\3/\3/\3/\3/\7/\u0298\n/\f/\16/\u029b\13/\3/\5/\u029e\n/\3\60\3\60"+
		"\3\60\3\60\3\60\5\60\u02a5\n\60\3\61\3\61\3\62\3\62\3\63\3\63\3\63\3\63"+
		"\3\63\3\63\3\63\3\63\3\63\3\63\3\63\3\63\3\63\3\63\3\63\3\63\3\63\3\63"+
		"\3\63\5\63\u02be\n\63\3\63\3\63\3\63\7\63\u02c3\n\63\f\63\16\63\u02c6"+
		"\13\63\3\64\3\64\3\64\3\64\3\65\3\65\3\65\3\65\7\65\u02d0\n\65\f\65\16"+
		"\65\u02d3\13\65\5\65\u02d5\n\65\3\66\3\66\3\66\3\66\7\66\u02db\n\66\f"+
		"\66\16\66\u02de\13\66\5\66\u02e0\n\66\3\67\3\67\3\67\3\67\38\38\58\u02e8"+
		"\n8\38\38\38\38\38\58\u02ef\n8\38\38\38\38\58\u02f5\n8\38\38\38\58\u02fa"+
		"\n8\38\38\38\38\58\u0300\n8\38\58\u0303\n8\38\38\38\78\u0308\n8\f8\16"+
		"8\u030b\138\39\39\39\39\39\39\39\39\39\39\39\39\39\39\39\39\39\79\u031e"+
		"\n9\f9\169\u0321\139\59\u0323\n9\39\39\39\39\79\u0329\n9\f9\169\u032c"+
		"\139\59\u032e\n9\39\39\39\39\79\u0334\n9\f9\169\u0337\139\59\u0339\n9"+
		"\39\39\59\u033d\n9\39\39\39\39\39\39\79\u0345\n9\f9\169\u0348\139\3:\3"+
		":\3:\3:\3;\3;\3;\3;\3<\3<\3<\3<\3=\3=\3=\3=\5=\u035a\n=\3>\3>\3>\3>\3"+
		">\3>\5>\u0362\n>\3?\3?\3?\2\f.\60\62BDFLdnp@\2\4\6\b\n\f\16\20\22\24\26"+
		"\30\32\34\36 \"$&(*,.\60\62\64\668:<>@BDFHJLNPRTVXZ\\^`bdfhjlnprtvxz|"+
		"\2\33\3\2/\60\3\2\22\23\3\2\24\25\3\2|~\3\2\u0084\u0086\3\2\u0088\u0089"+
		"\3\2(-\3\2\65\66\3\2pq\3\2\u0081\u0082\3\2?@\3\2JL\4\2:>MM\3\2\u0091\u0096"+
		"\5\2AGII\u0097\u0097\3\2\30\31\4\2\3\5\b\b\3\2*+\5\2PPaaee\5\2PP__ff\4"+
		"\2PPgg\3\2rt\3\2vx\3\2y{\4\2PP[m\u03b5\2~\3\2\2\2\4\u0081\3\2\2\2\6\u0084"+
		"\3\2\2\2\b\u0089\3\2\2\2\n\u008d\3\2\2\2\f\u0097\3\2\2\2\16\u00a1\3\2"+
		"\2\2\20\u00a8\3\2\2\2\22\u00ab\3\2\2\2\24\u00ae\3\2\2\2\26\u00b1\3\2\2"+
		"\2\30\u00b4\3\2\2\2\32\u00b7\3\2\2\2\34\u00ba\3\2\2\2\36\u00bd\3\2\2\2"+
		" \u00c0\3\2\2\2\"\u00c3\3\2\2\2$\u00c6\3\2\2\2&\u00c9\3\2\2\2(\u00cf\3"+
		"\2\2\2*\u00d5\3\2\2\2,\u00d7\3\2\2\2.\u011e\3\2\2\2\60\u015e\3\2\2\2\62"+
		"\u01b1\3\2\2\2\64\u01c4\3\2\2\2\66\u01c6\3\2\2\28\u01c8\3\2\2\2:\u01ca"+
		"\3\2\2\2<\u01cc\3\2\2\2>\u01ce\3\2\2\2@\u01d2\3\2\2\2B\u01f1\3\2\2\2D"+
		"\u0216\3\2\2\2F\u0220\3\2\2\2H\u022b\3\2\2\2J\u022d\3\2\2\2L\u023b\3\2"+
		"\2\2N\u0256\3\2\2\2P\u026a\3\2\2\2R\u026c\3\2\2\2T\u0286\3\2\2\2V\u0288"+
		"\3\2\2\2X\u028b\3\2\2\2Z\u028f\3\2\2\2\\\u029d\3\2\2\2^\u02a4\3\2\2\2"+
		"`\u02a6\3\2\2\2b\u02a8\3\2\2\2d\u02bd\3\2\2\2f\u02c7\3\2\2\2h\u02d4\3"+
		"\2\2\2j\u02df\3\2\2\2l\u02e1\3\2\2\2n\u0302\3\2\2\2p\u033c\3\2\2\2r\u0349"+
		"\3\2\2\2t\u034d\3\2\2\2v\u0351\3\2\2\2x\u0359\3\2\2\2z\u0361\3\2\2\2|"+
		"\u0363\3\2\2\2~\177\5.\30\2\177\u0080\7\2\2\3\u0080\3\3\2\2\2\u0081\u0082"+
		"\5\60\31\2\u0082\u0083\7\2\2\3\u0083\5\3\2\2\2\u0084\u0085\5\62\32\2\u0085"+
		"\u0086\7\2\2\3\u0086\7\3\2\2\2\u0087\u008a\5.\30\2\u0088\u008a\7\61\2"+
		"\2\u0089\u0087\3\2\2\2\u0089\u0088\3\2\2\2\u008a\u008b\3\2\2\2\u008b\u008c"+
		"\7\2\2\3\u008c\t\3\2\2\2\u008d\u0092\5.\30\2\u008e\u008f\7\36\2\2\u008f"+
		"\u0091\5.\30\2\u0090\u008e\3\2\2\2\u0091\u0094\3\2\2\2\u0092\u0090\3\2"+
		"\2\2\u0092\u0093\3\2\2\2\u0093\u0095\3\2\2\2\u0094\u0092\3\2\2\2\u0095"+
		"\u0096\7\2\2\3\u0096\13\3\2\2\2\u0097\u009c\5\60\31\2\u0098\u0099\7\36"+
		"\2\2\u0099\u009b\5\60\31\2\u009a\u0098\3\2\2\2\u009b\u009e\3\2\2\2\u009c"+
		"\u009a\3\2\2\2\u009c\u009d\3\2\2\2\u009d\u009f\3\2\2\2\u009e\u009c\3\2"+
		"\2\2\u009f\u00a0\7\2\2\3\u00a0\r\3\2\2\2\u00a1\u00a4\5d\63\2\u00a2\u00a3"+
		"\7 \2\2\u00a3\u00a5\5h\65\2\u00a4\u00a2\3\2\2\2\u00a4\u00a5\3\2\2\2\u00a5"+
		"\u00a6\3\2\2\2\u00a6\u00a7\7\2\2\3\u00a7\17\3\2\2\2\u00a8\u00a9\5^\60"+
		"\2\u00a9\u00aa\7\2\2\3\u00aa\21\3\2\2\2\u00ab\u00ac\5\62\32\2\u00ac\u00ad"+
		"\7\2\2\3\u00ad\23\3\2\2\2\u00ae\u00af\5`\61\2\u00af\u00b0\7\2\2\3\u00b0"+
		"\25\3\2\2\2\u00b1\u00b2\5j\66\2\u00b2\u00b3\7\2\2\3\u00b3\27\3\2\2\2\u00b4"+
		"\u00b5\7\62\2\2\u00b5\u00b6\7\2\2\3\u00b6\31\3\2\2\2\u00b7\u00b8\5n8\2"+
		"\u00b8\u00b9\7\2\2\3\u00b9\33\3\2\2\2\u00ba\u00bb\5p9\2\u00bb\u00bc\7"+
		"\2\2\3\u00bc\35\3\2\2\2\u00bd\u00be\5P)\2\u00be\u00bf\7\2\2\3\u00bf\37"+
		"\3\2\2\2\u00c0\u00c1\5X-\2\u00c1\u00c2\7\2\2\3\u00c2!\3\2\2\2\u00c3\u00c4"+
		"\5Z.\2\u00c4\u00c5\7\2\2\3\u00c5#\3\2\2\2\u00c6\u00c7\5B\"\2\u00c7\u00c8"+
		"\7\2\2\3\u00c8%\3\2\2\2\u00c9\u00ca\5D#\2\u00ca\u00cb\7\2\2\3\u00cb\'"+
		"\3\2\2\2\u00cc\u00d0\7Q\2\2\u00cd\u00d0\7`\2\2\u00ce\u00d0\7d\2\2\u00cf"+
		"\u00cc\3\2\2\2\u00cf\u00cd\3\2\2\2\u00cf\u00ce\3\2\2\2\u00d0)\3\2\2\2"+
		"\u00d1\u00d6\5(\25\2\u00d2\u00d6\7R\2\2\u00d3\u00d6\7a\2\2\u00d4\u00d6"+
		"\7e\2\2\u00d5\u00d1\3\2\2\2\u00d5\u00d2\3\2\2\2\u00d5\u00d3\3\2\2\2\u00d5"+
		"\u00d4\3\2\2\2\u00d6+\3\2\2\2\u00d7\u00d8\t\2\2\2\u00d8-\3\2\2\2\u00d9"+
		"\u00da\b\30\1\2\u00da\u00db\7\25\2\2\u00db\u011f\5.\30\20\u00dc\u011f"+
		"\5(\25\2\u00dd\u00de\7.\2\2\u00de\u011f\7[\2\2\u00df\u00e0\7!\2\2\u00e0"+
		"\u00e1\5.\30\2\u00e1\u00e2\7\"\2\2\u00e2\u011f\3\2\2\2\u00e3\u00e4\5\64"+
		"\33\2\u00e4\u00e5\7#\2\2\u00e5\u00e6\5.\30\2\u00e6\u00e7\7$\2\2\u00e7"+
		"\u011f\3\2\2\2\u00e8\u00e9\5\66\34\2\u00e9\u00ea\7#\2\2\u00ea\u00eb\5"+
		"\60\31\2\u00eb\u00ec\7$\2\2\u00ec\u011f\3\2\2\2\u00ed\u00ee\58\35\2\u00ee"+
		"\u00ef\7#\2\2\u00ef\u00f0\5.\30\2\u00f0\u00f1\7\36\2\2\u00f1\u00f2\5."+
		"\30\2\u00f2\u00f3\7$\2\2\u00f3\u011f\3\2\2\2\u00f4\u00f5\7N\2\2\u00f5"+
		"\u00f6\7#\2\2\u00f6\u00f7\5\62\32\2\u00f7\u00f8\7\36\2\2\u00f8\u00f9\5"+
		".\30\2\u00f9\u00fa\7\36\2\2\u00fa\u00fb\5.\30\2\u00fb\u00fc\7$\2\2\u00fc"+
		"\u011f\3\2\2\2\u00fd\u00fe\7\t\2\2\u00fe\u00ff\5\62\32\2\u00ff\u0100\7"+
		"\37\2\2\u0100\u0101\5.\30\2\u0101\u0102\7 \2\2\u0102\u0103\7\16\2\2\u0103"+
		"\u0104\5.\30\2\u0104\u0105\7 \2\2\u0105\u011f\3\2\2\2\u0106\u0107\78\2"+
		"\2\u0107\u0108\7#\2\2\u0108\u0109\5D#\2\u0109\u010a\7$\2\2\u010a\u011f"+
		"\3\2\2\2\u010b\u010c\7\67\2\2\u010c\u010d\7#\2\2\u010d\u010e\5L\'\2\u010e"+
		"\u010f\7$\2\2\u010f\u011f\3\2\2\2\u0110\u0111\7\u0080\2\2\u0111\u0112"+
		"\7!\2\2\u0112\u0117\7[\2\2\u0113\u0114\7\36\2\2\u0114\u0116\7[\2\2\u0115"+
		"\u0113\3\2\2\2\u0116\u0119\3\2\2\2\u0117\u0115\3\2\2\2\u0117\u0118\3\2"+
		"\2\2\u0118\u011a\3\2\2\2\u0119\u0117\3\2\2\2\u011a\u011f\7\"\2\2\u011b"+
		"\u011f\7P\2\2\u011c\u011d\7.\2\2\u011d\u011f\7P\2\2\u011e\u00d9\3\2\2"+
		"\2\u011e\u00dc\3\2\2\2\u011e\u00dd\3\2\2\2\u011e\u00df\3\2\2\2\u011e\u00e3"+
		"\3\2\2\2\u011e\u00e8\3\2\2\2\u011e\u00ed\3\2\2\2\u011e\u00f4\3\2\2\2\u011e"+
		"\u00fd\3\2\2\2\u011e\u0106\3\2\2\2\u011e\u010b\3\2\2\2\u011e\u0110\3\2"+
		"\2\2\u011e\u011b\3\2\2\2\u011e\u011c\3\2\2\2\u011f\u0128\3\2\2\2\u0120"+
		"\u0121\f\22\2\2\u0121\u0122\t\3\2\2\u0122\u0127\5.\30\23\u0123\u0124\f"+
		"\21\2\2\u0124\u0125\t\4\2\2\u0125\u0127\5.\30\22\u0126\u0120\3\2\2\2\u0126"+
		"\u0123\3\2\2\2\u0127\u012a\3\2\2\2\u0128\u0126\3\2\2\2\u0128\u0129\3\2"+
		"\2\2\u0129/\3\2\2\2\u012a\u0128\3\2\2\2\u012b\u012c\b\31\1\2\u012c\u012d"+
		"\7\25\2\2\u012d\u015f\5\60\31\16\u012e\u015f\5*\26\2\u012f\u0130\7.\2"+
		"\2\u0130\u015f\7\\\2\2\u0131\u015f\5.\30\2\u0132\u0133\7!\2\2\u0133\u0134"+
		"\5\60\31\2\u0134\u0135\7\"\2\2\u0135\u015f\3\2\2\2\u0136\u0139\5<\37\2"+
		"\u0137\u0139\5\64\33\2\u0138\u0136\3\2\2\2\u0138\u0137\3\2\2\2\u0139\u013a"+
		"\3\2\2\2\u013a\u013b\7#\2\2\u013b\u013c\5\60\31\2\u013c\u013d\7$\2\2\u013d"+
		"\u015f\3\2\2\2\u013e\u0141\58\35\2\u013f\u0141\5:\36\2\u0140\u013e\3\2"+
		"\2\2\u0140\u013f\3\2\2\2\u0141\u0142\3\2\2\2\u0142\u0143\7#\2\2\u0143"+
		"\u0144\5\60\31\2\u0144\u0145\7\36\2\2\u0145\u0146\5\60\31\2\u0146\u0147"+
		"\7$\2\2\u0147\u015f\3\2\2\2\u0148\u0149\7N\2\2\u0149\u014a\7#\2\2\u014a"+
		"\u014b\5\62\32\2\u014b\u014c\7\36\2\2\u014c\u014d\5\60\31\2\u014d\u014e"+
		"\7\36\2\2\u014e\u014f\5\60\31\2\u014f\u0150\7$\2\2\u0150\u015f\3\2\2\2"+
		"\u0151\u0152\7\t\2\2\u0152\u0153\5\62\32\2\u0153\u0154\7\37\2\2\u0154"+
		"\u0155\5\60\31\2\u0155\u0156\7 \2\2\u0156\u0157\7\16\2\2\u0157\u0158\5"+
		"\60\31\2\u0158\u0159\7 \2\2\u0159\u015f\3\2\2\2\u015a\u015f\7\u0090\2"+
		"\2\u015b\u015c\6\31\4\2\u015c\u015f\5b\62\2\u015d\u015f\7P\2\2\u015e\u012b"+
		"\3\2\2\2\u015e\u012e\3\2\2\2\u015e\u012f\3\2\2\2\u015e\u0131\3\2\2\2\u015e"+
		"\u0132\3\2\2\2\u015e\u0138\3\2\2\2\u015e\u0140\3\2\2\2\u015e\u0148\3\2"+
		"\2\2\u015e\u0151\3\2\2\2\u015e\u015a\3\2\2\2\u015e\u015b\3\2\2\2\u015e"+
		"\u015d\3\2\2\2\u015f\u0168\3\2\2\2\u0160\u0161\f\20\2\2\u0161\u0162\t"+
		"\3\2\2\u0162\u0167\5\60\31\21\u0163\u0164\f\17\2\2\u0164\u0165\t\4\2\2"+
		"\u0165\u0167\5\60\31\20\u0166\u0160\3\2\2\2\u0166\u0163\3\2\2\2\u0167"+
		"\u016a\3\2\2\2\u0168\u0166\3\2\2\2\u0168\u0169\3\2\2\2\u0169\61\3\2\2"+
		"\2\u016a\u0168\3\2\2\2\u016b\u016c\b\32\1\2\u016c\u016d\7\33\2\2\u016d"+
		"\u01b2\5\62\32\33\u016e\u016f\5x=\2\u016f\u0170\5\62\32\16\u0170\u01b2"+
		"\3\2\2\2\u0171\u0172\t\5\2\2\u0172\u01b2\5\62\32\13\u0173\u0174\t\6\2"+
		"\2\u0174\u01b2\5\62\32\7\u0175\u0176\t\7\2\2\u0176\u01b2\5\62\32\4\u0177"+
		"\u0178\5z>\2\u0178\u0179\5\62\32\3\u0179\u01b2\3\2\2\2\u017a\u01b2\5,"+
		"\27\2\u017b\u017c\5.\30\2\u017c\u017d\t\b\2\2\u017d\u017e\5.\30\2\u017e"+
		"\u01b2\3\2\2\2\u017f\u0180\5\60\31\2\u0180\u0181\t\b\2\2\u0181\u0182\5"+
		"\60\31\2\u0182\u01b2\3\2\2\2\u0183\u0184\7!\2\2\u0184\u0185\5\62\32\2"+
		"\u0185\u0186\7\"\2\2\u0186\u01b2\3\2\2\2\u0187\u01b2\7P\2\2\u0188\u01b2"+
		"\7g\2\2\u0189\u018a\5L\'\2\u018a\u018b\t\b\2\2\u018b\u018c\5L\'\2\u018c"+
		"\u01b2\3\2\2\2\u018d\u018e\5L\'\2\u018e\u018f\t\t\2\2\u018f\u0190\5N("+
		"\2\u0190\u01b2\3\2\2\2\u0191\u0192\t\n\2\2\u0192\u0193\7!\2\2\u0193\u0194"+
		"\5\62\32\2\u0194\u0195\7u\2\2\u0195\u0196\5\62\32\2\u0196\u0197\7\"\2"+
		"\2\u0197\u01b2\3\2\2\2\u0198\u0199\t\n\2\2\u0199\u019a\7#\2\2\u019a\u019b"+
		"\5\62\32\2\u019b\u019c\7u\2\2\u019c\u019d\5\62\32\2\u019d\u019e\7$\2\2"+
		"\u019e\u01b2\3\2\2\2\u019f\u01a0\7\177\2\2\u01a0\u01a1\7!\2\2\u01a1\u01a2"+
		"\7_\2\2\u01a2\u01b2\7\"\2\2\u01a3\u01b2\t\13\2\2\u01a4\u01b2\7\u0083\2"+
		"\2\u01a5\u01a6\7!\2\2\u01a6\u01a7\5\62\32\2\u01a7\u01a8\7\u0087\2\2\u01a8"+
		"\u01a9\5\62\32\2\u01a9\u01aa\7\"\2\2\u01aa\u01b2\3\2\2\2\u01ab\u01ac\7"+
		"#\2\2\u01ac\u01ad\5\62\32\2\u01ad\u01ae\7\u0087\2\2\u01ae\u01af\5\62\32"+
		"\2\u01af\u01b0\7$\2\2\u01b0\u01b2\3\2\2\2\u01b1\u016b\3\2\2\2\u01b1\u016e"+
		"\3\2\2\2\u01b1\u0171\3\2\2\2\u01b1\u0173\3\2\2\2\u01b1\u0175\3\2\2\2\u01b1"+
		"\u0177\3\2\2\2\u01b1\u017a\3\2\2\2\u01b1\u017b\3\2\2\2\u01b1\u017f\3\2"+
		"\2\2\u01b1\u0183\3\2\2\2\u01b1\u0187\3\2\2\2\u01b1\u0188\3\2\2\2\u01b1"+
		"\u0189\3\2\2\2\u01b1\u018d\3\2\2\2\u01b1\u0191\3\2\2\2\u01b1\u0198\3\2"+
		"\2\2\u01b1\u019f\3\2\2\2\u01b1\u01a3\3\2\2\2\u01b1\u01a4\3\2\2\2\u01b1"+
		"\u01a5\3\2\2\2\u01b1\u01ab\3\2\2\2\u01b2\u01c1\3\2\2\2\u01b3\u01b4\f\32"+
		"\2\2\u01b4\u01b5\7\26\2\2\u01b5\u01c0\5\62\32\33\u01b6\u01b7\f\31\2\2"+
		"\u01b7\u01b8\7\27\2\2\u01b8\u01c0\5\62\32\32\u01b9\u01ba\f\30\2\2\u01ba"+
		"\u01bb\7\34\2\2\u01bb\u01c0\5\62\32\31\u01bc\u01bd\f\27\2\2\u01bd\u01be"+
		"\7\35\2\2\u01be\u01c0\5\62\32\30\u01bf\u01b3\3\2\2\2\u01bf\u01b6\3\2\2"+
		"\2\u01bf\u01b9\3\2\2\2\u01bf\u01bc\3\2\2\2\u01c0\u01c3\3\2\2\2\u01c1\u01bf"+
		"\3\2\2\2\u01c1\u01c2\3\2\2\2\u01c2\63\3\2\2\2\u01c3\u01c1\3\2\2\2\u01c4"+
		"\u01c5\t\f\2\2\u01c5\65\3\2\2\2\u01c6\u01c7\t\r\2\2\u01c7\67\3\2\2\2\u01c8"+
		"\u01c9\t\16\2\2\u01c99\3\2\2\2\u01ca\u01cb\t\17\2\2\u01cb;\3\2\2\2\u01cc"+
		"\u01cd\t\20\2\2\u01cd=\3\2\2\2\u01ce\u01cf\7#\2\2\u01cf\u01d0\5\62\32"+
		"\2\u01d0\u01d1\7$\2\2\u01d1?\3\2\2\2\u01d2\u01d3\7#\2\2\u01d3\u01d4\5"+
		"\62\32\2\u01d4\u01d5\7$\2\2\u01d5A\3\2\2\2\u01d6\u01d7\b\"\1\2\u01d7\u01d8"+
		"\7!\2\2\u01d8\u01d9\5B\"\2\u01d9\u01da\7\"\2\2\u01da\u01f2\3\2\2\2\u01db"+
		"\u01dd\5\60\31\2\u01dc\u01db\3\2\2\2\u01dc\u01dd\3\2\2\2\u01dd\u01df\3"+
		"\2\2\2\u01de\u01e0\5> \2\u01df\u01de\3\2\2\2\u01df\u01e0\3\2\2\2\u01e0"+
		"\u01e1\3\2\2\2\u01e1\u01e2\7*\2\2\u01e2\u01e7\5F$\2\u01e3\u01e4\7\36\2"+
		"\2\u01e4\u01e6\5F$\2\u01e5\u01e3\3\2\2\2\u01e6\u01e9\3\2\2\2\u01e7\u01e5"+
		"\3\2\2\2\u01e7\u01e8\3\2\2\2\u01e8\u01ea\3\2\2\2\u01e9\u01e7\3\2\2\2\u01ea"+
		"\u01ec\7+\2\2\u01eb\u01ed\5@!\2\u01ec\u01eb\3\2\2\2\u01ec\u01ed\3\2\2"+
		"\2\u01ed\u01f2\3\2\2\2\u01ee\u01ef\7.\2\2\u01ef\u01f2\7^\2\2\u01f0\u01f2"+
		"\7c\2\2\u01f1\u01d6\3\2\2\2\u01f1\u01dc\3\2\2\2\u01f1\u01ee\3\2\2\2\u01f1"+
		"\u01f0\3\2\2\2\u01f2\u01f8\3\2\2\2\u01f3\u01f4\f\6\2\2\u01f4\u01f5\t\4"+
		"\2\2\u01f5\u01f7\5B\"\7\u01f6\u01f3\3\2\2\2\u01f7\u01fa\3\2\2\2\u01f8"+
		"\u01f6\3\2\2\2\u01f8\u01f9\3\2\2\2\u01f9C\3\2\2\2\u01fa\u01f8\3\2\2\2"+
		"\u01fb\u01fc\b#\1\2\u01fc\u01fd\7!\2\2\u01fd\u01fe\5D#\2\u01fe\u01ff\7"+
		"\"\2\2\u01ff\u0217\3\2\2\2\u0200\u0202\5.\30\2\u0201\u0200\3\2\2\2\u0201"+
		"\u0202\3\2\2\2\u0202\u0204\3\2\2\2\u0203\u0205\5> \2\u0204\u0203\3\2\2"+
		"\2\u0204\u0205\3\2\2\2\u0205\u0206\3\2\2\2\u0206\u0207\7*\2\2\u0207\u020c"+
		"\5F$\2\u0208\u0209\7\36\2\2\u0209\u020b\5F$\2\u020a\u0208\3\2\2\2\u020b"+
		"\u020e\3\2\2\2\u020c\u020a\3\2\2\2\u020c\u020d\3\2\2\2\u020d\u020f\3\2"+
		"\2\2\u020e\u020c\3\2\2\2\u020f\u0211\7+\2\2\u0210\u0212\5@!\2\u0211\u0210"+
		"\3\2\2\2\u0211\u0212\3\2\2\2\u0212\u0217\3\2\2\2\u0213\u0214\7.\2\2\u0214"+
		"\u0217\7]\2\2\u0215\u0217\7b\2\2\u0216\u01fb\3\2\2\2\u0216\u0201\3\2\2"+
		"\2\u0216\u0213\3\2\2\2\u0216\u0215\3\2\2\2\u0217\u021d\3\2\2\2\u0218\u0219"+
		"\f\6\2\2\u0219\u021a\t\4\2\2\u021a\u021c\5D#\7\u021b\u0218\3\2\2\2\u021c"+
		"\u021f\3\2\2\2\u021d\u021b\3\2\2\2\u021d\u021e\3\2\2\2\u021eE\3\2\2\2"+
		"\u021f\u021d\3\2\2\2\u0220\u0221\b$\1\2\u0221\u0222\5N(\2\u0222\u0228"+
		"\3\2\2\2\u0223\u0224\f\4\2\2\u0224\u0225\t\4\2\2\u0225\u0227\5F$\5\u0226"+
		"\u0223\3\2\2\2\u0227\u022a\3\2\2\2\u0228\u0226\3\2\2\2\u0228\u0229\3\2"+
		"\2\2\u0229G\3\2\2\2\u022a\u0228\3\2\2\2\u022b\u022c\7k\2\2\u022cI\3\2"+
		"\2\2\u022d\u022e\7l\2\2\u022eK\3\2\2\2\u022f\u0230\b\'\1\2\u0230\u023c"+
		"\5H%\2\u0231\u023c\7m\2\2\u0232\u0234\7\20\2\2\u0233\u0235\7i\2\2\u0234"+
		"\u0233\3\2\2\2\u0234\u0235\3\2\2\2\u0235\u0239\3\2\2\2\u0236\u0237\7#"+
		"\2\2\u0237\u0238\7Q\2\2\u0238\u023a\7$\2\2\u0239\u0236\3\2\2\2\u0239\u023a"+
		"\3\2\2\2\u023a\u023c\3\2\2\2\u023b\u022f\3\2\2\2\u023b\u0231\3\2\2\2\u023b"+
		"\u0232\3\2\2\2\u023c\u0241\3\2\2\2\u023d\u023e\f\6\2\2\u023e\u0240\t\21"+
		"\2\2\u023f\u023d\3\2\2\2\u0240\u0243\3\2\2\2\u0241\u023f\3\2\2\2\u0241"+
		"\u0242\3\2\2\2\u0242M\3\2\2\2\u0243\u0241\3\2\2\2\u0244\u0257\5L\'\2\u0245"+
		"\u0257\7\n\2\2\u0246\u0257\7i\2\2\u0247\u0248\7n\2\2\u0248\u0257\7i\2"+
		"\2\u0249\u0257\5J&\2\u024a\u024b\7n\2\2\u024b\u0257\5J&\2\u024c\u024d"+
		"\79\2\2\u024d\u024e\7#\2\2\u024e\u024f\5L\'\2\u024f\u0250\7$\2\2\u0250"+
		"\u0257\3\2\2\2\u0251\u0252\7o\2\2\u0252\u0253\7!\2\2\u0253\u0254\5L\'"+
		"\2\u0254\u0255\7\"\2\2\u0255\u0257\3\2\2\2\u0256\u0244\3\2\2\2\u0256\u0245"+
		"\3\2\2\2\u0256\u0246\3\2\2\2\u0256\u0247\3\2\2\2\u0256\u0249\3\2\2\2\u0256"+
		"\u024a\3\2\2\2\u0256\u024c\3\2\2\2\u0256\u0251\3\2\2\2\u0257O\3\2\2\2"+
		"\u0258\u025a\5R*\2\u0259\u0258\3\2\2\2\u0259\u025a\3\2\2\2\u025a\u025b"+
		"\3\2\2\2\u025b\u0260\5T+\2\u025c\u025d\7\24\2\2\u025d\u025f\5T+\2\u025e"+
		"\u025c\3\2\2\2\u025f\u0262\3\2\2\2\u0260\u025e\3\2\2\2\u0260\u0261\3\2"+
		"\2\2\u0261\u026b\3\2\2\2\u0262\u0260\3\2\2\2\u0263\u0266\7i\2\2\u0264"+
		"\u0265\7\22\2\2\u0265\u0267\7i\2\2\u0266\u0264\3\2\2\2\u0267\u0268\3\2"+
		"\2\2\u0268\u0266\3\2\2\2\u0268\u0269\3\2\2\2\u0269\u026b\3\2\2\2\u026a"+
		"\u0259\3\2\2\2\u026a\u0263\3\2\2\2\u026bQ\3\2\2\2\u026c\u026d\t\22\2\2"+
		"\u026dS\3\2\2\2\u026e\u026f\7%\2\2\u026f\u0274\7P\2\2\u0270\u0271\7\36"+
		"\2\2\u0271\u0273\7P\2\2\u0272\u0270\3\2\2\2\u0273\u0276\3\2\2\2\u0274"+
		"\u0272\3\2\2\2\u0274\u0275\3\2\2\2\u0275\u0277\3\2\2\2\u0276\u0274\3\2"+
		"\2\2\u0277\u0279\7&\2\2\u0278\u027a\5V,\2\u0279\u0278\3\2\2\2\u0279\u027a"+
		"\3\2\2\2\u027a\u0287\3\2\2\2\u027b\u027d\5|?\2\u027c\u027b\3\2\2\2\u027c"+
		"\u027d\3\2\2\2\u027d\u027e\3\2\2\2\u027e\u027f\7%\2\2\u027f\u0280\5(\25"+
		"\2\u0280\u0281\7\'\2\2\u0281\u0282\5(\25\2\u0282\u0284\7&\2\2\u0283\u0285"+
		"\5V,\2\u0284\u0283\3\2\2\2\u0284\u0285\3\2\2\2\u0285\u0287\3\2\2\2\u0286"+
		"\u026e\3\2\2\2\u0286\u027c\3\2\2\2\u0287U\3\2\2\2\u0288\u0289\7\13\2\2"+
		"\u0289\u028a\7P\2\2\u028aW\3\2\2\2\u028b\u028c\7i\2\2\u028cY\3\2\2\2\u028d"+
		"\u0290\7i\2\2\u028e\u0290\7j\2\2\u028f\u028d\3\2\2\2\u028f\u028e\3\2\2"+
		"\2\u0290[\3\2\2\2\u0291\u0292\7%\2\2\u0292\u029e\7&\2\2\u0293\u0294\7"+
		"%\2\2\u0294\u0299\7f\2\2\u0295\u0296\7\36\2\2\u0296\u0298\7f\2\2\u0297"+
		"\u0295\3\2\2\2\u0298\u029b\3\2\2\2\u0299\u0297\3\2\2\2\u0299\u029a\3\2"+
		"\2\2\u029a\u029c\3\2\2\2\u029b\u0299\3\2\2\2\u029c\u029e\7&\2\2\u029d"+
		"\u0291\3\2\2\2\u029d\u0293\3\2\2\2\u029e]\3\2\2\2\u029f\u02a5\7\63\2\2"+
		"\u02a0\u02a1\7\63\2\2\u02a1\u02a2\7\6\2\2\u02a2\u02a5\5\\/\2\u02a3\u02a5"+
		"\5\\/\2\u02a4\u029f\3\2\2\2\u02a4\u02a0\3\2\2\2\u02a4\u02a3\3\2\2\2\u02a5"+
		"_\3\2\2\2\u02a6\u02a7\7_\2\2\u02a7a\3\2\2\2\u02a8\u02a9\7h\2\2\u02a9c"+
		"\3\2\2\2\u02aa\u02ab\b\63\1\2\u02ab\u02ac\5\60\31\2\u02ac\u02ad\7*\2\2"+
		"\u02ad\u02ae\5b\62\2\u02ae\u02af\7*\2\2\u02af\u02b0\5\60\31\2\u02b0\u02be"+
		"\3\2\2\2\u02b1\u02b2\5b\62\2\u02b2\u02b3\t\23\2\2\u02b3\u02b4\5\60\31"+
		"\2\u02b4\u02be\3\2\2\2\u02b5\u02b6\5\60\31\2\u02b6\u02b7\t\23\2\2\u02b7"+
		"\u02b8\5b\62\2\u02b8\u02be\3\2\2\2\u02b9\u02ba\5b\62\2\u02ba\u02bb\7\7"+
		"\2\2\u02bb\u02bc\5\60\31\2\u02bc\u02be\3\2\2\2\u02bd\u02aa\3\2\2\2\u02bd"+
		"\u02b1\3\2\2\2\u02bd\u02b5\3\2\2\2\u02bd\u02b9\3\2\2\2\u02be\u02c4\3\2"+
		"\2\2\u02bf\u02c0\f\3\2\2\u02c0\u02c1\7\26\2\2\u02c1\u02c3\5d\63\4\u02c2"+
		"\u02bf\3\2\2\2\u02c3\u02c6\3\2\2\2\u02c4\u02c2\3\2\2\2\u02c4\u02c5\3\2"+
		"\2\2\u02c5e\3\2\2\2\u02c6\u02c4\3\2\2\2\u02c7\u02c8\5b\62\2\u02c8\u02c9"+
		"\7\7\2\2\u02c9\u02ca\5\60\31\2\u02cag\3\2\2\2\u02cb\u02d5\3\2\2\2\u02cc"+
		"\u02d1\5f\64\2\u02cd\u02ce\7\36\2\2\u02ce\u02d0\5f\64\2\u02cf\u02cd\3"+
		"\2\2\2\u02d0\u02d3\3\2\2\2\u02d1\u02cf\3\2\2\2\u02d1\u02d2\3\2\2\2\u02d2"+
		"\u02d5\3\2\2\2\u02d3\u02d1\3\2\2\2\u02d4\u02cb\3\2\2\2\u02d4\u02cc\3\2"+
		"\2\2\u02d5i\3\2\2\2\u02d6\u02e0\3\2\2\2\u02d7\u02dc\5l\67\2\u02d8\u02d9"+
		"\7\36\2\2\u02d9\u02db\5l\67\2\u02da\u02d8\3\2\2\2\u02db\u02de\3\2\2\2"+
		"\u02dc\u02da\3\2\2\2\u02dc\u02dd\3\2\2\2\u02dd\u02e0\3\2\2\2\u02de\u02dc"+
		"\3\2\2\2\u02df\u02d6\3\2\2\2\u02df\u02d7\3\2\2\2\u02e0k\3\2\2\2\u02e1"+
		"\u02e2\5b\62\2\u02e2\u02e3\7\7\2\2\u02e3\u02e4\5\60\31\2\u02e4m\3\2\2"+
		"\2\u02e5\u02e7\b8\1\2\u02e6\u02e8\5*\26\2\u02e7\u02e6\3\2\2\2\u02e7\u02e8"+
		"\3\2\2\2\u02e8\u02e9\3\2\2\2\u02e9\u02ea\7\21\2\2\u02ea\u02eb\5\62\32"+
		"\2\u02eb\u02ec\7&\2\2\u02ec\u0303\3\2\2\2\u02ed\u02ef\5*\26\2\u02ee\u02ed"+
		"\3\2\2\2\u02ee\u02ef\3\2\2\2\u02ef\u02f0\3\2\2\2\u02f0\u02f1\7\f\2\2\u02f1"+
		"\u02f4\5.\30\2\u02f2\u02f3\7\23\2\2\u02f3\u02f5\5\62\32\2\u02f4\u02f2"+
		"\3\2\2\2\u02f4\u02f5\3\2\2\2\u02f5\u02f6\3\2\2\2\u02f6\u02f7\7&\2\2\u02f7"+
		"\u0303\3\2\2\2\u02f8\u02fa\5*\26\2\u02f9\u02f8\3\2\2\2\u02f9\u02fa\3\2"+
		"\2\2\u02fa\u02fb\3\2\2\2\u02fb\u02fc\7\17\2\2\u02fc\u02ff\7_\2\2\u02fd"+
		"\u02fe\7\23\2\2\u02fe\u0300\5\62\32\2\u02ff\u02fd\3\2\2\2\u02ff\u0300"+
		"\3\2\2\2\u0300\u0301\3\2\2\2\u0301\u0303\7&\2\2\u0302\u02e5\3\2\2\2\u0302"+
		"\u02ee\3\2\2\2\u0302\u02f9\3\2\2\2\u0303\u0309\3\2\2\2\u0304\u0305\f\6"+
		"\2\2\u0305\u0306\t\4\2\2\u0306\u0308\5n8\7\u0307\u0304\3\2\2\2\u0308\u030b"+
		"\3\2\2\2\u0309\u0307\3\2\2\2\u0309\u030a\3\2\2\2\u030ao\3\2\2\2\u030b"+
		"\u0309\3\2\2\2\u030c\u030d\b9\1\2\u030d\u030e\7\33\2\2\u030e\u033d\5p"+
		"9\6\u030f\u033d\5\62\32\2\u0310\u033d\7[\2\2\u0311\u0312\7!\2\2\u0312"+
		"\u0313\5p9\2\u0313\u0314\7\"\2\2\u0314\u033d\3\2\2\2\u0315\u0316\7\64"+
		"\2\2\u0316\u0317\t\b\2\2\u0317\u0318\5*\26\2\u0318\u0319\7P\2\2\u0319"+
		"\u0322\7!\2\2\u031a\u031f\5r:\2\u031b\u031c\7\36\2\2\u031c\u031e\5r:\2"+
		"\u031d\u031b\3\2\2\2\u031e\u0321\3\2\2\2\u031f\u031d\3\2\2\2\u031f\u0320"+
		"\3\2\2\2\u0320\u0323\3\2\2\2\u0321\u031f\3\2\2\2\u0322\u031a\3\2\2\2\u0322"+
		"\u0323\3\2\2\2\u0323\u0324\3\2\2\2\u0324\u032d\7\r\2\2\u0325\u032a\5t"+
		";\2\u0326\u0327\7\36\2\2\u0327\u0329\5t;\2\u0328\u0326\3\2\2\2\u0329\u032c"+
		"\3\2\2\2\u032a\u0328\3\2\2\2\u032a\u032b\3\2\2\2\u032b\u032e\3\2\2\2\u032c"+
		"\u032a\3\2\2\2\u032d\u0325\3\2\2\2\u032d\u032e\3\2\2\2\u032e\u032f\3\2"+
		"\2\2\u032f\u0338\7\r\2\2\u0330\u0335\5v<\2\u0331\u0332\7\36\2\2\u0332"+
		"\u0334\5v<\2\u0333\u0331\3\2\2\2\u0334\u0337\3\2\2\2\u0335\u0333\3\2\2"+
		"\2\u0335\u0336\3\2\2\2\u0336\u0339\3\2\2\2\u0337\u0335\3\2\2\2\u0338\u0330"+
		"\3\2\2\2\u0338\u0339\3\2\2\2\u0339\u033a\3\2\2\2\u033a\u033b\7\"\2\2\u033b"+
		"\u033d\3\2\2\2\u033c\u030c\3\2\2\2\u033c\u030f\3\2\2\2\u033c\u0310\3\2"+
		"\2\2\u033c\u0311\3\2\2\2\u033c\u0315\3\2\2\2\u033d\u0346\3\2\2\2\u033e"+
		"\u033f\f\5\2\2\u033f\u0340\7\26\2\2\u0340\u0345\5p9\6\u0341\u0342\f\4"+
		"\2\2\u0342\u0343\7\27\2\2\u0343\u0345\5p9\5\u0344\u033e\3\2\2\2\u0344"+
		"\u0341\3\2\2\2\u0345\u0348\3\2\2\2\u0346\u0344\3\2\2\2\u0346\u0347\3\2"+
		"\2\2\u0347q\3\2\2\2\u0348\u0346\3\2\2\2\u0349\u034a\t\24\2\2\u034a\u034b"+
		"\7\7\2\2\u034b\u034c\5*\26\2\u034cs\3\2\2\2\u034d\u034e\t\25\2\2\u034e"+
		"\u034f\7\7\2\2\u034f\u0350\7_\2\2\u0350u\3\2\2\2\u0351\u0352\t\26\2\2"+
		"\u0352\u0353\7\7\2\2\u0353\u0354\5p9\2\u0354w\3\2\2\2\u0355\u0356\t\n"+
		"\2\2\u0356\u035a\t\27\2\2\u0357\u035a\t\30\2\2\u0358\u035a\t\31\2\2\u0359"+
		"\u0355\3\2\2\2\u0359\u0357\3\2\2\2\u0359\u0358\3\2\2\2\u035ay\3\2\2\2"+
		"\u035b\u0362\7\u008a\2\2\u035c\u0362\7\u008b\2\2\u035d\u0362\7\u008c\2"+
		"\2\u035e\u0362\7\u008d\2\2\u035f\u0362\7\u008e\2\2\u0360\u0362\7\u008f"+
		"\2\2\u0361\u035b\3\2\2\2\u0361\u035c\3\2\2\2\u0361\u035d\3\2\2\2\u0361"+
		"\u035e\3\2\2\2\u0361\u035f\3\2\2\2\u0361\u0360\3\2\2\2\u0362{\3\2\2\2"+
		"\u0363\u0364\t\32\2\2\u0364}\3\2\2\2K\u0089\u0092\u009c\u00a4\u00cf\u00d5"+
		"\u0117\u011e\u0126\u0128\u0138\u0140\u015e\u0166\u0168\u01b1\u01bf\u01c1"+
		"\u01dc\u01df\u01e7\u01ec\u01f1\u01f8\u0201\u0204\u020c\u0211\u0216\u021d"+
		"\u0228\u0234\u0239\u023b\u0241\u0256\u0259\u0260\u0268\u026a\u0274\u0279"+
		"\u027c\u0284\u0286\u028f\u0299\u029d\u02a4\u02bd\u02c4\u02d1\u02d4\u02dc"+
		"\u02df\u02e7\u02ee\u02f4\u02f9\u02ff\u0302\u0309\u031f\u0322\u032a\u032d"+
		"\u0335\u0338\u033c\u0344\u0346\u0359\u0361";
	public static final ATN _ATN =
		new ATNDeserializer().deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
		for (int i = 0; i < _ATN.getNumberOfDecisions(); i++) {
			_decisionToDFA[i] = new DFA(_ATN.getDecisionState(i), i);
		}
	}
}