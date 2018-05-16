/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.io;

import editor.Main;
import editor.domain.elements.ClockVar;
import editor.domain.elements.DtaEdge;
import editor.domain.elements.DtaLocation;
import editor.domain.elements.DtaPage;
import editor.domain.Edge;
import editor.domain.elements.GspnEdge;
import editor.domain.elements.GspnPage;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.elements.Place;
import editor.domain.ProjectData;
import editor.domain.ProjectFile;
import editor.domain.ProjectImageResource;
import editor.domain.ProjectPage;
import editor.domain.ProjectResource;
import editor.domain.elements.ColorClass;
import editor.domain.elements.ColorVar;
import editor.domain.elements.Transition;
import editor.domain.elements.ConstantID;
import editor.domain.elements.TemplateVariable;
import editor.domain.elements.TextBox;
import editor.domain.grammar.VarMultiAssignment;
import editor.domain.measures.AbstractMeasure;
import editor.domain.measures.AllMeasuresResult;
import editor.domain.measures.CosmosSolverParams;
import editor.domain.measures.DocumentResource;
import editor.domain.measures.FormulaMeasure;
import editor.domain.measures.GreatSPNSolverParams;
import editor.domain.measures.MC4CSLTASolverParams;
import editor.domain.measures.MeasurePage;
import editor.domain.measures.ModelCheckingResultEntry;
import editor.domain.measures.NSolveParams;
import editor.domain.measures.PdfResultEntry;
import editor.domain.measures.RGMEDD2SolverParams;
import editor.domain.measures.ScalarResultEntry;
import editor.domain.measures.SolverParams;
import editor.domain.measures.StatResultEntry;
import editor.domain.measures.StochasticMcResult;
import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.UUID;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

/** Methods for reading and writing the PNPRO (Petri Net PROject) XML file format.
 *
 * @author elvio
 */
public class PnProFormat {
    
    public static final Map<Class, String> XML_CLASS_TO_NODE;    
    public static final Map<String, Class> XML_NODE_TO_CLASS;    
    private static final Set<String> XML_NET_PAGES;
    private static final Set<String> XML_NODES;
    private static final Set<String> XML_EDGES;
    private static final Set<String> XML_SOLVER_PARAMS;
    private static final Set<String> XML_MEASURES;
    private static final Set<String> XML_RESOURCES;
    public static final Set<String> XML_RESULT_ENTRIES;
    
