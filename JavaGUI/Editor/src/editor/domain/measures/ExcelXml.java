/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import common.Util;
import editor.domain.NetObject;
import java.awt.Component;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.util.ArrayList;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import javax.swing.filechooser.FileFilter;
import javax.swing.filechooser.FileNameExtensionFilter;

/** Custom class to export a set of data tables in Excel XML format
 *
 * @author elvio
 */
public class ExcelXml {
    
    public static class WorkSheet {
        // name of the worksheet (small description, no special characters)
        public String sheetName;
        // First line of the worksheet, which can be a long textual description
        public String longDescr;
        // Comment added by the user
        public String optionalComment;
        
        public String[] headers;
        public Object[][] data;

        public WorkSheet(String sheetName, String longDescr, String optionalComment, 
                         String[] headers, Object[][] data) 
        {
            this.sheetName = sheetName;
            this.longDescr = longDescr;
            this.optionalComment = optionalComment;
            this.headers = headers;
            this.data = data;
        }
    }
    
    private static final FileFilter excelFileFilter 
            = new FileNameExtensionFilter("Excel XML Files (*.xls)", new String[]{ "xls" });
    
    public static boolean exportWorkSheets(ArrayList<WorkSheet> wss, String proposedFilename, Component parent) {
        assert wss.size() > 0;
        File file = null;
        if (file == null || !file.canWrite()) {
            boolean repeatChooser;
            do {
                repeatChooser = false;
                final JFileChooser fileChooser = new JFileChooser();
                fileChooser.setDialogTitle("Export Excel data...");
                String curDir = Util.getPreferences().get("export-excel-dir", null);
                fileChooser.setCurrentDirectory(curDir!=null ? new File(curDir) : null);
                if (file == null) // propose the initial file name in the current directory
                    file = new File(proposedFilename + ".xls");
                fileChooser.setSelectedFile(file);
                fileChooser.addChoosableFileFilter(excelFileFilter);
                fileChooser.setFileFilter(excelFileFilter);
                if (fileChooser.showSaveDialog(parent) != JFileChooser.APPROVE_OPTION)
                    return false;
                file = fileChooser.getSelectedFile();
                if (fileChooser.getFileFilter() instanceof FileNameExtensionFilter) {
                    FileNameExtensionFilter fnef = (FileNameExtensionFilter)fileChooser.getFileFilter();
                    String path = file.getPath();
                    int lastDot = path.lastIndexOf(".");
                    if (lastDot != -1)
                        path = path.substring(0, lastDot);
                    file = new File(path + "." + fnef.getExtensions()[0]);
                }
                curDir = fileChooser.getCurrentDirectory().getAbsolutePath();
                Util.getPreferences().put("export-excel-dir", curDir);
                if (file.exists()) {
                    int r = JOptionPane.showConfirmDialog(parent, 
                             "The file \""+file+"\" already exists! Overwrite it?", 
                            "Overwrite file", 
                            JOptionPane.YES_NO_CANCEL_OPTION, 
                            JOptionPane.WARNING_MESSAGE);
                    if (r == JOptionPane.NO_OPTION)
                        repeatChooser = true;
                    else if (r == JOptionPane.CANCEL_OPTION)
                        return false;
                }
            } while (repeatChooser);
        }
        // save to file
        try {
            writeWorkSheets(wss, new PrintWriter(file));
            return true;
        }
        catch (FileNotFoundException e) {
            e.printStackTrace();
            JOptionPane.showMessageDialog(parent, 
                                          "An error happened while exporting Excel data.\n"
                                          + "Reason: "+e.getMessage(),
                                          "Export Excel data...", 
                                          JOptionPane.ERROR_MESSAGE);
            return false;
        }        
    }
    
