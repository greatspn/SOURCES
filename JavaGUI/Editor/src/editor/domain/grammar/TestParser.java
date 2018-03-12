/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.grammar;

import editor.domain.values.EvaluatedFormula;
import java.util.Collections;
import java.util.List;
import org.antlr.v4.runtime.ANTLRInputStream;
import org.antlr.v4.runtime.BaseErrorListener;
import org.antlr.v4.runtime.CharStream;
import org.antlr.v4.runtime.CommonTokenStream;
import org.antlr.v4.runtime.Parser;
import org.antlr.v4.runtime.RecognitionException;
import org.antlr.v4.runtime.Recognizer;
import org.antlr.v4.runtime.TokenStream;
import org.antlr.v4.runtime.tree.ParseTree;

/**
 *
 * @author elvio
 */
public class TestParser {
    
    public static void main(String[] args) {
//        String text = "If[34 >= Sqrt[3.14 * 67], 12, 34 + Ceil[Binomial[3,2]]] + Round[Fract[Pow[3+2,5],5*Factorial[6]]]";
//        String text = "2 + Ceil[Sin[3] + Cos[Fract[3,4]] + Pow[Atan[3.14 * 78], 4]]";
        String text = "1.0/16";
        CharStream stream = new ANTLRInputStream(text);
        ExprLangLexer lexer = new ExprLangLexer(stream);
        TokenStream tokens = new CommonTokenStream(lexer);
        ExprLangParser parser = new ExprLangParser(tokens);

        // Setup the parser and lexer contexts
        ParserContext context = new ParserContext(null);
        lexer.context = context;
        parser.context = context;
        
        parser.setBuildParseTree(true);
//        parser.addParseListener(new ExprLangBaseListener(){
////
////            @Override
////            public void exitIntExpr(ExprLangParser.IntExprContext ctx) {
////                System.err.println("exit("+ctx+")");
////                System.err.println(ctx.getStart());
////                System.err.println(ctx.getStop());
////            }
//
//            @Override
//            public void visitTerminal(TerminalNode node) {
//                System.out.println("terminal("+node.getSymbol().getText()+") type="+node.getSymbol().getType());
//            }
//                        
//        });
        //ExprLangParser.MainBoolExprContext uu = parser.mainBoolExpr();
        parser.removeErrorListeners(); // remove ConsoleErrorListener
        parser.addErrorListener(new BaseErrorListener() {

            @Override
            public void syntaxError(Recognizer<?, ?> recognizer, Object offendingSymbol, int line, 
                                    int charPositionInLine, String msg, RecognitionException e) 
            {
                List<String> stack = ((Parser)recognizer).getRuleInvocationStack(); 
                Collections.reverse(stack); 
                System.err.println("rule stack: "+stack); 
                System.err.println("line "+line+":"+charPositionInLine+" at "+
                                   offendingSymbol+": "+msg);
            }
            
            
            
        });
        int parseFlags = 0;
        ParseTree tree = parser.mainIntExpr();
        ExprLangVisitor<FormattedFormula> semanticParser = new SemanticParser(context, parseFlags);
        FormattedFormula result = semanticParser.visit(tree);
        System.out.println(result.getFormula());
        
        ExprLangVisitor<EvaluatedFormula> evaluator = new FormulaEvaluator(context, EvaluationArguments.NO_ARGS);
        EvaluatedFormula value = evaluator.visit(tree);
        System.out.println(value);
        
        //System.out.println(uu.toStringTree());
    }
}
