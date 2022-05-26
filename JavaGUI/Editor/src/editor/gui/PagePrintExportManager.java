/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui;

import common.ModalLogDialog;
import common.Util;
import de.rototor.pdfbox.graphics2d.PdfBoxGraphics2D;
import editor.Main;
import editor.domain.NetPage;
import editor.domain.elements.DtaPage;
import editor.domain.elements.GspnPage;
import editor.domain.grammar.ParserContext;
import editor.domain.io.ApnnFormat;
import editor.domain.io.DtaFormat;
import editor.domain.io.GRMLFormat;
import editor.domain.io.GreatSpnFormat;
import editor.domain.io.NetLogoFormat;
import editor.domain.io.PNMLFormat;
import editor.domain.struct.StructInfo;
import editor.gui.net.ShowNetMatricesDialog;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.awt.print.PageFormat;
import java.awt.print.Paper;
import java.io.File;
import java.io.IOException;
import javax.imageio.ImageIO;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import javax.swing.filechooser.FileFilter;
import javax.swing.filechooser.FileNameExtensionFilter;
import org.apache.pdfbox.pdmodel.PDDocument;
import org.apache.pdfbox.pdmodel.PDPage;
import org.apache.pdfbox.pdmodel.PDPageContentStream;
import org.apache.pdfbox.pdmodel.common.PDRectangle;
import org.apache.pdfbox.pdmodel.graphics.form.PDFormXObject;
import org.apache.pdfbox.util.Matrix;

/**
 *
 * @author elvio
 */
public class PagePrintExportManager {
    
    // implement the awt PageFormat class
    public static class MyPageFormat extends PageFormat {
        
        double width, height;

        public MyPageFormat(double width, double height) {
            this.width = width;
            this.height = height;
        }
        
        @Override public double[] getMatrix() {
            throw new UnsupportedOperationException();
        }

        @Override public int getOrientation() {
            throw new UnsupportedOperationException();
        }

        @Override public void setOrientation(int orientation) throws IllegalArgumentException {
            throw new UnsupportedOperationException();
        }

        @Override public void setPaper(Paper paper) {
            throw new UnsupportedOperationException();
        }

        @Override public Paper getPaper() {
            throw new UnsupportedOperationException();
        }

        @Override public double getImageableHeight() {
            return height;
        }

        @Override public double getImageableWidth() {
            return width;
        }

        @Override public double getImageableY() {
            return 0;
        }

        @Override public double getImageableX() {
            return 0;
        }

        @Override public double getHeight() {
            throw new UnsupportedOperationException();
        }

        @Override public double getWidth() {
            throw new UnsupportedOperationException();
        }
    }
    
    public static final FileFilter pdfFileFilter = 
            new FileNameExtensionFilter("Portable Document Format file (*.pdf)", new String[]{"pdf"});