    static {
        XML_CLASS_TO_NODE = new HashMap<>();
        XML_CLASS_TO_NODE.put(GspnPage.class, "gspn");
        XML_CLASS_TO_NODE.put(DtaPage.class, "dta");
        XML_CLASS_TO_NODE.put(Place.class, "place");
        XML_CLASS_TO_NODE.put(Transition.class, "transition");
        XML_CLASS_TO_NODE.put(ConstantID.class, "constant");
        XML_CLASS_TO_NODE.put(ClockVar.class, "clock");
        XML_CLASS_TO_NODE.put(TemplateVariable.class, "template");
        XML_CLASS_TO_NODE.put(ColorClass.class, "color-class");
        XML_CLASS_TO_NODE.put(ColorVar.class, "color-var");
        XML_CLASS_TO_NODE.put(TextBox.class, "text-box");
        XML_CLASS_TO_NODE.put(DtaLocation.class, "location");
        XML_CLASS_TO_NODE.put(GspnEdge.class, "arc");
        XML_CLASS_TO_NODE.put(DtaEdge.class, "edge");
        XML_CLASS_TO_NODE.put(MeasurePage.class, "measures");
        XML_CLASS_TO_NODE.put(VarMultiAssignment.class, "assignment");
        XML_CLASS_TO_NODE.put(MC4CSLTASolverParams.class, "mc4cslta");
//        XML_CLASS_TO_NODE.put(RGMEDDSolverParams.class, "rgmedd");
        XML_CLASS_TO_NODE.put(RGMEDD2SolverParams.class, "rgmedd2");
        XML_CLASS_TO_NODE.put(GreatSPNSolverParams.class, "greatspn");
        XML_CLASS_TO_NODE.put(CosmosSolverParams.class, "cosmos");
        XML_CLASS_TO_NODE.put(NSolveParams.class, "nsolve");
        XML_CLASS_TO_NODE.put(FormulaMeasure.class, "formula");
        XML_CLASS_TO_NODE.put(ProjectImageResource.class, "image");
        XML_CLASS_TO_NODE.put(DocumentResource.class, "document-log");
        XML_CLASS_TO_NODE.put(ScalarResultEntry.class, "scalar-result");
        XML_CLASS_TO_NODE.put(AllMeasuresResult.class, "all-measures");
        XML_CLASS_TO_NODE.put(ModelCheckingResultEntry.class, "mc-result");
        XML_CLASS_TO_NODE.put(PdfResultEntry.class, "pdf-result");
        XML_CLASS_TO_NODE.put(StatResultEntry.class, "stat-result");
        XML_CLASS_TO_NODE.put(StochasticMcResult.class, "stochastic-mc-result");
               
        XML_NODE_TO_CLASS = new HashMap<>();
        for (Entry<Class, String> e : XML_CLASS_TO_NODE.entrySet())
            XML_NODE_TO_CLASS.put(e.getValue(), e.getKey());
        
        XML_NET_PAGES = new HashSet<>();
        XML_NET_PAGES.add("gspn");
        XML_NET_PAGES.add("dta");

        XML_NODES = new HashSet<>();
        XML_NODES.add("place");
        XML_NODES.add("transition");
        XML_NODES.add("constant");
        XML_NODES.add("location");
        XML_NODES.add("template");
        XML_NODES.add("color-class");
        XML_NODES.add("color-var");
        XML_NODES.add("clock");
        XML_NODES.add("text-box");
        
        XML_EDGES = new HashSet<>();
        XML_EDGES.add("arc");
        XML_EDGES.add("edge");
        
        XML_SOLVER_PARAMS = new HashSet<>();
        XML_SOLVER_PARAMS.add("mc4cslta");
        XML_SOLVER_PARAMS.add("rgmedd");
        XML_SOLVER_PARAMS.add("rgmedd2");
        XML_SOLVER_PARAMS.add("greatspn");
        XML_SOLVER_PARAMS.add("nsolve");
        XML_SOLVER_PARAMS.add("cosmos");
        
        XML_MEASURES = new HashSet<>();
        XML_MEASURES.add("formula");

        XML_RESOURCES = new HashSet<>();
        XML_RESOURCES.add("image");
        XML_RESOURCES.add("document-log");
        
        XML_RESULT_ENTRIES = new HashSet<>();
        XML_RESULT_ENTRIES.add("scalar-result");
        XML_RESULT_ENTRIES.add("all-measures");
        XML_RESULT_ENTRIES.add("mc-result");
        XML_RESULT_ENTRIES.add("pdf-result");
        XML_RESULT_ENTRIES.add("stat-result");
        XML_RESULT_ENTRIES.add("stochastic-mc-result");
    }
    
    
    // Original version number
    private static final int FIRST_VERSION_NUMBER = 100;
    // After adding color support
    private static final int COLOR_SUPPORT_VERSION_NUMBER = 110;
    // After XML cleanup
    private static final int XML_CLEANUP_VERSION_NUMBER = 120;

