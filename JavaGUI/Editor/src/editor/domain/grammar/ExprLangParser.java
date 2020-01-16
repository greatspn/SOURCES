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
		T__13=1, T__12=2, T__11=3, T__10=4, T__9=5, T__8=6, T__7=7, T__6=8, T__5=9, 
		T__4=10, T__3=11, T__2=12, T__1=13, T__0=14, MUL=15, DIV=16, ADD=17, SUB=18, 
		AND=19, OR=20, POSTINCR=21, POSTDECR=22, HAT=23, NOT=24, IMPLY=25, BIIMPLY=26, 
		COMMA=27, COLON=28, SEMICOLON=29, OP_PAREN=30, CL_PAREN=31, OP_SQPAR=32, 
		CL_SQPAR=33, OP_BRACK=34, CL_BRACK=35, TWODOTS=36, EQUAL=37, NOT_EQUAL=38, 
		LESS=39, GREATER=40, LESS_EQ=41, GREATER_EQ=42, SHARP=43, TRUE=44, FALSE=45, 
		INFINITY=46, CLOCK=47, ACT=48, PROB_TA=49, CONTAINS=50, DONT_CONTAINS=51, 
		COLOR_ORDINAL=52, MULTISET_CARD=53, MULTISET_SUBCLASS=54, MIN_FN=55, MAX_FN=56, 
		MOD_FN=57, POW_FN=58, FRACT_FN=59, FACTORIAL_FN=60, ABS_FN=61, SIN_FN=62, 
		COS_FN=63, TAN_FN=64, ARCSIN_FN=65, ARCCOS_FN=66, ARCTAN_FN=67, EXP_FN=68, 
		LOG_FN=69, SQRT_FN=70, CEIL_FN=71, FLOOR_FN=72, ROUND_FN=73, BINOMIAL_FN=74, 
		IF_FN=75, WHITESPACES=76, ID=77, INT=78, REAL=79, INT_TO_REAL=80, UNION=81, 
		SETMINUS=82, SAFE_DIV=83, LESS_ZEROINF=84, MULTISET_MIN=85, MULTISET_MAX=86, 
		MULTISET_FILTER_NEGATIVES=87, INT_PLACE_ID=88, REAL_PLACE_ID=89, INT_MSET_PLACE_ID=90, 
		REAL_MSET_PLACE_ID=91, TRANSITION_ID=92, INT_CONST_ID=93, REAL_CONST_ID=94, 
		INT_MSET_CONST_ID=95, REAL_MSET_CONST_ID=96, INT_TEMPLATE_ID=97, REAL_TEMPLATE_ID=98, 
		ACTION_ID=99, STATEPROP_ID=100, CLOCK_ID=101, SIMPLECOLORCLASS_ID=102, 
		COLORDOMAIN_ID=103, COLORVAR_ID=104, COLORSUBCLASS_ID=105, COLOR_ID=106, 
		GREATSPN_ALL=107, GREATSPN_SUBCLASS=108, EXISTS=109, FORALL=110, NEXT=111, 
		FUTURE=112, GLOBALLY=113, UNTIL=114, EXISTS_NEXT=115, EXISTS_FUTURE=116, 
		EXISTS_GLOBALLY=117, FORALL_NEXT=118, FORALL_FUTURE=119, FORALL_GLOBALLY=120, 
		POSSIBLY=121, IMPOSSIBLY=122, INVARIANTLY=123, ENABLED=124, BOUNDS=125, 
		DEADLOCK=126, NO_DEADLOCK=127, CTLSTAR_NEXT=128, CTLSTAR_FUTURE=129, CTLSTAR_GLOBALLY=130, 
		CTLSTAR_UNTIL=131, CTLSTAR_EXISTS=132, CTLSTAR_FORALL=133, CTLSTAR_EXISTS_NEXT=134, 
		CTLSTAR_EXISTS_FUTURE=135, CTLSTAR_EXISTS_GLOBALLY=136, CTLSTAR_FORALL_NEXT=137, 
		CTLSTAR_FORALL_FUTURE=138, CTLSTAR_FORALL_GLOBALLY=139, PDF_X_VAR=140, 
		RECT_FN=141, UNIFORM_FN=142, TRIANGULAR_FN=143, ERLANG_FN=144, TRUNCATED_EXP_FN=145, 
		PARETO_FN=146, DIRAC_DELTA_FN=147;
	public static final String[] tokenNames = {
		"<INVALID>", "'ordered'", "'circular'", "'unordered'", "'\\'", "'='", 
		"'enum'", "'when'", "'All'", "'is'", "'E{'", "'|'", "'ever'", "'X{'", 
		"'P{'", "'*'", "'/'", "'+'", "'-'", "'&&'", "'||'", "'++'", "'--'", "'^'", 
		"'!'", "'->'", "'<->'", "','", "':'", "';'", "'('", "')'", "'['", "']'", 
		"'{'", "'}'", "'..'", "'=='", "'!='", "'<'", "'>'", "'<='", "'>='", "'#'", 
		"'True'", "'False'", "'Infinite'", "'clock'", "'Act'", "'PROB_TA'", "'in'", 
		"'!in'", "'CN'", "'Card'", "'Subclass'", "'Min'", "'Max'", "'Mod'", "'Pow'", 
		"'Fract'", "'Factorial'", "'Abs'", "'Sin'", "'Cos'", "'Tan'", "'Asin'", 
		"'Acos'", "'Atan'", "'Exp'", "'Log'", "'Sqrt'", "'Ceil'", "'Floor'", "'Round'", 
		"'Binomial'", "'If'", "WHITESPACES", "ID", "INT", "REAL", "INT_TO_REAL", 
		"UNION", "SETMINUS", "SAFE_DIV", "LESS_ZEROINF", "MULTISET_MIN", "MULTISET_MAX", 
		"MULTISET_FILTER_NEGATIVES", "INT_PLACE_ID", "REAL_PLACE_ID", "INT_MSET_PLACE_ID", 
		"REAL_MSET_PLACE_ID", "TRANSITION_ID", "INT_CONST_ID", "REAL_CONST_ID", 
		"INT_MSET_CONST_ID", "REAL_MSET_CONST_ID", "INT_TEMPLATE_ID", "REAL_TEMPLATE_ID", 
		"ACTION_ID", "STATEPROP_ID", "CLOCK_ID", "SIMPLECOLORCLASS_ID", "COLORDOMAIN_ID", 
		"COLORVAR_ID", "COLORSUBCLASS_ID", "COLOR_ID", "GREATSPN_ALL", "GREATSPN_SUBCLASS", 
		"EXISTS", "FORALL", "NEXT", "FUTURE", "GLOBALLY", "UNTIL", "EXISTS_NEXT", 
		"EXISTS_FUTURE", "EXISTS_GLOBALLY", "FORALL_NEXT", "FORALL_FUTURE", "FORALL_GLOBALLY", 
		"POSSIBLY", "IMPOSSIBLY", "INVARIANTLY", "ENABLED", "BOUNDS", "DEADLOCK", 
		"NO_DEADLOCK", "CTLSTAR_NEXT", "CTLSTAR_FUTURE", "CTLSTAR_GLOBALLY", "CTLSTAR_UNTIL", 
		"CTLSTAR_EXISTS", "CTLSTAR_FORALL", "CTLSTAR_EXISTS_NEXT", "CTLSTAR_EXISTS_FUTURE", 
		"CTLSTAR_EXISTS_GLOBALLY", "CTLSTAR_FORALL_NEXT", "CTLSTAR_FORALL_FUTURE", 
		"CTLSTAR_FORALL_GLOBALLY", "PDF_X_VAR", "RECT_FN", "UNIFORM_FN", "TRIANGULAR_FN", 
		"ERLANG_FN", "TRUNCATED_EXP_FN", "PARETO_FN", "DIRAC_DELTA_FN"
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
		RULE_mSetPredicate = 30, RULE_realMSetExpr = 31, RULE_intMSetExpr = 32, 
		RULE_multiSetElem = 33, RULE_colorVar = 34, RULE_colorSubclass = 35, RULE_colorTerm = 36, 
		RULE_colorSet = 37, RULE_colorClassDef = 38, RULE_colorClassOrd = 39, 
		RULE_colorList = 40, RULE_colorSubclassName = 41, RULE_colorVarDef = 42, 
		RULE_placeColorDomainDef = 43, RULE_actList = 44, RULE_actSet = 45, RULE_actBinding = 46, 
		RULE_clockVar = 47, RULE_clockConstr = 48, RULE_varUpdate = 49, RULE_varUpdateList = 50, 
		RULE_varFlowList = 51, RULE_varFlow = 52, RULE_measure = 53, RULE_csltaExpr = 54, 
		RULE_real_assign = 55, RULE_act_assign = 56, RULE_sp_assign = 57, RULE_temporal_op = 58, 
		RULE_composite_temporal_op_ctlstar = 59, RULE_anyID = 60;
	public static final String[] ruleNames = {
		"mainIntExpr", "mainRealExpr", "mainBoolExpr", "mainIntOrInfiniteExpr", 
		"mainIntExprList", "mainRealExprList", "mainClockConstr", "mainActSet", 
		"mainStatePropExpr", "mainActBinding", "mainVarFlow", "mainClockDef", 
		"mainMeasure", "mainCsltaExpr", "mainColorClassDef", "mainColorVarDef", 
		"mainPlaceColorDomainDef", "mainRealMSetExpr", "mainIntMSetExpr", "intConst", 
		"realConst", "boolConst", "intExpr", "realExpr", "boolExpr", "unaryIntFn", 
		"unaryIntRealFn", "binaryIntFn", "binaryRealFn", "unaryRealFn", "mSetPredicate", 
		"realMSetExpr", "intMSetExpr", "multiSetElem", "colorVar", "colorSubclass", 
		"colorTerm", "colorSet", "colorClassDef", "colorClassOrd", "colorList", 
		"colorSubclassName", "colorVarDef", "placeColorDomainDef", "actList", 
		"actSet", "actBinding", "clockVar", "clockConstr", "varUpdate", "varUpdateList", 
		"varFlowList", "varFlow", "measure", "csltaExpr", "real_assign", "act_assign", 
		"sp_assign", "temporal_op", "composite_temporal_op_ctlstar", "anyID"
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
			setState(122); intExpr(0);
			setState(123); match(EOF);
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
			setState(125); realExpr(0);
			setState(126); match(EOF);
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
			setState(128); boolExpr(0);
			setState(129); match(EOF);
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
			setState(133);
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
				setState(131); intExpr(0);
				}
				break;
			case INFINITY:
				{
				setState(132); match(INFINITY);
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
			setState(135); match(EOF);
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
			setState(137); intExpr(0);
			setState(142);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==COMMA) {
				{
				{
				setState(138); match(COMMA);
				setState(139); intExpr(0);
				}
				}
				setState(144);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			setState(145); match(EOF);
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
			setState(147); realExpr(0);
			setState(152);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==COMMA) {
				{
				{
				setState(148); match(COMMA);
				setState(149); realExpr(0);
				}
				}
				setState(154);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			setState(155); match(EOF);
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
			setState(157); clockConstr(0);
			setState(160);
			_la = _input.LA(1);
			if (_la==SEMICOLON) {
				{
				setState(158); match(SEMICOLON);
				setState(159); varUpdateList();
				}
			}

			setState(162); match(EOF);
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
			setState(164); actSet();
			setState(165); match(EOF);
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
			setState(167); boolExpr(0);
			setState(168); match(EOF);
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
			setState(170); actBinding();
			setState(171); match(EOF);
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
			setState(173); varFlowList();
			setState(174); match(EOF);
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
			setState(176); match(CLOCK);
			setState(177); match(EOF);
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
			setState(179); measure(0);
			setState(180); match(EOF);
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
			setState(182); csltaExpr(0);
			setState(183); match(EOF);
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
			setState(185); colorClassDef();
			setState(186); match(EOF);
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
			setState(188); colorVarDef();
			setState(189); match(EOF);
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
			setState(191); placeColorDomainDef();
			setState(192); match(EOF);
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
			setState(194); realMSetExpr(0);
			setState(195); match(EOF);
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
			setState(197); intMSetExpr(0);
			setState(198); match(EOF);
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
			setState(203);
			switch (_input.LA(1)) {
			case INT:
				_localctx = new IntConstLiteralContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(200); match(INT);
				}
				break;
			case INT_CONST_ID:
				_localctx = new IntConstIdContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(201); match(INT_CONST_ID);
				}
				break;
			case INT_TEMPLATE_ID:
				_localctx = new IntTemplateIdContext(_localctx);
				enterOuterAlt(_localctx, 3);
				{
				setState(202); match(INT_TEMPLATE_ID);
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
			setState(209);
			switch (_input.LA(1)) {
			case INT:
			case INT_CONST_ID:
			case INT_TEMPLATE_ID:
				_localctx = new RealFromIntConstContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(205); intConst();
				}
				break;
			case REAL:
				_localctx = new RealConstLiteralContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(206); match(REAL);
				}
				break;
			case REAL_CONST_ID:
				_localctx = new RealConstIdContext(_localctx);
				enterOuterAlt(_localctx, 3);
				{
				setState(207); match(REAL_CONST_ID);
				}
				break;
			case REAL_TEMPLATE_ID:
				_localctx = new RealTemplateIdContext(_localctx);
				enterOuterAlt(_localctx, 4);
				{
				setState(208); match(REAL_TEMPLATE_ID);
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
			setState(211);
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
			setState(282);
			switch ( getInterpreter().adaptivePredict(_input,7,_ctx) ) {
			case 1:
				{
				_localctx = new IntExprNegateContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(214); match(SUB);
				setState(215); intExpr(14);
				}
				break;

			case 2:
				{
				_localctx = new IntExprConstContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(216); intConst();
				}
				break;

			case 3:
				{
				_localctx = new IntExprPlaceMarkingContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(217); match(SHARP);
				setState(218); match(INT_PLACE_ID);
				}
				break;

			case 4:
				{
				_localctx = new IntExprParenContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(219); match(OP_PAREN);
				setState(220); intExpr(0);
				setState(221); match(CL_PAREN);
				}
				break;

			case 5:
				{
				_localctx = new IntExprUnaryFnContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(223); unaryIntFn();
				setState(224); match(OP_SQPAR);
				setState(225); intExpr(0);
				setState(226); match(CL_SQPAR);
				}
				break;

			case 6:
				{
				_localctx = new IntExprUnaryRealFnContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(228); unaryIntRealFn();
				setState(229); match(OP_SQPAR);
				setState(230); realExpr(0);
				setState(231); match(CL_SQPAR);
				}
				break;

			case 7:
				{
				_localctx = new IntExprBinaryFnContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(233); binaryIntFn();
				setState(234); match(OP_SQPAR);
				setState(235); intExpr(0);
				setState(236); match(COMMA);
				setState(237); intExpr(0);
				setState(238); match(CL_SQPAR);
				}
				break;

			case 8:
				{
				_localctx = new IntExprCondContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(240); match(IF_FN);
				setState(241); match(OP_SQPAR);
				setState(242); boolExpr(0);
				setState(243); match(COMMA);
				setState(244); intExpr(0);
				setState(245); match(COMMA);
				setState(246); intExpr(0);
				setState(247); match(CL_SQPAR);
				}
				break;

			case 9:
				{
				_localctx = new IntExprCond2Context(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(249); match(7);
				setState(250); boolExpr(0);
				setState(251); match(COLON);
				setState(252); intExpr(0);
				setState(253); match(SEMICOLON);
				setState(254); match(12);
				setState(255); intExpr(0);
				setState(256); match(SEMICOLON);
				}
				break;

			case 10:
				{
				_localctx = new IntExprCardMSetContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(258); match(MULTISET_CARD);
				setState(259); match(OP_SQPAR);
				setState(260); intMSetExpr(0);
				setState(261); match(CL_SQPAR);
				}
				break;

			case 11:
				{
				_localctx = new IntExprColorNumContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(263); match(COLOR_ORDINAL);
				setState(264); match(OP_SQPAR);
				setState(265); colorTerm(0);
				setState(266); match(CL_SQPAR);
				}
				break;

			case 12:
				{
				_localctx = new IntExprCTLBoundContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(268); match(BOUNDS);
				setState(269); match(OP_PAREN);
				setState(270); match(INT_PLACE_ID);
				setState(275);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==COMMA) {
					{
					{
					setState(271); match(COMMA);
					setState(272); match(INT_PLACE_ID);
					}
					}
					setState(277);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				setState(278); match(CL_PAREN);
				}
				break;

			case 13:
				{
				_localctx = new IntExprUnknownIdContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(279); match(ID);
				}
				break;

			case 14:
				{
				_localctx = new IntExprUnknownPlaceIdContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(280); match(SHARP);
				setState(281); match(ID);
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(292);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,9,_ctx);
			while ( _alt!=2 && _alt!=-1 ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					setState(290);
					switch ( getInterpreter().adaptivePredict(_input,8,_ctx) ) {
					case 1:
						{
						_localctx = new IntExprMulDivContext(new IntExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_intExpr);
						setState(284);
						if (!(precpred(_ctx, 16))) throw new FailedPredicateException(this, "precpred(_ctx, 16)");
						setState(285);
						((IntExprMulDivContext)_localctx).op = _input.LT(1);
						_la = _input.LA(1);
						if ( !(_la==MUL || _la==DIV) ) {
							((IntExprMulDivContext)_localctx).op = (Token)_errHandler.recoverInline(this);
						}
						consume();
						setState(286); intExpr(17);
						}
						break;

					case 2:
						{
						_localctx = new IntExprAddSubContext(new IntExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_intExpr);
						setState(287);
						if (!(precpred(_ctx, 15))) throw new FailedPredicateException(this, "precpred(_ctx, 15)");
						setState(288);
						((IntExprAddSubContext)_localctx).op = _input.LT(1);
						_la = _input.LA(1);
						if ( !(_la==ADD || _la==SUB) ) {
							((IntExprAddSubContext)_localctx).op = (Token)_errHandler.recoverInline(this);
						}
						consume();
						setState(289); intExpr(16);
						}
						break;
					}
					} 
				}
				setState(294);
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
			setState(346);
			switch ( getInterpreter().adaptivePredict(_input,12,_ctx) ) {
			case 1:
				{
				_localctx = new RealExprNegateContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(296); match(SUB);
				setState(297); realExpr(12);
				}
				break;

			case 2:
				{
				_localctx = new RealExprConstContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(298); realConst();
				}
				break;

			case 3:
				{
				_localctx = new RealExprPlaceMarkingContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(299); match(SHARP);
				setState(300); match(REAL_PLACE_ID);
				}
				break;

			case 4:
				{
				_localctx = new RealExprIntContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(301); intExpr(0);
				}
				break;

			case 5:
				{
				_localctx = new RealExprParenContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(302); match(OP_PAREN);
				setState(303); realExpr(0);
				setState(304); match(CL_PAREN);
				}
				break;

			case 6:
				{
				_localctx = new RealExprUnaryFnContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(308);
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
					setState(306); unaryRealFn();
					}
					break;
				case FACTORIAL_FN:
				case ABS_FN:
					{
					setState(307); unaryIntFn();
					}
					break;
				default:
					throw new NoViableAltException(this);
				}
				setState(310); match(OP_SQPAR);
				setState(311); realExpr(0);
				setState(312); match(CL_SQPAR);
				}
				break;

			case 7:
				{
				_localctx = new RealExprBinaryFnContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(316);
				switch (_input.LA(1)) {
				case MIN_FN:
				case MAX_FN:
				case MOD_FN:
				case POW_FN:
				case FRACT_FN:
				case BINOMIAL_FN:
					{
					setState(314); binaryIntFn();
					}
					break;
				case RECT_FN:
				case UNIFORM_FN:
				case TRIANGULAR_FN:
				case ERLANG_FN:
				case TRUNCATED_EXP_FN:
				case PARETO_FN:
					{
					setState(315); binaryRealFn();
					}
					break;
				default:
					throw new NoViableAltException(this);
				}
				setState(318); match(OP_SQPAR);
				setState(319); realExpr(0);
				setState(320); match(COMMA);
				setState(321); realExpr(0);
				setState(322); match(CL_SQPAR);
				}
				break;

			case 8:
				{
				_localctx = new RealExprCondContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(324); match(IF_FN);
				setState(325); match(OP_SQPAR);
				setState(326); boolExpr(0);
				setState(327); match(COMMA);
				setState(328); realExpr(0);
				setState(329); match(COMMA);
				setState(330); realExpr(0);
				setState(331); match(CL_SQPAR);
				}
				break;

			case 9:
				{
				_localctx = new RealExprCond2Context(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(333); match(7);
				setState(334); boolExpr(0);
				setState(335); match(COLON);
				setState(336); realExpr(0);
				setState(337); match(SEMICOLON);
				setState(338); match(12);
				setState(339); realExpr(0);
				setState(340); match(SEMICOLON);
				}
				break;

			case 10:
				{
				_localctx = new RealExprPdfXVarContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(342); match(PDF_X_VAR);
				}
				break;

			case 11:
				{
				_localctx = new RealExprClockVarContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(343);
				if (!(clockVarInExpr)) throw new FailedPredicateException(this, "clockVarInExpr");
				setState(344); clockVar();
				}
				break;

			case 12:
				{
				_localctx = new RealExprUnknownIdContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(345); match(ID);
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(356);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,14,_ctx);
			while ( _alt!=2 && _alt!=-1 ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					setState(354);
					switch ( getInterpreter().adaptivePredict(_input,13,_ctx) ) {
					case 1:
						{
						_localctx = new RealExprMulDivContext(new RealExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_realExpr);
						setState(348);
						if (!(precpred(_ctx, 14))) throw new FailedPredicateException(this, "precpred(_ctx, 14)");
						setState(349);
						((RealExprMulDivContext)_localctx).op = _input.LT(1);
						_la = _input.LA(1);
						if ( !(_la==MUL || _la==DIV) ) {
							((RealExprMulDivContext)_localctx).op = (Token)_errHandler.recoverInline(this);
						}
						consume();
						setState(350); realExpr(15);
						}
						break;

					case 2:
						{
						_localctx = new RealExprAddSubContext(new RealExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_realExpr);
						setState(351);
						if (!(precpred(_ctx, 13))) throw new FailedPredicateException(this, "precpred(_ctx, 13)");
						setState(352);
						((RealExprAddSubContext)_localctx).op = _input.LT(1);
						_la = _input.LA(1);
						if ( !(_la==ADD || _la==SUB) ) {
							((RealExprAddSubContext)_localctx).op = (Token)_errHandler.recoverInline(this);
						}
						consume();
						setState(353); realExpr(14);
						}
						break;
					}
					} 
				}
				setState(358);
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
			setState(428);
			switch ( getInterpreter().adaptivePredict(_input,15,_ctx) ) {
			case 1:
				{
				_localctx = new BoolExprNotContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(360); match(NOT);
				setState(361); boolExpr(24);
				}
				break;

			case 2:
				{
				_localctx = new BoolExprCTLContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(362); temporal_op();
				setState(363); boolExpr(11);
				}
				break;

			case 3:
				{
				_localctx = new BoolExprCTLpinContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(365);
				((BoolExprCTLpinContext)_localctx).pin = _input.LT(1);
				_la = _input.LA(1);
				if ( !(((((_la - 121)) & ~0x3f) == 0 && ((1L << (_la - 121)) & ((1L << (POSSIBLY - 121)) | (1L << (IMPOSSIBLY - 121)) | (1L << (INVARIANTLY - 121)))) != 0)) ) {
					((BoolExprCTLpinContext)_localctx).pin = (Token)_errHandler.recoverInline(this);
				}
				consume();
				setState(366); boolExpr(8);
				}
				break;

			case 4:
				{
				_localctx = new BoolExprCTLStarContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(367);
				((BoolExprCTLStarContext)_localctx).op = _input.LT(1);
				_la = _input.LA(1);
				if ( !(((((_la - 128)) & ~0x3f) == 0 && ((1L << (_la - 128)) & ((1L << (CTLSTAR_NEXT - 128)) | (1L << (CTLSTAR_FUTURE - 128)) | (1L << (CTLSTAR_GLOBALLY - 128)))) != 0)) ) {
					((BoolExprCTLStarContext)_localctx).op = (Token)_errHandler.recoverInline(this);
				}
				consume();
				setState(368); boolExpr(5);
				}
				break;

			case 5:
				{
				_localctx = new BoolExprCTLStarQuantifContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(369);
				((BoolExprCTLStarQuantifContext)_localctx).q = _input.LT(1);
				_la = _input.LA(1);
				if ( !(_la==CTLSTAR_EXISTS || _la==CTLSTAR_FORALL) ) {
					((BoolExprCTLStarQuantifContext)_localctx).q = (Token)_errHandler.recoverInline(this);
				}
				consume();
				setState(370); boolExpr(2);
				}
				break;

			case 6:
				{
				_localctx = new BoolExprCTLStar2Context(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(371); composite_temporal_op_ctlstar();
				setState(372); boolExpr(1);
				}
				break;

			case 7:
				{
				_localctx = new BoolExprConstContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(374); boolConst();
				}
				break;

			case 8:
				{
				_localctx = new BoolExprIntCompContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(375); intExpr(0);
				setState(376);
				((BoolExprIntCompContext)_localctx).op = _input.LT(1);
				_la = _input.LA(1);
				if ( !((((_la) & ~0x3f) == 0 && ((1L << _la) & ((1L << EQUAL) | (1L << NOT_EQUAL) | (1L << LESS) | (1L << GREATER) | (1L << LESS_EQ) | (1L << GREATER_EQ))) != 0)) ) {
					((BoolExprIntCompContext)_localctx).op = (Token)_errHandler.recoverInline(this);
				}
				consume();
				setState(377); intExpr(0);
				}
				break;

			case 9:
				{
				_localctx = new BoolExprRealCompContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(379); realExpr(0);
				setState(380);
				((BoolExprRealCompContext)_localctx).op = _input.LT(1);
				_la = _input.LA(1);
				if ( !((((_la) & ~0x3f) == 0 && ((1L << _la) & ((1L << EQUAL) | (1L << NOT_EQUAL) | (1L << LESS) | (1L << GREATER) | (1L << LESS_EQ) | (1L << GREATER_EQ))) != 0)) ) {
					((BoolExprRealCompContext)_localctx).op = (Token)_errHandler.recoverInline(this);
				}
				consume();
				setState(381); realExpr(0);
				}
				break;

			case 10:
				{
				_localctx = new BoolExprParenContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(383); match(OP_PAREN);
				setState(384); boolExpr(0);
				setState(385); match(CL_PAREN);
				}
				break;

			case 11:
				{
				_localctx = new BoolExprUnknownIdContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(387); match(ID);
				}
				break;

			case 12:
				{
				_localctx = new BoolExprStatePropIdContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(388); match(STATEPROP_ID);
				}
				break;

			case 13:
				{
				_localctx = new BoolExprColorTermCompContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(389); colorTerm(0);
				setState(390);
				((BoolExprColorTermCompContext)_localctx).op = _input.LT(1);
				_la = _input.LA(1);
				if ( !((((_la) & ~0x3f) == 0 && ((1L << _la) & ((1L << EQUAL) | (1L << NOT_EQUAL) | (1L << LESS) | (1L << GREATER) | (1L << LESS_EQ) | (1L << GREATER_EQ))) != 0)) ) {
					((BoolExprColorTermCompContext)_localctx).op = (Token)_errHandler.recoverInline(this);
				}
				consume();
				setState(391); colorTerm(0);
				}
				break;

			case 14:
				{
				_localctx = new BoolExprColorTermInContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(393); colorTerm(0);
				setState(394);
				_la = _input.LA(1);
				if ( !(_la==CONTAINS || _la==DONT_CONTAINS) ) {
				_errHandler.recoverInline(this);
				}
				consume();
				setState(395); colorSet();
				}
				break;

			case 15:
				{
				_localctx = new BoolExprCTLUntilContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(397);
				_la = _input.LA(1);
				if ( !(_la==EXISTS || _la==FORALL) ) {
				_errHandler.recoverInline(this);
				}
				consume();
				setState(398); match(OP_PAREN);
				setState(399); boolExpr(0);
				setState(400); match(UNTIL);
				setState(401); boolExpr(0);
				setState(402); match(CL_PAREN);
				}
				break;

			case 16:
				{
				_localctx = new BoolExprCTLUntil2Context(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(404);
				_la = _input.LA(1);
				if ( !(_la==EXISTS || _la==FORALL) ) {
				_errHandler.recoverInline(this);
				}
				consume();
				setState(405); match(OP_SQPAR);
				setState(406); boolExpr(0);
				setState(407); match(UNTIL);
				setState(408); boolExpr(0);
				setState(409); match(CL_SQPAR);
				}
				break;

			case 17:
				{
				_localctx = new BoolExprCTLenabledContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(411); match(ENABLED);
				setState(412); match(OP_PAREN);
				setState(413); match(TRANSITION_ID);
				setState(414); match(CL_PAREN);
				}
				break;

			case 18:
				{
				_localctx = new BoolExprCTLdeadlocksContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(415);
				_la = _input.LA(1);
				if ( !(_la==DEADLOCK || _la==NO_DEADLOCK) ) {
				_errHandler.recoverInline(this);
				}
				consume();
				}
				break;

			case 19:
				{
				_localctx = new BoolExprCTLStarUntilContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(416); match(OP_PAREN);
				setState(417); boolExpr(0);
				setState(418); match(CTLSTAR_UNTIL);
				setState(419); boolExpr(0);
				setState(420); match(CL_PAREN);
				}
				break;

			case 20:
				{
				_localctx = new BoolExprCTLStarUntil2Context(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(422); match(OP_SQPAR);
				setState(423); boolExpr(0);
				setState(424); match(CTLSTAR_UNTIL);
				setState(425); boolExpr(0);
				setState(426); match(CL_SQPAR);
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(444);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,17,_ctx);
			while ( _alt!=2 && _alt!=-1 ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					setState(442);
					switch ( getInterpreter().adaptivePredict(_input,16,_ctx) ) {
					case 1:
						{
						_localctx = new BoolExprAndContext(new BoolExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_boolExpr);
						setState(430);
						if (!(precpred(_ctx, 23))) throw new FailedPredicateException(this, "precpred(_ctx, 23)");
						setState(431); match(AND);
						setState(432); boolExpr(24);
						}
						break;

					case 2:
						{
						_localctx = new BoolExprOrContext(new BoolExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_boolExpr);
						setState(433);
						if (!(precpred(_ctx, 22))) throw new FailedPredicateException(this, "precpred(_ctx, 22)");
						setState(434); match(OR);
						setState(435); boolExpr(23);
						}
						break;

					case 3:
						{
						_localctx = new BoolExprImplyContext(new BoolExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_boolExpr);
						setState(436);
						if (!(precpred(_ctx, 21))) throw new FailedPredicateException(this, "precpred(_ctx, 21)");
						setState(437); match(IMPLY);
						setState(438); boolExpr(22);
						}
						break;

					case 4:
						{
						_localctx = new BoolExprBiimplyContext(new BoolExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_boolExpr);
						setState(439);
						if (!(precpred(_ctx, 20))) throw new FailedPredicateException(this, "precpred(_ctx, 20)");
						setState(440); match(BIIMPLY);
						setState(441); boolExpr(21);
						}
						break;
					}
					} 
				}
				setState(446);
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
			setState(447);
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
			setState(449);
			((UnaryIntRealFnContext)_localctx).fn = _input.LT(1);
			_la = _input.LA(1);
			if ( !(((((_la - 71)) & ~0x3f) == 0 && ((1L << (_la - 71)) & ((1L << (CEIL_FN - 71)) | (1L << (FLOOR_FN - 71)) | (1L << (ROUND_FN - 71)))) != 0)) ) {
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
			setState(451);
			((BinaryIntFnContext)_localctx).fn = _input.LT(1);
			_la = _input.LA(1);
			if ( !(((((_la - 55)) & ~0x3f) == 0 && ((1L << (_la - 55)) & ((1L << (MIN_FN - 55)) | (1L << (MAX_FN - 55)) | (1L << (MOD_FN - 55)) | (1L << (POW_FN - 55)) | (1L << (FRACT_FN - 55)) | (1L << (BINOMIAL_FN - 55)))) != 0)) ) {
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
			setState(453);
			((BinaryRealFnContext)_localctx).fn = _input.LT(1);
			_la = _input.LA(1);
			if ( !(((((_la - 141)) & ~0x3f) == 0 && ((1L << (_la - 141)) & ((1L << (RECT_FN - 141)) | (1L << (UNIFORM_FN - 141)) | (1L << (TRIANGULAR_FN - 141)) | (1L << (ERLANG_FN - 141)) | (1L << (TRUNCATED_EXP_FN - 141)) | (1L << (PARETO_FN - 141)))) != 0)) ) {
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
			setState(455);
			((UnaryRealFnContext)_localctx).fn = _input.LT(1);
			_la = _input.LA(1);
			if ( !(((((_la - 62)) & ~0x3f) == 0 && ((1L << (_la - 62)) & ((1L << (SIN_FN - 62)) | (1L << (COS_FN - 62)) | (1L << (TAN_FN - 62)) | (1L << (ARCSIN_FN - 62)) | (1L << (ARCCOS_FN - 62)) | (1L << (ARCTAN_FN - 62)) | (1L << (EXP_FN - 62)) | (1L << (SQRT_FN - 62)))) != 0) || _la==DIRAC_DELTA_FN) ) {
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
			setState(457); match(OP_SQPAR);
			setState(458); boolExpr(0);
			setState(459); match(CL_SQPAR);
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
		int _startState = 62;
		enterRecursionRule(_localctx, 62, RULE_realMSetExpr, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(486);
			switch ( getInterpreter().adaptivePredict(_input,21,_ctx) ) {
			case 1:
				{
				_localctx = new RealMSetExprParenContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(462); match(OP_PAREN);
				setState(463); realMSetExpr(0);
				setState(464); match(CL_PAREN);
				}
				break;

			case 2:
				{
				_localctx = new RealMSetExprElemProductContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(467);
				switch ( getInterpreter().adaptivePredict(_input,18,_ctx) ) {
				case 1:
					{
					setState(466); realExpr(0);
					}
					break;
				}
				setState(470);
				_la = _input.LA(1);
				if (_la==OP_SQPAR) {
					{
					setState(469); mSetPredicate();
					}
				}

				setState(472); match(LESS);
				setState(473); multiSetElem(0);
				setState(478);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==COMMA) {
					{
					{
					setState(474); match(COMMA);
					setState(475); multiSetElem(0);
					}
					}
					setState(480);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				setState(481); match(GREATER);
				}
				break;

			case 3:
				{
				_localctx = new RealMsetExprPlaceMarkingContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(483); match(SHARP);
				setState(484); match(REAL_MSET_PLACE_ID);
				}
				break;

			case 4:
				{
				_localctx = new RealMsetExprConstContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(485); match(REAL_MSET_CONST_ID);
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(493);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,22,_ctx);
			while ( _alt!=2 && _alt!=-1 ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					{
					_localctx = new RealMSetExprAddSubContext(new RealMSetExprContext(_parentctx, _parentState));
					pushNewRecursionContext(_localctx, _startState, RULE_realMSetExpr);
					setState(488);
					if (!(precpred(_ctx, 4))) throw new FailedPredicateException(this, "precpred(_ctx, 4)");
					setState(489);
					((RealMSetExprAddSubContext)_localctx).op = _input.LT(1);
					_la = _input.LA(1);
					if ( !(_la==ADD || _la==SUB) ) {
						((RealMSetExprAddSubContext)_localctx).op = (Token)_errHandler.recoverInline(this);
					}
					consume();
					setState(490); realMSetExpr(5);
					}
					} 
				}
				setState(495);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,22,_ctx);
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
		int _startState = 64;
		enterRecursionRule(_localctx, 64, RULE_intMSetExpr, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(521);
			switch ( getInterpreter().adaptivePredict(_input,26,_ctx) ) {
			case 1:
				{
				_localctx = new IntMSetExprParenContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(497); match(OP_PAREN);
				setState(498); intMSetExpr(0);
				setState(499); match(CL_PAREN);
				}
				break;

			case 2:
				{
				_localctx = new IntMSetExprElemProductContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(502);
				_la = _input.LA(1);
				if ((((_la) & ~0x3f) == 0 && ((1L << _la) & ((1L << 7) | (1L << SUB) | (1L << OP_PAREN) | (1L << SHARP) | (1L << COLOR_ORDINAL) | (1L << MULTISET_CARD) | (1L << MIN_FN) | (1L << MAX_FN) | (1L << MOD_FN) | (1L << POW_FN) | (1L << FRACT_FN) | (1L << FACTORIAL_FN) | (1L << ABS_FN))) != 0) || ((((_la - 71)) & ~0x3f) == 0 && ((1L << (_la - 71)) & ((1L << (CEIL_FN - 71)) | (1L << (FLOOR_FN - 71)) | (1L << (ROUND_FN - 71)) | (1L << (BINOMIAL_FN - 71)) | (1L << (IF_FN - 71)) | (1L << (ID - 71)) | (1L << (INT - 71)) | (1L << (INT_CONST_ID - 71)) | (1L << (INT_TEMPLATE_ID - 71)) | (1L << (BOUNDS - 71)))) != 0)) {
					{
					setState(501); intExpr(0);
					}
				}

				setState(505);
				_la = _input.LA(1);
				if (_la==OP_SQPAR) {
					{
					setState(504); mSetPredicate();
					}
				}

				setState(507); match(LESS);
				setState(508); multiSetElem(0);
				setState(513);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==COMMA) {
					{
					{
					setState(509); match(COMMA);
					setState(510); multiSetElem(0);
					}
					}
					setState(515);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				setState(516); match(GREATER);
				}
				break;

			case 3:
				{
				_localctx = new IntMsetExprPlaceMarkingContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(518); match(SHARP);
				setState(519); match(INT_MSET_PLACE_ID);
				}
				break;

			case 4:
				{
				_localctx = new IntMSetExprConstContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(520); match(INT_MSET_CONST_ID);
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(528);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,27,_ctx);
			while ( _alt!=2 && _alt!=-1 ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					{
					_localctx = new IntMSetExprAddSubContext(new IntMSetExprContext(_parentctx, _parentState));
					pushNewRecursionContext(_localctx, _startState, RULE_intMSetExpr);
					setState(523);
					if (!(precpred(_ctx, 4))) throw new FailedPredicateException(this, "precpred(_ctx, 4)");
					setState(524);
					((IntMSetExprAddSubContext)_localctx).op = _input.LT(1);
					_la = _input.LA(1);
					if ( !(_la==ADD || _la==SUB) ) {
						((IntMSetExprAddSubContext)_localctx).op = (Token)_errHandler.recoverInline(this);
					}
					consume();
					setState(525); intMSetExpr(5);
					}
					} 
				}
				setState(530);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,27,_ctx);
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
		int _startState = 66;
		enterRecursionRule(_localctx, 66, RULE_multiSetElem, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			{
			_localctx = new MSetElemColorTermContext(_localctx);
			_ctx = _localctx;
			_prevctx = _localctx;

			setState(532); colorSet();
			}
			_ctx.stop = _input.LT(-1);
			setState(539);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,28,_ctx);
			while ( _alt!=2 && _alt!=-1 ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					{
					_localctx = new MSetElemAddSubContext(new MultiSetElemContext(_parentctx, _parentState));
					pushNewRecursionContext(_localctx, _startState, RULE_multiSetElem);
					setState(534);
					if (!(precpred(_ctx, 2))) throw new FailedPredicateException(this, "precpred(_ctx, 2)");
					setState(535);
					((MSetElemAddSubContext)_localctx).op = _input.LT(1);
					_la = _input.LA(1);
					if ( !(_la==ADD || _la==SUB) ) {
						((MSetElemAddSubContext)_localctx).op = (Token)_errHandler.recoverInline(this);
					}
					consume();
					setState(536); multiSetElem(3);
					}
					} 
				}
				setState(541);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,28,_ctx);
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
		enterRule(_localctx, 68, RULE_colorVar);
		try {
			_localctx = new ColorVarIdentifierContext(_localctx);
			enterOuterAlt(_localctx, 1);
			{
			setState(542); match(COLORVAR_ID);
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
		enterRule(_localctx, 70, RULE_colorSubclass);
		try {
			_localctx = new ColorSubclassIdentifierContext(_localctx);
			enterOuterAlt(_localctx, 1);
			{
			setState(544); match(COLORSUBCLASS_ID);
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

	public final ColorTermContext colorTerm() throws RecognitionException {
		return colorTerm(0);
	}

	private ColorTermContext colorTerm(int _p) throws RecognitionException {
		ParserRuleContext _parentctx = _ctx;
		int _parentState = getState();
		ColorTermContext _localctx = new ColorTermContext(_ctx, _parentState);
		ColorTermContext _prevctx = _localctx;
		int _startState = 72;
		enterRecursionRule(_localctx, 72, RULE_colorTerm, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(549);
			switch (_input.LA(1)) {
			case COLORVAR_ID:
				{
				_localctx = new ColorTermVarContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(547); colorVar();
				}
				break;
			case COLOR_ID:
				{
				_localctx = new ColorTermColorContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(548); match(COLOR_ID);
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
			_ctx.stop = _input.LT(-1);
			setState(555);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,30,_ctx);
			while ( _alt!=2 && _alt!=-1 ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					{
					_localctx = new ColorTermNextPrevContext(new ColorTermContext(_parentctx, _parentState));
					pushNewRecursionContext(_localctx, _startState, RULE_colorTerm);
					setState(551);
					if (!(precpred(_ctx, 3))) throw new FailedPredicateException(this, "precpred(_ctx, 3)");
					setState(552);
					((ColorTermNextPrevContext)_localctx).op = _input.LT(1);
					_la = _input.LA(1);
					if ( !(_la==POSTINCR || _la==POSTDECR) ) {
						((ColorTermNextPrevContext)_localctx).op = (Token)_errHandler.recoverInline(this);
					}
					consume();
					}
					} 
				}
				setState(557);
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
		enterRule(_localctx, 74, RULE_colorSet);
		try {
			setState(576);
			switch ( getInterpreter().adaptivePredict(_input,31,_ctx) ) {
			case 1:
				_localctx = new ColorSetTermContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(558); colorTerm(0);
				}
				break;

			case 2:
				_localctx = new ColorSetAllContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(559); match(8);
				}
				break;

			case 3:
				_localctx = new ColorSetClassContext(_localctx);
				enterOuterAlt(_localctx, 3);
				{
				setState(560); match(SIMPLECOLORCLASS_ID);
				}
				break;

			case 4:
				_localctx = new ColorSetClass2Context(_localctx);
				enterOuterAlt(_localctx, 4);
				{
				setState(561); match(GREATSPN_ALL);
				setState(562); match(SIMPLECOLORCLASS_ID);
				}
				break;

			case 5:
				_localctx = new ColorSetSubClassContext(_localctx);
				enterOuterAlt(_localctx, 5);
				{
				setState(563); colorSubclass();
				}
				break;

			case 6:
				_localctx = new ColorSetSubClass2Context(_localctx);
				enterOuterAlt(_localctx, 6);
				{
				setState(564); match(GREATSPN_ALL);
				setState(565); colorSubclass();
				}
				break;

			case 7:
				_localctx = new ColorSetSubclassOfContext(_localctx);
				enterOuterAlt(_localctx, 7);
				{
				setState(566); match(MULTISET_SUBCLASS);
				setState(567); match(OP_SQPAR);
				setState(568); colorTerm(0);
				setState(569); match(CL_SQPAR);
				}
				break;

			case 8:
				_localctx = new ColorSetSubclassOf2Context(_localctx);
				enterOuterAlt(_localctx, 8);
				{
				setState(571); match(GREATSPN_SUBCLASS);
				setState(572); match(OP_PAREN);
				setState(573); colorTerm(0);
				setState(574); match(CL_PAREN);
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
		enterRule(_localctx, 76, RULE_colorClassDef);
		int _la;
		try {
			setState(596);
			switch ( getInterpreter().adaptivePredict(_input,35,_ctx) ) {
			case 1:
				_localctx = new ColorClassDefNamedSetContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(579);
				_la = _input.LA(1);
				if ((((_la) & ~0x3f) == 0 && ((1L << _la) & ((1L << 1) | (1L << 2) | (1L << 3) | (1L << 6))) != 0)) {
					{
					setState(578); colorClassOrd();
					}
				}

				setState(581); colorList();
				setState(586);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==ADD) {
					{
					{
					setState(582); match(ADD);
					setState(583); colorList();
					}
					}
					setState(588);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				}
				break;

			case 2:
				_localctx = new ColorClassDefProductContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(589); match(SIMPLECOLORCLASS_ID);
				setState(592); 
				_errHandler.sync(this);
				_la = _input.LA(1);
				do {
					{
					{
					setState(590); match(MUL);
					setState(591); match(SIMPLECOLORCLASS_ID);
					}
					}
					setState(594); 
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
		enterRule(_localctx, 78, RULE_colorClassOrd);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(598);
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
		enterRule(_localctx, 80, RULE_colorList);
		int _la;
		try {
			setState(624);
			switch ( getInterpreter().adaptivePredict(_input,40,_ctx) ) {
			case 1:
				_localctx = new ColorListIDsContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(600); match(OP_BRACK);
				setState(601); match(ID);
				setState(606);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==COMMA) {
					{
					{
					setState(602); match(COMMA);
					setState(603); match(ID);
					}
					}
					setState(608);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				setState(609); match(CL_BRACK);
				setState(611);
				_la = _input.LA(1);
				if (_la==9) {
					{
					setState(610); colorSubclassName();
					}
				}

				}
				break;

			case 2:
				_localctx = new ColorListIntervalContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(614);
				_la = _input.LA(1);
				if (((((_la - 77)) & ~0x3f) == 0 && ((1L << (_la - 77)) & ((1L << (ID - 77)) | (1L << (INT_PLACE_ID - 77)) | (1L << (REAL_PLACE_ID - 77)) | (1L << (INT_MSET_PLACE_ID - 77)) | (1L << (REAL_MSET_PLACE_ID - 77)) | (1L << (TRANSITION_ID - 77)) | (1L << (INT_CONST_ID - 77)) | (1L << (REAL_CONST_ID - 77)) | (1L << (INT_MSET_CONST_ID - 77)) | (1L << (REAL_MSET_CONST_ID - 77)) | (1L << (INT_TEMPLATE_ID - 77)) | (1L << (REAL_TEMPLATE_ID - 77)) | (1L << (ACTION_ID - 77)) | (1L << (STATEPROP_ID - 77)) | (1L << (CLOCK_ID - 77)) | (1L << (SIMPLECOLORCLASS_ID - 77)) | (1L << (COLORDOMAIN_ID - 77)) | (1L << (COLORVAR_ID - 77)) | (1L << (COLORSUBCLASS_ID - 77)) | (1L << (COLOR_ID - 77)))) != 0)) {
					{
					setState(613); anyID();
					}
				}

				setState(616); match(OP_BRACK);
				setState(617); intConst();
				setState(618); match(TWODOTS);
				setState(619); intConst();
				setState(620); match(CL_BRACK);
				setState(622);
				_la = _input.LA(1);
				if (_la==9) {
					{
					setState(621); colorSubclassName();
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
		enterRule(_localctx, 82, RULE_colorSubclassName);
		try {
			_localctx = new ColorSubclassNameDefContext(_localctx);
			enterOuterAlt(_localctx, 1);
			{
			setState(626); match(9);
			setState(627); match(ID);
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
		enterRule(_localctx, 84, RULE_colorVarDef);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(629); match(SIMPLECOLORCLASS_ID);
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
		enterRule(_localctx, 86, RULE_placeColorDomainDef);
		try {
			setState(633);
			switch (_input.LA(1)) {
			case SIMPLECOLORCLASS_ID:
				_localctx = new PlaceDomainColorClassContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(631); match(SIMPLECOLORCLASS_ID);
				}
				break;
			case COLORDOMAIN_ID:
				_localctx = new PlaceDomainColorDomainContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(632); match(COLORDOMAIN_ID);
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
		enterRule(_localctx, 88, RULE_actList);
		int _la;
		try {
			setState(647);
			switch ( getInterpreter().adaptivePredict(_input,43,_ctx) ) {
			case 1:
				_localctx = new ActListEmptyContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(635); match(OP_BRACK);
				setState(636); match(CL_BRACK);
				}
				break;

			case 2:
				_localctx = new ActListListContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(637); match(OP_BRACK);
				setState(638); match(ACTION_ID);
				setState(643);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==COMMA) {
					{
					{
					setState(639); match(COMMA);
					setState(640); match(ACTION_ID);
					}
					}
					setState(645);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				setState(646); match(CL_BRACK);
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
		enterRule(_localctx, 90, RULE_actSet);
		try {
			setState(654);
			switch ( getInterpreter().adaptivePredict(_input,44,_ctx) ) {
			case 1:
				_localctx = new ActSetAllContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(649); match(ACT);
				}
				break;

			case 2:
				_localctx = new ActSetAllExceptListContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(650); match(ACT);
				setState(651); match(4);
				setState(652); actList();
				}
				break;

			case 3:
				_localctx = new ActSetListContext(_localctx);
				enterOuterAlt(_localctx, 3);
				{
				setState(653); actList();
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
		enterRule(_localctx, 92, RULE_actBinding);
		try {
			_localctx = new ActBindingTransitionContext(_localctx);
			enterOuterAlt(_localctx, 1);
			{
			setState(656); match(TRANSITION_ID);
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
		enterRule(_localctx, 94, RULE_clockVar);
		try {
			_localctx = new ClockVarIdContext(_localctx);
			enterOuterAlt(_localctx, 1);
			{
			setState(658); match(CLOCK_ID);
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
		int _startState = 96;
		enterRecursionRule(_localctx, 96, RULE_clockConstr, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(679);
			switch ( getInterpreter().adaptivePredict(_input,45,_ctx) ) {
			case 1:
				{
				_localctx = new ClockConstrBetweenContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(661); realExpr(0);
				setState(662); match(LESS);
				setState(663); clockVar();
				setState(664); match(LESS);
				setState(665); realExpr(0);
				}
				break;

			case 2:
				{
				_localctx = new ClockConstrIdCmpRealContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(667); clockVar();
				setState(668);
				((ClockConstrIdCmpRealContext)_localctx).op = _input.LT(1);
				_la = _input.LA(1);
				if ( !(_la==LESS || _la==GREATER) ) {
					((ClockConstrIdCmpRealContext)_localctx).op = (Token)_errHandler.recoverInline(this);
				}
				consume();
				setState(669); realExpr(0);
				}
				break;

			case 3:
				{
				_localctx = new ClockConstrRealCmpIdContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(671); realExpr(0);
				setState(672);
				((ClockConstrRealCmpIdContext)_localctx).op = _input.LT(1);
				_la = _input.LA(1);
				if ( !(_la==LESS || _la==GREATER) ) {
					((ClockConstrRealCmpIdContext)_localctx).op = (Token)_errHandler.recoverInline(this);
				}
				consume();
				setState(673); clockVar();
				}
				break;

			case 4:
				{
				_localctx = new ClockConstrEqualsContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(675); clockVar();
				setState(676); match(5);
				setState(677); realExpr(0);
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(686);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,46,_ctx);
			while ( _alt!=2 && _alt!=-1 ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					{
					_localctx = new ClockConstrAndContext(new ClockConstrContext(_parentctx, _parentState));
					pushNewRecursionContext(_localctx, _startState, RULE_clockConstr);
					setState(681);
					if (!(precpred(_ctx, 1))) throw new FailedPredicateException(this, "precpred(_ctx, 1)");
					setState(682); match(AND);
					setState(683); clockConstr(2);
					}
					} 
				}
				setState(688);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,46,_ctx);
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
		enterRule(_localctx, 98, RULE_varUpdate);
		try {
			_localctx = new VarUpdateValueContext(_localctx);
			enterOuterAlt(_localctx, 1);
			{
			setState(689); clockVar();
			setState(690); match(5);
			setState(691); realExpr(0);
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
		enterRule(_localctx, 100, RULE_varUpdateList);
		int _la;
		try {
			setState(702);
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
				setState(694); varUpdate();
				setState(699);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==COMMA) {
					{
					{
					setState(695); match(COMMA);
					setState(696); varUpdate();
					}
					}
					setState(701);
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
		enterRule(_localctx, 102, RULE_varFlowList);
		int _la;
		try {
			setState(713);
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
				setState(705); varFlow();
				setState(710);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==COMMA) {
					{
					{
					setState(706); match(COMMA);
					setState(707); varFlow();
					}
					}
					setState(712);
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
		enterRule(_localctx, 104, RULE_varFlow);
		try {
			_localctx = new VarFlowDefinitionContext(_localctx);
			enterOuterAlt(_localctx, 1);
			{
			setState(715); clockVar();
			setState(716); match(5);
			setState(717); realExpr(0);
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
		int _startState = 106;
		enterRecursionRule(_localctx, 106, RULE_measure, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(748);
			switch ( getInterpreter().adaptivePredict(_input,56,_ctx) ) {
			case 1:
				{
				_localctx = new MeasurePContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(721);
				_la = _input.LA(1);
				if (((((_la - 78)) & ~0x3f) == 0 && ((1L << (_la - 78)) & ((1L << (INT - 78)) | (1L << (REAL - 78)) | (1L << (INT_CONST_ID - 78)) | (1L << (REAL_CONST_ID - 78)) | (1L << (INT_TEMPLATE_ID - 78)) | (1L << (REAL_TEMPLATE_ID - 78)))) != 0)) {
					{
					setState(720); realConst();
					}
				}

				setState(723); match(14);
				setState(724); boolExpr(0);
				setState(725); match(CL_BRACK);
				}
				break;

			case 2:
				{
				_localctx = new MeasureEContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(728);
				_la = _input.LA(1);
				if (((((_la - 78)) & ~0x3f) == 0 && ((1L << (_la - 78)) & ((1L << (INT - 78)) | (1L << (REAL - 78)) | (1L << (INT_CONST_ID - 78)) | (1L << (REAL_CONST_ID - 78)) | (1L << (INT_TEMPLATE_ID - 78)) | (1L << (REAL_TEMPLATE_ID - 78)))) != 0)) {
					{
					setState(727); realConst();
					}
				}

				setState(730); match(10);
				setState(731); intExpr(0);
				setState(734);
				_la = _input.LA(1);
				if (_la==DIV) {
					{
					setState(732); match(DIV);
					setState(733); boolExpr(0);
					}
				}

				setState(736); match(CL_BRACK);
				}
				break;

			case 3:
				{
				_localctx = new MeasureXContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(739);
				_la = _input.LA(1);
				if (((((_la - 78)) & ~0x3f) == 0 && ((1L << (_la - 78)) & ((1L << (INT - 78)) | (1L << (REAL - 78)) | (1L << (INT_CONST_ID - 78)) | (1L << (REAL_CONST_ID - 78)) | (1L << (INT_TEMPLATE_ID - 78)) | (1L << (REAL_TEMPLATE_ID - 78)))) != 0)) {
					{
					setState(738); realConst();
					}
				}

				setState(741); match(13);
				setState(742); match(TRANSITION_ID);
				setState(745);
				_la = _input.LA(1);
				if (_la==DIV) {
					{
					setState(743); match(DIV);
					setState(744); boolExpr(0);
					}
				}

				setState(747); match(CL_BRACK);
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(755);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,57,_ctx);
			while ( _alt!=2 && _alt!=-1 ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					{
					_localctx = new MeasureAddSubContext(new MeasureContext(_parentctx, _parentState));
					pushNewRecursionContext(_localctx, _startState, RULE_measure);
					setState(750);
					if (!(precpred(_ctx, 4))) throw new FailedPredicateException(this, "precpred(_ctx, 4)");
					setState(751);
					((MeasureAddSubContext)_localctx).op = _input.LT(1);
					_la = _input.LA(1);
					if ( !(_la==ADD || _la==SUB) ) {
						((MeasureAddSubContext)_localctx).op = (Token)_errHandler.recoverInline(this);
					}
					consume();
					setState(752); measure(5);
					}
					} 
				}
				setState(757);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,57,_ctx);
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
		int _startState = 108;
		enterRecursionRule(_localctx, 108, RULE_csltaExpr, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(806);
			switch ( getInterpreter().adaptivePredict(_input,64,_ctx) ) {
			case 1:
				{
				_localctx = new CsltaExprNotContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(759); match(NOT);
				setState(760); csltaExpr(4);
				}
				break;

			case 2:
				{
				_localctx = new CsltaExprBoolContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(761); boolExpr(0);
				}
				break;

			case 3:
				{
				_localctx = new CsltaExprPlaceContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(762); match(INT_PLACE_ID);
				}
				break;

			case 4:
				{
				_localctx = new CsltaExprParenContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(763); match(OP_PAREN);
				setState(764); csltaExpr(0);
				setState(765); match(CL_PAREN);
				}
				break;

			case 5:
				{
				_localctx = new CsltaExprProbTAContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(767); match(PROB_TA);
				setState(768);
				((CsltaExprProbTAContext)_localctx).op = _input.LT(1);
				_la = _input.LA(1);
				if ( !((((_la) & ~0x3f) == 0 && ((1L << _la) & ((1L << EQUAL) | (1L << NOT_EQUAL) | (1L << LESS) | (1L << GREATER) | (1L << LESS_EQ) | (1L << GREATER_EQ))) != 0)) ) {
					((CsltaExprProbTAContext)_localctx).op = (Token)_errHandler.recoverInline(this);
				}
				consume();
				setState(769); ((CsltaExprProbTAContext)_localctx).q = realConst();
				setState(770); ((CsltaExprProbTAContext)_localctx).dtaName = match(ID);
				setState(771); match(OP_PAREN);
				setState(780);
				_la = _input.LA(1);
				if (((((_la - 77)) & ~0x3f) == 0 && ((1L << (_la - 77)) & ((1L << (ID - 77)) | (1L << (REAL_CONST_ID - 77)) | (1L << (REAL_TEMPLATE_ID - 77)))) != 0)) {
					{
					setState(772); real_assign();
					setState(777);
					_errHandler.sync(this);
					_la = _input.LA(1);
					while (_la==COMMA) {
						{
						{
						setState(773); match(COMMA);
						setState(774); real_assign();
						}
						}
						setState(779);
						_errHandler.sync(this);
						_la = _input.LA(1);
					}
					}
				}

				setState(782); match(11);
				setState(791);
				_la = _input.LA(1);
				if (((((_la - 77)) & ~0x3f) == 0 && ((1L << (_la - 77)) & ((1L << (ID - 77)) | (1L << (TRANSITION_ID - 77)) | (1L << (ACTION_ID - 77)))) != 0)) {
					{
					setState(783); act_assign();
					setState(788);
					_errHandler.sync(this);
					_la = _input.LA(1);
					while (_la==COMMA) {
						{
						{
						setState(784); match(COMMA);
						setState(785); act_assign();
						}
						}
						setState(790);
						_errHandler.sync(this);
						_la = _input.LA(1);
					}
					}
				}

				setState(793); match(11);
				setState(802);
				_la = _input.LA(1);
				if (_la==ID || _la==STATEPROP_ID) {
					{
					setState(794); sp_assign();
					setState(799);
					_errHandler.sync(this);
					_la = _input.LA(1);
					while (_la==COMMA) {
						{
						{
						setState(795); match(COMMA);
						setState(796); sp_assign();
						}
						}
						setState(801);
						_errHandler.sync(this);
						_la = _input.LA(1);
					}
					}
				}

				setState(804); match(CL_PAREN);
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(816);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,66,_ctx);
			while ( _alt!=2 && _alt!=-1 ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					setState(814);
					switch ( getInterpreter().adaptivePredict(_input,65,_ctx) ) {
					case 1:
						{
						_localctx = new CsltaExprAndContext(new CsltaExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_csltaExpr);
						setState(808);
						if (!(precpred(_ctx, 3))) throw new FailedPredicateException(this, "precpred(_ctx, 3)");
						setState(809); match(AND);
						setState(810); csltaExpr(4);
						}
						break;

					case 2:
						{
						_localctx = new CsltaExprOrContext(new CsltaExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_csltaExpr);
						setState(811);
						if (!(precpred(_ctx, 2))) throw new FailedPredicateException(this, "precpred(_ctx, 2)");
						setState(812); match(OR);
						setState(813); csltaExpr(3);
						}
						break;
					}
					} 
				}
				setState(818);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,66,_ctx);
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
		enterRule(_localctx, 110, RULE_real_assign);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(819);
			((Real_assignContext)_localctx).label = _input.LT(1);
			_la = _input.LA(1);
			if ( !(((((_la - 77)) & ~0x3f) == 0 && ((1L << (_la - 77)) & ((1L << (ID - 77)) | (1L << (REAL_CONST_ID - 77)) | (1L << (REAL_TEMPLATE_ID - 77)))) != 0)) ) {
				((Real_assignContext)_localctx).label = (Token)_errHandler.recoverInline(this);
			}
			consume();
			setState(820); match(5);
			setState(821); realConst();
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
		enterRule(_localctx, 112, RULE_act_assign);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(823);
			((Act_assignContext)_localctx).label = _input.LT(1);
			_la = _input.LA(1);
			if ( !(((((_la - 77)) & ~0x3f) == 0 && ((1L << (_la - 77)) & ((1L << (ID - 77)) | (1L << (TRANSITION_ID - 77)) | (1L << (ACTION_ID - 77)))) != 0)) ) {
				((Act_assignContext)_localctx).label = (Token)_errHandler.recoverInline(this);
			}
			consume();
			setState(824); match(5);
			setState(825); ((Act_assignContext)_localctx).trn = match(TRANSITION_ID);
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
		enterRule(_localctx, 114, RULE_sp_assign);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(827);
			((Sp_assignContext)_localctx).label = _input.LT(1);
			_la = _input.LA(1);
			if ( !(_la==ID || _la==STATEPROP_ID) ) {
				((Sp_assignContext)_localctx).label = (Token)_errHandler.recoverInline(this);
			}
			consume();
			setState(828); match(5);
			setState(829); csltaExpr(0);
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
		enterRule(_localctx, 116, RULE_temporal_op);
		int _la;
		try {
			setState(835);
			switch (_input.LA(1)) {
			case EXISTS:
			case FORALL:
				_localctx = new TemporalOp2TContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(831);
				_la = _input.LA(1);
				if ( !(_la==EXISTS || _la==FORALL) ) {
				_errHandler.recoverInline(this);
				}
				consume();
				setState(832);
				_la = _input.LA(1);
				if ( !(((((_la - 111)) & ~0x3f) == 0 && ((1L << (_la - 111)) & ((1L << (NEXT - 111)) | (1L << (FUTURE - 111)) | (1L << (GLOBALLY - 111)))) != 0)) ) {
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
				setState(833);
				_la = _input.LA(1);
				if ( !(((((_la - 115)) & ~0x3f) == 0 && ((1L << (_la - 115)) & ((1L << (EXISTS_NEXT - 115)) | (1L << (EXISTS_FUTURE - 115)) | (1L << (EXISTS_GLOBALLY - 115)))) != 0)) ) {
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
				setState(834);
				_la = _input.LA(1);
				if ( !(((((_la - 118)) & ~0x3f) == 0 && ((1L << (_la - 118)) & ((1L << (FORALL_NEXT - 118)) | (1L << (FORALL_FUTURE - 118)) | (1L << (FORALL_GLOBALLY - 118)))) != 0)) ) {
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
		enterRule(_localctx, 118, RULE_composite_temporal_op_ctlstar);
		try {
			setState(843);
			switch (_input.LA(1)) {
			case CTLSTAR_EXISTS_NEXT:
				_localctx = new ComposTempOpEXContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(837); match(CTLSTAR_EXISTS_NEXT);
				}
				break;
			case CTLSTAR_EXISTS_FUTURE:
				_localctx = new ComposTempOpEFContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(838); match(CTLSTAR_EXISTS_FUTURE);
				}
				break;
			case CTLSTAR_EXISTS_GLOBALLY:
				_localctx = new ComposTempOpEGContext(_localctx);
				enterOuterAlt(_localctx, 3);
				{
				setState(839); match(CTLSTAR_EXISTS_GLOBALLY);
				}
				break;
			case CTLSTAR_FORALL_NEXT:
				_localctx = new ComposTempOpAXContext(_localctx);
				enterOuterAlt(_localctx, 4);
				{
				setState(840); match(CTLSTAR_FORALL_NEXT);
				}
				break;
			case CTLSTAR_FORALL_FUTURE:
				_localctx = new ComposTempOpFContext(_localctx);
				enterOuterAlt(_localctx, 5);
				{
				setState(841); match(CTLSTAR_FORALL_FUTURE);
				}
				break;
			case CTLSTAR_FORALL_GLOBALLY:
				_localctx = new ComposTempOpAGContext(_localctx);
				enterOuterAlt(_localctx, 6);
				{
				setState(842); match(CTLSTAR_FORALL_GLOBALLY);
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
		enterRule(_localctx, 120, RULE_anyID);
		int _la;
		try {
			_localctx = new AnyIdentifierContext(_localctx);
			enterOuterAlt(_localctx, 1);
			{
			setState(845);
			_la = _input.LA(1);
			if ( !(((((_la - 77)) & ~0x3f) == 0 && ((1L << (_la - 77)) & ((1L << (ID - 77)) | (1L << (INT_PLACE_ID - 77)) | (1L << (REAL_PLACE_ID - 77)) | (1L << (INT_MSET_PLACE_ID - 77)) | (1L << (REAL_MSET_PLACE_ID - 77)) | (1L << (TRANSITION_ID - 77)) | (1L << (INT_CONST_ID - 77)) | (1L << (REAL_CONST_ID - 77)) | (1L << (INT_MSET_CONST_ID - 77)) | (1L << (REAL_MSET_CONST_ID - 77)) | (1L << (INT_TEMPLATE_ID - 77)) | (1L << (REAL_TEMPLATE_ID - 77)) | (1L << (ACTION_ID - 77)) | (1L << (STATEPROP_ID - 77)) | (1L << (CLOCK_ID - 77)) | (1L << (SIMPLECOLORCLASS_ID - 77)) | (1L << (COLORDOMAIN_ID - 77)) | (1L << (COLORVAR_ID - 77)) | (1L << (COLORSUBCLASS_ID - 77)) | (1L << (COLOR_ID - 77)))) != 0)) ) {
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

		case 31: return realMSetExpr_sempred((RealMSetExprContext)_localctx, predIndex);

		case 32: return intMSetExpr_sempred((IntMSetExprContext)_localctx, predIndex);

		case 33: return multiSetElem_sempred((MultiSetElemContext)_localctx, predIndex);

		case 36: return colorTerm_sempred((ColorTermContext)_localctx, predIndex);

		case 48: return clockConstr_sempred((ClockConstrContext)_localctx, predIndex);

		case 53: return measure_sempred((MeasureContext)_localctx, predIndex);

		case 54: return csltaExpr_sempred((CsltaExprContext)_localctx, predIndex);
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
		case 5: return precpred(_ctx, 23);

		case 6: return precpred(_ctx, 22);

		case 7: return precpred(_ctx, 21);

		case 8: return precpred(_ctx, 20);
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
		case 12: return precpred(_ctx, 3);
		}
		return true;
	}

	public static final String _serializedATN =
		"\3\u0430\ud6d1\u8206\uad2d\u4417\uaef1\u8d80\uaadd\3\u0095\u0352\4\2\t"+
		"\2\4\3\t\3\4\4\t\4\4\5\t\5\4\6\t\6\4\7\t\7\4\b\t\b\4\t\t\t\4\n\t\n\4\13"+
		"\t\13\4\f\t\f\4\r\t\r\4\16\t\16\4\17\t\17\4\20\t\20\4\21\t\21\4\22\t\22"+
		"\4\23\t\23\4\24\t\24\4\25\t\25\4\26\t\26\4\27\t\27\4\30\t\30\4\31\t\31"+
		"\4\32\t\32\4\33\t\33\4\34\t\34\4\35\t\35\4\36\t\36\4\37\t\37\4 \t \4!"+
		"\t!\4\"\t\"\4#\t#\4$\t$\4%\t%\4&\t&\4\'\t\'\4(\t(\4)\t)\4*\t*\4+\t+\4"+
		",\t,\4-\t-\4.\t.\4/\t/\4\60\t\60\4\61\t\61\4\62\t\62\4\63\t\63\4\64\t"+
		"\64\4\65\t\65\4\66\t\66\4\67\t\67\48\t8\49\t9\4:\t:\4;\t;\4<\t<\4=\t="+
		"\4>\t>\3\2\3\2\3\2\3\3\3\3\3\3\3\4\3\4\3\4\3\5\3\5\5\5\u0088\n\5\3\5\3"+
		"\5\3\6\3\6\3\6\7\6\u008f\n\6\f\6\16\6\u0092\13\6\3\6\3\6\3\7\3\7\3\7\7"+
		"\7\u0099\n\7\f\7\16\7\u009c\13\7\3\7\3\7\3\b\3\b\3\b\5\b\u00a3\n\b\3\b"+
		"\3\b\3\t\3\t\3\t\3\n\3\n\3\n\3\13\3\13\3\13\3\f\3\f\3\f\3\r\3\r\3\r\3"+
		"\16\3\16\3\16\3\17\3\17\3\17\3\20\3\20\3\20\3\21\3\21\3\21\3\22\3\22\3"+
		"\22\3\23\3\23\3\23\3\24\3\24\3\24\3\25\3\25\3\25\5\25\u00ce\n\25\3\26"+
		"\3\26\3\26\3\26\5\26\u00d4\n\26\3\27\3\27\3\30\3\30\3\30\3\30\3\30\3\30"+
		"\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30"+
		"\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30"+
		"\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30"+
		"\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\3\30\7\30\u0114"+
		"\n\30\f\30\16\30\u0117\13\30\3\30\3\30\3\30\3\30\5\30\u011d\n\30\3\30"+
		"\3\30\3\30\3\30\3\30\3\30\7\30\u0125\n\30\f\30\16\30\u0128\13\30\3\31"+
		"\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\5\31\u0137"+
		"\n\31\3\31\3\31\3\31\3\31\3\31\3\31\5\31\u013f\n\31\3\31\3\31\3\31\3\31"+
		"\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31"+
		"\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\5\31\u015d\n\31\3\31"+
		"\3\31\3\31\3\31\3\31\3\31\7\31\u0165\n\31\f\31\16\31\u0168\13\31\3\32"+
		"\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32"+
		"\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32"+
		"\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32"+
		"\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32"+
		"\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\5\32\u01af"+
		"\n\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\7\32"+
		"\u01bd\n\32\f\32\16\32\u01c0\13\32\3\33\3\33\3\34\3\34\3\35\3\35\3\36"+
		"\3\36\3\37\3\37\3 \3 \3 \3 \3!\3!\3!\3!\3!\3!\5!\u01d6\n!\3!\5!\u01d9"+
		"\n!\3!\3!\3!\3!\7!\u01df\n!\f!\16!\u01e2\13!\3!\3!\3!\3!\3!\5!\u01e9\n"+
		"!\3!\3!\3!\7!\u01ee\n!\f!\16!\u01f1\13!\3\"\3\"\3\"\3\"\3\"\3\"\5\"\u01f9"+
		"\n\"\3\"\5\"\u01fc\n\"\3\"\3\"\3\"\3\"\7\"\u0202\n\"\f\"\16\"\u0205\13"+
		"\"\3\"\3\"\3\"\3\"\3\"\5\"\u020c\n\"\3\"\3\"\3\"\7\"\u0211\n\"\f\"\16"+
		"\"\u0214\13\"\3#\3#\3#\3#\3#\3#\7#\u021c\n#\f#\16#\u021f\13#\3$\3$\3%"+
		"\3%\3&\3&\3&\5&\u0228\n&\3&\3&\7&\u022c\n&\f&\16&\u022f\13&\3\'\3\'\3"+
		"\'\3\'\3\'\3\'\3\'\3\'\3\'\3\'\3\'\3\'\3\'\3\'\3\'\3\'\3\'\3\'\5\'\u0243"+
		"\n\'\3(\5(\u0246\n(\3(\3(\3(\7(\u024b\n(\f(\16(\u024e\13(\3(\3(\3(\6("+
		"\u0253\n(\r(\16(\u0254\5(\u0257\n(\3)\3)\3*\3*\3*\3*\7*\u025f\n*\f*\16"+
		"*\u0262\13*\3*\3*\5*\u0266\n*\3*\5*\u0269\n*\3*\3*\3*\3*\3*\3*\5*\u0271"+
		"\n*\5*\u0273\n*\3+\3+\3+\3,\3,\3-\3-\5-\u027c\n-\3.\3.\3.\3.\3.\3.\7."+
		"\u0284\n.\f.\16.\u0287\13.\3.\5.\u028a\n.\3/\3/\3/\3/\3/\5/\u0291\n/\3"+
		"\60\3\60\3\61\3\61\3\62\3\62\3\62\3\62\3\62\3\62\3\62\3\62\3\62\3\62\3"+
		"\62\3\62\3\62\3\62\3\62\3\62\3\62\3\62\3\62\5\62\u02aa\n\62\3\62\3\62"+
		"\3\62\7\62\u02af\n\62\f\62\16\62\u02b2\13\62\3\63\3\63\3\63\3\63\3\64"+
		"\3\64\3\64\3\64\7\64\u02bc\n\64\f\64\16\64\u02bf\13\64\5\64\u02c1\n\64"+
		"\3\65\3\65\3\65\3\65\7\65\u02c7\n\65\f\65\16\65\u02ca\13\65\5\65\u02cc"+
		"\n\65\3\66\3\66\3\66\3\66\3\67\3\67\5\67\u02d4\n\67\3\67\3\67\3\67\3\67"+
		"\3\67\5\67\u02db\n\67\3\67\3\67\3\67\3\67\5\67\u02e1\n\67\3\67\3\67\3"+
		"\67\5\67\u02e6\n\67\3\67\3\67\3\67\3\67\5\67\u02ec\n\67\3\67\5\67\u02ef"+
		"\n\67\3\67\3\67\3\67\7\67\u02f4\n\67\f\67\16\67\u02f7\13\67\38\38\38\3"+
		"8\38\38\38\38\38\38\38\38\38\38\38\38\38\78\u030a\n8\f8\168\u030d\138"+
		"\58\u030f\n8\38\38\38\38\78\u0315\n8\f8\168\u0318\138\58\u031a\n8\38\3"+
		"8\38\38\78\u0320\n8\f8\168\u0323\138\58\u0325\n8\38\38\58\u0329\n8\38"+
		"\38\38\38\38\38\78\u0331\n8\f8\168\u0334\138\39\39\39\39\3:\3:\3:\3:\3"+
		";\3;\3;\3;\3<\3<\3<\3<\5<\u0346\n<\3=\3=\3=\3=\3=\3=\5=\u034e\n=\3>\3"+
		">\3>\2\f.\60\62@BDJbln?\2\4\6\b\n\f\16\20\22\24\26\30\32\34\36 \"$&(*"+
		",.\60\62\64\668:<>@BDFHJLNPRTVXZ\\^`bdfhjlnprtvxz\2\33\3\2./\3\2\21\22"+
		"\3\2\23\24\3\2{}\3\2\u0082\u0084\3\2\u0086\u0087\3\2\',\3\2\64\65\3\2"+
		"op\3\2\u0080\u0081\3\2>?\3\2IK\4\29=LL\3\2\u008f\u0094\5\2@FHH\u0095\u0095"+
		"\3\2\27\30\4\2\3\5\b\b\3\2)*\5\2OO``dd\5\2OO^^ee\4\2OOff\3\2qs\3\2uw\3"+
		"\2xz\4\2OOZl\u039c\2|\3\2\2\2\4\177\3\2\2\2\6\u0082\3\2\2\2\b\u0087\3"+
		"\2\2\2\n\u008b\3\2\2\2\f\u0095\3\2\2\2\16\u009f\3\2\2\2\20\u00a6\3\2\2"+
		"\2\22\u00a9\3\2\2\2\24\u00ac\3\2\2\2\26\u00af\3\2\2\2\30\u00b2\3\2\2\2"+
		"\32\u00b5\3\2\2\2\34\u00b8\3\2\2\2\36\u00bb\3\2\2\2 \u00be\3\2\2\2\"\u00c1"+
		"\3\2\2\2$\u00c4\3\2\2\2&\u00c7\3\2\2\2(\u00cd\3\2\2\2*\u00d3\3\2\2\2,"+
		"\u00d5\3\2\2\2.\u011c\3\2\2\2\60\u015c\3\2\2\2\62\u01ae\3\2\2\2\64\u01c1"+
		"\3\2\2\2\66\u01c3\3\2\2\28\u01c5\3\2\2\2:\u01c7\3\2\2\2<\u01c9\3\2\2\2"+
		">\u01cb\3\2\2\2@\u01e8\3\2\2\2B\u020b\3\2\2\2D\u0215\3\2\2\2F\u0220\3"+
		"\2\2\2H\u0222\3\2\2\2J\u0227\3\2\2\2L\u0242\3\2\2\2N\u0256\3\2\2\2P\u0258"+
		"\3\2\2\2R\u0272\3\2\2\2T\u0274\3\2\2\2V\u0277\3\2\2\2X\u027b\3\2\2\2Z"+
		"\u0289\3\2\2\2\\\u0290\3\2\2\2^\u0292\3\2\2\2`\u0294\3\2\2\2b\u02a9\3"+
		"\2\2\2d\u02b3\3\2\2\2f\u02c0\3\2\2\2h\u02cb\3\2\2\2j\u02cd\3\2\2\2l\u02ee"+
		"\3\2\2\2n\u0328\3\2\2\2p\u0335\3\2\2\2r\u0339\3\2\2\2t\u033d\3\2\2\2v"+
		"\u0345\3\2\2\2x\u034d\3\2\2\2z\u034f\3\2\2\2|}\5.\30\2}~\7\2\2\3~\3\3"+
		"\2\2\2\177\u0080\5\60\31\2\u0080\u0081\7\2\2\3\u0081\5\3\2\2\2\u0082\u0083"+
		"\5\62\32\2\u0083\u0084\7\2\2\3\u0084\7\3\2\2\2\u0085\u0088\5.\30\2\u0086"+
		"\u0088\7\60\2\2\u0087\u0085\3\2\2\2\u0087\u0086\3\2\2\2\u0088\u0089\3"+
		"\2\2\2\u0089\u008a\7\2\2\3\u008a\t\3\2\2\2\u008b\u0090\5.\30\2\u008c\u008d"+
		"\7\35\2\2\u008d\u008f\5.\30\2\u008e\u008c\3\2\2\2\u008f\u0092\3\2\2\2"+
		"\u0090\u008e\3\2\2\2\u0090\u0091\3\2\2\2\u0091\u0093\3\2\2\2\u0092\u0090"+
		"\3\2\2\2\u0093\u0094\7\2\2\3\u0094\13\3\2\2\2\u0095\u009a\5\60\31\2\u0096"+
		"\u0097\7\35\2\2\u0097\u0099\5\60\31\2\u0098\u0096\3\2\2\2\u0099\u009c"+
		"\3\2\2\2\u009a\u0098\3\2\2\2\u009a\u009b\3\2\2\2\u009b\u009d\3\2\2\2\u009c"+
		"\u009a\3\2\2\2\u009d\u009e\7\2\2\3\u009e\r\3\2\2\2\u009f\u00a2\5b\62\2"+
		"\u00a0\u00a1\7\37\2\2\u00a1\u00a3\5f\64\2\u00a2\u00a0\3\2\2\2\u00a2\u00a3"+
		"\3\2\2\2\u00a3\u00a4\3\2\2\2\u00a4\u00a5\7\2\2\3\u00a5\17\3\2\2\2\u00a6"+
		"\u00a7\5\\/\2\u00a7\u00a8\7\2\2\3\u00a8\21\3\2\2\2\u00a9\u00aa\5\62\32"+
		"\2\u00aa\u00ab\7\2\2\3\u00ab\23\3\2\2\2\u00ac\u00ad\5^\60\2\u00ad\u00ae"+
		"\7\2\2\3\u00ae\25\3\2\2\2\u00af\u00b0\5h\65\2\u00b0\u00b1\7\2\2\3\u00b1"+
		"\27\3\2\2\2\u00b2\u00b3\7\61\2\2\u00b3\u00b4\7\2\2\3\u00b4\31\3\2\2\2"+
		"\u00b5\u00b6\5l\67\2\u00b6\u00b7\7\2\2\3\u00b7\33\3\2\2\2\u00b8\u00b9"+
		"\5n8\2\u00b9\u00ba\7\2\2\3\u00ba\35\3\2\2\2\u00bb\u00bc\5N(\2\u00bc\u00bd"+
		"\7\2\2\3\u00bd\37\3\2\2\2\u00be\u00bf\5V,\2\u00bf\u00c0\7\2\2\3\u00c0"+
		"!\3\2\2\2\u00c1\u00c2\5X-\2\u00c2\u00c3\7\2\2\3\u00c3#\3\2\2\2\u00c4\u00c5"+
		"\5@!\2\u00c5\u00c6\7\2\2\3\u00c6%\3\2\2\2\u00c7\u00c8\5B\"\2\u00c8\u00c9"+
		"\7\2\2\3\u00c9\'\3\2\2\2\u00ca\u00ce\7P\2\2\u00cb\u00ce\7_\2\2\u00cc\u00ce"+
		"\7c\2\2\u00cd\u00ca\3\2\2\2\u00cd\u00cb\3\2\2\2\u00cd\u00cc\3\2\2\2\u00ce"+
		")\3\2\2\2\u00cf\u00d4\5(\25\2\u00d0\u00d4\7Q\2\2\u00d1\u00d4\7`\2\2\u00d2"+
		"\u00d4\7d\2\2\u00d3\u00cf\3\2\2\2\u00d3\u00d0\3\2\2\2\u00d3\u00d1\3\2"+
		"\2\2\u00d3\u00d2\3\2\2\2\u00d4+\3\2\2\2\u00d5\u00d6\t\2\2\2\u00d6-\3\2"+
		"\2\2\u00d7\u00d8\b\30\1\2\u00d8\u00d9\7\24\2\2\u00d9\u011d\5.\30\20\u00da"+
		"\u011d\5(\25\2\u00db\u00dc\7-\2\2\u00dc\u011d\7Z\2\2\u00dd\u00de\7 \2"+
		"\2\u00de\u00df\5.\30\2\u00df\u00e0\7!\2\2\u00e0\u011d\3\2\2\2\u00e1\u00e2"+
		"\5\64\33\2\u00e2\u00e3\7\"\2\2\u00e3\u00e4\5.\30\2\u00e4\u00e5\7#\2\2"+
		"\u00e5\u011d\3\2\2\2\u00e6\u00e7\5\66\34\2\u00e7\u00e8\7\"\2\2\u00e8\u00e9"+
		"\5\60\31\2\u00e9\u00ea\7#\2\2\u00ea\u011d\3\2\2\2\u00eb\u00ec\58\35\2"+
		"\u00ec\u00ed\7\"\2\2\u00ed\u00ee\5.\30\2\u00ee\u00ef\7\35\2\2\u00ef\u00f0"+
		"\5.\30\2\u00f0\u00f1\7#\2\2\u00f1\u011d\3\2\2\2\u00f2\u00f3\7M\2\2\u00f3"+
		"\u00f4\7\"\2\2\u00f4\u00f5\5\62\32\2\u00f5\u00f6\7\35\2\2\u00f6\u00f7"+
		"\5.\30\2\u00f7\u00f8\7\35\2\2\u00f8\u00f9\5.\30\2\u00f9\u00fa\7#\2\2\u00fa"+
		"\u011d\3\2\2\2\u00fb\u00fc\7\t\2\2\u00fc\u00fd\5\62\32\2\u00fd\u00fe\7"+
		"\36\2\2\u00fe\u00ff\5.\30\2\u00ff\u0100\7\37\2\2\u0100\u0101\7\16\2\2"+
		"\u0101\u0102\5.\30\2\u0102\u0103\7\37\2\2\u0103\u011d\3\2\2\2\u0104\u0105"+
		"\7\67\2\2\u0105\u0106\7\"\2\2\u0106\u0107\5B\"\2\u0107\u0108\7#\2\2\u0108"+
		"\u011d\3\2\2\2\u0109\u010a\7\66\2\2\u010a\u010b\7\"\2\2\u010b\u010c\5"+
		"J&\2\u010c\u010d\7#\2\2\u010d\u011d\3\2\2\2\u010e\u010f\7\177\2\2\u010f"+
		"\u0110\7 \2\2\u0110\u0115\7Z\2\2\u0111\u0112\7\35\2\2\u0112\u0114\7Z\2"+
		"\2\u0113\u0111\3\2\2\2\u0114\u0117\3\2\2\2\u0115\u0113\3\2\2\2\u0115\u0116"+
		"\3\2\2\2\u0116\u0118\3\2\2\2\u0117\u0115\3\2\2\2\u0118\u011d\7!\2\2\u0119"+
		"\u011d\7O\2\2\u011a\u011b\7-\2\2\u011b\u011d\7O\2\2\u011c\u00d7\3\2\2"+
		"\2\u011c\u00da\3\2\2\2\u011c\u00db\3\2\2\2\u011c\u00dd\3\2\2\2\u011c\u00e1"+
		"\3\2\2\2\u011c\u00e6\3\2\2\2\u011c\u00eb\3\2\2\2\u011c\u00f2\3\2\2\2\u011c"+
		"\u00fb\3\2\2\2\u011c\u0104\3\2\2\2\u011c\u0109\3\2\2\2\u011c\u010e\3\2"+
		"\2\2\u011c\u0119\3\2\2\2\u011c\u011a\3\2\2\2\u011d\u0126\3\2\2\2\u011e"+
		"\u011f\f\22\2\2\u011f\u0120\t\3\2\2\u0120\u0125\5.\30\23\u0121\u0122\f"+
		"\21\2\2\u0122\u0123\t\4\2\2\u0123\u0125\5.\30\22\u0124\u011e\3\2\2\2\u0124"+
		"\u0121\3\2\2\2\u0125\u0128\3\2\2\2\u0126\u0124\3\2\2\2\u0126\u0127\3\2"+
		"\2\2\u0127/\3\2\2\2\u0128\u0126\3\2\2\2\u0129\u012a\b\31\1\2\u012a\u012b"+
		"\7\24\2\2\u012b\u015d\5\60\31\16\u012c\u015d\5*\26\2\u012d\u012e\7-\2"+
		"\2\u012e\u015d\7[\2\2\u012f\u015d\5.\30\2\u0130\u0131\7 \2\2\u0131\u0132"+
		"\5\60\31\2\u0132\u0133\7!\2\2\u0133\u015d\3\2\2\2\u0134\u0137\5<\37\2"+
		"\u0135\u0137\5\64\33\2\u0136\u0134\3\2\2\2\u0136\u0135\3\2\2\2\u0137\u0138"+
		"\3\2\2\2\u0138\u0139\7\"\2\2\u0139\u013a\5\60\31\2\u013a\u013b\7#\2\2"+
		"\u013b\u015d\3\2\2\2\u013c\u013f\58\35\2\u013d\u013f\5:\36\2\u013e\u013c"+
		"\3\2\2\2\u013e\u013d\3\2\2\2\u013f\u0140\3\2\2\2\u0140\u0141\7\"\2\2\u0141"+
		"\u0142\5\60\31\2\u0142\u0143\7\35\2\2\u0143\u0144\5\60\31\2\u0144\u0145"+
		"\7#\2\2\u0145\u015d\3\2\2\2\u0146\u0147\7M\2\2\u0147\u0148\7\"\2\2\u0148"+
		"\u0149\5\62\32\2\u0149\u014a\7\35\2\2\u014a\u014b\5\60\31\2\u014b\u014c"+
		"\7\35\2\2\u014c\u014d\5\60\31\2\u014d\u014e\7#\2\2\u014e\u015d\3\2\2\2"+
		"\u014f\u0150\7\t\2\2\u0150\u0151\5\62\32\2\u0151\u0152\7\36\2\2\u0152"+
		"\u0153\5\60\31\2\u0153\u0154\7\37\2\2\u0154\u0155\7\16\2\2\u0155\u0156"+
		"\5\60\31\2\u0156\u0157\7\37\2\2\u0157\u015d\3\2\2\2\u0158\u015d\7\u008e"+
		"\2\2\u0159\u015a\6\31\4\2\u015a\u015d\5`\61\2\u015b\u015d\7O\2\2\u015c"+
		"\u0129\3\2\2\2\u015c\u012c\3\2\2\2\u015c\u012d\3\2\2\2\u015c\u012f\3\2"+
		"\2\2\u015c\u0130\3\2\2\2\u015c\u0136\3\2\2\2\u015c\u013e\3\2\2\2\u015c"+
		"\u0146\3\2\2\2\u015c\u014f\3\2\2\2\u015c\u0158\3\2\2\2\u015c\u0159\3\2"+
		"\2\2\u015c\u015b\3\2\2\2\u015d\u0166\3\2\2\2\u015e\u015f\f\20\2\2\u015f"+
		"\u0160\t\3\2\2\u0160\u0165\5\60\31\21\u0161\u0162\f\17\2\2\u0162\u0163"+
		"\t\4\2\2\u0163\u0165\5\60\31\20\u0164\u015e\3\2\2\2\u0164\u0161\3\2\2"+
		"\2\u0165\u0168\3\2\2\2\u0166\u0164\3\2\2\2\u0166\u0167\3\2\2\2\u0167\61"+
		"\3\2\2\2\u0168\u0166\3\2\2\2\u0169\u016a\b\32\1\2\u016a\u016b\7\32\2\2"+
		"\u016b\u01af\5\62\32\32\u016c\u016d\5v<\2\u016d\u016e\5\62\32\r\u016e"+
		"\u01af\3\2\2\2\u016f\u0170\t\5\2\2\u0170\u01af\5\62\32\n\u0171\u0172\t"+
		"\6\2\2\u0172\u01af\5\62\32\7\u0173\u0174\t\7\2\2\u0174\u01af\5\62\32\4"+
		"\u0175\u0176\5x=\2\u0176\u0177\5\62\32\3\u0177\u01af\3\2\2\2\u0178\u01af"+
		"\5,\27\2\u0179\u017a\5.\30\2\u017a\u017b\t\b\2\2\u017b\u017c\5.\30\2\u017c"+
		"\u01af\3\2\2\2\u017d\u017e\5\60\31\2\u017e\u017f\t\b\2\2\u017f\u0180\5"+
		"\60\31\2\u0180\u01af\3\2\2\2\u0181\u0182\7 \2\2\u0182\u0183\5\62\32\2"+
		"\u0183\u0184\7!\2\2\u0184\u01af\3\2\2\2\u0185\u01af\7O\2\2\u0186\u01af"+
		"\7f\2\2\u0187\u0188\5J&\2\u0188\u0189\t\b\2\2\u0189\u018a\5J&\2\u018a"+
		"\u01af\3\2\2\2\u018b\u018c\5J&\2\u018c\u018d\t\t\2\2\u018d\u018e\5L\'"+
		"\2\u018e\u01af\3\2\2\2\u018f\u0190\t\n\2\2\u0190\u0191\7 \2\2\u0191\u0192"+
		"\5\62\32\2\u0192\u0193\7t\2\2\u0193\u0194\5\62\32\2\u0194\u0195\7!\2\2"+
		"\u0195\u01af\3\2\2\2\u0196\u0197\t\n\2\2\u0197\u0198\7\"\2\2\u0198\u0199"+
		"\5\62\32\2\u0199\u019a\7t\2\2\u019a\u019b\5\62\32\2\u019b\u019c\7#\2\2"+
		"\u019c\u01af\3\2\2\2\u019d\u019e\7~\2\2\u019e\u019f\7 \2\2\u019f\u01a0"+
		"\7^\2\2\u01a0\u01af\7!\2\2\u01a1\u01af\t\13\2\2\u01a2\u01a3\7 \2\2\u01a3"+
		"\u01a4\5\62\32\2\u01a4\u01a5\7\u0085\2\2\u01a5\u01a6\5\62\32\2\u01a6\u01a7"+
		"\7!\2\2\u01a7\u01af\3\2\2\2\u01a8\u01a9\7\"\2\2\u01a9\u01aa\5\62\32\2"+
		"\u01aa\u01ab\7\u0085\2\2\u01ab\u01ac\5\62\32\2\u01ac\u01ad\7#\2\2\u01ad"+
		"\u01af\3\2\2\2\u01ae\u0169\3\2\2\2\u01ae\u016c\3\2\2\2\u01ae\u016f\3\2"+
		"\2\2\u01ae\u0171\3\2\2\2\u01ae\u0173\3\2\2\2\u01ae\u0175\3\2\2\2\u01ae"+
		"\u0178\3\2\2\2\u01ae\u0179\3\2\2\2\u01ae\u017d\3\2\2\2\u01ae\u0181\3\2"+
		"\2\2\u01ae\u0185\3\2\2\2\u01ae\u0186\3\2\2\2\u01ae\u0187\3\2\2\2\u01ae"+
		"\u018b\3\2\2\2\u01ae\u018f\3\2\2\2\u01ae\u0196\3\2\2\2\u01ae\u019d\3\2"+
		"\2\2\u01ae\u01a1\3\2\2\2\u01ae\u01a2\3\2\2\2\u01ae\u01a8\3\2\2\2\u01af"+
		"\u01be\3\2\2\2\u01b0\u01b1\f\31\2\2\u01b1\u01b2\7\25\2\2\u01b2\u01bd\5"+
		"\62\32\32\u01b3\u01b4\f\30\2\2\u01b4\u01b5\7\26\2\2\u01b5\u01bd\5\62\32"+
		"\31\u01b6\u01b7\f\27\2\2\u01b7\u01b8\7\33\2\2\u01b8\u01bd\5\62\32\30\u01b9"+
		"\u01ba\f\26\2\2\u01ba\u01bb\7\34\2\2\u01bb\u01bd\5\62\32\27\u01bc\u01b0"+
		"\3\2\2\2\u01bc\u01b3\3\2\2\2\u01bc\u01b6\3\2\2\2\u01bc\u01b9\3\2\2\2\u01bd"+
		"\u01c0\3\2\2\2\u01be\u01bc\3\2\2\2\u01be\u01bf\3\2\2\2\u01bf\63\3\2\2"+
		"\2\u01c0\u01be\3\2\2\2\u01c1\u01c2\t\f\2\2\u01c2\65\3\2\2\2\u01c3\u01c4"+
		"\t\r\2\2\u01c4\67\3\2\2\2\u01c5\u01c6\t\16\2\2\u01c69\3\2\2\2\u01c7\u01c8"+
		"\t\17\2\2\u01c8;\3\2\2\2\u01c9\u01ca\t\20\2\2\u01ca=\3\2\2\2\u01cb\u01cc"+
		"\7\"\2\2\u01cc\u01cd\5\62\32\2\u01cd\u01ce\7#\2\2\u01ce?\3\2\2\2\u01cf"+
		"\u01d0\b!\1\2\u01d0\u01d1\7 \2\2\u01d1\u01d2\5@!\2\u01d2\u01d3\7!\2\2"+
		"\u01d3\u01e9\3\2\2\2\u01d4\u01d6\5\60\31\2\u01d5\u01d4\3\2\2\2\u01d5\u01d6"+
		"\3\2\2\2\u01d6\u01d8\3\2\2\2\u01d7\u01d9\5> \2\u01d8\u01d7\3\2\2\2\u01d8"+
		"\u01d9\3\2\2\2\u01d9\u01da\3\2\2\2\u01da\u01db\7)\2\2\u01db\u01e0\5D#"+
		"\2\u01dc\u01dd\7\35\2\2\u01dd\u01df\5D#\2\u01de\u01dc\3\2\2\2\u01df\u01e2"+
		"\3\2\2\2\u01e0\u01de\3\2\2\2\u01e0\u01e1\3\2\2\2\u01e1\u01e3\3\2\2\2\u01e2"+
		"\u01e0\3\2\2\2\u01e3\u01e4\7*\2\2\u01e4\u01e9\3\2\2\2\u01e5\u01e6\7-\2"+
		"\2\u01e6\u01e9\7]\2\2\u01e7\u01e9\7b\2\2\u01e8\u01cf\3\2\2\2\u01e8\u01d5"+
		"\3\2\2\2\u01e8\u01e5\3\2\2\2\u01e8\u01e7\3\2\2\2\u01e9\u01ef\3\2\2\2\u01ea"+
		"\u01eb\f\6\2\2\u01eb\u01ec\t\4\2\2\u01ec\u01ee\5@!\7\u01ed\u01ea\3\2\2"+
		"\2\u01ee\u01f1\3\2\2\2\u01ef\u01ed\3\2\2\2\u01ef\u01f0\3\2\2\2\u01f0A"+
		"\3\2\2\2\u01f1\u01ef\3\2\2\2\u01f2\u01f3\b\"\1\2\u01f3\u01f4\7 \2\2\u01f4"+
		"\u01f5\5B\"\2\u01f5\u01f6\7!\2\2\u01f6\u020c\3\2\2\2\u01f7\u01f9\5.\30"+
		"\2\u01f8\u01f7\3\2\2\2\u01f8\u01f9\3\2\2\2\u01f9\u01fb\3\2\2\2\u01fa\u01fc"+
		"\5> \2\u01fb\u01fa\3\2\2\2\u01fb\u01fc\3\2\2\2\u01fc\u01fd\3\2\2\2\u01fd"+
		"\u01fe\7)\2\2\u01fe\u0203\5D#\2\u01ff\u0200\7\35\2\2\u0200\u0202\5D#\2"+
		"\u0201\u01ff\3\2\2\2\u0202\u0205\3\2\2\2\u0203\u0201\3\2\2\2\u0203\u0204"+
		"\3\2\2\2\u0204\u0206\3\2\2\2\u0205\u0203\3\2\2\2\u0206\u0207\7*\2\2\u0207"+
		"\u020c\3\2\2\2\u0208\u0209\7-\2\2\u0209\u020c\7\\\2\2\u020a\u020c\7a\2"+
		"\2\u020b\u01f2\3\2\2\2\u020b\u01f8\3\2\2\2\u020b\u0208\3\2\2\2\u020b\u020a"+
		"\3\2\2\2\u020c\u0212\3\2\2\2\u020d\u020e\f\6\2\2\u020e\u020f\t\4\2\2\u020f"+
		"\u0211\5B\"\7\u0210\u020d\3\2\2\2\u0211\u0214\3\2\2\2\u0212\u0210\3\2"+
		"\2\2\u0212\u0213\3\2\2\2\u0213C\3\2\2\2\u0214\u0212\3\2\2\2\u0215\u0216"+
		"\b#\1\2\u0216\u0217\5L\'\2\u0217\u021d\3\2\2\2\u0218\u0219\f\4\2\2\u0219"+
		"\u021a\t\4\2\2\u021a\u021c\5D#\5\u021b\u0218\3\2\2\2\u021c\u021f\3\2\2"+
		"\2\u021d\u021b\3\2\2\2\u021d\u021e\3\2\2\2\u021eE\3\2\2\2\u021f\u021d"+
		"\3\2\2\2\u0220\u0221\7j\2\2\u0221G\3\2\2\2\u0222\u0223\7k\2\2\u0223I\3"+
		"\2\2\2\u0224\u0225\b&\1\2\u0225\u0228\5F$\2\u0226\u0228\7l\2\2\u0227\u0224"+
		"\3\2\2\2\u0227\u0226\3\2\2\2\u0228\u022d\3\2\2\2\u0229\u022a\f\5\2\2\u022a"+
		"\u022c\t\21\2\2\u022b\u0229\3\2\2\2\u022c\u022f\3\2\2\2\u022d\u022b\3"+
		"\2\2\2\u022d\u022e\3\2\2\2\u022eK\3\2\2\2\u022f\u022d\3\2\2\2\u0230\u0243"+
		"\5J&\2\u0231\u0243\7\n\2\2\u0232\u0243\7h\2\2\u0233\u0234\7m\2\2\u0234"+
		"\u0243\7h\2\2\u0235\u0243\5H%\2\u0236\u0237\7m\2\2\u0237\u0243\5H%\2\u0238"+
		"\u0239\78\2\2\u0239\u023a\7\"\2\2\u023a\u023b\5J&\2\u023b\u023c\7#\2\2"+
		"\u023c\u0243\3\2\2\2\u023d\u023e\7n\2\2\u023e\u023f\7 \2\2\u023f\u0240"+
		"\5J&\2\u0240\u0241\7!\2\2\u0241\u0243\3\2\2\2\u0242\u0230\3\2\2\2\u0242"+
		"\u0231\3\2\2\2\u0242\u0232\3\2\2\2\u0242\u0233\3\2\2\2\u0242\u0235\3\2"+
		"\2\2\u0242\u0236\3\2\2\2\u0242\u0238\3\2\2\2\u0242\u023d\3\2\2\2\u0243"+
		"M\3\2\2\2\u0244\u0246\5P)\2\u0245\u0244\3\2\2\2\u0245\u0246\3\2\2\2\u0246"+
		"\u0247\3\2\2\2\u0247\u024c\5R*\2\u0248\u0249\7\23\2\2\u0249\u024b\5R*"+
		"\2\u024a\u0248\3\2\2\2\u024b\u024e\3\2\2\2\u024c\u024a\3\2\2\2\u024c\u024d"+
		"\3\2\2\2\u024d\u0257\3\2\2\2\u024e\u024c\3\2\2\2\u024f\u0252\7h\2\2\u0250"+
		"\u0251\7\21\2\2\u0251\u0253\7h\2\2\u0252\u0250\3\2\2\2\u0253\u0254\3\2"+
		"\2\2\u0254\u0252\3\2\2\2\u0254\u0255\3\2\2\2\u0255\u0257\3\2\2\2\u0256"+
		"\u0245\3\2\2\2\u0256\u024f\3\2\2\2\u0257O\3\2\2\2\u0258\u0259\t\22\2\2"+
		"\u0259Q\3\2\2\2\u025a\u025b\7$\2\2\u025b\u0260\7O\2\2\u025c\u025d\7\35"+
		"\2\2\u025d\u025f\7O\2\2\u025e\u025c\3\2\2\2\u025f\u0262\3\2\2\2\u0260"+
		"\u025e\3\2\2\2\u0260\u0261\3\2\2\2\u0261\u0263\3\2\2\2\u0262\u0260\3\2"+
		"\2\2\u0263\u0265\7%\2\2\u0264\u0266\5T+\2\u0265\u0264\3\2\2\2\u0265\u0266"+
		"\3\2\2\2\u0266\u0273\3\2\2\2\u0267\u0269\5z>\2\u0268\u0267\3\2\2\2\u0268"+
		"\u0269\3\2\2\2\u0269\u026a\3\2\2\2\u026a\u026b\7$\2\2\u026b\u026c\5(\25"+
		"\2\u026c\u026d\7&\2\2\u026d\u026e\5(\25\2\u026e\u0270\7%\2\2\u026f\u0271"+
		"\5T+\2\u0270\u026f\3\2\2\2\u0270\u0271\3\2\2\2\u0271\u0273\3\2\2\2\u0272"+
		"\u025a\3\2\2\2\u0272\u0268\3\2\2\2\u0273S\3\2\2\2\u0274\u0275\7\13\2\2"+
		"\u0275\u0276\7O\2\2\u0276U\3\2\2\2\u0277\u0278\7h\2\2\u0278W\3\2\2\2\u0279"+
		"\u027c\7h\2\2\u027a\u027c\7i\2\2\u027b\u0279\3\2\2\2\u027b\u027a\3\2\2"+
		"\2\u027cY\3\2\2\2\u027d\u027e\7$\2\2\u027e\u028a\7%\2\2\u027f\u0280\7"+
		"$\2\2\u0280\u0285\7e\2\2\u0281\u0282\7\35\2\2\u0282\u0284\7e\2\2\u0283"+
		"\u0281\3\2\2\2\u0284\u0287\3\2\2\2\u0285\u0283\3\2\2\2\u0285\u0286\3\2"+
		"\2\2\u0286\u0288\3\2\2\2\u0287\u0285\3\2\2\2\u0288\u028a\7%\2\2\u0289"+
		"\u027d\3\2\2\2\u0289\u027f\3\2\2\2\u028a[\3\2\2\2\u028b\u0291\7\62\2\2"+
		"\u028c\u028d\7\62\2\2\u028d\u028e\7\6\2\2\u028e\u0291\5Z.\2\u028f\u0291"+
		"\5Z.\2\u0290\u028b\3\2\2\2\u0290\u028c\3\2\2\2\u0290\u028f\3\2\2\2\u0291"+
		"]\3\2\2\2\u0292\u0293\7^\2\2\u0293_\3\2\2\2\u0294\u0295\7g\2\2\u0295a"+
		"\3\2\2\2\u0296\u0297\b\62\1\2\u0297\u0298\5\60\31\2\u0298\u0299\7)\2\2"+
		"\u0299\u029a\5`\61\2\u029a\u029b\7)\2\2\u029b\u029c\5\60\31\2\u029c\u02aa"+
		"\3\2\2\2\u029d\u029e\5`\61\2\u029e\u029f\t\23\2\2\u029f\u02a0\5\60\31"+
		"\2\u02a0\u02aa\3\2\2\2\u02a1\u02a2\5\60\31\2\u02a2\u02a3\t\23\2\2\u02a3"+
		"\u02a4\5`\61\2\u02a4\u02aa\3\2\2\2\u02a5\u02a6\5`\61\2\u02a6\u02a7\7\7"+
		"\2\2\u02a7\u02a8\5\60\31\2\u02a8\u02aa\3\2\2\2\u02a9\u0296\3\2\2\2\u02a9"+
		"\u029d\3\2\2\2\u02a9\u02a1\3\2\2\2\u02a9\u02a5\3\2\2\2\u02aa\u02b0\3\2"+
		"\2\2\u02ab\u02ac\f\3\2\2\u02ac\u02ad\7\25\2\2\u02ad\u02af\5b\62\4\u02ae"+
		"\u02ab\3\2\2\2\u02af\u02b2\3\2\2\2\u02b0\u02ae\3\2\2\2\u02b0\u02b1\3\2"+
		"\2\2\u02b1c\3\2\2\2\u02b2\u02b0\3\2\2\2\u02b3\u02b4\5`\61\2\u02b4\u02b5"+
		"\7\7\2\2\u02b5\u02b6\5\60\31\2\u02b6e\3\2\2\2\u02b7\u02c1\3\2\2\2\u02b8"+
		"\u02bd\5d\63\2\u02b9\u02ba\7\35\2\2\u02ba\u02bc\5d\63\2\u02bb\u02b9\3"+
		"\2\2\2\u02bc\u02bf\3\2\2\2\u02bd\u02bb\3\2\2\2\u02bd\u02be\3\2\2\2\u02be"+
		"\u02c1\3\2\2\2\u02bf\u02bd\3\2\2\2\u02c0\u02b7\3\2\2\2\u02c0\u02b8\3\2"+
		"\2\2\u02c1g\3\2\2\2\u02c2\u02cc\3\2\2\2\u02c3\u02c8\5j\66\2\u02c4\u02c5"+
		"\7\35\2\2\u02c5\u02c7\5j\66\2\u02c6\u02c4\3\2\2\2\u02c7\u02ca\3\2\2\2"+
		"\u02c8\u02c6\3\2\2\2\u02c8\u02c9\3\2\2\2\u02c9\u02cc\3\2\2\2\u02ca\u02c8"+
		"\3\2\2\2\u02cb\u02c2\3\2\2\2\u02cb\u02c3\3\2\2\2\u02cci\3\2\2\2\u02cd"+
		"\u02ce\5`\61\2\u02ce\u02cf\7\7\2\2\u02cf\u02d0\5\60\31\2\u02d0k\3\2\2"+
		"\2\u02d1\u02d3\b\67\1\2\u02d2\u02d4\5*\26\2\u02d3\u02d2\3\2\2\2\u02d3"+
		"\u02d4\3\2\2\2\u02d4\u02d5\3\2\2\2\u02d5\u02d6\7\20\2\2\u02d6\u02d7\5"+
		"\62\32\2\u02d7\u02d8\7%\2\2\u02d8\u02ef\3\2\2\2\u02d9\u02db\5*\26\2\u02da"+
		"\u02d9\3\2\2\2\u02da\u02db\3\2\2\2\u02db\u02dc\3\2\2\2\u02dc\u02dd\7\f"+
		"\2\2\u02dd\u02e0\5.\30\2\u02de\u02df\7\22\2\2\u02df\u02e1\5\62\32\2\u02e0"+
		"\u02de\3\2\2\2\u02e0\u02e1\3\2\2\2\u02e1\u02e2\3\2\2\2\u02e2\u02e3\7%"+
		"\2\2\u02e3\u02ef\3\2\2\2\u02e4\u02e6\5*\26\2\u02e5\u02e4\3\2\2\2\u02e5"+
		"\u02e6\3\2\2\2\u02e6\u02e7\3\2\2\2\u02e7\u02e8\7\17\2\2\u02e8\u02eb\7"+
		"^\2\2\u02e9\u02ea\7\22\2\2\u02ea\u02ec\5\62\32\2\u02eb\u02e9\3\2\2\2\u02eb"+
		"\u02ec\3\2\2\2\u02ec\u02ed\3\2\2\2\u02ed\u02ef\7%\2\2\u02ee\u02d1\3\2"+
		"\2\2\u02ee\u02da\3\2\2\2\u02ee\u02e5\3\2\2\2\u02ef\u02f5\3\2\2\2\u02f0"+
		"\u02f1\f\6\2\2\u02f1\u02f2\t\4\2\2\u02f2\u02f4\5l\67\7\u02f3\u02f0\3\2"+
		"\2\2\u02f4\u02f7\3\2\2\2\u02f5\u02f3\3\2\2\2\u02f5\u02f6\3\2\2\2\u02f6"+
		"m\3\2\2\2\u02f7\u02f5\3\2\2\2\u02f8\u02f9\b8\1\2\u02f9\u02fa\7\32\2\2"+
		"\u02fa\u0329\5n8\6\u02fb\u0329\5\62\32\2\u02fc\u0329\7Z\2\2\u02fd\u02fe"+
		"\7 \2\2\u02fe\u02ff\5n8\2\u02ff\u0300\7!\2\2\u0300\u0329\3\2\2\2\u0301"+
		"\u0302\7\63\2\2\u0302\u0303\t\b\2\2\u0303\u0304\5*\26\2\u0304\u0305\7"+
		"O\2\2\u0305\u030e\7 \2\2\u0306\u030b\5p9\2\u0307\u0308\7\35\2\2\u0308"+
		"\u030a\5p9\2\u0309\u0307\3\2\2\2\u030a\u030d\3\2\2\2\u030b\u0309\3\2\2"+
		"\2\u030b\u030c\3\2\2\2\u030c\u030f\3\2\2\2\u030d\u030b\3\2\2\2\u030e\u0306"+
		"\3\2\2\2\u030e\u030f\3\2\2\2\u030f\u0310\3\2\2\2\u0310\u0319\7\r\2\2\u0311"+
		"\u0316\5r:\2\u0312\u0313\7\35\2\2\u0313\u0315\5r:\2\u0314\u0312\3\2\2"+
		"\2\u0315\u0318\3\2\2\2\u0316\u0314\3\2\2\2\u0316\u0317\3\2\2\2\u0317\u031a"+
		"\3\2\2\2\u0318\u0316\3\2\2\2\u0319\u0311\3\2\2\2\u0319\u031a\3\2\2\2\u031a"+
		"\u031b\3\2\2\2\u031b\u0324\7\r\2\2\u031c\u0321\5t;\2\u031d\u031e\7\35"+
		"\2\2\u031e\u0320\5t;\2\u031f\u031d\3\2\2\2\u0320\u0323\3\2\2\2\u0321\u031f"+
		"\3\2\2\2\u0321\u0322\3\2\2\2\u0322\u0325\3\2\2\2\u0323\u0321\3\2\2\2\u0324"+
		"\u031c\3\2\2\2\u0324\u0325\3\2\2\2\u0325\u0326\3\2\2\2\u0326\u0327\7!"+
		"\2\2\u0327\u0329\3\2\2\2\u0328\u02f8\3\2\2\2\u0328\u02fb\3\2\2\2\u0328"+
		"\u02fc\3\2\2\2\u0328\u02fd\3\2\2\2\u0328\u0301\3\2\2\2\u0329\u0332\3\2"+
		"\2\2\u032a\u032b\f\5\2\2\u032b\u032c\7\25\2\2\u032c\u0331\5n8\6\u032d"+
		"\u032e\f\4\2\2\u032e\u032f\7\26\2\2\u032f\u0331\5n8\5\u0330\u032a\3\2"+
		"\2\2\u0330\u032d\3\2\2\2\u0331\u0334\3\2\2\2\u0332\u0330\3\2\2\2\u0332"+
		"\u0333\3\2\2\2\u0333o\3\2\2\2\u0334\u0332\3\2\2\2\u0335\u0336\t\24\2\2"+
		"\u0336\u0337\7\7\2\2\u0337\u0338\5*\26\2\u0338q\3\2\2\2\u0339\u033a\t"+
		"\25\2\2\u033a\u033b\7\7\2\2\u033b\u033c\7^\2\2\u033cs\3\2\2\2\u033d\u033e"+
		"\t\26\2\2\u033e\u033f\7\7\2\2\u033f\u0340\5n8\2\u0340u\3\2\2\2\u0341\u0342"+
		"\t\n\2\2\u0342\u0346\t\27\2\2\u0343\u0346\t\30\2\2\u0344\u0346\t\31\2"+
		"\2\u0345\u0341\3\2\2\2\u0345\u0343\3\2\2\2\u0345\u0344\3\2\2\2\u0346w"+
		"\3\2\2\2\u0347\u034e\7\u0088\2\2\u0348\u034e\7\u0089\2\2\u0349\u034e\7"+
		"\u008a\2\2\u034a\u034e\7\u008b\2\2\u034b\u034e\7\u008c\2\2\u034c\u034e"+
		"\7\u008d\2\2\u034d\u0347\3\2\2\2\u034d\u0348\3\2\2\2\u034d\u0349\3\2\2"+
		"\2\u034d\u034a\3\2\2\2\u034d\u034b\3\2\2\2\u034d\u034c\3\2\2\2\u034ey"+
		"\3\2\2\2\u034f\u0350\t\32\2\2\u0350{\3\2\2\2G\u0087\u0090\u009a\u00a2"+
		"\u00cd\u00d3\u0115\u011c\u0124\u0126\u0136\u013e\u015c\u0164\u0166\u01ae"+
		"\u01bc\u01be\u01d5\u01d8\u01e0\u01e8\u01ef\u01f8\u01fb\u0203\u020b\u0212"+
		"\u021d\u0227\u022d\u0242\u0245\u024c\u0254\u0256\u0260\u0265\u0268\u0270"+
		"\u0272\u027b\u0285\u0289\u0290\u02a9\u02b0\u02bd\u02c0\u02c8\u02cb\u02d3"+
		"\u02da\u02e0\u02e5\u02eb\u02ee\u02f5\u030b\u030e\u0316\u0319\u0321\u0324"+
		"\u0328\u0330\u0332\u0345\u034d";
	public static final ATN _ATN =
		new ATNDeserializer().deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
		for (int i = 0; i < _ATN.getNumberOfDecisions(); i++) {
			_decisionToDFA[i] = new DFA(_ATN.getDecisionState(i), i);
		}
	}
}