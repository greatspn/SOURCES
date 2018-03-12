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
import java.util.Scanner;
import org.w3c.dom.Element;

/** The scalar result of a performance measure
 *
 * @author elvio
 */
public class ScalarResultEntry extends ResultEntry {
    
    // The computed result
    private ComputedScalar value;

    public ScalarResultEntry() { }
    
    public ScalarResultEntry(String entryName, EvaluatedBinding assign) {
        super(entryName, assign);
    }

    public void setComputedValue(ComputedScalar value) {
        this.value = value;
        setComputedOk();
    }
    
    // Is an interval of confidence, or an exact value?
    public boolean isInterval() {
        return (value != null && value.isInterval());
    }

    @Override
    protected String resultToString() {
        assert value != null;
        return value.toString();
    }

    @Override
    protected void parseResult(Scanner scanner, NetPage targetNet) {
        ComputedScalar nv = ComputedScalar.makeFromScanner(scanner);
        if (nv != null)
            setComputedValue(nv);
        else
            setComputeFailed("<data format error>");
    }
    
    @Override
    protected boolean isMultiLine() {
        return false;
    }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        super.exchangeXML(el, exDir);
        
        if (isComputedOk()) {
            if (exDir.XmlToFields())
                value = new ComputedScalar();
            if (value != null)
                value.exchangeXML(el, exDir);
        }
    }
}
