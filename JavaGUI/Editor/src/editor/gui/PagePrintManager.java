/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui;

import common.Util;
import de.rototor.pdfbox.graphics2d.PdfBoxGraphics2D;
import editor.domain.DrawHelper;
import editor.domain.NetPage;
import editor.domain.io.DtaFormat;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.awt.print.PageFormat;
import java.awt.print.Paper;
import java.io.File;
import java.io.IOException;
import javax.imageio.ImageIO;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
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
public class PagePrintManager {
    
    // implement the awt PageFormat class
    protected static class MyPageFormat extends PageFormat {
        
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
            fileChooser.addChoosableFileFilter(DtaFormat.fileFilter);
            fileChooser.setFileFilter(DtaFormat.fileFilter);
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
            fileChooser.addChoosableFileFilter(DtaFormat.fileFilter);
            fileChooser.setFileFilter(DtaFormat.fileFilter);
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
    
}