    public static final FileFilter pngFileFilter = 
            new FileNameExtensionFilter("Portable Network Graphics file (*.png)", new String[]{"png"});

    
    public static void printAsPdf(MainWindowInterface mainInterface, NetPage page) {
        File pdfFile;
        boolean repeatChooser;
        do {
            repeatChooser = false;
            final JFileChooser fileChooser = new JFileChooser();
            fileChooser.setDialogTitle("Export \""+page.getPageName()+"\" in PDF format...");
            String curDir = Util.getPreferences().get("pdf-export-dir", System.getProperty("user.home"));
            fileChooser.setCurrentDirectory(curDir!=null ? new File(curDir) : null);
            fileChooser.setSelectedFile(curDir!=null ? new File(curDir+File.separator+page.getPageName()+".pdf") : null);
            fileChooser.addChoosableFileFilter(pdfFileFilter);
            fileChooser.setFileFilter(pdfFileFilter);
            if (fileChooser.showSaveDialog(mainInterface.getWindowFrame()) != JFileChooser.APPROVE_OPTION)
                return;
            pdfFile = fileChooser.getSelectedFile();
            
            curDir = fileChooser.getCurrentDirectory().getAbsolutePath();
            Util.getPreferences().put("pdf-export-dir", curDir);
            if (pdfFile.exists()) {
                int r = JOptionPane.showConfirmDialog(mainInterface.getWindowFrame(),
                            "The file \"" + pdfFile + "\" already exists! Overwrite it?",
                            "Overwrite file",
                            JOptionPane.YES_NO_CANCEL_OPTION,
                            JOptionPane.WARNING_MESSAGE);
                if (r == JOptionPane.NO_OPTION)
                    repeatChooser = true;
                else if (r == JOptionPane.CANCEL_OPTION)
                    return;
            }
        } while (repeatChooser);
        
        try {
            PDDocument document = new PDDocument();
        
            float xBorder = 0, yBorder = 0;
            Rectangle2D pageBounds = page.getPageBounds();
            float pdfScaleFactor = 12.0f;
            float width = (float)pageBounds.getWidth() * pdfScaleFactor;
            float height = (float)pageBounds.getHeight() * pdfScaleFactor;
//            System.out.println("width="+width+" height="+height);
            PDRectangle pageRect = new PDRectangle(width + 2 * xBorder, height + 2 * yBorder);
            PDPage pdfPage = new PDPage(pageRect);
            document.addPage(pdfPage);

            PdfBoxGraphics2D pdfBoxGraphics2D = new PdfBoxGraphics2D(document, width, height);
//            pdfBoxGraphics2D.draw(new Rectangle2D.Float(0, 0, width, height));

            MyPageFormat pf = new MyPageFormat(width, height);
            page.print(pdfBoxGraphics2D, pf);
            

            pdfBoxGraphics2D.dispose();
            PDFormXObject xform = pdfBoxGraphics2D.getXFormObject();

            // Build a matrix to place the form
            Matrix matrix = new Matrix();
            // Note: As PDF coordinates start at the bottom left corner, we move up from there.
            matrix.translate(xBorder, yBorder);
            PDPageContentStream contentStream = new PDPageContentStream(document, pdfPage);
            contentStream.transform(matrix);

            // Now finally draw the form. As we not do any scaling, the form drawn has a size of 5,5 x 5,5 inches,
            // because PDF uses 72 DPI for its lengths by default. If you want to scale, skew or rotate the form you can
            // of course do this. And you can also draw the form more then once. Think of the XForm as a stamper.
            contentStream.drawForm(xform);

            contentStream.close();

            document.save(pdfFile);
            document.close();
            
            mainInterface.setStatus("PDF exported.", true);
        } 
        catch (IOException e) {
            JOptionPane.showMessageDialog(mainInterface.getWindowFrame(), 
                                          "An error happened while exporting the page in PDF format.\n"
                                          + "Reason: "+e.getMessage(),
                                          "Export \""+page.getPageName()+"\" in PDF format...", 
                                          JOptionPane.ERROR_MESSAGE);            
            mainInterface.setStatus("could not export in PDF format.", true);
        }
    }
    
    
    public static void printAsPng(MainWindowInterface mainInterface, NetPage page) {
        File pngFile;
        boolean repeatChooser;
        do {
            repeatChooser = false;
            final JFileChooser fileChooser = new JFileChooser();
            fileChooser.setDialogTitle("Export \""+page.getPageName()+"\" in PNG format...");
            String curDir = Util.getPreferences().get("png-export-dir", System.getProperty("user.home"));
            fileChooser.setCurrentDirectory(curDir!=null ? new File(curDir) : null);
            fileChooser.setSelectedFile(curDir!=null ? new File(curDir+File.separator+page.getPageName()+".png") : null);
            fileChooser.addChoosableFileFilter(pngFileFilter);
            fileChooser.setFileFilter(pngFileFilter);
            if (fileChooser.showSaveDialog(mainInterface.getWindowFrame()) != JFileChooser.APPROVE_OPTION)
                return;
            pngFile = fileChooser.getSelectedFile();
            
            curDir = fileChooser.getCurrentDirectory().getAbsolutePath();
            Util.getPreferences().put("png-export-dir", curDir);
            if (pngFile.exists()) {
                int r = JOptionPane.showConfirmDialog(mainInterface.getWindowFrame(), 
                            "The file \""+pngFile+"\" already exists! Overwrite it?", 
                            "Overwrite file", 
                            JOptionPane.YES_NO_CANCEL_OPTION, 
                            JOptionPane.WARNING_MESSAGE);
                if (r == JOptionPane.NO_OPTION)
                    repeatChooser = true;
                else if (r == JOptionPane.CANCEL_OPTION)
                    return;
            }
        } while (repeatChooser);
        
        try {
            float xBorder = 0, yBorder = 0;
            Rectangle2D pageBounds = page.getPageBounds();
            float pngScaleFactor = 32.0f;
            int width = (int)(pageBounds.getWidth() * pngScaleFactor);
            int height = (int)(pageBounds.getHeight() * pngScaleFactor);
//            System.out.println("width="+width+" height="+height);
            
            BufferedImage image = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
            Graphics2D g2 = image.createGraphics();
            g2.setColor(Color.white);
            g2.fillRect(0, 0, width, height);
            
            MyPageFormat pf = new MyPageFormat(pageBounds.getWidth(), pageBounds.getHeight());
            g2.scale(pngScaleFactor, pngScaleFactor);
            page.print(g2, pf);

            ImageIO.write(image, "png", pngFile.getAbsoluteFile());
            
            mainInterface.setStatus("PDF exported.", true);
        } 
        catch (IOException e) {
            JOptionPane.showMessageDialog(mainInterface.getWindowFrame(), 
                                          "An error happened while exporting the page in PNG format.\n"
                                          + "Reason: "+e.getMessage(),
                                          "Export \""+page.getPageName()+"\" in PNG format...", 
                                          JOptionPane.ERROR_MESSAGE);            
            mainInterface.setStatus("could not export in PNG format.", true);
        }
    }
    
    
    public static void showNetMatrices(MainWindowInterface mainInterface, GspnPage gspn) {
        ShowNetMatricesDialog dlg = new ShowNetMatricesDialog(mainInterface.getWindowFrame(), true, gspn);
        dlg.setVisible(true);
    }

    
    public static void exportGspnInGreatSPNFormat(MainWindowInterface mainInterface, GspnPage gspn) {
        assert gspn.isPageCorrect();
        
        File netFile, defFile;
        boolean repeatChooser;
        do {
            repeatChooser = false;
            final JFileChooser fileChooser = new JFileChooser();
            fileChooser.setDialogTitle("Export \""+gspn.getPageName()+"\" in GreatSPN format...");
            String curDir = Util.getPreferences().get("greatspn-export-dir", System.getProperty("user.home"));
            fileChooser.setCurrentDirectory(curDir!=null ? new File(curDir) : null);
            fileChooser.setSelectedFile(curDir!=null ? new File(curDir+File.separator+gspn.getPageName()+".net") : null);
            fileChooser.addChoosableFileFilter(GreatSpnFormat.fileFilter);
            fileChooser.setFileFilter(GreatSpnFormat.fileFilter);
            if (fileChooser.showSaveDialog(mainInterface.getWindowFrame()) != JFileChooser.APPROVE_OPTION)
                return;
            netFile = fileChooser.getSelectedFile();
            
            // Generate the .net and .def filenames
            String path = netFile.getPath();
            int lastDot = path.lastIndexOf(".");
            //System.out.println("path.substring(lastDot) = "+path.substring(lastDot));
            if (lastDot != -1 && path.substring(lastDot).equalsIgnoreCase(".net"))
                path = path.substring(0, lastDot);
            netFile = new File(path + ".net");
            defFile = new File(path + ".def");
            curDir = fileChooser.getCurrentDirectory().getAbsolutePath();
            Util.getPreferences().put("greatspn-export-dir", curDir);
            if (netFile.exists()) {
                int r = JOptionPane.showConfirmDialog(mainInterface.getWindowFrame(), 
                         "The file \""+netFile+"\" already exists! Overwrite it?", 
                                                       "Overwrite file", 
                                                       JOptionPane.YES_NO_CANCEL_OPTION, 
                                                       JOptionPane.WARNING_MESSAGE);
                if (r == JOptionPane.NO_OPTION)
                    repeatChooser = true;
                else if (r == JOptionPane.CANCEL_OPTION)
                    return;
            }
        } while (repeatChooser);
        
        System.out.println("netFile = "+netFile);
        System.out.println("defFile = "+defFile);
        
        try {
            String log = GreatSpnFormat.exportGspn((GspnPage)gspn, netFile, defFile, 
                                                    Main.isGreatSPNExtAllowed(),
                                                    Main.areGreatSPNMdepArcsAllowed());
            if (log != null)
                new ModalLogDialog(mainInterface.getWindowFrame(), log).setVisible(true);
            mainInterface.setStatus("GSPN exported.", true);
        }
        catch (Exception e) {
            JOptionPane.showMessageDialog(mainInterface.getWindowFrame(), 
                                          "An error happened while exporting the page in GreatSPN format.\n"
                                          + "Reason: "+e.getMessage(),
                                          "Export \""+gspn.getPageName()+"\" in GreatSPN format...", 
                                          JOptionPane.ERROR_MESSAGE);            
            mainInterface.setStatus("could not export GSPN.", true);
        }
    }
    
