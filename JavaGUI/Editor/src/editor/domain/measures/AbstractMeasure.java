/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import editor.domain.EditableCell;
import editor.domain.EditableValue;
import editor.domain.MovementHandle;
import editor.domain.NetPage;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.SelectableObject;
import editor.domain.ViewProfile;
import editor.domain.grammar.ParserContext;
import editor.domain.grammar.VarListMultiAssignment;
import editor.domain.io.PnProFormat;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;
import editor.domain.io.XmlExchangeable;
import java.awt.geom.Rectangle2D;
import java.io.Serializable;
import java.util.ArrayList;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

/** A measure described by a formula in a given measure language.
 *
 * @author elvio
 */
public abstract class AbstractMeasure extends SelectableObject 
    implements Serializable, XmlExchangeable 
{
    // A comment describing the measure
    private String comment = "";
    // Is the comment shown in the main panel?
    private boolean commentShown = false;
    // The computed results
    private ResultTable results = null;

    public AbstractMeasure(String comment, boolean commentShown) {
        this.comment = comment;
        this.commentShown = commentShown;
    }
    
    public AbstractMeasure() { }
    
    // Check the correctness of the measure' formula
    public abstract void checkMeasureCorrectness(MeasurePage page, ParserContext context); 

    public String getComment() {
        return comment;
    }

    public void setComment(String comment) {
        this.comment = comment;
    }

    public boolean isCommentShown() {
        return commentShown;
    }

    public void setCommentShown(boolean commentShown) {
        this.commentShown = commentShown;
    }

    public ResultTable getResults() {
        return results;
    }
    public boolean hasResults() {
        return results != null;
    }
    public boolean hasResultsMarkedOk() {
        if (!hasResults())
            return false;
        for (ResultEntry re : results.table)
            if (re.isComputedOk())
                return true;
        return false;
    }

    public void setResults(ResultTable results) {
        this.results = results;
    }
    
    public EditableValue getCommentEditable() {
        return new EditableValue() {
            @Override public boolean isEditable() { 
                return true;
            }
            @Override public Object getValue() {
                return comment;
            }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
                return true;
            }
            @Override public boolean isCurrentValueValid() {
                return true;
            }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
                comment = (String)value;
            }
        };
    }    

    @Override public MovementHandle getCenterHandle(NetPage thisPage) 
    {   throw new UnsupportedOperationException("Not supported."); }

    @Override public MovementHandle[] getSubObjectHandles(NetPage thisPage, ViewProfile viewProfile) 
    {   throw new UnsupportedOperationException("Not supported."); }

    @Override public boolean intersectRectangle(Rectangle2D rect, ViewProfile viewProfile, boolean includeDecors) 
    {   throw new UnsupportedOperationException("Not supported."); }

    @Override public EditableCell getCentralEditable(double px, double py) 
    {   throw new UnsupportedOperationException("Not supported."); }
    
    @Override public boolean isLocked() { return false; }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        bindXMLAttrib(this, el, exDir, "comment", "comment", "");
        bindXMLAttrib(this, el, exDir, "comment-shown", "commentShown", false);
        
        if (exDir.FieldsToXml()) {
            if (results != null) {
                Element resultTableEl = exDir.getDocument().createElement("result-table");
                for (ResultEntry re : results.table) {
                    Element resultEl = exDir.getDocument().createElement
                        (PnProFormat.XML_CLASS_TO_NODE.get(re.getClass()));
                    re.exchangeXML(resultEl, exDir);
                    resultTableEl.appendChild(resultEl);
                }
                el.appendChild(resultTableEl);
            }
        }
        else {
            NodeList resultTableEls = el.getElementsByTagName("result-table");
            if (resultTableEls.getLength() == 1) {
                Element resultTableEl = (Element)resultTableEls.item(0);
                NodeList list = resultTableEl.getChildNodes();
                for (int i=0; i<list.getLength(); i++) {
                    org.w3c.dom.Node item = list.item(i);
                    if (item.getNodeType() != org.w3c.dom.Node.ELEMENT_NODE)
                        continue;
                    Element elem = (Element)item;
                    if (PnProFormat.XML_RESULT_ENTRIES.contains(elem.getNodeName())) {
                        // Load this result entry
                        try {
                            ResultEntry re = (ResultEntry)
                                    PnProFormat.XML_NODE_TO_CLASS.get(elem.getNodeName()).newInstance();
                            re.exchangeXML(elem, exDir);
                            if (results == null)
                                results = new ResultTable();
                            results.table.add(re);
                        }
                        catch (InstantiationException | IllegalAccessException e) {
                            throw new XmlExchangeException(e);
                        }
                    }
                }
            }
        }
    }
    
    // Compile the results of this abstract measure in a format that can be exported  
    // as an Excel workbook or a CSV file.
    public abstract void getResultsAsWorkSheets(VarListMultiAssignment vma, ArrayList<ExcelXml.WorkSheet> ws);
    
    public abstract boolean canExportResultsAsWorkSheets();
    
    public abstract String getSuggestedWorkSheetTitle();
}
