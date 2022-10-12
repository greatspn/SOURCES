// Generated from JavaGUI/Editor/src/editor/domain/grammar/ExprLang.g4 by ANTLR 4.2.1
package editor.domain.grammar;

import editor.domain.*;
import editor.domain.elements.*;

import org.antlr.v4.runtime.misc.NotNull;
import org.antlr.v4.runtime.tree.ParseTreeVisitor;

/**
 * This interface defines a complete generic visitor for a parse tree produced
 * by {@link ExprLangParser}.
 *
 * @param <T> The return type of the visit operation. Use {@link Void} for
 * operations with no return type.
 */
public interface ExprLangVisitor<T> extends ParseTreeVisitor<T> {
	/**
	 * Visit a parse tree produced by {@link ExprLangParser#mainClockDef}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMainClockDef(@NotNull ExprLangParser.MainClockDefContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#CsltaExprBool}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitCsltaExprBool(@NotNull ExprLangParser.CsltaExprBoolContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprCTLinitState}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprCTLinitState(@NotNull ExprLangParser.BoolExprCTLinitStateContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealConstLiteral}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealConstLiteral(@NotNull ExprLangParser.RealConstLiteralContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ComposTempOpAG}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitComposTempOpAG(@NotNull ExprLangParser.ComposTempOpAGContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntExprConst}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntExprConst(@NotNull ExprLangParser.IntExprConstContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#binaryIntFn}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBinaryIntFn(@NotNull ExprLangParser.BinaryIntFnContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#MSetElemAddSub}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMSetElemAddSub(@NotNull ExprLangParser.MSetElemAddSubContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ActSetAllExceptList}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitActSetAllExceptList(@NotNull ExprLangParser.ActSetAllExceptListContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#mainActBinding}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMainActBinding(@NotNull ExprLangParser.MainActBindingContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntExprParen}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntExprParen(@NotNull ExprLangParser.IntExprParenContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ColorSetSubclassOf}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitColorSetSubclassOf(@NotNull ExprLangParser.ColorSetSubclassOfContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealExprFromTable}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealExprFromTable(@NotNull ExprLangParser.RealExprFromTableContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ColorSetSubClass}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitColorSetSubClass(@NotNull ExprLangParser.ColorSetSubClassContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ComposTempOpAX}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitComposTempOpAX(@NotNull ExprLangParser.ComposTempOpAXContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntExprCond2}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntExprCond2(@NotNull ExprLangParser.IntExprCond2Context ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#mainStatePropExpr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMainStatePropExpr(@NotNull ExprLangParser.MainStatePropExprContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprCTLStarQuantif}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprCTLStarQuantif(@NotNull ExprLangParser.BoolExprCTLStarQuantifContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprImply}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprImply(@NotNull ExprLangParser.BoolExprImplyContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ClockVarId}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitClockVarId(@NotNull ExprLangParser.ClockVarIdContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprCTLpin}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprCTLpin(@NotNull ExprLangParser.BoolExprCTLpinContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprBiimply}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprBiimply(@NotNull ExprLangParser.BoolExprBiimplyContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#TemporalOpForallX}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitTemporalOpForallX(@NotNull ExprLangParser.TemporalOpForallXContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ColorSetAll}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitColorSetAll(@NotNull ExprLangParser.ColorSetAllContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ActListEmpty}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitActListEmpty(@NotNull ExprLangParser.ActListEmptyContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealMSetExprAddSub}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealMSetExprAddSub(@NotNull ExprLangParser.RealMSetExprAddSubContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#VarUpdateListEmpty}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitVarUpdateListEmpty(@NotNull ExprLangParser.VarUpdateListEmptyContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealExprCall}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealExprCall(@NotNull ExprLangParser.RealExprCallContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ColorSetSubClass2}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitColorSetSubClass2(@NotNull ExprLangParser.ColorSetSubClass2Context ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntExprUnknownPlaceId}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntExprUnknownPlaceId(@NotNull ExprLangParser.IntExprUnknownPlaceIdContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprCTLenabled}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprCTLenabled(@NotNull ExprLangParser.BoolExprCTLenabledContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntExprCardMSet}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntExprCardMSet(@NotNull ExprLangParser.IntExprCardMSetContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprStatePropId}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprStatePropId(@NotNull ExprLangParser.BoolExprStatePropIdContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ColorListIDs}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitColorListIDs(@NotNull ExprLangParser.ColorListIDsContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ColorTermColor}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitColorTermColor(@NotNull ExprLangParser.ColorTermColorContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ColorSetTerm}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitColorSetTerm(@NotNull ExprLangParser.ColorSetTermContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#mainRealExprList}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMainRealExprList(@NotNull ExprLangParser.MainRealExprListContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprCTLdeadlocks}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprCTLdeadlocks(@NotNull ExprLangParser.BoolExprCTLdeadlocksContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#colorVarDef}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitColorVarDef(@NotNull ExprLangParser.ColorVarDefContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealMSetExprElemProduct}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealMSetExprElemProduct(@NotNull ExprLangParser.RealMSetExprElemProductContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#boolConst}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolConst(@NotNull ExprLangParser.BoolConstContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprCTLStarUntil2}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprCTLStarUntil2(@NotNull ExprLangParser.BoolExprCTLStarUntil2Context ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealExprClockVar}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealExprClockVar(@NotNull ExprLangParser.RealExprClockVarContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealExprInt}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealExprInt(@NotNull ExprLangParser.RealExprIntContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ClockConstrEquals}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitClockConstrEquals(@NotNull ExprLangParser.ClockConstrEqualsContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#mainMeasure}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMainMeasure(@NotNull ExprLangParser.MainMeasureContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntExprCTLBound}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntExprCTLBound(@NotNull ExprLangParser.IntExprCTLBoundContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealExprPdfXVar}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealExprPdfXVar(@NotNull ExprLangParser.RealExprPdfXVarContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ColorListInterval}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitColorListInterval(@NotNull ExprLangParser.ColorListIntervalContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntExprCond}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntExprCond(@NotNull ExprLangParser.IntExprCondContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#VarUpdateListList}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitVarUpdateListList(@NotNull ExprLangParser.VarUpdateListListContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#TagComplDefinition}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitTagComplDefinition(@NotNull ExprLangParser.TagComplDefinitionContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprColorTermComp}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprColorTermComp(@NotNull ExprLangParser.BoolExprColorTermCompContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ColorTermFilterThis}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitColorTermFilterThis(@NotNull ExprLangParser.ColorTermFilterThisContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ClockConstrAnd}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitClockConstrAnd(@NotNull ExprLangParser.ClockConstrAndContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#colorClassOrd}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitColorClassOrd(@NotNull ExprLangParser.ColorClassOrdContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ColorSetClass}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitColorSetClass(@NotNull ExprLangParser.ColorSetClassContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#mainCsltaExpr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMainCsltaExpr(@NotNull ExprLangParser.MainCsltaExprContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprCTLStarUntil}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprCTLStarUntil(@NotNull ExprLangParser.BoolExprCTLStarUntilContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntExprUnaryFn}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntExprUnaryFn(@NotNull ExprLangParser.IntExprUnaryFnContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealConstId}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealConstId(@NotNull ExprLangParser.RealConstIdContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#VarUpdateValue}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitVarUpdateValue(@NotNull ExprLangParser.VarUpdateValueContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#mainClockConstr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMainClockConstr(@NotNull ExprLangParser.MainClockConstrContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntExprPlaceMarking}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntExprPlaceMarking(@NotNull ExprLangParser.IntExprPlaceMarkingContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprCTLStar}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprCTLStar(@NotNull ExprLangParser.BoolExprCTLStarContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealExprFromTimeTable}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealExprFromTimeTable(@NotNull ExprLangParser.RealExprFromTimeTableContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealExprUnaryFn}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealExprUnaryFn(@NotNull ExprLangParser.RealExprUnaryFnContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntConstId}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntConstId(@NotNull ExprLangParser.IntConstIdContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprRealComp}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprRealComp(@NotNull ExprLangParser.BoolExprRealCompContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#MSetElemBoolPredicate}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMSetElemBoolPredicate(@NotNull ExprLangParser.MSetElemBoolPredicateContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealExprCond}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealExprCond(@NotNull ExprLangParser.RealExprCondContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#CsltaExprProbTA}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitCsltaExprProbTA(@NotNull ExprLangParser.CsltaExprProbTAContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ColorVarIdentifier}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitColorVarIdentifier(@NotNull ExprLangParser.ColorVarIdentifierContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#MeasureE}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMeasureE(@NotNull ExprLangParser.MeasureEContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#TemporalOpExistX}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitTemporalOpExistX(@NotNull ExprLangParser.TemporalOpExistXContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#MSetBoolPredicate}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMSetBoolPredicate(@NotNull ExprLangParser.MSetBoolPredicateContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ActListList}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitActListList(@NotNull ExprLangParser.ActListListContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntConstLiteral}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntConstLiteral(@NotNull ExprLangParser.IntConstLiteralContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#binaryRealFn}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBinaryRealFn(@NotNull ExprLangParser.BinaryRealFnContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntOrRealListEmptyList}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntOrRealListEmptyList(@NotNull ExprLangParser.IntOrRealListEmptyListContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#mainIntExpr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMainIntExpr(@NotNull ExprLangParser.MainIntExprContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#mainIntMSetExpr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMainIntMSetExpr(@NotNull ExprLangParser.MainIntMSetExprContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ColorClassDefProduct}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitColorClassDefProduct(@NotNull ExprLangParser.ColorClassDefProductContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#MSetElemColorTerm}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMSetElemColorTerm(@NotNull ExprLangParser.MSetElemColorTermContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprUnknownId}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprUnknownId(@NotNull ExprLangParser.BoolExprUnknownIdContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#PlaceDomainColorDomain}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitPlaceDomainColorDomain(@NotNull ExprLangParser.PlaceDomainColorDomainContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#MeasureP}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMeasureP(@NotNull ExprLangParser.MeasurePContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntMsetExprPlaceMarking}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntMsetExprPlaceMarking(@NotNull ExprLangParser.IntMsetExprPlaceMarkingContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntExprUnaryRealFn}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntExprUnaryRealFn(@NotNull ExprLangParser.IntExprUnaryRealFnContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntExprUnknownId}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntExprUnknownId(@NotNull ExprLangParser.IntExprUnknownIdContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#TagRewriteListEmpty}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitTagRewriteListEmpty(@NotNull ExprLangParser.TagRewriteListEmptyContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntOrRealListReal}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntOrRealListReal(@NotNull ExprLangParser.IntOrRealListRealContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#MeasureX}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMeasureX(@NotNull ExprLangParser.MeasureXContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealMsetExprConst}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealMsetExprConst(@NotNull ExprLangParser.RealMsetExprConstContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealExprParen}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealExprParen(@NotNull ExprLangParser.RealExprParenContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealTemplateId}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealTemplateId(@NotNull ExprLangParser.RealTemplateIdContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ComposTempOpEG}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitComposTempOpEG(@NotNull ExprLangParser.ComposTempOpEGContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ComposTempOpF}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitComposTempOpF(@NotNull ExprLangParser.ComposTempOpFContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ComposTempOpEF}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitComposTempOpEF(@NotNull ExprLangParser.ComposTempOpEFContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#TagRewriteRule}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitTagRewriteRule(@NotNull ExprLangParser.TagRewriteRuleContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ClockConstrBetween}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitClockConstrBetween(@NotNull ExprLangParser.ClockConstrBetweenContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntExprBinaryFn}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntExprBinaryFn(@NotNull ExprLangParser.IntExprBinaryFnContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealExprConst}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealExprConst(@NotNull ExprLangParser.RealExprConstContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#VarFlowListList}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitVarFlowListList(@NotNull ExprLangParser.VarFlowListListContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealExprPlaceMarking}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealExprPlaceMarking(@NotNull ExprLangParser.RealExprPlaceMarkingContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#PlaceDomainColorClass}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitPlaceDomainColorClass(@NotNull ExprLangParser.PlaceDomainColorClassContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprOr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprOr(@NotNull ExprLangParser.BoolExprOrContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntMSetExprAddSub}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntMSetExprAddSub(@NotNull ExprLangParser.IntMSetExprAddSubContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#real_assign}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitReal_assign(@NotNull ExprLangParser.Real_assignContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntExprNegate}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntExprNegate(@NotNull ExprLangParser.IntExprNegateContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ComposTempOpEX}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitComposTempOpEX(@NotNull ExprLangParser.ComposTempOpEXContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ColorSubclassNameDef}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitColorSubclassNameDef(@NotNull ExprLangParser.ColorSubclassNameDefContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprAnd}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprAnd(@NotNull ExprLangParser.BoolExprAndContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#mainColorClassDef}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMainColorClassDef(@NotNull ExprLangParser.MainColorClassDefContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealExprMulDiv}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealExprMulDiv(@NotNull ExprLangParser.RealExprMulDivContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprNot}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprNot(@NotNull ExprLangParser.BoolExprNotContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#mainActSet}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMainActSet(@NotNull ExprLangParser.MainActSetContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealExprAddSub}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealExprAddSub(@NotNull ExprLangParser.RealExprAddSubContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealExprBinaryFn}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealExprBinaryFn(@NotNull ExprLangParser.RealExprBinaryFnContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#MeasureAddSub}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMeasureAddSub(@NotNull ExprLangParser.MeasureAddSubContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprConst}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprConst(@NotNull ExprLangParser.BoolExprConstContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntMSetExprConst}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntMSetExprConst(@NotNull ExprLangParser.IntMSetExprConstContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntOrRealListInt}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntOrRealListInt(@NotNull ExprLangParser.IntOrRealListIntContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#CsltaExprAnd}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitCsltaExprAnd(@NotNull ExprLangParser.CsltaExprAndContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealFromIntConst}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealFromIntConst(@NotNull ExprLangParser.RealFromIntConstContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ColorClassDefNamedSet}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitColorClassDefNamedSet(@NotNull ExprLangParser.ColorClassDefNamedSetContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#mainColorVarDef}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMainColorVarDef(@NotNull ExprLangParser.MainColorVarDefContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#CsltaExprNot}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitCsltaExprNot(@NotNull ExprLangParser.CsltaExprNotContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#mainIntOrInfiniteExpr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMainIntOrInfiniteExpr(@NotNull ExprLangParser.MainIntOrInfiniteExprContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprCTLUntil}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprCTLUntil(@NotNull ExprLangParser.BoolExprCTLUntilContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealExprFromList}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealExprFromList(@NotNull ExprLangParser.RealExprFromListContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#unaryIntFn}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitUnaryIntFn(@NotNull ExprLangParser.UnaryIntFnContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ColorTermVar}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitColorTermVar(@NotNull ExprLangParser.ColorTermVarContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ActBindingTransition}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitActBindingTransition(@NotNull ExprLangParser.ActBindingTransitionContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#unaryIntRealFn}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitUnaryIntRealFn(@NotNull ExprLangParser.UnaryIntRealFnContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#VarFlowDefinition}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitVarFlowDefinition(@NotNull ExprLangParser.VarFlowDefinitionContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#mainVarFlow}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMainVarFlow(@NotNull ExprLangParser.MainVarFlowContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealExprUnknownId}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealExprUnknownId(@NotNull ExprLangParser.RealExprUnknownIdContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntExprAddSub}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntExprAddSub(@NotNull ExprLangParser.IntExprAddSubContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#mainTagRewriteList}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMainTagRewriteList(@NotNull ExprLangParser.MainTagRewriteListContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntTemplateId}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntTemplateId(@NotNull ExprLangParser.IntTemplateIdContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealMsetExprPlaceMarking}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealMsetExprPlaceMarking(@NotNull ExprLangParser.RealMsetExprPlaceMarkingContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprCTLUntil2}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprCTLUntil2(@NotNull ExprLangParser.BoolExprCTLUntil2Context ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#mainIntExprList}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMainIntExprList(@NotNull ExprLangParser.MainIntExprListContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ColorSetClass2}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitColorSetClass2(@NotNull ExprLangParser.ColorSetClass2Context ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealMSetExprParen}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealMSetExprParen(@NotNull ExprLangParser.RealMSetExprParenContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#TagDefinition}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitTagDefinition(@NotNull ExprLangParser.TagDefinitionContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntMSetExprElemProduct}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntMSetExprElemProduct(@NotNull ExprLangParser.IntMSetExprElemProductContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#sp_assign}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitSp_assign(@NotNull ExprLangParser.Sp_assignContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#unaryRealFn}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitUnaryRealFn(@NotNull ExprLangParser.UnaryRealFnContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ColorTermNextPrev}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitColorTermNextPrev(@NotNull ExprLangParser.ColorTermNextPrevContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ClockConstrIdCmpReal}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitClockConstrIdCmpReal(@NotNull ExprLangParser.ClockConstrIdCmpRealContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprCTLStar2}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprCTLStar2(@NotNull ExprLangParser.BoolExprCTLStar2Context ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprCTL}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprCTL(@NotNull ExprLangParser.BoolExprCTLContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#mainPlaceColorDomainDef}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMainPlaceColorDomainDef(@NotNull ExprLangParser.MainPlaceColorDomainDefContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#anyIdentifier}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitAnyIdentifier(@NotNull ExprLangParser.AnyIdentifierContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprIntComp}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprIntComp(@NotNull ExprLangParser.BoolExprIntCompContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ClockConstrRealCmpId}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitClockConstrRealCmpId(@NotNull ExprLangParser.ClockConstrRealCmpIdContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ActSetAll}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitActSetAll(@NotNull ExprLangParser.ActSetAllContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ColorSubclassIdentifier}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitColorSubclassIdentifier(@NotNull ExprLangParser.ColorSubclassIdentifierContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#CsltaExprPlace}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitCsltaExprPlace(@NotNull ExprLangParser.CsltaExprPlaceContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprParen}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprParen(@NotNull ExprLangParser.BoolExprParenContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ActSetList}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitActSetList(@NotNull ExprLangParser.ActSetListContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#TemporalOp2T}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitTemporalOp2T(@NotNull ExprLangParser.TemporalOp2TContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#mainBoolExpr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMainBoolExpr(@NotNull ExprLangParser.MainBoolExprContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntMSetExprParen}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntMSetExprParen(@NotNull ExprLangParser.IntMSetExprParenContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#CsltaExprParen}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitCsltaExprParen(@NotNull ExprLangParser.CsltaExprParenContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealExprCond2}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealExprCond2(@NotNull ExprLangParser.RealExprCond2Context ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntExprColorNum}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntExprColorNum(@NotNull ExprLangParser.IntExprColorNumContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#BoolExprColorTermIn}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolExprColorTermIn(@NotNull ExprLangParser.BoolExprColorTermInContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#CsltaExprOr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitCsltaExprOr(@NotNull ExprLangParser.CsltaExprOrContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#mainRealExpr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMainRealExpr(@NotNull ExprLangParser.MainRealExprContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#act_assign}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitAct_assign(@NotNull ExprLangParser.Act_assignContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#mainRealMSetExpr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMainRealMSetExpr(@NotNull ExprLangParser.MainRealMSetExprContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#IntExprMulDiv}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntExprMulDiv(@NotNull ExprLangParser.IntExprMulDivContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#TagRewriteListList}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitTagRewriteListList(@NotNull ExprLangParser.TagRewriteListListContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#ColorSetSubclassOf2}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitColorSetSubclassOf2(@NotNull ExprLangParser.ColorSetSubclassOf2Context ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#VarFlowListEmpty}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitVarFlowListEmpty(@NotNull ExprLangParser.VarFlowListEmptyContext ctx);

	/**
	 * Visit a parse tree produced by {@link ExprLangParser#RealExprNegate}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealExprNegate(@NotNull ExprLangParser.RealExprNegateContext ctx);
}