    public static void exportGspnInGRMLFormat(MainWindowInterface mainInterface, GspnPage gspn) {
        assert gspn.isPageCorrect();
        
        File grmlFile;
        boolean repeatChooser;
        do {
            repeatChooser = false;
            final JFileChooser fileChooser = new JFileChooser();
            fileChooser.setDialogTitle("Export \""+gspn.getPageName()+"\" in GrML format...");
            String curDir = Util.getPreferences().get("grml-export-dir", System.getProperty("user.home"));
            fileChooser.setCurrentDirectory(curDir!=null ? new File(curDir) : null);
            fileChooser.setSelectedFile(curDir!=null ? new File(curDir+File.separator+gspn.getPageName()+".grml") : null);
            fileChooser.addChoosableFileFilter(GRMLFormat.fileFilter);
            fileChooser.setFileFilter(GRMLFormat.fileFilter);
            if (fileChooser.showSaveDialog(mainInterface.getWindowFrame()) != JFileChooser.APPROVE_OPTION)
                return;
            grmlFile = fileChooser.getSelectedFile();
            curDir = fileChooser.getCurrentDirectory().getAbsolutePath();
            Util.getPreferences().put("grml-export-dir", curDir);
            if (grmlFile.exists()) {
                int r = JOptionPane.showConfirmDialog(mainInterface.getWindowFrame(), 
                         "The file \""+grmlFile+"\" already exists! Overwrite it?", 
                                                       "Overwrite file", 
                                                       JOptionPane.YES_NO_CANCEL_OPTION, 
                                                       JOptionPane.WARNING_MESSAGE);
                if (r == JOptionPane.NO_OPTION)
                    repeatChooser = true;
                else if (r == JOptionPane.CANCEL_OPTION)
                    return;
            }
        } while (repeatChooser);
        
        try {
            //StructInfo struct = StructInfo.computeStructInfo(mainInterface.getWindowFrame(), 
            //                                                 (GspnPage)gspn, null, null);
            String log = GRMLFormat.exportGspn((GspnPage)gspn, grmlFile);
            if (log != null)
                new ModalLogDialog(mainInterface.getWindowFrame(), log).setVisible(true);
            mainInterface.setStatus("GSPN exported in GrML format.", true);
        }
        catch (Exception e) {
            JOptionPane.showMessageDialog(mainInterface.getWindowFrame(), 
                                          "An error happened while exporting the page in GrML format.\n"
                                          + "Reason: "+e.getMessage(),
                                          "Export \""+gspn.getPageName()+"\" in GrML format...", 
                                          JOptionPane.ERROR_MESSAGE);            
            mainInterface.setStatus("could not export GSPN in GrML format.", true);
        }        
    }
    