    // Write the worksheets in Excel 14.0 XML format
    public static void writeWorkSheets(ArrayList<WorkSheet> wss, PrintWriter out) {
        // Write XML header
        out.println("<?xml version=\"1.0\"?>\n" +
                    "<Workbook xmlns=\"urn:schemas-microsoft-com:office:spreadsheet\"\n" +
                    " xmlns:o=\"urn:schemas-microsoft-com:office:office\"\n" +
                    " xmlns:x=\"urn:schemas-microsoft-com:office:excel\"\n" +
                    " xmlns:ss=\"urn:schemas-microsoft-com:office:spreadsheet\"\n" +
                    " xmlns:html=\"http://www.w3.org/TR/REC-html40\">\n" +
                    " <DocumentProperties xmlns=\"urn:schemas-microsoft-com:office:office\">\n" +
                    "  <Version>14.0</Version>\n" +
                    " </DocumentProperties>\n" +
                    " <OfficeDocumentSettings xmlns=\"urn:schemas-microsoft-com:office:office\">\n" +
                    "  <AllowPNG/>\n" +
                    " </OfficeDocumentSettings>\n" +
                    " <ExcelWorkbook xmlns=\"urn:schemas-microsoft-com:office:excel\">\n" +
                    "  <WindowHeight>12400</WindowHeight>\n" +
                    "  <WindowWidth>18480</WindowWidth>\n" +
                    "  <WindowTopX>120</WindowTopX>\n" +
                    "  <WindowTopY>40</WindowTopY>\n" +
                    "  <ProtectStructure>False</ProtectStructure>\n" +
                    "  <ProtectWindows>False</ProtectWindows>\n" +
                    " </ExcelWorkbook>\n" +
                    " <Styles>\n" +
                    // Default style
                    "  <Style ss:ID=\"Default\" ss:Name=\"Normal\">\n" +
                    "   <Alignment ss:Vertical=\"Bottom\"/>\n" +
                    "   <Borders/>\n" +
                    "   <Font ss:FontName=\"Calibri\" x:Family=\"Swiss\" ss:Size=\"12\" ss:Color=\"#000000\"/>\n" +
                    "   <Interior/>\n" +
                    "   <NumberFormat/>\n" +
                    "   <Protection/>\n" +
                    "  </Style>\n" +
                    //Style of column headers
                    "  <Style ss:ID=\"1\">\n" +
                    "   <Alignment ss:Vertical=\"Bottom\"/>\n" +
                    "   <Borders/>\n" +
                    "   <Font ss:Bold=\"1\" ss:Size=\"12\"/>\n" +
                    "   <Interior/>\n" +
                    "   <NumberFormat/>\n" +
                    "   <Protection/>\n" +
                    "  </Style>\n" +
                    //Style of long description
                    "  <Style ss:ID=\"2\">\n" +
                    "   <Alignment ss:Vertical=\"Bottom\"/>\n" +
                    "   <Borders/>\n" +
                    "   <Font ss:Bold=\"1\" ss:Size=\"13\"/>\n" +
                    "   <Interior/>\n" +
                    "   <NumberFormat/>\n" +
                    "   <Protection/>\n" +
                    "  </Style>\n" +
                    " </Styles>");
        
        // Write worksheets
        for (WorkSheet ws : wss) {
            out.println(" <Worksheet ss:Name=\""+escape(ws.sheetName)+"\">\n" +
                        "  <Table>");
            
            // Write column sizes
            for (int col=0; col<ws.headers.length; col++) {
                int size = Math.max(50, 5 * ws.headers[col].length());
                for (Object[] dataRow : ws.data) {
                    if (dataRow[col] != null)
                        size = Math.max(size, 5 * dataRow[col].toString().length());
                }
                out.println("   <Column ss:Width=\""+size+"\"/>");
            }
            // Write comment
            if (ws.optionalComment != null && ws.optionalComment.length() > 0) {
                out.println("   <Row ss:StyleID=\"Normal\">");
                out.println("    <Cell><Data ss:Type=\"String\">" + escape(ws.optionalComment) + "</Data></Cell>");
                out.println("   </Row>");
            }
            // Write long description
            if (ws.longDescr != null && ws.longDescr.length() > 0) {
                out.println("   <Row ss:StyleID=\"2\">");
                out.println("    <Cell><Data ss:Type=\"String\">" + escape(ws.longDescr) + "</Data></Cell>");
                out.println("   </Row>");
            }
            
            // Write column headers in bold text
            out.println("   <Row ss:StyleID=\"1\">");
            for (String header : ws.headers) {
                out.println("    <Cell><Data ss:Type=\"String\">" + 
                            escape(header) + "</Data></Cell>");
            }
            out.println("   </Row>");
            
            // Write table data
            for (Object[] dataRow : ws.data) {
                out.println("   <Row>");
                for (Object cell : dataRow) {
                    out.print("    <Cell>");
                    if (cell != null) {
                        if (NetObject.isDouble(cell.toString())) {
                            out.print("<Data ss:Type=\"Number\">" +
                                        escape(cell.toString()) + "</Data>");
                        }
                        else {
                            out.print("<Data ss:Type=\"String\">" + 
                                        escape(cell.toString()) + "</Data>");
                        }
                    }
                    out.println("</Cell>");
                }
                out.println("   </Row>");
            }
            
            out.println("  </Table>\n" +
                        "  <WorksheetOptions xmlns=\"urn:schemas-microsoft-com:office:excel\">\n" +
                        "   <PageLayoutZoom>0</PageLayoutZoom>\n" +
                        "   <Selected/>\n" +
                        "   <ProtectObjects>False</ProtectObjects>\n" +
                        "   <ProtectScenarios>False</ProtectScenarios>\n" +
                        "  </WorksheetOptions>\n" +
                        " </Worksheet>");
        }
        
        // Write XML footer
        out.println("</Workbook>");
        out.flush();
    }
    
    // Escape XML String
    private static String escape(String t) {
        StringBuilder sb = new StringBuilder();
        for(int i = 0; i < t.length(); i++) {
           char c = t.charAt(i);
           switch(c) {
           case '<': sb.append("&lt;"); break;
           case '>': sb.append("&gt;"); break;
           case '\"': sb.append("&quot;"); break;
           case '&': sb.append("&amp;"); break;
           case '\'': sb.append("&apos;"); break;
           default:
              if(c>0x7e) {
                 sb.append("&#").append((int)c).append(";");
              }
              else {
                 sb.append(c);
              }
           }
        }
        return sb.toString();
    }
}
