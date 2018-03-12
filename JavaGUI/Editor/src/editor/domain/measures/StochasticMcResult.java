/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import editor.domain.NetPage;
import editor.domain.grammar.EvaluatedBinding;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import editor.domain.io.XmlExchangeUtils;
import java.util.Scanner;
import org.w3c.dom.Element;

/** The result of a stochastic model checker on a single state.
 * This result is made by a truth value, that says if the model checkign
 * query is satisfied, and an optional probability value.
 *
 * @author elvio
 */
public class StochasticMcResult extends ResultEntry {
    private double prob = -1;
    private boolean satisfied;
    
    public StochasticMcResult() { }

    public StochasticMcResult(String entryName, EvaluatedBinding assign) {
        super(entryName, assign);
    }

    public void setComputedValue(double prob, boolean satisfied) {
        this.prob = prob;
        this.satisfied = satisfied;
        setComputedOk();
    }

    @Override
    protected String resultToString() {
        if (prob == -1)
            return ""+satisfied;
        else
            return ""+satisfied+" "+prob;
    }
    
    @Override
    protected void parseResult(Scanner scanner, NetPage targetNet) {
        // Get the SAT value
        boolean isSat = scanner.nextBoolean();
        // Get the result in double
        double probability = -1;
        if (scanner.hasNext())
            probability = Double.parseDouble(scanner.next());
        
        setComputedValue(probability, isSat);
    }

    @Override
    protected boolean isMultiLine() {
        return false;
    }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        super.exchangeXML(el, exDir); 
        
        if (isComputedOk()) {
            XmlExchangeUtils.bindXMLAttrib(this, el, exDir, "probability", "prob", -1.0);
            XmlExchangeUtils.bindXMLAttrib(this, el, exDir, "satisfied", "satisfied", null, boolean.class);
        }
    }
}