    public static void exportGspnInPNMLFormat(MainWindowInterface mainInterface, GspnPage gspn) {
        assert gspn.isPageCorrect();
        
        File pnmlFile;
        boolean repeatChooser;
        do {
            repeatChooser = false;
            final JFileChooser fileChooser = new JFileChooser();
            fileChooser.setDialogTitle("Export \""+gspn.getPageName()+"\" in PNML format...");
            String curDir = Util.getPreferences().get("pnml-export-dir", System.getProperty("user.home"));
            fileChooser.setCurrentDirectory(curDir!=null ? new File(curDir) : null);
            fileChooser.setSelectedFile(curDir!=null ? new File(curDir+File.separator+gspn.getPageName()+".pnml") : null);
            fileChooser.addChoosableFileFilter(PNMLFormat.fileFilter);
            fileChooser.setFileFilter(PNMLFormat.fileFilter);
            if (fileChooser.showSaveDialog(mainInterface.getWindowFrame()) != JFileChooser.APPROVE_OPTION)
                return;
            pnmlFile = fileChooser.getSelectedFile();
            curDir = fileChooser.getCurrentDirectory().getAbsolutePath();
            Util.getPreferences().put("pnml-export-dir", curDir);
            if (pnmlFile.exists()) {
                int r = JOptionPane.showConfirmDialog(mainInterface.getWindowFrame(), 
                         "The file \""+pnmlFile+"\" already exists! Overwrite it?", 
                                                       "Overwrite file", 
                                                       JOptionPane.YES_NO_CANCEL_OPTION, 
                                                       JOptionPane.WARNING_MESSAGE);
                if (r == JOptionPane.NO_OPTION)
                    repeatChooser = true;
                else if (r == JOptionPane.CANCEL_OPTION)
                    return;
            }
        } while (repeatChooser);
        
        try {
            //StructInfo struct = StructInfo.computeStructInfo(mainInterface.getWindowFrame(), 
            //                                                 (GspnPage)gspn, null, null);
            String log = PNMLFormat.exportGspn((GspnPage)gspn, pnmlFile, true);
            if (log != null)
                new ModalLogDialog(mainInterface.getWindowFrame(), log).setVisible(true);
            mainInterface.setStatus("GSPN exported in PNML format.", true);
        }
        catch (Exception e) {
            JOptionPane.showMessageDialog(mainInterface.getWindowFrame(), 
                                          "An error happened while exporting the page in PNML format.\n"
                                          + "Reason: "+e.getMessage(),
                                          "Export \""+gspn.getPageName()+"\" in PNML format...", 
                                          JOptionPane.ERROR_MESSAGE);            
            mainInterface.setStatus("could not export GSPN in PNML format.", true);
        }       
    }
    