    private static final int CURRENT_FILE_VERSION_NUMBER = 121;
    
        
    public static void saveXML(ProjectData pdata, File file) throws Exception {
        //assert file != null;
        
        DocumentBuilderFactory docFactory = DocumentBuilderFactory.newInstance();
        DocumentBuilder docBuilder = docFactory.newDocumentBuilder();
        Document doc = docBuilder.newDocument();
        final XmlExchangeDirection exDir = new XmlExchangeDirection(false, doc);
        exDir.arcKindTag = "kind";
        exDir.afterXmlCleanup = CURRENT_FILE_VERSION_NUMBER >= XML_CLEANUP_VERSION_NUMBER;
        
        // root elements
        Element projectElem = doc.createElement("project");
        doc.appendChild(doc.createComment(" This project file has been saved by the "+Main.APP_NAME+", v."+
                                          Main.VERSION_NUMBER+" "));
        doc.appendChild(projectElem);
        
        projectElem.setAttribute("name", pdata.projName);
        projectElem.setAttribute("version", ""+CURRENT_FILE_VERSION_NUMBER);
        for (int i=0; i<pdata.getPageCount(); i++) {
            ProjectPage page = pdata.getPageAt(i);
            if (page instanceof NetPage) {
                NetPage netPage = (NetPage)page;
                Element pageElem = doc.createElement(XML_CLASS_TO_NODE.get(netPage.getClass()));
                projectElem.appendChild(pageElem);
                pageElem.setAttribute("name", netPage.getPageName());
                
                // Save page data
                netPage.viewProfile.exchangeXML(pageElem, exDir);
                
                // Save nodes
                Element nodeListElem = doc.createElement("nodes");
                pageElem.appendChild(nodeListElem);

                for (Node node : netPage.nodes) {
                    Element nodeElem = doc.createElement(XML_CLASS_TO_NODE.get(node.getClass()));                    
                    nodeListElem.appendChild(nodeElem);
                    node.exchangeXML(nodeElem, exDir);
                }
                
                // Save edges
                Element edgeListElem = doc.createElement("edges");
                pageElem.appendChild(edgeListElem);

                for (Edge edge : netPage.edges) {
                    Element edgeElem = doc.createElement(XML_CLASS_TO_NODE.get(edge.getClass()));                    
                    edgeListElem.appendChild(edgeElem);
                    edge.exchangeXML(edgeElem, exDir);
                }
            }
            else if (page instanceof MeasurePage) {
                MeasurePage measPage = (MeasurePage)page;
                Element pageElem = doc.createElement(XML_CLASS_TO_NODE.get(measPage.getClass()));
                projectElem.appendChild(pageElem);
                pageElem.setAttribute("name", measPage.getPageName());
                pageElem.setAttribute("gspn-name", measPage.targetGspnName);
                if (!measPage.rapidMeasureType.isEmpty())
                    pageElem.setAttribute("rapid-type", measPage.rapidMeasureType);
                pageElem.setAttribute("simplified-UI", measPage.simplifiedUI ? "true" : "false");
                if (!measPage.lastLog.isNull())
                    pageElem.setAttribute("log-uuid", measPage.lastLog.resRef().getResourceID().toString());
                
                // Save parameter assignments
                Element assignListElem = doc.createElement("assignments");
                pageElem.appendChild(assignListElem);
                
                for (VarMultiAssignment assign : measPage.varListAssignments.assigns) {
                    Element assignElem = doc.createElement(XML_CLASS_TO_NODE.get(assign.getClass()));                    
                    assignListElem.appendChild(assignElem);
                    assign.exchangeXML(assignElem, exDir);
                }
                
                // Save tool parameters
                Element solverElem = doc.createElement(XML_CLASS_TO_NODE.get(measPage.solverParams.getClass()));
                pageElem.appendChild(solverElem);
                measPage.solverParams.exchangeXML(solverElem, exDir);
                
                // Save measures
                Element measListElem = doc.createElement("formulas");
                pageElem.appendChild(measListElem);
                
                for (AbstractMeasure meas : measPage.measures) {
                    Element measElem = doc.createElement(XML_CLASS_TO_NODE.get(meas.getClass()));                    
                    measListElem.appendChild(measElem);
                    meas.exchangeXML(measElem, exDir);
                }
            }
            else throw new UnsupportedOperationException("Unknown page type.");
        }
        
        // write the project resources
        Iterator<Entry<UUID, ProjectResource>> resIt = pdata.getResourceIterator();
        if (resIt.hasNext()) {
            Element resListElem = doc.createElement("resource-list");
            projectElem.appendChild(resListElem);

            // Save each resource
            while (resIt.hasNext()) {
                Entry<UUID, ProjectResource> entry = resIt.next();
                Element resElem = doc.createElement(XML_CLASS_TO_NODE.get(entry.getValue().getClass()));
                entry.getValue().exchangeXML(resElem, exDir);
                resListElem.appendChild(resElem);
            }
        }
        
        // write the content into xml file
        TransformerFactory transformerFactory = TransformerFactory.newInstance();
        try {
            transformerFactory.setAttribute("indent-number", 2);
        }
        catch (Exception e) { Main.logException(e, false); }
        Transformer transformer = transformerFactory.newTransformer();
        DOMSource source = new DOMSource(doc);
        StreamResult result;
        if (file != null)
            result = new StreamResult(file);
        else 
            result = new StreamResult(System.out);
//        StreamResult result = new StreamResult(System.out); // Output to console for testing
        transformer.setOutputProperty(OutputKeys.INDENT, "yes");
        transformer.setOutputProperty("{http://xml.apache.org/xslt}indent-amount", "2");
        transformer.transform(source, result);
    }
    
    
    
