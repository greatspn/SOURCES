/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package editor.domain.grammar;

import editor.domain.Node;
import editor.domain.elements.Place;
import java.util.HashSet;
import java.util.Set;
import org.antlr.v4.runtime.tree.ErrorNode;
import org.antlr.v4.runtime.tree.ParseTree;
import org.antlr.v4.runtime.tree.RuleNode;
import org.antlr.v4.runtime.tree.TerminalNode;

/**
 *
 * @author Irene
 */
public class ExprSubFormulaAnalysist implements ExprRewriter {
    
    private final Set<String> places;
    
    public ExprSubFormulaAnalysist(Set<String> places){
        this.places = places;  
    }
    

    @Override
    public void startRewriting() {
    }
    
    public Set<String> getPlaces(){
        return places;
    }

    @Override
    public String getRewrittenExpr(String expr) {
        if(!places.isEmpty()){
            return "Places Present";
        }
        return null;
    }

    @Override
    public Object visit(ParseTree pt) {
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
    public Object visitChildren(RuleNode rn) {
                for (int i=0; i<rn.getChildCount(); i++) {
            visit(rn.getChild(i));
        }
        return null;
    }

    @Override
    public Object visitTerminal(TerminalNode tn) {
         switch (tn.getSymbol().getType()) {
            case ExprLangParser.EOF:
                break;
            case ExprLangParser.INT_PLACE_ID:
                this.places.add(tn.getText());
            case ExprLangParser.REAL_PLACE_ID:
                this.places.add(tn.getText());
            case ExprLangParser.INT_MSET_PLACE_ID:
                this.places.add(tn.getText());
            case ExprLangParser.REAL_MSET_PLACE_ID:
                this.places.add(tn.getText());


         }
        return null;
        
    }

    @Override
    public Object visitErrorNode(ErrorNode en) {
        return null;
    }
    

}
