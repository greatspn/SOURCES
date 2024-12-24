// Generated from ExprLang.g4 by ANTLR 4.2.1
package editor.domain.grammar;

import editor.domain.*;
import editor.domain.elements.*;

import org.antlr.v4.runtime.misc.NotNull;
import org.antlr.v4.runtime.tree.ParseTreeListener;

/**
 * This interface defines a complete listener for a parse tree produced by
 * {@link ExprLangParser}.
 */
public interface ExprLangListener extends ParseTreeListener {
	/**
	 * Enter a parse tree produced by {@link ExprLangParser#mainClockDef}.
	 * @param ctx the parse tree
	 */
	void enterMainClockDef(@NotNull ExprLangParser.MainClockDefContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#mainClockDef}.
	 * @param ctx the parse tree
	 */
	void exitMainClockDef(@NotNull ExprLangParser.MainClockDefContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#CsltaExprBool}.
	 * @param ctx the parse tree
	 */
	void enterCsltaExprBool(@NotNull ExprLangParser.CsltaExprBoolContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#CsltaExprBool}.
	 * @param ctx the parse tree
	 */
	void exitCsltaExprBool(@NotNull ExprLangParser.CsltaExprBoolContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprCTLinitState}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprCTLinitState(@NotNull ExprLangParser.BoolExprCTLinitStateContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprCTLinitState}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprCTLinitState(@NotNull ExprLangParser.BoolExprCTLinitStateContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealConstLiteral}.
	 * @param ctx the parse tree
	 */
	void enterRealConstLiteral(@NotNull ExprLangParser.RealConstLiteralContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealConstLiteral}.
	 * @param ctx the parse tree
	 */
	void exitRealConstLiteral(@NotNull ExprLangParser.RealConstLiteralContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ComposTempOpAG}.
	 * @param ctx the parse tree
	 */
	void enterComposTempOpAG(@NotNull ExprLangParser.ComposTempOpAGContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ComposTempOpAG}.
	 * @param ctx the parse tree
	 */
	void exitComposTempOpAG(@NotNull ExprLangParser.ComposTempOpAGContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntExprConst}.
	 * @param ctx the parse tree
	 */
	void enterIntExprConst(@NotNull ExprLangParser.IntExprConstContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntExprConst}.
	 * @param ctx the parse tree
	 */
	void exitIntExprConst(@NotNull ExprLangParser.IntExprConstContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealExprFBA}.
	 * @param ctx the parse tree
	 */
	void enterRealExprFBA(@NotNull ExprLangParser.RealExprFBAContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealExprFBA}.
	 * @param ctx the parse tree
	 */
	void exitRealExprFBA(@NotNull ExprLangParser.RealExprFBAContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#binaryIntFn}.
	 * @param ctx the parse tree
	 */
	void enterBinaryIntFn(@NotNull ExprLangParser.BinaryIntFnContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#binaryIntFn}.
	 * @param ctx the parse tree
	 */
	void exitBinaryIntFn(@NotNull ExprLangParser.BinaryIntFnContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#MSetElemAddSub}.
	 * @param ctx the parse tree
	 */
	void enterMSetElemAddSub(@NotNull ExprLangParser.MSetElemAddSubContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#MSetElemAddSub}.
	 * @param ctx the parse tree
	 */
	void exitMSetElemAddSub(@NotNull ExprLangParser.MSetElemAddSubContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ActSetAllExceptList}.
	 * @param ctx the parse tree
	 */
	void enterActSetAllExceptList(@NotNull ExprLangParser.ActSetAllExceptListContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ActSetAllExceptList}.
	 * @param ctx the parse tree
	 */
	void exitActSetAllExceptList(@NotNull ExprLangParser.ActSetAllExceptListContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#mainActBinding}.
	 * @param ctx the parse tree
	 */
	void enterMainActBinding(@NotNull ExprLangParser.MainActBindingContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#mainActBinding}.
	 * @param ctx the parse tree
	 */
	void exitMainActBinding(@NotNull ExprLangParser.MainActBindingContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntExprParen}.
	 * @param ctx the parse tree
	 */
	void enterIntExprParen(@NotNull ExprLangParser.IntExprParenContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntExprParen}.
	 * @param ctx the parse tree
	 */
	void exitIntExprParen(@NotNull ExprLangParser.IntExprParenContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ColorSetSubclassOf}.
	 * @param ctx the parse tree
	 */
	void enterColorSetSubclassOf(@NotNull ExprLangParser.ColorSetSubclassOfContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ColorSetSubclassOf}.
	 * @param ctx the parse tree
	 */
	void exitColorSetSubclassOf(@NotNull ExprLangParser.ColorSetSubclassOfContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealExprFromTable}.
	 * @param ctx the parse tree
	 */
	void enterRealExprFromTable(@NotNull ExprLangParser.RealExprFromTableContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealExprFromTable}.
	 * @param ctx the parse tree
	 */
	void exitRealExprFromTable(@NotNull ExprLangParser.RealExprFromTableContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ColorSetSubClass}.
	 * @param ctx the parse tree
	 */
	void enterColorSetSubClass(@NotNull ExprLangParser.ColorSetSubClassContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ColorSetSubClass}.
	 * @param ctx the parse tree
	 */
	void exitColorSetSubClass(@NotNull ExprLangParser.ColorSetSubClassContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ComposTempOpAX}.
	 * @param ctx the parse tree
	 */
	void enterComposTempOpAX(@NotNull ExprLangParser.ComposTempOpAXContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ComposTempOpAX}.
	 * @param ctx the parse tree
	 */
	void exitComposTempOpAX(@NotNull ExprLangParser.ComposTempOpAXContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntExprCond2}.
	 * @param ctx the parse tree
	 */
	void enterIntExprCond2(@NotNull ExprLangParser.IntExprCond2Context ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntExprCond2}.
	 * @param ctx the parse tree
	 */
	void exitIntExprCond2(@NotNull ExprLangParser.IntExprCond2Context ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#mainStatePropExpr}.
	 * @param ctx the parse tree
	 */
	void enterMainStatePropExpr(@NotNull ExprLangParser.MainStatePropExprContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#mainStatePropExpr}.
	 * @param ctx the parse tree
	 */
	void exitMainStatePropExpr(@NotNull ExprLangParser.MainStatePropExprContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprCTLStarQuantif}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprCTLStarQuantif(@NotNull ExprLangParser.BoolExprCTLStarQuantifContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprCTLStarQuantif}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprCTLStarQuantif(@NotNull ExprLangParser.BoolExprCTLStarQuantifContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprImply}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprImply(@NotNull ExprLangParser.BoolExprImplyContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprImply}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprImply(@NotNull ExprLangParser.BoolExprImplyContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ClockVarId}.
	 * @param ctx the parse tree
	 */
	void enterClockVarId(@NotNull ExprLangParser.ClockVarIdContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ClockVarId}.
	 * @param ctx the parse tree
	 */
	void exitClockVarId(@NotNull ExprLangParser.ClockVarIdContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprCTLpin}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprCTLpin(@NotNull ExprLangParser.BoolExprCTLpinContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprCTLpin}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprCTLpin(@NotNull ExprLangParser.BoolExprCTLpinContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprBiimply}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprBiimply(@NotNull ExprLangParser.BoolExprBiimplyContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprBiimply}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprBiimply(@NotNull ExprLangParser.BoolExprBiimplyContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#TemporalOpForallX}.
	 * @param ctx the parse tree
	 */
	void enterTemporalOpForallX(@NotNull ExprLangParser.TemporalOpForallXContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#TemporalOpForallX}.
	 * @param ctx the parse tree
	 */
	void exitTemporalOpForallX(@NotNull ExprLangParser.TemporalOpForallXContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ColorSetAll}.
	 * @param ctx the parse tree
	 */
	void enterColorSetAll(@NotNull ExprLangParser.ColorSetAllContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ColorSetAll}.
	 * @param ctx the parse tree
	 */
	void exitColorSetAll(@NotNull ExprLangParser.ColorSetAllContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ActListEmpty}.
	 * @param ctx the parse tree
	 */
	void enterActListEmpty(@NotNull ExprLangParser.ActListEmptyContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ActListEmpty}.
	 * @param ctx the parse tree
	 */
	void exitActListEmpty(@NotNull ExprLangParser.ActListEmptyContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealMSetExprAddSub}.
	 * @param ctx the parse tree
	 */
	void enterRealMSetExprAddSub(@NotNull ExprLangParser.RealMSetExprAddSubContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealMSetExprAddSub}.
	 * @param ctx the parse tree
	 */
	void exitRealMSetExprAddSub(@NotNull ExprLangParser.RealMSetExprAddSubContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#VarUpdateListEmpty}.
	 * @param ctx the parse tree
	 */
	void enterVarUpdateListEmpty(@NotNull ExprLangParser.VarUpdateListEmptyContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#VarUpdateListEmpty}.
	 * @param ctx the parse tree
	 */
	void exitVarUpdateListEmpty(@NotNull ExprLangParser.VarUpdateListEmptyContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealExprCall}.
	 * @param ctx the parse tree
	 */
	void enterRealExprCall(@NotNull ExprLangParser.RealExprCallContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealExprCall}.
	 * @param ctx the parse tree
	 */
	void exitRealExprCall(@NotNull ExprLangParser.RealExprCallContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ColorSetSubClass2}.
	 * @param ctx the parse tree
	 */
	void enterColorSetSubClass2(@NotNull ExprLangParser.ColorSetSubClass2Context ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ColorSetSubClass2}.
	 * @param ctx the parse tree
	 */
	void exitColorSetSubClass2(@NotNull ExprLangParser.ColorSetSubClass2Context ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntExprUnknownPlaceId}.
	 * @param ctx the parse tree
	 */
	void enterIntExprUnknownPlaceId(@NotNull ExprLangParser.IntExprUnknownPlaceIdContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntExprUnknownPlaceId}.
	 * @param ctx the parse tree
	 */
	void exitIntExprUnknownPlaceId(@NotNull ExprLangParser.IntExprUnknownPlaceIdContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprCTLenabled}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprCTLenabled(@NotNull ExprLangParser.BoolExprCTLenabledContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprCTLenabled}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprCTLenabled(@NotNull ExprLangParser.BoolExprCTLenabledContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntExprCardMSet}.
	 * @param ctx the parse tree
	 */
	void enterIntExprCardMSet(@NotNull ExprLangParser.IntExprCardMSetContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntExprCardMSet}.
	 * @param ctx the parse tree
	 */
	void exitIntExprCardMSet(@NotNull ExprLangParser.IntExprCardMSetContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprStatePropId}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprStatePropId(@NotNull ExprLangParser.BoolExprStatePropIdContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprStatePropId}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprStatePropId(@NotNull ExprLangParser.BoolExprStatePropIdContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ColorListIDs}.
	 * @param ctx the parse tree
	 */
	void enterColorListIDs(@NotNull ExprLangParser.ColorListIDsContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ColorListIDs}.
	 * @param ctx the parse tree
	 */
	void exitColorListIDs(@NotNull ExprLangParser.ColorListIDsContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ColorTermColor}.
	 * @param ctx the parse tree
	 */
	void enterColorTermColor(@NotNull ExprLangParser.ColorTermColorContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ColorTermColor}.
	 * @param ctx the parse tree
	 */
	void exitColorTermColor(@NotNull ExprLangParser.ColorTermColorContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ColorSetTerm}.
	 * @param ctx the parse tree
	 */
	void enterColorSetTerm(@NotNull ExprLangParser.ColorSetTermContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ColorSetTerm}.
	 * @param ctx the parse tree
	 */
	void exitColorSetTerm(@NotNull ExprLangParser.ColorSetTermContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#mainRealExprList}.
	 * @param ctx the parse tree
	 */
	void enterMainRealExprList(@NotNull ExprLangParser.MainRealExprListContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#mainRealExprList}.
	 * @param ctx the parse tree
	 */
	void exitMainRealExprList(@NotNull ExprLangParser.MainRealExprListContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprCTLdeadlocks}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprCTLdeadlocks(@NotNull ExprLangParser.BoolExprCTLdeadlocksContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprCTLdeadlocks}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprCTLdeadlocks(@NotNull ExprLangParser.BoolExprCTLdeadlocksContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#colorVarDef}.
	 * @param ctx the parse tree
	 */
	void enterColorVarDef(@NotNull ExprLangParser.ColorVarDefContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#colorVarDef}.
	 * @param ctx the parse tree
	 */
	void exitColorVarDef(@NotNull ExprLangParser.ColorVarDefContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealMSetExprElemProduct}.
	 * @param ctx the parse tree
	 */
	void enterRealMSetExprElemProduct(@NotNull ExprLangParser.RealMSetExprElemProductContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealMSetExprElemProduct}.
	 * @param ctx the parse tree
	 */
	void exitRealMSetExprElemProduct(@NotNull ExprLangParser.RealMSetExprElemProductContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#boolConst}.
	 * @param ctx the parse tree
	 */
	void enterBoolConst(@NotNull ExprLangParser.BoolConstContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#boolConst}.
	 * @param ctx the parse tree
	 */
	void exitBoolConst(@NotNull ExprLangParser.BoolConstContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprCTLStarUntil2}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprCTLStarUntil2(@NotNull ExprLangParser.BoolExprCTLStarUntil2Context ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprCTLStarUntil2}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprCTLStarUntil2(@NotNull ExprLangParser.BoolExprCTLStarUntil2Context ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealExprClockVar}.
	 * @param ctx the parse tree
	 */
	void enterRealExprClockVar(@NotNull ExprLangParser.RealExprClockVarContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealExprClockVar}.
	 * @param ctx the parse tree
	 */
	void exitRealExprClockVar(@NotNull ExprLangParser.RealExprClockVarContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealExprInt}.
	 * @param ctx the parse tree
	 */
	void enterRealExprInt(@NotNull ExprLangParser.RealExprIntContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealExprInt}.
	 * @param ctx the parse tree
	 */
	void exitRealExprInt(@NotNull ExprLangParser.RealExprIntContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ClockConstrEquals}.
	 * @param ctx the parse tree
	 */
	void enterClockConstrEquals(@NotNull ExprLangParser.ClockConstrEqualsContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ClockConstrEquals}.
	 * @param ctx the parse tree
	 */
	void exitClockConstrEquals(@NotNull ExprLangParser.ClockConstrEqualsContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#mainMeasure}.
	 * @param ctx the parse tree
	 */
	void enterMainMeasure(@NotNull ExprLangParser.MainMeasureContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#mainMeasure}.
	 * @param ctx the parse tree
	 */
	void exitMainMeasure(@NotNull ExprLangParser.MainMeasureContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntExprCTLBound}.
	 * @param ctx the parse tree
	 */
	void enterIntExprCTLBound(@NotNull ExprLangParser.IntExprCTLBoundContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntExprCTLBound}.
	 * @param ctx the parse tree
	 */
	void exitIntExprCTLBound(@NotNull ExprLangParser.IntExprCTLBoundContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealExprPdfXVar}.
	 * @param ctx the parse tree
	 */
	void enterRealExprPdfXVar(@NotNull ExprLangParser.RealExprPdfXVarContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealExprPdfXVar}.
	 * @param ctx the parse tree
	 */
	void exitRealExprPdfXVar(@NotNull ExprLangParser.RealExprPdfXVarContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ColorListInterval}.
	 * @param ctx the parse tree
	 */
	void enterColorListInterval(@NotNull ExprLangParser.ColorListIntervalContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ColorListInterval}.
	 * @param ctx the parse tree
	 */
	void exitColorListInterval(@NotNull ExprLangParser.ColorListIntervalContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntExprCond}.
	 * @param ctx the parse tree
	 */
	void enterIntExprCond(@NotNull ExprLangParser.IntExprCondContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntExprCond}.
	 * @param ctx the parse tree
	 */
	void exitIntExprCond(@NotNull ExprLangParser.IntExprCondContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#VarUpdateListList}.
	 * @param ctx the parse tree
	 */
	void enterVarUpdateListList(@NotNull ExprLangParser.VarUpdateListListContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#VarUpdateListList}.
	 * @param ctx the parse tree
	 */
	void exitVarUpdateListList(@NotNull ExprLangParser.VarUpdateListListContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#TagComplDefinition}.
	 * @param ctx the parse tree
	 */
	void enterTagComplDefinition(@NotNull ExprLangParser.TagComplDefinitionContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#TagComplDefinition}.
	 * @param ctx the parse tree
	 */
	void exitTagComplDefinition(@NotNull ExprLangParser.TagComplDefinitionContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprColorTermComp}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprColorTermComp(@NotNull ExprLangParser.BoolExprColorTermCompContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprColorTermComp}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprColorTermComp(@NotNull ExprLangParser.BoolExprColorTermCompContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ColorTermFilterThis}.
	 * @param ctx the parse tree
	 */
	void enterColorTermFilterThis(@NotNull ExprLangParser.ColorTermFilterThisContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ColorTermFilterThis}.
	 * @param ctx the parse tree
	 */
	void exitColorTermFilterThis(@NotNull ExprLangParser.ColorTermFilterThisContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ClockConstrAnd}.
	 * @param ctx the parse tree
	 */
	void enterClockConstrAnd(@NotNull ExprLangParser.ClockConstrAndContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ClockConstrAnd}.
	 * @param ctx the parse tree
	 */
	void exitClockConstrAnd(@NotNull ExprLangParser.ClockConstrAndContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#colorClassOrd}.
	 * @param ctx the parse tree
	 */
	void enterColorClassOrd(@NotNull ExprLangParser.ColorClassOrdContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#colorClassOrd}.
	 * @param ctx the parse tree
	 */
	void exitColorClassOrd(@NotNull ExprLangParser.ColorClassOrdContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ColorSetClass}.
	 * @param ctx the parse tree
	 */
	void enterColorSetClass(@NotNull ExprLangParser.ColorSetClassContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ColorSetClass}.
	 * @param ctx the parse tree
	 */
	void exitColorSetClass(@NotNull ExprLangParser.ColorSetClassContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#mainCsltaExpr}.
	 * @param ctx the parse tree
	 */
	void enterMainCsltaExpr(@NotNull ExprLangParser.MainCsltaExprContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#mainCsltaExpr}.
	 * @param ctx the parse tree
	 */
	void exitMainCsltaExpr(@NotNull ExprLangParser.MainCsltaExprContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprCTLStarUntil}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprCTLStarUntil(@NotNull ExprLangParser.BoolExprCTLStarUntilContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprCTLStarUntil}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprCTLStarUntil(@NotNull ExprLangParser.BoolExprCTLStarUntilContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntExprUnaryFn}.
	 * @param ctx the parse tree
	 */
	void enterIntExprUnaryFn(@NotNull ExprLangParser.IntExprUnaryFnContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntExprUnaryFn}.
	 * @param ctx the parse tree
	 */
	void exitIntExprUnaryFn(@NotNull ExprLangParser.IntExprUnaryFnContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealConstId}.
	 * @param ctx the parse tree
	 */
	void enterRealConstId(@NotNull ExprLangParser.RealConstIdContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealConstId}.
	 * @param ctx the parse tree
	 */
	void exitRealConstId(@NotNull ExprLangParser.RealConstIdContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#VarUpdateValue}.
	 * @param ctx the parse tree
	 */
	void enterVarUpdateValue(@NotNull ExprLangParser.VarUpdateValueContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#VarUpdateValue}.
	 * @param ctx the parse tree
	 */
	void exitVarUpdateValue(@NotNull ExprLangParser.VarUpdateValueContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#mainClockConstr}.
	 * @param ctx the parse tree
	 */
	void enterMainClockConstr(@NotNull ExprLangParser.MainClockConstrContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#mainClockConstr}.
	 * @param ctx the parse tree
	 */
	void exitMainClockConstr(@NotNull ExprLangParser.MainClockConstrContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntExprPlaceMarking}.
	 * @param ctx the parse tree
	 */
	void enterIntExprPlaceMarking(@NotNull ExprLangParser.IntExprPlaceMarkingContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntExprPlaceMarking}.
	 * @param ctx the parse tree
	 */
	void exitIntExprPlaceMarking(@NotNull ExprLangParser.IntExprPlaceMarkingContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprCTLStar}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprCTLStar(@NotNull ExprLangParser.BoolExprCTLStarContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprCTLStar}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprCTLStar(@NotNull ExprLangParser.BoolExprCTLStarContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealExprFromTimeTable}.
	 * @param ctx the parse tree
	 */
	void enterRealExprFromTimeTable(@NotNull ExprLangParser.RealExprFromTimeTableContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealExprFromTimeTable}.
	 * @param ctx the parse tree
	 */
	void exitRealExprFromTimeTable(@NotNull ExprLangParser.RealExprFromTimeTableContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealExprUnaryFn}.
	 * @param ctx the parse tree
	 */
	void enterRealExprUnaryFn(@NotNull ExprLangParser.RealExprUnaryFnContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealExprUnaryFn}.
	 * @param ctx the parse tree
	 */
	void exitRealExprUnaryFn(@NotNull ExprLangParser.RealExprUnaryFnContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntConstId}.
	 * @param ctx the parse tree
	 */
	void enterIntConstId(@NotNull ExprLangParser.IntConstIdContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntConstId}.
	 * @param ctx the parse tree
	 */
	void exitIntConstId(@NotNull ExprLangParser.IntConstIdContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprRealComp}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprRealComp(@NotNull ExprLangParser.BoolExprRealCompContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprRealComp}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprRealComp(@NotNull ExprLangParser.BoolExprRealCompContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#MSetElemBoolPredicate}.
	 * @param ctx the parse tree
	 */
	void enterMSetElemBoolPredicate(@NotNull ExprLangParser.MSetElemBoolPredicateContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#MSetElemBoolPredicate}.
	 * @param ctx the parse tree
	 */
	void exitMSetElemBoolPredicate(@NotNull ExprLangParser.MSetElemBoolPredicateContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealExprCond}.
	 * @param ctx the parse tree
	 */
	void enterRealExprCond(@NotNull ExprLangParser.RealExprCondContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealExprCond}.
	 * @param ctx the parse tree
	 */
	void exitRealExprCond(@NotNull ExprLangParser.RealExprCondContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#CsltaExprProbTA}.
	 * @param ctx the parse tree
	 */
	void enterCsltaExprProbTA(@NotNull ExprLangParser.CsltaExprProbTAContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#CsltaExprProbTA}.
	 * @param ctx the parse tree
	 */
	void exitCsltaExprProbTA(@NotNull ExprLangParser.CsltaExprProbTAContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ColorVarIdentifier}.
	 * @param ctx the parse tree
	 */
	void enterColorVarIdentifier(@NotNull ExprLangParser.ColorVarIdentifierContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ColorVarIdentifier}.
	 * @param ctx the parse tree
	 */
	void exitColorVarIdentifier(@NotNull ExprLangParser.ColorVarIdentifierContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#MeasureE}.
	 * @param ctx the parse tree
	 */
	void enterMeasureE(@NotNull ExprLangParser.MeasureEContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#MeasureE}.
	 * @param ctx the parse tree
	 */
	void exitMeasureE(@NotNull ExprLangParser.MeasureEContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#TemporalOpExistX}.
	 * @param ctx the parse tree
	 */
	void enterTemporalOpExistX(@NotNull ExprLangParser.TemporalOpExistXContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#TemporalOpExistX}.
	 * @param ctx the parse tree
	 */
	void exitTemporalOpExistX(@NotNull ExprLangParser.TemporalOpExistXContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#MSetBoolPredicate}.
	 * @param ctx the parse tree
	 */
	void enterMSetBoolPredicate(@NotNull ExprLangParser.MSetBoolPredicateContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#MSetBoolPredicate}.
	 * @param ctx the parse tree
	 */
	void exitMSetBoolPredicate(@NotNull ExprLangParser.MSetBoolPredicateContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ActListList}.
	 * @param ctx the parse tree
	 */
	void enterActListList(@NotNull ExprLangParser.ActListListContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ActListList}.
	 * @param ctx the parse tree
	 */
	void exitActListList(@NotNull ExprLangParser.ActListListContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntConstLiteral}.
	 * @param ctx the parse tree
	 */
	void enterIntConstLiteral(@NotNull ExprLangParser.IntConstLiteralContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntConstLiteral}.
	 * @param ctx the parse tree
	 */
	void exitIntConstLiteral(@NotNull ExprLangParser.IntConstLiteralContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#binaryRealFn}.
	 * @param ctx the parse tree
	 */
	void enterBinaryRealFn(@NotNull ExprLangParser.BinaryRealFnContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#binaryRealFn}.
	 * @param ctx the parse tree
	 */
	void exitBinaryRealFn(@NotNull ExprLangParser.BinaryRealFnContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntOrRealListEmptyList}.
	 * @param ctx the parse tree
	 */
	void enterIntOrRealListEmptyList(@NotNull ExprLangParser.IntOrRealListEmptyListContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntOrRealListEmptyList}.
	 * @param ctx the parse tree
	 */
	void exitIntOrRealListEmptyList(@NotNull ExprLangParser.IntOrRealListEmptyListContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#mainIntExpr}.
	 * @param ctx the parse tree
	 */
	void enterMainIntExpr(@NotNull ExprLangParser.MainIntExprContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#mainIntExpr}.
	 * @param ctx the parse tree
	 */
	void exitMainIntExpr(@NotNull ExprLangParser.MainIntExprContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#mainIntMSetExpr}.
	 * @param ctx the parse tree
	 */
	void enterMainIntMSetExpr(@NotNull ExprLangParser.MainIntMSetExprContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#mainIntMSetExpr}.
	 * @param ctx the parse tree
	 */
	void exitMainIntMSetExpr(@NotNull ExprLangParser.MainIntMSetExprContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ColorClassDefProduct}.
	 * @param ctx the parse tree
	 */
	void enterColorClassDefProduct(@NotNull ExprLangParser.ColorClassDefProductContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ColorClassDefProduct}.
	 * @param ctx the parse tree
	 */
	void exitColorClassDefProduct(@NotNull ExprLangParser.ColorClassDefProductContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#MSetElemColorTerm}.
	 * @param ctx the parse tree
	 */
	void enterMSetElemColorTerm(@NotNull ExprLangParser.MSetElemColorTermContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#MSetElemColorTerm}.
	 * @param ctx the parse tree
	 */
	void exitMSetElemColorTerm(@NotNull ExprLangParser.MSetElemColorTermContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprUnknownId}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprUnknownId(@NotNull ExprLangParser.BoolExprUnknownIdContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprUnknownId}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprUnknownId(@NotNull ExprLangParser.BoolExprUnknownIdContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#PlaceDomainColorDomain}.
	 * @param ctx the parse tree
	 */
	void enterPlaceDomainColorDomain(@NotNull ExprLangParser.PlaceDomainColorDomainContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#PlaceDomainColorDomain}.
	 * @param ctx the parse tree
	 */
	void exitPlaceDomainColorDomain(@NotNull ExprLangParser.PlaceDomainColorDomainContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#MeasureP}.
	 * @param ctx the parse tree
	 */
	void enterMeasureP(@NotNull ExprLangParser.MeasurePContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#MeasureP}.
	 * @param ctx the parse tree
	 */
	void exitMeasureP(@NotNull ExprLangParser.MeasurePContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntMsetExprPlaceMarking}.
	 * @param ctx the parse tree
	 */
	void enterIntMsetExprPlaceMarking(@NotNull ExprLangParser.IntMsetExprPlaceMarkingContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntMsetExprPlaceMarking}.
	 * @param ctx the parse tree
	 */
	void exitIntMsetExprPlaceMarking(@NotNull ExprLangParser.IntMsetExprPlaceMarkingContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntExprUnaryRealFn}.
	 * @param ctx the parse tree
	 */
	void enterIntExprUnaryRealFn(@NotNull ExprLangParser.IntExprUnaryRealFnContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntExprUnaryRealFn}.
	 * @param ctx the parse tree
	 */
	void exitIntExprUnaryRealFn(@NotNull ExprLangParser.IntExprUnaryRealFnContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntExprUnknownId}.
	 * @param ctx the parse tree
	 */
	void enterIntExprUnknownId(@NotNull ExprLangParser.IntExprUnknownIdContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntExprUnknownId}.
	 * @param ctx the parse tree
	 */
	void exitIntExprUnknownId(@NotNull ExprLangParser.IntExprUnknownIdContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#TagRewriteListEmpty}.
	 * @param ctx the parse tree
	 */
	void enterTagRewriteListEmpty(@NotNull ExprLangParser.TagRewriteListEmptyContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#TagRewriteListEmpty}.
	 * @param ctx the parse tree
	 */
	void exitTagRewriteListEmpty(@NotNull ExprLangParser.TagRewriteListEmptyContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntOrRealListReal}.
	 * @param ctx the parse tree
	 */
	void enterIntOrRealListReal(@NotNull ExprLangParser.IntOrRealListRealContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntOrRealListReal}.
	 * @param ctx the parse tree
	 */
	void exitIntOrRealListReal(@NotNull ExprLangParser.IntOrRealListRealContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#MeasureX}.
	 * @param ctx the parse tree
	 */
	void enterMeasureX(@NotNull ExprLangParser.MeasureXContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#MeasureX}.
	 * @param ctx the parse tree
	 */
	void exitMeasureX(@NotNull ExprLangParser.MeasureXContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealMsetExprConst}.
	 * @param ctx the parse tree
	 */
	void enterRealMsetExprConst(@NotNull ExprLangParser.RealMsetExprConstContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealMsetExprConst}.
	 * @param ctx the parse tree
	 */
	void exitRealMsetExprConst(@NotNull ExprLangParser.RealMsetExprConstContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealExprParen}.
	 * @param ctx the parse tree
	 */
	void enterRealExprParen(@NotNull ExprLangParser.RealExprParenContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealExprParen}.
	 * @param ctx the parse tree
	 */
	void exitRealExprParen(@NotNull ExprLangParser.RealExprParenContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealTemplateId}.
	 * @param ctx the parse tree
	 */
	void enterRealTemplateId(@NotNull ExprLangParser.RealTemplateIdContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealTemplateId}.
	 * @param ctx the parse tree
	 */
	void exitRealTemplateId(@NotNull ExprLangParser.RealTemplateIdContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ComposTempOpEG}.
	 * @param ctx the parse tree
	 */
	void enterComposTempOpEG(@NotNull ExprLangParser.ComposTempOpEGContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ComposTempOpEG}.
	 * @param ctx the parse tree
	 */
	void exitComposTempOpEG(@NotNull ExprLangParser.ComposTempOpEGContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ComposTempOpF}.
	 * @param ctx the parse tree
	 */
	void enterComposTempOpF(@NotNull ExprLangParser.ComposTempOpFContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ComposTempOpF}.
	 * @param ctx the parse tree
	 */
	void exitComposTempOpF(@NotNull ExprLangParser.ComposTempOpFContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ComposTempOpEF}.
	 * @param ctx the parse tree
	 */
	void enterComposTempOpEF(@NotNull ExprLangParser.ComposTempOpEFContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ComposTempOpEF}.
	 * @param ctx the parse tree
	 */
	void exitComposTempOpEF(@NotNull ExprLangParser.ComposTempOpEFContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#TagRewriteRule}.
	 * @param ctx the parse tree
	 */
	void enterTagRewriteRule(@NotNull ExprLangParser.TagRewriteRuleContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#TagRewriteRule}.
	 * @param ctx the parse tree
	 */
	void exitTagRewriteRule(@NotNull ExprLangParser.TagRewriteRuleContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ClockConstrBetween}.
	 * @param ctx the parse tree
	 */
	void enterClockConstrBetween(@NotNull ExprLangParser.ClockConstrBetweenContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ClockConstrBetween}.
	 * @param ctx the parse tree
	 */
	void exitClockConstrBetween(@NotNull ExprLangParser.ClockConstrBetweenContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntExprBinaryFn}.
	 * @param ctx the parse tree
	 */
	void enterIntExprBinaryFn(@NotNull ExprLangParser.IntExprBinaryFnContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntExprBinaryFn}.
	 * @param ctx the parse tree
	 */
	void exitIntExprBinaryFn(@NotNull ExprLangParser.IntExprBinaryFnContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealExprConst}.
	 * @param ctx the parse tree
	 */
	void enterRealExprConst(@NotNull ExprLangParser.RealExprConstContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealExprConst}.
	 * @param ctx the parse tree
	 */
	void exitRealExprConst(@NotNull ExprLangParser.RealExprConstContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#VarFlowListList}.
	 * @param ctx the parse tree
	 */
	void enterVarFlowListList(@NotNull ExprLangParser.VarFlowListListContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#VarFlowListList}.
	 * @param ctx the parse tree
	 */
	void exitVarFlowListList(@NotNull ExprLangParser.VarFlowListListContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealExprPlaceMarking}.
	 * @param ctx the parse tree
	 */
	void enterRealExprPlaceMarking(@NotNull ExprLangParser.RealExprPlaceMarkingContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealExprPlaceMarking}.
	 * @param ctx the parse tree
	 */
	void exitRealExprPlaceMarking(@NotNull ExprLangParser.RealExprPlaceMarkingContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#PlaceDomainColorClass}.
	 * @param ctx the parse tree
	 */
	void enterPlaceDomainColorClass(@NotNull ExprLangParser.PlaceDomainColorClassContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#PlaceDomainColorClass}.
	 * @param ctx the parse tree
	 */
	void exitPlaceDomainColorClass(@NotNull ExprLangParser.PlaceDomainColorClassContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprOr}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprOr(@NotNull ExprLangParser.BoolExprOrContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprOr}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprOr(@NotNull ExprLangParser.BoolExprOrContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntMSetExprAddSub}.
	 * @param ctx the parse tree
	 */
	void enterIntMSetExprAddSub(@NotNull ExprLangParser.IntMSetExprAddSubContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntMSetExprAddSub}.
	 * @param ctx the parse tree
	 */
	void exitIntMSetExprAddSub(@NotNull ExprLangParser.IntMSetExprAddSubContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#real_assign}.
	 * @param ctx the parse tree
	 */
	void enterReal_assign(@NotNull ExprLangParser.Real_assignContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#real_assign}.
	 * @param ctx the parse tree
	 */
	void exitReal_assign(@NotNull ExprLangParser.Real_assignContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntExprNegate}.
	 * @param ctx the parse tree
	 */
	void enterIntExprNegate(@NotNull ExprLangParser.IntExprNegateContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntExprNegate}.
	 * @param ctx the parse tree
	 */
	void exitIntExprNegate(@NotNull ExprLangParser.IntExprNegateContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ComposTempOpEX}.
	 * @param ctx the parse tree
	 */
	void enterComposTempOpEX(@NotNull ExprLangParser.ComposTempOpEXContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ComposTempOpEX}.
	 * @param ctx the parse tree
	 */
	void exitComposTempOpEX(@NotNull ExprLangParser.ComposTempOpEXContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ColorSubclassNameDef}.
	 * @param ctx the parse tree
	 */
	void enterColorSubclassNameDef(@NotNull ExprLangParser.ColorSubclassNameDefContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ColorSubclassNameDef}.
	 * @param ctx the parse tree
	 */
	void exitColorSubclassNameDef(@NotNull ExprLangParser.ColorSubclassNameDefContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprAnd}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprAnd(@NotNull ExprLangParser.BoolExprAndContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprAnd}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprAnd(@NotNull ExprLangParser.BoolExprAndContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#mainColorClassDef}.
	 * @param ctx the parse tree
	 */
	void enterMainColorClassDef(@NotNull ExprLangParser.MainColorClassDefContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#mainColorClassDef}.
	 * @param ctx the parse tree
	 */
	void exitMainColorClassDef(@NotNull ExprLangParser.MainColorClassDefContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealExprMulDiv}.
	 * @param ctx the parse tree
	 */
	void enterRealExprMulDiv(@NotNull ExprLangParser.RealExprMulDivContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealExprMulDiv}.
	 * @param ctx the parse tree
	 */
	void exitRealExprMulDiv(@NotNull ExprLangParser.RealExprMulDivContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprNot}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprNot(@NotNull ExprLangParser.BoolExprNotContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprNot}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprNot(@NotNull ExprLangParser.BoolExprNotContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#mainActSet}.
	 * @param ctx the parse tree
	 */
	void enterMainActSet(@NotNull ExprLangParser.MainActSetContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#mainActSet}.
	 * @param ctx the parse tree
	 */
	void exitMainActSet(@NotNull ExprLangParser.MainActSetContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealExprAddSub}.
	 * @param ctx the parse tree
	 */
	void enterRealExprAddSub(@NotNull ExprLangParser.RealExprAddSubContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealExprAddSub}.
	 * @param ctx the parse tree
	 */
	void exitRealExprAddSub(@NotNull ExprLangParser.RealExprAddSubContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealExprBinaryFn}.
	 * @param ctx the parse tree
	 */
	void enterRealExprBinaryFn(@NotNull ExprLangParser.RealExprBinaryFnContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealExprBinaryFn}.
	 * @param ctx the parse tree
	 */
	void exitRealExprBinaryFn(@NotNull ExprLangParser.RealExprBinaryFnContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#MeasureAddSub}.
	 * @param ctx the parse tree
	 */
	void enterMeasureAddSub(@NotNull ExprLangParser.MeasureAddSubContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#MeasureAddSub}.
	 * @param ctx the parse tree
	 */
	void exitMeasureAddSub(@NotNull ExprLangParser.MeasureAddSubContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprConst}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprConst(@NotNull ExprLangParser.BoolExprConstContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprConst}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprConst(@NotNull ExprLangParser.BoolExprConstContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntMSetExprConst}.
	 * @param ctx the parse tree
	 */
	void enterIntMSetExprConst(@NotNull ExprLangParser.IntMSetExprConstContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntMSetExprConst}.
	 * @param ctx the parse tree
	 */
	void exitIntMSetExprConst(@NotNull ExprLangParser.IntMSetExprConstContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntOrRealListInt}.
	 * @param ctx the parse tree
	 */
	void enterIntOrRealListInt(@NotNull ExprLangParser.IntOrRealListIntContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntOrRealListInt}.
	 * @param ctx the parse tree
	 */
	void exitIntOrRealListInt(@NotNull ExprLangParser.IntOrRealListIntContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#CsltaExprAnd}.
	 * @param ctx the parse tree
	 */
	void enterCsltaExprAnd(@NotNull ExprLangParser.CsltaExprAndContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#CsltaExprAnd}.
	 * @param ctx the parse tree
	 */
	void exitCsltaExprAnd(@NotNull ExprLangParser.CsltaExprAndContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealFromIntConst}.
	 * @param ctx the parse tree
	 */
	void enterRealFromIntConst(@NotNull ExprLangParser.RealFromIntConstContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealFromIntConst}.
	 * @param ctx the parse tree
	 */
	void exitRealFromIntConst(@NotNull ExprLangParser.RealFromIntConstContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ColorClassDefNamedSet}.
	 * @param ctx the parse tree
	 */
	void enterColorClassDefNamedSet(@NotNull ExprLangParser.ColorClassDefNamedSetContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ColorClassDefNamedSet}.
	 * @param ctx the parse tree
	 */
	void exitColorClassDefNamedSet(@NotNull ExprLangParser.ColorClassDefNamedSetContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#mainColorVarDef}.
	 * @param ctx the parse tree
	 */
	void enterMainColorVarDef(@NotNull ExprLangParser.MainColorVarDefContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#mainColorVarDef}.
	 * @param ctx the parse tree
	 */
	void exitMainColorVarDef(@NotNull ExprLangParser.MainColorVarDefContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#CsltaExprNot}.
	 * @param ctx the parse tree
	 */
	void enterCsltaExprNot(@NotNull ExprLangParser.CsltaExprNotContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#CsltaExprNot}.
	 * @param ctx the parse tree
	 */
	void exitCsltaExprNot(@NotNull ExprLangParser.CsltaExprNotContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#mainIntOrInfiniteExpr}.
	 * @param ctx the parse tree
	 */
	void enterMainIntOrInfiniteExpr(@NotNull ExprLangParser.MainIntOrInfiniteExprContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#mainIntOrInfiniteExpr}.
	 * @param ctx the parse tree
	 */
	void exitMainIntOrInfiniteExpr(@NotNull ExprLangParser.MainIntOrInfiniteExprContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprCTLUntil}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprCTLUntil(@NotNull ExprLangParser.BoolExprCTLUntilContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprCTLUntil}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprCTLUntil(@NotNull ExprLangParser.BoolExprCTLUntilContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealExprFromList}.
	 * @param ctx the parse tree
	 */
	void enterRealExprFromList(@NotNull ExprLangParser.RealExprFromListContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealExprFromList}.
	 * @param ctx the parse tree
	 */
	void exitRealExprFromList(@NotNull ExprLangParser.RealExprFromListContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#unaryIntFn}.
	 * @param ctx the parse tree
	 */
	void enterUnaryIntFn(@NotNull ExprLangParser.UnaryIntFnContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#unaryIntFn}.
	 * @param ctx the parse tree
	 */
	void exitUnaryIntFn(@NotNull ExprLangParser.UnaryIntFnContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ColorTermVar}.
	 * @param ctx the parse tree
	 */
	void enterColorTermVar(@NotNull ExprLangParser.ColorTermVarContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ColorTermVar}.
	 * @param ctx the parse tree
	 */
	void exitColorTermVar(@NotNull ExprLangParser.ColorTermVarContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ActBindingTransition}.
	 * @param ctx the parse tree
	 */
	void enterActBindingTransition(@NotNull ExprLangParser.ActBindingTransitionContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ActBindingTransition}.
	 * @param ctx the parse tree
	 */
	void exitActBindingTransition(@NotNull ExprLangParser.ActBindingTransitionContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#unaryIntRealFn}.
	 * @param ctx the parse tree
	 */
	void enterUnaryIntRealFn(@NotNull ExprLangParser.UnaryIntRealFnContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#unaryIntRealFn}.
	 * @param ctx the parse tree
	 */
	void exitUnaryIntRealFn(@NotNull ExprLangParser.UnaryIntRealFnContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#VarFlowDefinition}.
	 * @param ctx the parse tree
	 */
	void enterVarFlowDefinition(@NotNull ExprLangParser.VarFlowDefinitionContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#VarFlowDefinition}.
	 * @param ctx the parse tree
	 */
	void exitVarFlowDefinition(@NotNull ExprLangParser.VarFlowDefinitionContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#mainVarFlow}.
	 * @param ctx the parse tree
	 */
	void enterMainVarFlow(@NotNull ExprLangParser.MainVarFlowContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#mainVarFlow}.
	 * @param ctx the parse tree
	 */
	void exitMainVarFlow(@NotNull ExprLangParser.MainVarFlowContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealExprUnknownId}.
	 * @param ctx the parse tree
	 */
	void enterRealExprUnknownId(@NotNull ExprLangParser.RealExprUnknownIdContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealExprUnknownId}.
	 * @param ctx the parse tree
	 */
	void exitRealExprUnknownId(@NotNull ExprLangParser.RealExprUnknownIdContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntExprAddSub}.
	 * @param ctx the parse tree
	 */
	void enterIntExprAddSub(@NotNull ExprLangParser.IntExprAddSubContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntExprAddSub}.
	 * @param ctx the parse tree
	 */
	void exitIntExprAddSub(@NotNull ExprLangParser.IntExprAddSubContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#mainTagRewriteList}.
	 * @param ctx the parse tree
	 */
	void enterMainTagRewriteList(@NotNull ExprLangParser.MainTagRewriteListContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#mainTagRewriteList}.
	 * @param ctx the parse tree
	 */
	void exitMainTagRewriteList(@NotNull ExprLangParser.MainTagRewriteListContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntTemplateId}.
	 * @param ctx the parse tree
	 */
	void enterIntTemplateId(@NotNull ExprLangParser.IntTemplateIdContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntTemplateId}.
	 * @param ctx the parse tree
	 */
	void exitIntTemplateId(@NotNull ExprLangParser.IntTemplateIdContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealMsetExprPlaceMarking}.
	 * @param ctx the parse tree
	 */
	void enterRealMsetExprPlaceMarking(@NotNull ExprLangParser.RealMsetExprPlaceMarkingContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealMsetExprPlaceMarking}.
	 * @param ctx the parse tree
	 */
	void exitRealMsetExprPlaceMarking(@NotNull ExprLangParser.RealMsetExprPlaceMarkingContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprCTLUntil2}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprCTLUntil2(@NotNull ExprLangParser.BoolExprCTLUntil2Context ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprCTLUntil2}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprCTLUntil2(@NotNull ExprLangParser.BoolExprCTLUntil2Context ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#mainIntExprList}.
	 * @param ctx the parse tree
	 */
	void enterMainIntExprList(@NotNull ExprLangParser.MainIntExprListContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#mainIntExprList}.
	 * @param ctx the parse tree
	 */
	void exitMainIntExprList(@NotNull ExprLangParser.MainIntExprListContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ColorSetClass2}.
	 * @param ctx the parse tree
	 */
	void enterColorSetClass2(@NotNull ExprLangParser.ColorSetClass2Context ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ColorSetClass2}.
	 * @param ctx the parse tree
	 */
	void exitColorSetClass2(@NotNull ExprLangParser.ColorSetClass2Context ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealMSetExprParen}.
	 * @param ctx the parse tree
	 */
	void enterRealMSetExprParen(@NotNull ExprLangParser.RealMSetExprParenContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealMSetExprParen}.
	 * @param ctx the parse tree
	 */
	void exitRealMSetExprParen(@NotNull ExprLangParser.RealMSetExprParenContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#TagDefinition}.
	 * @param ctx the parse tree
	 */
	void enterTagDefinition(@NotNull ExprLangParser.TagDefinitionContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#TagDefinition}.
	 * @param ctx the parse tree
	 */
	void exitTagDefinition(@NotNull ExprLangParser.TagDefinitionContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntMSetExprElemProduct}.
	 * @param ctx the parse tree
	 */
	void enterIntMSetExprElemProduct(@NotNull ExprLangParser.IntMSetExprElemProductContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntMSetExprElemProduct}.
	 * @param ctx the parse tree
	 */
	void exitIntMSetExprElemProduct(@NotNull ExprLangParser.IntMSetExprElemProductContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#sp_assign}.
	 * @param ctx the parse tree
	 */
	void enterSp_assign(@NotNull ExprLangParser.Sp_assignContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#sp_assign}.
	 * @param ctx the parse tree
	 */
	void exitSp_assign(@NotNull ExprLangParser.Sp_assignContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#unaryRealFn}.
	 * @param ctx the parse tree
	 */
	void enterUnaryRealFn(@NotNull ExprLangParser.UnaryRealFnContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#unaryRealFn}.
	 * @param ctx the parse tree
	 */
	void exitUnaryRealFn(@NotNull ExprLangParser.UnaryRealFnContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ColorTermNextPrev}.
	 * @param ctx the parse tree
	 */
	void enterColorTermNextPrev(@NotNull ExprLangParser.ColorTermNextPrevContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ColorTermNextPrev}.
	 * @param ctx the parse tree
	 */
	void exitColorTermNextPrev(@NotNull ExprLangParser.ColorTermNextPrevContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ClockConstrIdCmpReal}.
	 * @param ctx the parse tree
	 */
	void enterClockConstrIdCmpReal(@NotNull ExprLangParser.ClockConstrIdCmpRealContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ClockConstrIdCmpReal}.
	 * @param ctx the parse tree
	 */
	void exitClockConstrIdCmpReal(@NotNull ExprLangParser.ClockConstrIdCmpRealContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprCTLStar2}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprCTLStar2(@NotNull ExprLangParser.BoolExprCTLStar2Context ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprCTLStar2}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprCTLStar2(@NotNull ExprLangParser.BoolExprCTLStar2Context ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprCTL}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprCTL(@NotNull ExprLangParser.BoolExprCTLContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprCTL}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprCTL(@NotNull ExprLangParser.BoolExprCTLContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#mainPlaceColorDomainDef}.
	 * @param ctx the parse tree
	 */
	void enterMainPlaceColorDomainDef(@NotNull ExprLangParser.MainPlaceColorDomainDefContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#mainPlaceColorDomainDef}.
	 * @param ctx the parse tree
	 */
	void exitMainPlaceColorDomainDef(@NotNull ExprLangParser.MainPlaceColorDomainDefContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#anyIdentifier}.
	 * @param ctx the parse tree
	 */
	void enterAnyIdentifier(@NotNull ExprLangParser.AnyIdentifierContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#anyIdentifier}.
	 * @param ctx the parse tree
	 */
	void exitAnyIdentifier(@NotNull ExprLangParser.AnyIdentifierContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprIntComp}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprIntComp(@NotNull ExprLangParser.BoolExprIntCompContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprIntComp}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprIntComp(@NotNull ExprLangParser.BoolExprIntCompContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ClockConstrRealCmpId}.
	 * @param ctx the parse tree
	 */
	void enterClockConstrRealCmpId(@NotNull ExprLangParser.ClockConstrRealCmpIdContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ClockConstrRealCmpId}.
	 * @param ctx the parse tree
	 */
	void exitClockConstrRealCmpId(@NotNull ExprLangParser.ClockConstrRealCmpIdContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ActSetAll}.
	 * @param ctx the parse tree
	 */
	void enterActSetAll(@NotNull ExprLangParser.ActSetAllContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ActSetAll}.
	 * @param ctx the parse tree
	 */
	void exitActSetAll(@NotNull ExprLangParser.ActSetAllContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ColorSubclassIdentifier}.
	 * @param ctx the parse tree
	 */
	void enterColorSubclassIdentifier(@NotNull ExprLangParser.ColorSubclassIdentifierContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ColorSubclassIdentifier}.
	 * @param ctx the parse tree
	 */
	void exitColorSubclassIdentifier(@NotNull ExprLangParser.ColorSubclassIdentifierContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#CsltaExprPlace}.
	 * @param ctx the parse tree
	 */
	void enterCsltaExprPlace(@NotNull ExprLangParser.CsltaExprPlaceContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#CsltaExprPlace}.
	 * @param ctx the parse tree
	 */
	void exitCsltaExprPlace(@NotNull ExprLangParser.CsltaExprPlaceContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprParen}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprParen(@NotNull ExprLangParser.BoolExprParenContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprParen}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprParen(@NotNull ExprLangParser.BoolExprParenContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ActSetList}.
	 * @param ctx the parse tree
	 */
	void enterActSetList(@NotNull ExprLangParser.ActSetListContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ActSetList}.
	 * @param ctx the parse tree
	 */
	void exitActSetList(@NotNull ExprLangParser.ActSetListContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#TemporalOp2T}.
	 * @param ctx the parse tree
	 */
	void enterTemporalOp2T(@NotNull ExprLangParser.TemporalOp2TContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#TemporalOp2T}.
	 * @param ctx the parse tree
	 */
	void exitTemporalOp2T(@NotNull ExprLangParser.TemporalOp2TContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprCTLGlobalProperty}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprCTLGlobalProperty(@NotNull ExprLangParser.BoolExprCTLGlobalPropertyContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprCTLGlobalProperty}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprCTLGlobalProperty(@NotNull ExprLangParser.BoolExprCTLGlobalPropertyContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#mainBoolExpr}.
	 * @param ctx the parse tree
	 */
	void enterMainBoolExpr(@NotNull ExprLangParser.MainBoolExprContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#mainBoolExpr}.
	 * @param ctx the parse tree
	 */
	void exitMainBoolExpr(@NotNull ExprLangParser.MainBoolExprContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntMSetExprParen}.
	 * @param ctx the parse tree
	 */
	void enterIntMSetExprParen(@NotNull ExprLangParser.IntMSetExprParenContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntMSetExprParen}.
	 * @param ctx the parse tree
	 */
	void exitIntMSetExprParen(@NotNull ExprLangParser.IntMSetExprParenContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#CsltaExprParen}.
	 * @param ctx the parse tree
	 */
	void enterCsltaExprParen(@NotNull ExprLangParser.CsltaExprParenContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#CsltaExprParen}.
	 * @param ctx the parse tree
	 */
	void exitCsltaExprParen(@NotNull ExprLangParser.CsltaExprParenContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealExprCond2}.
	 * @param ctx the parse tree
	 */
	void enterRealExprCond2(@NotNull ExprLangParser.RealExprCond2Context ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealExprCond2}.
	 * @param ctx the parse tree
	 */
	void exitRealExprCond2(@NotNull ExprLangParser.RealExprCond2Context ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntExprColorNum}.
	 * @param ctx the parse tree
	 */
	void enterIntExprColorNum(@NotNull ExprLangParser.IntExprColorNumContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntExprColorNum}.
	 * @param ctx the parse tree
	 */
	void exitIntExprColorNum(@NotNull ExprLangParser.IntExprColorNumContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#BoolExprColorTermIn}.
	 * @param ctx the parse tree
	 */
	void enterBoolExprColorTermIn(@NotNull ExprLangParser.BoolExprColorTermInContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#BoolExprColorTermIn}.
	 * @param ctx the parse tree
	 */
	void exitBoolExprColorTermIn(@NotNull ExprLangParser.BoolExprColorTermInContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#CsltaExprOr}.
	 * @param ctx the parse tree
	 */
	void enterCsltaExprOr(@NotNull ExprLangParser.CsltaExprOrContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#CsltaExprOr}.
	 * @param ctx the parse tree
	 */
	void exitCsltaExprOr(@NotNull ExprLangParser.CsltaExprOrContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#mainRealExpr}.
	 * @param ctx the parse tree
	 */
	void enterMainRealExpr(@NotNull ExprLangParser.MainRealExprContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#mainRealExpr}.
	 * @param ctx the parse tree
	 */
	void exitMainRealExpr(@NotNull ExprLangParser.MainRealExprContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#act_assign}.
	 * @param ctx the parse tree
	 */
	void enterAct_assign(@NotNull ExprLangParser.Act_assignContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#act_assign}.
	 * @param ctx the parse tree
	 */
	void exitAct_assign(@NotNull ExprLangParser.Act_assignContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#mainRealMSetExpr}.
	 * @param ctx the parse tree
	 */
	void enterMainRealMSetExpr(@NotNull ExprLangParser.MainRealMSetExprContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#mainRealMSetExpr}.
	 * @param ctx the parse tree
	 */
	void exitMainRealMSetExpr(@NotNull ExprLangParser.MainRealMSetExprContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#IntExprMulDiv}.
	 * @param ctx the parse tree
	 */
	void enterIntExprMulDiv(@NotNull ExprLangParser.IntExprMulDivContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#IntExprMulDiv}.
	 * @param ctx the parse tree
	 */
	void exitIntExprMulDiv(@NotNull ExprLangParser.IntExprMulDivContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#TagRewriteListList}.
	 * @param ctx the parse tree
	 */
	void enterTagRewriteListList(@NotNull ExprLangParser.TagRewriteListListContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#TagRewriteListList}.
	 * @param ctx the parse tree
	 */
	void exitTagRewriteListList(@NotNull ExprLangParser.TagRewriteListListContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#ColorSetSubclassOf2}.
	 * @param ctx the parse tree
	 */
	void enterColorSetSubclassOf2(@NotNull ExprLangParser.ColorSetSubclassOf2Context ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#ColorSetSubclassOf2}.
	 * @param ctx the parse tree
	 */
	void exitColorSetSubclassOf2(@NotNull ExprLangParser.ColorSetSubclassOf2Context ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#VarFlowListEmpty}.
	 * @param ctx the parse tree
	 */
	void enterVarFlowListEmpty(@NotNull ExprLangParser.VarFlowListEmptyContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#VarFlowListEmpty}.
	 * @param ctx the parse tree
	 */
	void exitVarFlowListEmpty(@NotNull ExprLangParser.VarFlowListEmptyContext ctx);

	/**
	 * Enter a parse tree produced by {@link ExprLangParser#RealExprNegate}.
	 * @param ctx the parse tree
	 */
	void enterRealExprNegate(@NotNull ExprLangParser.RealExprNegateContext ctx);
	/**
	 * Exit a parse tree produced by {@link ExprLangParser#RealExprNegate}.
	 * @param ctx the parse tree
	 */
	void exitRealExprNegate(@NotNull ExprLangParser.RealExprNegateContext ctx);
}