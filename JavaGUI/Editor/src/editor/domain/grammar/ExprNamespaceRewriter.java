/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.grammar;

import java.util.ArrayList;
import org.antlr.v4.runtime.tree.ErrorNode;
import org.antlr.v4.runtime.tree.ParseTree;
import org.antlr.v4.runtime.tree.RuleNode;
import org.antlr.v4.runtime.tree.TerminalNode;

/** Appends a prefix to all identifiers that enters in a namespace,
 * like: 1 + #P0 * R   ->  1 + #net.P0 * net.R
 *
 * @author elvio
 */
public class ExprNamespaceRewriter implements ExprRewriter {
    
    // Appended prefix
    private final String prefix;
    
    // Where prefix should be appended
    private final ArrayList<Integer> prefixPos = new ArrayList<>();
    private boolean foundErrors = false;
    
    public ExprNamespaceRewriter(String prefix) {
        this.prefix = prefix;
    }
    
    @Override
    public void startRewriting() {
        foundErrors = false;
        prefixPos.clear();
    }
    
    // Generate the rewritten expression by inserting the prefix in the right positions.
    @Override
    public String getRewrittenExpr(String expr) {
        if (foundErrors)
            return expr; // No rewriting
        for (int i=prefixPos.size()-1; i>=0; i--) {
            int pos = prefixPos.get(i);
            expr = expr.substring(0, pos) + prefix + expr.substring(pos);
        }
        return expr;
    }

    @Override
    public Void visit(ParseTree pt) {
        if (pt instanceof TerminalNode)
            return visitTerminal((TerminalNode)pt);
        else if (pt instanceof RuleNode)
            return visitChildren((RuleNode)pt);
        else if (pt instanceof ErrorNode)
            return visitErrorNode((ErrorNode)pt);
        else
            throw new IllegalStateException("Unknown ParseTree node type.");
    }

    @Override
    public Void visitChildren(RuleNode rn) {
        for (int i=0; i<rn.getChildCount(); i++) {
            visit(rn.getChild(i));
        }
        return null;
    }

    @Override
    public Void visitTerminal(TerminalNode tn) {
        switch (tn.getSymbol().getType()) {
            case ExprLangParser.EOF:
                break;
            
                // Places
            case ExprLangParser.INT_PLACE_ID:
            case ExprLangParser.REAL_PLACE_ID:
                // Transitions
            case ExprLangParser.TRANSITION_ID:
                // Constants
            case ExprLangParser.INT_CONST_ID:
            case ExprLangParser.REAL_CONST_ID:
            case ExprLangParser.INT_MSET_CONST_ID:
            case ExprLangParser.REAL_MSET_CONST_ID:
                // Color classes, domains and variables
            case ExprLangParser.COLORDOMAIN_ID:
            case ExprLangParser.SIMPLECOLORCLASS_ID:
            case ExprLangParser.COLORVAR_ID:
                // Template variables
            case ExprLangParser.INT_TEMPLATE_ID:
            case ExprLangParser.REAL_TEMPLATE_ID:
            case ExprLangParser.CLOCK_ID:
            case ExprLangParser.ACTION_ID:
            case ExprLangParser.STATEPROP_ID:
                prefixPos.add(tn.getSymbol().getCharPositionInLine());
                break;
                
            default:
                break;
        }
        return null;
    }

    @Override
    public Void visitErrorNode(ErrorNode en) {
        foundErrors = true;
        return null;
    }
    
    
}
