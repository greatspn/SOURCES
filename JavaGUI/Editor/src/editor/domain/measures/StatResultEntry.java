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
import java.util.Map;
import java.util.Scanner;
import java.util.TreeMap;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

/**
 *
 * @author elvio
 */
public class StatResultEntry extends ResultEntry {

    public static String[][] getKnownStatKeys() {
        return knownStatKeys;
    }
        
    public Map<String, String> statValues = new TreeMap<>();
    
    private static final String knownStatKeys[][] = {
        { "num_tangible_markings",           "Number of Tangible Markings:" },
        { "num_vanishing_markings",          "Number of Vanishing Markings:" },
        { "num_dead_markings",               "Number of Dead Markings:" },
        { "visited_vanishing_markings",      "Visited Vanishing Markings:" },
        { "num_ordinary_tangible_markings",  "Number of ordinary Tangible Markings:" },
        { "num_ordinary_vanishing_markings", "Number of ordinary Vanishing Markings:" },
        { "num_ordinary_dead_markings",      "Number of ordinary Dead Markings:" },
        { "home_state",                      "Initial Marking is a home state?" },
        { "multiple_initial_tangible_markings", "Number of Initial Tangible Markings:" },
        { "num_macrostates",                 "Number of macro-states:" },
        { "num_total_states",                "Total number of states:" },
        // Transition firings:
        { "immediate_firings",               "# of Immediate transitions fired:" },
        { "timed_firings",                   "# of Timed transitions fired:" },
        { "total_firings",                   "# of transitions fired:" },
        // Nodes in a BDD/MDD data structure
        { "num_rs_nodes",                    "DD nodes for RS:" },
        // RG Class
        { "rg_class",                        "Process Class:" },
        // Timings & Resuls
        { "build_time",                      "Total Build Time:" },
        { "steady_state_solution_time",      "Steady-state solution time:" },
        { "transient_solution_time",         "Transient solution time:" },
        { "estimated_accuracy",              "Estimated Accuracy:" },
        { "no_of_iterations",                "Num. of iterations:" },
        { "backward_steady_state_solution_time", "Backward steady-state solution time:" },
        { "CTL_time",                        "CTL solution time:" },
        // Simulation
        { "firings_per_second",              "# of firings per second:" },
        { "total_events",                    "Total events fired:" },
        { "simulation_time",                 "Simulation time (sec):" },
        { "total_simulated_time",            "Total simulated model time:" },
        { "num_batches",                     "Number of simulation batches:" },
    };

    public StatResultEntry() {
    }

    public StatResultEntry(String entryName, EvaluatedBinding assign) {
        super(entryName, assign);
    }

    
    
    @Override
    protected String resultToString() {
        if (isComputedOk())
            return "<OK>";
        else
            return "<missing>";
    }
    
    public String getStatValue(String key,  String defaultValue) {
        assert isComputedOk();
        if (statValues.containsKey(key))
            return statValues.get(key);
        return defaultValue;
    }

    @Override
    protected void parseResult(Scanner scanner, NetPage targetNet) {
        String name, value;
        if (!scanner.hasNext()) 
            return;
        name = scanner.next();
        
        if (!scanner.hasNext())
            return;
        value = scanner.nextLine().trim();
        
//        System.out.println("STAT '"+name+"' = '"+value+"'    "+name.length());
        
        statValues.put(name, value);
        setComputedOk();
    }

    @Override
    protected boolean isMultiLine() {
        return true;
    }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        super.exchangeXML(el, exDir);
                
        if (isComputedOk()) {
            if (exDir.FieldsToXml()) {
                for (Map.Entry<String, String> e : statValues.entrySet()) {
                    //el.setAttribute(e.getKey(), e.getValue());
                    Element statEl = exDir.getDocument().createElement("stat");
                    statEl.setAttribute("key", e.getKey());
                    statEl.setAttribute("value", e.getValue());
                    el.appendChild(statEl);
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
                        case "stat": {
                            String key = elem.getAttribute("key");
                            String val = elem.getAttribute("value");
                            if (key != null && val != null)
                                statValues.put(key, val);
                        }
                        break;
                    }
                }
//                NamedNodeMap attrList = el.getAttributes();
//                for (int i=0; i<attrList.getLength(); i++) {
//                    org.w3c.dom.Node attr = attrList.item(i);
//                    System.out.println("STAT: "+attr.getNodeName()+" = "+attr.getNodeValue());
//                    statValues.put(attr.getNodeName(), attr.getNodeValue());
//                }
            }
        }
    }
}
