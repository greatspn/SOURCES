/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import editor.domain.elements.Transition;
import editor.domain.grammar.EvaluatedBinding;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import java.awt.Color;
import java.util.ArrayList;
import java.util.Map;
import java.util.Scanner;
import java.util.TreeMap;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

/** Basic Petri net measures.
 *
 * @author elvio
 */
public class AllMeasuresResult extends ResultEntry {
    
    // Average tokens in a place
    private final Map<String, ComputedScalar> meanP = new TreeMap<>();
    // Throughput of transitions
    private final Map<String, ComputedScalar> throughputs = new TreeMap<>();
    // Token distributions in each place
    private final Map<String, ComputedScalar[]> distrP = new TreeMap<>();

    public AllMeasuresResult() { }
    
    public AllMeasuresResult(String entryName, EvaluatedBinding assign) {
        super(entryName, assign);
    }

    @Override
    protected String resultToString() {
        return "<OK>";
    }
    
    // Convert an indexed place name (like: #15, zero based) into the i-th net place
    private String convertIndexedPlaceName(String plName, NetPage targetNet) {
        if (!plName.startsWith("#"))
            return plName; // not an index name
        
        int index = Integer.parseInt(plName.substring(1));
        GspnPage gspn = (GspnPage)targetNet;
        for (Node node : gspn.nodes) {
            if (node instanceof Place) {
                if (index == 0)
                    return ((Place)node).getUniqueName();
                else
                    index--;
            }
        }
        
        System.out.println("ERROR: No place with indexed name "+plName);
        return plName;
    }
    
    // Convert an indexed transition name (like: #15, zero based) into the i-th transition
    private String convertIndexedTransitionName(String trnName, NetPage targetNet) {
        if (!trnName.startsWith("#"))
            return trnName; // not an index name
        
        int index = Integer.parseInt(trnName.substring(1));
        GspnPage gspn = (GspnPage)targetNet;
        for (Node node : gspn.nodes) {
            if (node instanceof Transition) {
                if (index == 0)
                    return ((Transition)node).getUniqueName();
                else
                    index--;
            }
        }
        
        System.out.println("ERROR: No transition with indexed name "+trnName);
        return trnName;
    }

    @Override
    protected void parseResult(Scanner scanner, NetPage targetNet) {
        // Result is parsed in multiple lines
        String what = scanner.next();
        switch (what) {
            case "MEAN": {
                    String place = convertIndexedPlaceName(scanner.next(), targetNet);
                    ComputedScalar mean = ComputedScalar.makeFromScanner(scanner);
                    meanP.put(place, mean);
                    break;
                }
            case "DISTRIB": {
                    String place = convertIndexedPlaceName(scanner.next(), targetNet);
                    ComputedScalar[] dist = ComputedScalar.makeTableFromScanner(scanner);
                    distrP.put(place, dist);
                    break;
                }
            case "THROUGHPUT":
                String transition = convertIndexedTransitionName(scanner.next(), targetNet);
                ComputedScalar x = ComputedScalar.makeFromScanner(scanner);
                throughputs.put(transition, x);
                break;
                
            default:
                throw new UnsupportedOperationException();
        }
        
        setComputedOk();   
    }

    @Override
    protected boolean isMultiLine() {
        return true;
    }
    
    public ComputedScalar getMeanTokensInPlace(String plName) {
        return meanP.get(plName);
    }
    public ComputedScalar getTransitionThroughput(String trnName) {
        return throughputs.get(trnName);
    }
    public ComputedScalar[] getTokenDistribOfPlace(String plName) {
        return distrP.get(plName);
    }
    
    public ComputedScalar getNodeMeasure(Node node) {
        if (node instanceof Place)
            return getMeanTokensInPlace(node.getUniqueName());
        if (node instanceof Transition)
            return getTransitionThroughput(node.getUniqueName());
        return null;
    }

    private static final Color PLACE_MEASURE_TEXT_CLR = new Color(40, 50, 180);
    private static final Color TRANSITION_MEASURE_TEXT_CLR = new Color(190, 40, 20);

    public Color measureColorFor(Node node) {
        if (node instanceof Place)
            return PLACE_MEASURE_TEXT_CLR;
        if (node instanceof Transition)
            return TRANSITION_MEASURE_TEXT_CLR;
        return Color.BLACK;
    }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        super.exchangeXML(el, exDir); 
        
