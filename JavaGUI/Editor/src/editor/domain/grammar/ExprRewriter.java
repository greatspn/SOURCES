/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.grammar;

import org.antlr.v4.runtime.tree.ParseTreeVisitor;

/** Expression rewriter (must implement ParseTreeVisitor)
 *
 * @author elvio
 */
public interface ExprRewriter extends ParseTreeVisitor<Object> {
    
    public void startRewriting();
    
    public String getRewrittenExpr(String expr);
}