    public static ProjectFile readXML(File file) throws Exception {
        
        DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
	DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
	Document doc = dBuilder.parse(file);
        final XmlExchangeDirection exDir = new XmlExchangeDirection(true, doc);
        
	//optional, but recommended
	//read this - http://stackoverflow.com/questions/13786607/normalization-in-dom-parsing-with-java-how-does-it-work
	doc.getDocumentElement().normalize();

        assert doc.getDocumentElement().getNodeName().equals("project");
        Element project = doc.getDocumentElement();
        int versionNum = Integer.parseInt(project.getAttribute("version"));
        if (versionNum > CURRENT_FILE_VERSION_NUMBER)
            throw new UnsupportedOperationException("Version number is not supported.");
        boolean hasColorSupport = (versionNum >= COLOR_SUPPORT_VERSION_NUMBER);
        exDir.arcKindTag = (hasColorSupport ? "kind" : "type");
        exDir.idToNode = new HashMap<>();
        exDir.afterXmlCleanup = versionNum >= XML_CLEANUP_VERSION_NUMBER;
        
        Map<UUID, ProjectResource> resourceTable = new HashMap<>();
        
        // read pages
        ArrayList<ProjectPage> pages = new ArrayList<>();
        NodeList pageList = project.getChildNodes();
        for (int p=0; p<pageList.getLength(); p++) {
            org.w3c.dom.Node pageNodeItem = pageList.item(p);
            if (pageNodeItem.getNodeType() != org.w3c.dom.Node.ELEMENT_NODE)
                continue;
            Element pageElem = (Element)pageNodeItem;

            // Read net pages
            if (XML_NET_PAGES.contains(pageElem.getNodeName())) {
                NetPage netPage = (NetPage)
                        XML_NODE_TO_CLASS.get(pageElem.getNodeName()).newInstance();
                
                // Read page data
                netPage.setPageName(pageElem.getAttribute("name"));
                netPage.viewProfile.exchangeXML(pageElem, exDir);

                // Read nodes
                NodeList nodeList = pageElem.getElementsByTagName("nodes").item(0).getChildNodes();
                for (int n=0; n<nodeList.getLength(); n++) {
                    org.w3c.dom.Node nodeItem = nodeList.item(n);
                    if (nodeItem.getNodeType() != org.w3c.dom.Node.ELEMENT_NODE)
                        continue;
                    Element nodeElem = (Element)nodeItem;
                    if (!XML_NODES.contains(nodeElem.getNodeName())) 
                        continue;
                    
                    Node node = (Node)XML_NODE_TO_CLASS.get(nodeElem.getNodeName()).newInstance();
                    node.exchangeXML(nodeElem, exDir);

                    netPage.nodes.add(node);
                    exDir.idToNode.put(node.getUniqueName(), node);
                }
                
                // read edges
                NodeList edgeList = pageElem.getElementsByTagName("edges").item(0).getChildNodes();
                for (int e=0; e<edgeList.getLength(); e++) {
                    org.w3c.dom.Node edgeItem = edgeList.item(e);
                    if (edgeItem.getNodeType() != org.w3c.dom.Node.ELEMENT_NODE)
                        continue;
                    Element edgeElem = (Element)edgeItem;
                    if (!XML_EDGES.contains(edgeElem.getNodeName())) 
                        continue;
                    
                    Edge edge = (Edge)XML_NODE_TO_CLASS.get(edgeElem.getNodeName()).newInstance();
                    edge.exchangeXML(edgeElem, exDir);
                    netPage.edges.add(edge);
                }

                pages.add(netPage);
            }
            else if (pageElem.getNodeName().equals("measures")) {
                MeasurePage measPage = new MeasurePage();
                measPage.setPageName(pageElem.getAttribute("name"));
                measPage.targetGspnName = safeParseString(pageElem.getAttribute("gspn-name"), "");
                measPage.rapidMeasureType = safeParseString(pageElem.getAttribute("rapid-type"), "");
                measPage.simplifiedUI = safeParseString(pageElem.getAttribute("simplified-UI"), "false").equals("true");
                measPage.lastLog.setID(safeParseUUID(pageElem.getAttribute("log-uuid")));
                
                // read gspn variable assignments
                NodeList assignList = pageElem.getElementsByTagName("assignments").item(0).getChildNodes();
                for (int i=0; i<assignList.getLength(); i++) {
                    org.w3c.dom.Node assignItem = assignList.item(i);
                    if (assignItem.getNodeType() != org.w3c.dom.Node.ELEMENT_NODE)
                        continue;
                    Element assignElem = (Element)assignItem;
                    
                    VarMultiAssignment assign = new VarMultiAssignment();
                    assign.exchangeXML(assignElem, exDir);
                    measPage.varListAssignments.assigns.add(assign);
                }
                
                // Read solver parameters
                NodeList solverList = pageElem.getChildNodes();
                for (int i=0; i<solverList.getLength(); i++) {
                    org.w3c.dom.Node solverItem = solverList.item(i);
                    if (solverItem.getNodeType() != org.w3c.dom.Node.ELEMENT_NODE)
                        continue;
                    Element solverElem = (Element)solverItem;
                    if (!XML_SOLVER_PARAMS.contains(solverElem.getNodeName()))
                        continue; // search next XML element in for loop
                    measPage.solverParams = (SolverParams)XML_NODE_TO_CLASS.get(solverElem.getNodeName()).newInstance();
                    measPage.solverParams.exchangeXML(solverElem, exDir);
                    break;
                }
                if (measPage.solverParams == null)
                    throw new UnsupportedOperationException("Missing solver.");
                
                // Read measure formulas
                NodeList formulaList = pageElem.getElementsByTagName("formulas").item(0).getChildNodes();
                for (int i=0; i<formulaList.getLength(); i++) {
                    org.w3c.dom.Node formulaItem = formulaList.item(i);
                    if (formulaItem.getNodeType() != org.w3c.dom.Node.ELEMENT_NODE)
                        continue;
                    Element formulaElem = (Element)formulaItem;
                    if (!XML_MEASURES.contains(formulaElem.getNodeName()))
                        continue;
                    AbstractMeasure meas = (AbstractMeasure)
                            XML_NODE_TO_CLASS.get(formulaElem.getNodeName()).newInstance();
                    meas.exchangeXML(formulaElem, exDir);
                    measPage.measures.add(meas);
                }
                
                pages.add(measPage);
            }
            else if (pageElem.getTagName().equals("resource-list")) {
                NodeList resourceList = pageElem.getChildNodes();
                for (int i=0; i<resourceList.getLength(); i++) {
                    org.w3c.dom.Node resourceItem = resourceList.item(i);
                    if (resourceItem.getNodeType() != org.w3c.dom.Node.ELEMENT_NODE)
                        continue;
                    Element resourceElem = (Element)resourceItem;
                    if (!XML_RESOURCES.contains(resourceElem.getNodeName())) 
                        continue;
                    
                    ProjectResource res = (ProjectResource)
                            XML_NODE_TO_CLASS.get(resourceElem.getNodeName()).newInstance();
                    res.exchangeXML(resourceElem, exDir);
                    resourceTable.put(res.getResourceID(), res);
                }
            }
            else throw new UnsupportedOperationException();
        }
        
        ProjectData projectData = new ProjectData(project.getAttribute("name"), pages);
        for (Entry<UUID, ProjectResource> entry : resourceTable.entrySet())
            projectData.addResource(entry.getValue());
        projectData.relinkPageResources();
        
        ProjectFile newProject = new ProjectFile(projectData);
        newProject.setFilename(file);
        //newProject.setSaved();
 
        return newProject;
    }
    
    
//    private static double safeParseDouble(String str, double defaultVal) {
//        if (str == null || str.length() == 0)
//            return defaultVal;
//        return Double.parseDouble(str);
//    }
//    
//    private static int safeParseInt(String str, int defaultVal) {
//        if (str == null || str.length() == 0)
//            return defaultVal;
//        return Integer.parseInt(str);
//    }
//    
//    private static boolean safeParseBoolean(String str, boolean defaultVal) {
//        if (str == null || str.length() == 0)
//            return defaultVal;
//        return Boolean.parseBoolean(str);
//    }
    
    private static UUID safeParseUUID(String str) {
        if (str == null || str.length() == 0)
            return null;
        return UUID.fromString(str);
    }
    
    private static String safeParseString(String str, String defaultVal) {
        if (str == null || str.length() == 0)
            return defaultVal;
        return str;
    }
    
//    private static String nonEmptyStringOrNull(String str) {
//        if (str.length() == 0)
//            return null;
//        return str;
//    }
    
    
//    public static void main(String[] args) throws Exception {
//        ArrayList<ProjectPage> schemas = new ArrayList<ProjectPage>();
//        schemas.add(new GspnPage("GSPN 1"));
//        schemas.add(new DtaPage("DTA 1"));
//        schemas.add(new DtaPage("DTA 2"));
//        ProjectData data = new ProjectData("New project", schemas);
//        ProjectFile proj = new ProjectFile(data);
//        
//        proj.setFilename(new File("/Users/elvio/x.PNPRO"));
//        proj.save();
////        
//        ProjectFile nproj = readXML(proj.getFilename());
//        saveXML(nproj.getCurrent(), new File("/Users/elvio/x1.PNPRO"));
//    }
}