        if (isComputedOk()) {
            if (exDir.FieldsToXml()) {
                for (Map.Entry<String, ComputedScalar> eP : meanP.entrySet()) {
                    Element meanEl = exDir.getDocument().createElement("mean");
                    meanEl.setAttribute("place", eP.getKey());
                    eP.getValue().exchangeXML(meanEl, exDir);
                    el.appendChild(meanEl);
                }
                for (Map.Entry<String, ComputedScalar> eX : throughputs.entrySet()) {
                    Element throEl = exDir.getDocument().createElement("throughput");
                    throEl.setAttribute("transition", eX.getKey());
                    eX.getValue().exchangeXML(throEl, exDir);
                    el.appendChild(throEl);
                }
                for (Map.Entry<String, ComputedScalar[]> eD : distrP.entrySet()) {
                    Element distEl = exDir.getDocument().createElement("distrib");
                    distEl.setAttribute("place", eD.getKey());
                    distEl.setAttribute("values", ComputedScalar.printTableFromScalars(eD.getValue()));
                    el.appendChild(distEl);
                }
            }
            else { // load data from XML
                NodeList list = el.getChildNodes();
                for (int i=0; i<list.getLength(); i++) {
                    org.w3c.dom.Node resourceItem = list.item(i);
                    if (resourceItem.getNodeType() != org.w3c.dom.Node.ELEMENT_NODE)
                        continue;
                    Element elem = (Element)resourceItem;
                    switch (elem.getNodeName()) {
                        case "mean": {
                            ComputedScalar cs = new ComputedScalar();
                            cs.exchangeXML(elem, exDir);
                            meanP.put(elem.getAttribute("place"), cs);
                        }
                        break;

                        case "throughput": {
                            ComputedScalar cs = new ComputedScalar();
                            cs.exchangeXML(elem, exDir);
                            throughputs.put(elem.getAttribute("transition"), cs);
                        }
                        break;

                        case "distrib": {
                            Scanner sc = new Scanner(elem.getAttribute("values"));
                            ComputedScalar[] distr = ComputedScalar.makeTableFromScanner(sc);
                            distrP.put(elem.getAttribute("place"), distr);
                        }
                        break;
                    }
                }
            }
        }
    }
    
    
    public void getResultsAsWorkSheets(ArrayList<ExcelXml.WorkSheet> ws, int progressiveNum) {
        StringBuilder bindingText = new StringBuilder();
        for (Map.Entry<String, String> params : assign.binding.entrySet())
            bindingText.append(bindingText.length() == 0 ? " for: " : ", ").append(params.getKey())
                    .append(" = ").append(params.getValue());
        
        // Write place averages
        String[] colsP = { "Place", "Average token count" };
        Object[][] data = new Object[meanP.size()][2];
        int row = 0;
        for (Map.Entry<String, ComputedScalar> entry : meanP.entrySet()) {
            data[row][0] = entry.getKey();
            data[row][1] = entry.getValue().toString();
            row++;
        }
        
        ws.add(new ExcelXml.WorkSheet("Place averages" +(progressiveNum == 0 ? "" : " "+progressiveNum), 
                "Average token count in places" + bindingText.toString(), null, colsP, data));
        
        // Write transition throughputs
        String[] colsT = { "Transition", "Throughput" };
        data = new Object[throughputs.size()][2];
        row = 0;
        for (Map.Entry<String, ComputedScalar> entry : throughputs.entrySet()) {
            data[row][0] = entry.getKey();
            data[row][1] = entry.getValue().toString();
            row++;
        }
        
        if (row > 0) {
            ws.add(new ExcelXml.WorkSheet("Transition throughputs" +(progressiveNum == 0 ? "" : " "+progressiveNum), 
                    "Expected transition throughputs" + bindingText.toString(), null, colsT, data));
        }
        
        // Write place distributions
        int maxCount = 0;
        for (ComputedScalar[] distr : distrP.values())
            maxCount = Math.max(maxCount, distr.length);
        String[] colsD = new String[maxCount + 2];
        colsD[0] = "Place";
        colsD[1] = "Bound";
        for (int i=0; i<maxCount; i++)
            colsD[i+2] = "Prob[#] == "+i;
        data = new Object[distrP.size()][colsD.length];
        row = 0;
        for (Map.Entry<String, ComputedScalar[]> entry : distrP.entrySet()) {
            data[row][0] = entry.getKey();
            data[row][1] = entry.getValue().length;
            for (int i=0; i<entry.getValue().length; i++)
                data[row][i+2] = entry.getValue()[i].toString();
            row++;
        }
        if (row > 0) {
            ws.add(new ExcelXml.WorkSheet("Place distributions" +(progressiveNum == 0 ? "" : " "+progressiveNum), 
                    "Tokens distribution of each place" + bindingText.toString(), null, colsD, data));
        }
    }
}