    public static void exportGspnInAPNNFormat(MainWindowInterface mainInterface, GspnPage gspn) {
        assert gspn.isPageCorrect();
        
        File apnnFile;
        boolean repeatChooser;
        do {
            repeatChooser = false;
            final JFileChooser fileChooser = new JFileChooser();
            fileChooser.setDialogTitle("Export \""+gspn.getPageName()+"\" in APNN format...");
            String curDir = Util.getPreferences().get("apnn-export-dir", System.getProperty("user.home"));
            fileChooser.setCurrentDirectory(curDir!=null ? new File(curDir) : null);
            fileChooser.setSelectedFile(curDir!=null ? new File(curDir+File.separator+gspn.getPageName()+".apnn") : null);
            fileChooser.addChoosableFileFilter(ApnnFormat.fileFilter);
            fileChooser.setFileFilter(ApnnFormat.fileFilter);
            if (fileChooser.showSaveDialog(mainInterface.getWindowFrame()) != JFileChooser.APPROVE_OPTION)
                return;
            apnnFile = fileChooser.getSelectedFile();
            curDir = fileChooser.getCurrentDirectory().getAbsolutePath();
            Util.getPreferences().put("apnn-export-dir", curDir);
            if (apnnFile.exists()) {
                int r = JOptionPane.showConfirmDialog(mainInterface.getWindowFrame(), 
                         "The file \""+apnnFile+"\" already exists! Overwrite it?", 
                                                       "Overwrite file", 
                                                       JOptionPane.YES_NO_CANCEL_OPTION, 
                                                       JOptionPane.WARNING_MESSAGE);
                if (r == JOptionPane.NO_OPTION)
                    repeatChooser = true;
                else if (r == JOptionPane.CANCEL_OPTION)
                    return;
            }
        } while (repeatChooser);
        
        try {
            StructInfo struct = StructInfo.computeStructInfo(mainInterface.getWindowFrame(), 
                                                             (GspnPage)gspn, null, null);
            String log = ApnnFormat.exportGspn((GspnPage)gspn, apnnFile, struct, null);
            if (log != null)
                new ModalLogDialog(mainInterface.getWindowFrame(), log).setVisible(true);
            mainInterface.setStatus("GSPN exported in APNN format.", true);
        }
        catch (Exception e) {
            JOptionPane.showMessageDialog(mainInterface.getWindowFrame(), 
                                          "An error happened while exporting the page in APNN format.\n"
                                          + "Reason: "+e.getMessage(),
                                          "Export \""+gspn.getPageName()+"\" in APNN format...", 
                                          JOptionPane.ERROR_MESSAGE);            
            mainInterface.setStatus("could not export GSPN in APNN format.", true);
        }
    }
    
    
    public static void exportGspnInNetLogoFormat(MainWindowInterface mainInterface, GspnPage gspn) {
        assert gspn.isPageCorrect();
        
        File nlogoFile;
        boolean repeatChooser;
        do {
            repeatChooser = false;
            final JFileChooser fileChooser = new JFileChooser();
            fileChooser.setDialogTitle("Export \""+gspn.getPageName()+"\" in NetLogo format...");
            String curDir = Util.getPreferences().get("netlogo-export-dir", System.getProperty("user.home"));
            fileChooser.setCurrentDirectory(curDir!=null ? new File(curDir) : null);
            fileChooser.setSelectedFile(curDir!=null ? new File(curDir+File.separator+gspn.getPageName()+".nlogo") : null);
            fileChooser.addChoosableFileFilter(NetLogoFormat.fileFilter);
            fileChooser.setFileFilter(NetLogoFormat.fileFilter);
            if (fileChooser.showSaveDialog(mainInterface.getWindowFrame()) != JFileChooser.APPROVE_OPTION)
                return;
            nlogoFile = fileChooser.getSelectedFile();
            curDir = fileChooser.getCurrentDirectory().getAbsolutePath();
            Util.getPreferences().put("netlogo-export-dir", curDir);
            if (nlogoFile.exists()) {
                int r = JOptionPane.showConfirmDialog(mainInterface.getWindowFrame(), 
                         "The file \""+nlogoFile+"\" already exists! Overwrite it?", 
                                                       "Overwrite file", 
                                                       JOptionPane.YES_NO_CANCEL_OPTION, 
                                                       JOptionPane.WARNING_MESSAGE);
                if (r == JOptionPane.NO_OPTION)
                    repeatChooser = true;
                else if (r == JOptionPane.CANCEL_OPTION)
                    return;
            }
        } while (repeatChooser);
        
        try {
//            StructInfo struct = StructInfo.computeStructInfo(mainInterface.getWindowFrame(), 
//                                                             (GspnPage)gspn, null, null);
            ParserContext context = new ParserContext(gspn);
            gspn.compileParsedInfo(context);
            String log = NetLogoFormat.export((GspnPage)gspn, nlogoFile, context, true);
            if (log != null)
                new ModalLogDialog(mainInterface.getWindowFrame(), log).setVisible(true);
            mainInterface.setStatus("GSPN exported in NetLogo format.", true);
        }
        catch (Exception e) {
            JOptionPane.showMessageDialog(mainInterface.getWindowFrame(), 
                                          "An error happened while exporting the page in NetLogo format.\n"
                                          + "Reason: "+e.getMessage(),
                                          "Export \""+gspn.getPageName()+"\" in NetLogo format...", 
                                          JOptionPane.ERROR_MESSAGE);            
            mainInterface.setStatus("could not export GSPN in NetLogo format.", true);
        }
    }
    
    
    public static void exportInDtaFormat(MainWindowInterface mainInterface, DtaPage dta) {
        assert dta.isPageCorrect();
        
        File dtaFile;
        boolean repeatChooser;
        do {
            repeatChooser = false;
            final JFileChooser fileChooser = new JFileChooser();
            fileChooser.setDialogTitle("Export \""+dta.getPageName()+"\" in MC4CSLTA format...");
            String curDir = Util.getPreferences().get("dta-export-dir", System.getProperty("user.home"));
            fileChooser.setCurrentDirectory(curDir!=null ? new File(curDir) : null);
            fileChooser.setSelectedFile(curDir!=null ? new File(curDir+File.separator+dta.getPageName()+".dta") : null);
            fileChooser.addChoosableFileFilter(DtaFormat.fileFilter);
            fileChooser.setFileFilter(DtaFormat.fileFilter);
            if (fileChooser.showSaveDialog(mainInterface.getWindowFrame()) != JFileChooser.APPROVE_OPTION)
                return;
            dtaFile = fileChooser.getSelectedFile();
            
            curDir = fileChooser.getCurrentDirectory().getAbsolutePath();
            Util.getPreferences().put("dta-export-dir", curDir);
            if (dtaFile.exists()) {
                int r = JOptionPane.showConfirmDialog(mainInterface.getWindowFrame(), 
                         "The file \""+dtaFile+"\" already exists! Overwrite it?", 
                                                       "Overwrite file", 
                                                       JOptionPane.YES_NO_CANCEL_OPTION, 
                                                       JOptionPane.WARNING_MESSAGE);
                if (r == JOptionPane.NO_OPTION)
                    repeatChooser = true;
                else if (r == JOptionPane.CANCEL_OPTION)
                    return;
            }
        } while (repeatChooser);
        
        try {
            String log = DtaFormat.export((DtaPage)dta, dtaFile);
            if (log != null)
                new ModalLogDialog(mainInterface.getWindowFrame(), log).setVisible(true);
            mainInterface.setStatus("DTA exported.", true);
        }
        catch (Exception e) {
            JOptionPane.showMessageDialog(mainInterface.getWindowFrame(), 
                                          "An error happened while exporting the DTA in MC4CSLTA format.\n"
                                          + "Reason: "+e.getMessage(),
                                          "Export \""+dta.getPageName()+"\" in MC4CSLTA format...", 
                                          JOptionPane.ERROR_MESSAGE);            
            mainInterface.setStatus("could not export DTA.", true);
        }        
    }
}
