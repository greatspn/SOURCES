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
import editor.domain.io.XmlExchangeable;
import java.io.Serializable;
import java.util.Map;
import java.util.Scanner;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

/** Base class of computed results, 
 * for a given binding assignment of the template parameters.
 *
 * @author elvio
 */
public abstract class ResultEntry implements Serializable, XmlExchangeable {
    
    // Matching string name in the solver output.
    public final String entryName;
    
    public static enum ResultState {
        NOT_YET_COMPUTED,
        COMPUTED_OK,
        FAIL_COMPUTE
    }
    // Has the solver computed this entry correctly?
    private ResultState state;
    private String failureReason ="";
        
    // Variables used to compute this result
    protected EvaluatedBinding assign;
    
    public ResultEntry() { 
        entryName = null;
        state = null;
        assign = null;
    }

    public ResultEntry(String entryName, EvaluatedBinding assign) {
        this.state = ResultState.NOT_YET_COMPUTED;
        this.entryName = entryName;
        this.assign = assign;
    }
    
    protected void setComputedOk() {
        this.state = ResultState.COMPUTED_OK;
    }
        
    public void setComputeFailed(String reason) {
        this.state = ResultState.FAIL_COMPUTE;
        this.failureReason = reason;
    }
    
    public boolean isNotYetComputed() {
        return state == ResultState.NOT_YET_COMPUTED;
    }
    public boolean isComputedOk() {
        return state == ResultState.COMPUTED_OK;
    }

    @Override
    public final String toString() {
        if (state == ResultState.NOT_YET_COMPUTED)
            return "<not yet computed>";
        else if (state == ResultState.FAIL_COMPUTE)
            return failureReason.length()==0 ? "<failed>" : failureReason;
        return resultToString();
    }
    
    protected abstract String resultToString();
    protected abstract void parseResult(Scanner scanner, NetPage targetNet);
    
    // Is result parsed with a single line or with multiple lines?
    protected abstract boolean isMultiLine();

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        XmlExchangeUtils.bindXMLAttrib(this, el, exDir, "name", "entryName", null, String.class);
        XmlExchangeUtils.bindXMLAttrib(this, el, exDir, "state", "state", ResultState.COMPUTED_OK);
        XmlExchangeUtils.bindXMLAttrib(this, el, exDir, "failure_reason", "failureReason", "");
        
        if (exDir.FieldsToXml()) {
            if (assign != null) {
                Element bindingsEl = exDir.getDocument().createElement("bindings");
                for (Map.Entry<String, String> binding : assign.binding.entrySet()) {
                    Element bindingEl = exDir.getDocument().createElement("binding");
                    bindingEl.setAttribute("name", binding.getKey());
                    bindingEl.setAttribute("value", binding.getValue());
                    bindingsEl.appendChild(bindingEl);
                }
                el.appendChild(bindingsEl);
            }
        }
        else { // XML -> fields
            NodeList resultList = el.getChildNodes();
            for (int i=0; i<resultList.getLength(); i++) {
                org.w3c.dom.Node resourceItem = resultList.item(i);
                if (resourceItem.getNodeType() != org.w3c.dom.Node.ELEMENT_NODE)
                    continue;
                Element resultElem = (Element)resourceItem;
                if (resultElem.getNodeName().equals("bindings")) {
                    assign = new EvaluatedBinding();
                    // Read variable->expr bindings
                    NodeList bindingList = el.getElementsByTagName("binding");
                    for (int bnd=0; bnd<bindingList.getLength(); bnd++) {
                        Element bindingEl = (Element)bindingList.item(bnd);
                        // Read variable->expr value binding
                        assign.bindEvaluatedValue(bindingEl.getAttribute("name"), 
                                                  bindingEl.getAttribute("value"));
                    }
                }
            }
        }
    }
}
