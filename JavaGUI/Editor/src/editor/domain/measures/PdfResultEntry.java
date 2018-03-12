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
import java.io.File;
import java.util.Scanner;
import org.w3c.dom.Element;

/** A result of a computation that is an external PDF file.
 *
 * @author elvio
 */
public class PdfResultEntry extends ResultEntry {
    
    // The PDF filename
    private File pdfFile;
    
    public PdfResultEntry() { 
        pdfFile = null;
    }

    public PdfResultEntry(String entryName, EvaluatedBinding assign, File pdfFile) {
        super(entryName, assign);
        this.pdfFile = pdfFile;
    }
    
    @Override
    protected String resultToString() {
        if (getPdfFile().canRead())
            return "<OK>";
        else
            return "<missing PDF>";
    }

    @Override
    protected void parseResult(Scanner scanner, NetPage targetNet) {
        if (getPdfFile().canRead())
            setComputedOk();
        else
            setComputeFailed("<missing PDF>");
    }

    @Override
    protected boolean isMultiLine() {
        return false;
    }

    @Override
    public boolean isComputedOk() {
        return super.isComputedOk() && getPdfFile()!=null && getPdfFile().canRead();
    }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        super.exchangeXML(el, exDir); 
        
        if (isComputedOk()) {
            if (exDir.FieldsToXml())
                el.setAttribute("filename", getPdfFile().getAbsolutePath());
            else {
                pdfFile = new File(el.getAttribute("filename"));
            }
        }
    }

    public File getPdfFile() {
        return pdfFile;
    }
}
