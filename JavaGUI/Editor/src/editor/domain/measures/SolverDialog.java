/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import common.Util;
import editor.Main;
import editor.gui.ResourceFactory;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Frame;
import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;
import javax.swing.Icon;
import javax.swing.JFrame;
import javax.swing.JScrollBar;
import javax.swing.JTextPane;
import javax.swing.SwingUtilities;
import javax.swing.text.AbstractDocument;
import javax.swing.text.BadLocationException;
import javax.swing.text.DefaultCaret;
import javax.swing.text.SimpleAttributeSet;
import javax.swing.text.StyleConstants;
import javax.swing.text.StyledDocument;

/**
 *
 * @author elvio
 */
public class SolverDialog extends javax.swing.JDialog {
    
    public interface InterruptibleSolverListener {
        public void interruptSolution();
    }
    
    private InterruptibleSolverListener interruptListener = null;

    private final SimpleAttributeSet styles[][][]; // bold * underline * foregrounds
    private static final Color[] foregrounds = {
        Color.BLACK, new Color(128,0,0),
        new Color(0,128,0), new Color(128,128,0),
        new Color(0,0,128), new Color(128,0,128),
        new Color(0,128,128), new Color(128,128,128),
    };
    private final SimpleAttributeSet sasIconOk;
    private final SimpleAttributeSet sasIconNo;
    private final SimpleAttributeSet sasIconLineSep;
    private int currColor = 0, currBold = 0, currUnder = 0;
    
    private class MyTextPane extends JTextPane {

        public MyTextPane() {
            setEditable(false);
        }
        
        // Keep tracks of the last dimension, to avoid flickering
        private Dimension lastDim = null;
        @Override
        public Dimension getPreferredSize() {
            Dimension d = super.getPreferredSize(); 
            if (lastDim != null && lastDim.width == d.width) {
                d.height = Math.max(d.height, lastDim.height);
            }
            lastDim = d;
            return d;
        }
    }
    private final MyTextPane textPane;
    
    /**
     * Creates new form SolverDialog
     * @param frame
     * @param modal
     */
    public SolverDialog(Frame frame, boolean modal) {
        super(frame, modal);
        initComponents();
        
        textPane = new MyTextPane();
        scrollPane.getViewport().setView(textPane);
        
        // Prepare document styles
        styles = new SimpleAttributeSet[2][2][8];
        for (int bold = 0; bold < 2; bold++) {
            for (int under = 0; under < 2; under++) {
                for (int clr = 0; clr < 8; clr ++) {
                    SimpleAttributeSet attr = new SimpleAttributeSet();
                    StyleConstants.setFontFamily(attr, "Monospaced");
                    StyleConstants.setFontSize(attr, (int)(13 * Util.UIscaleFactor));
                    StyleConstants.setForeground(attr, foregrounds[clr]);
                    StyleConstants.setBold(attr, bold != 0);
                    StyleConstants.setUnderline(attr, under != 0);
                    styles[bold][under][clr] = attr;
                }
            }
        }
        sasIconOk = new SimpleAttributeSet();
        StyleConstants.setIcon(sasIconOk, OK_ICON);
        sasIconNo = new SimpleAttributeSet();
        StyleConstants.setIcon(sasIconNo, NO_ICON);
        sasIconLineSep = new SimpleAttributeSet();
        StyleConstants.setIcon(sasIconLineSep, LINESEP_ICON);
        
        ((DefaultCaret)textPane.getCaret()).setUpdatePolicy(DefaultCaret.NEVER_UPDATE);
        
        Dimension dim = new Dimension((int)(frame.getWidth()*0.8), 
                                      (int)(frame.getHeight()*0.7));
        dim.width = Math.max(dim.width, 300);
        dim.height = Math.max(dim.height, 200);
        textPane.setMinimumSize(dim);
        textPane.setPreferredSize(dim);
        pack();
        setLocationRelativeTo(frame);
        jButtonClose.requestFocus(); // Enter will close the dialog window.
        getRootPane().setDefaultButton(jButtonClose);
    }
        
    private static final Icon OK_ICON = ResourceFactory.getInstance().getOk16();
    private static final Icon NO_ICON = ResourceFactory.getInstance().getError16();
    private static final Icon LINESEP_ICON = Main.loadImageIcon("LineSeparator.png");
    
    // Max number of VT-100 remove line command that can be issued
    private static final int MAX_REMOVABLE_LINES = 20;
    int[] lastEndlPos = new int[MAX_REMOVABLE_LINES];
//    int lastEndlPos1 = 0, lastEndlPos2 = 0;
    
    private int cursorPos = 0;
    private void insertTextAtCursor(String str, SimpleAttributeSet style) 
    {
        JScrollBar vertBar = scrollPane.getVerticalScrollBar();
//        System.out.println("v1="+(vertBar.getValue()+vertBar.getVisibleAmount())+
//                           "  max="+vertBar.getMaximum());
        boolean viewAtBottom = (vertBar.getValue()+vertBar.getVisibleAmount()) == vertBar.getMaximum();
        StyledDocument doc = textPane.getStyledDocument();
//        ((DefaultCaret)textPane.getCaret()).setUpdatePolicy(DefaultCaret.ALWAYS_UPDATE);
//            (viewAtBottom ? DefaultCaret.ALWAYS_UPDATE : DefaultCaret.NEVER_UPDATE);
        int selStart = textPane.getSelectionStart();
        int selEnd = textPane.getSelectionEnd();
        try {
            int len = doc.getLength();
//            System.out.println("cursorPos="+cursorPos+" len="+len+" str="+str.length()+
//                                " lastEndlPos1="+lastEndlPos1+" lastEndlPos2="+lastEndlPos2);
            assert cursorPos <= len;
            if (len == cursorPos) {
                // Append the text at the bottom
                doc.insertString(len, str, style);
                // Update cursor position - now at bottom
                cursorPos = doc.getLength();
            }
            else { // replace characters in the insertion
                int numCharToReplace = Math.min(str.length(), len - cursorPos);
                
                if (doc instanceof AbstractDocument) {
                    AbstractDocument abdoc = (AbstractDocument)doc;
                    abdoc.replace(cursorPos, numCharToReplace, str, style);
                }
                else {
                    doc.remove(cursorPos, numCharToReplace);
                    doc.insertString(cursorPos, str, style);
                }
                // Update cursor position - could be that it is not at the page bottom,
                // if we have received 'move cursor up' commands
                cursorPos += str.length();
            }
            
            if (selEnd > selStart) {
                textPane.setSelectionStart(selStart);
                textPane.setSelectionEnd(selEnd);
            }
            else if (viewAtBottom)
                textPane.setCaretPosition(cursorPos);
        }
        catch (BadLocationException e) {
            Main.logException(e, true);
        }
    }
    
    private final List<String> stringLog = new LinkedList<>();
    private final List<Boolean> isStdOutLog = new LinkedList<>();
    
    // Append a text with VT-100 style formatting
    public void append(String str, boolean isStdOut) {
        stringLog.add(str);
        isStdOutLog.add(isStdOut);
        while (str.length() > 0) {
            // Test if we start with an escape sequence
            if (str.length() > 2 && str.charAt(0)=='\033' && str.charAt(1)=='[') {
                // ANSI VT-100 key code
                if (str.startsWith("\033[0m")) { // Default display
                    currColor = 0;
                    currBold = 0;
                    currUnder = 0;
                }
                else if (str.startsWith("\033[1m")) {/* bold ON */
                    currBold = 1;
                }
                else if (str.startsWith("\033[4m")) {/* underline ON */ 
                    currUnder = 1;
                }
                else if (str.startsWith("\033[22m")) {/* bold OFF */
                    currBold = 0;
                }
                else if (str.startsWith("\033[24m")) {/* underline OFF */ 
                    currUnder = 0;
                }
                else if (str.startsWith("\033[30m")) { currColor = 0; } // black
                else if (str.startsWith("\033[31m")) { currColor = 1; } // red
                else if (str.startsWith("\033[32m")) { currColor = 2; } // green
                else if (str.startsWith("\033[33m")) { currColor = 3; } // yellow
                else if (str.startsWith("\033[34m")) { currColor = 4; } // blue
                else if (str.startsWith("\033[35m")) { currColor = 5; } // magenta
                else if (str.startsWith("\033[36m")) { currColor = 6; } // cyan
                else if (str.startsWith("\033[37m")) { currColor = 7; } // white
                else if (str.startsWith("\033[1A") || str.startsWith("\033[A")) { // remove last line
                    if (lastEndlPos[1] != -1) {
                        // Move the cursor position to the last '\n'
                        cursorPos = lastEndlPos[1];
                        for (int i=1; i<MAX_REMOVABLE_LINES; i++)
                            lastEndlPos[i - 1] = lastEndlPos[i];
                        lastEndlPos[MAX_REMOVABLE_LINES - 1] = -1;
                    }
                }
                // NON ANSI VT-100 CODES:
                else if (str.startsWith("\033[0X")) { // CUSTOM CODE: insert ok icon
                    insertTextAtCursor(" ", sasIconOk);
                }
                else if (str.startsWith("\033[1X")) { // CUSTOM CODE: insert error icon
                    insertTextAtCursor(" ", sasIconNo);
                }
                else if (str.startsWith("\033[2X")) { // CUSTOM CODE: insert line separator
                    insertTextAtCursor(" ", sasIconLineSep);
                }

                // Remove the ESC sequence and parse the rest of the string
                int beg = 2;
                while (!Character.isLetter(str.charAt(beg)))
                    beg++;
                assert beg < 7;
                str = str.substring(beg + 1);
            }
            else if (str.charAt(0)=='\033') {
                // Spurious ESC character
                str = str.substring(1);
            }
            else {
                // Add the text to the textPane.
                String text;
                int ESC_pos = str.indexOf('\033');
                int RET_pos = str.indexOf('\n');
                int pos = str.length();
                if (RET_pos != -1) 
                    pos = Math.min(pos, RET_pos + 1);
                if (ESC_pos != -1)
                    pos = Math.min(pos, ESC_pos);
                // There is an ESC sequence or a return, separate the text from the rest.
                text = str.substring(0, pos);
                str = str.substring(pos);

                // Append with the current style
                SimpleAttributeSet style = styles[currBold][currUnder][currColor];
                if (!isStdOut)
                    style = styles[0][0][1];
                insertTextAtCursor(text, style);

                if (text.endsWith("\n")) {
                    // Remember the position of the last N carriage returns
                    for (int i=MAX_REMOVABLE_LINES-1; i>0; i--)
                        lastEndlPos[i] = lastEndlPos[i - 1];
                    lastEndlPos[0] = cursorPos;
                }
            }
        }
    }

    public void setInterruptListener(InterruptibleSolverListener l) {
        this.interruptListener = l;
        SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
                if (interruptListener != null)
                    jButtonClose.setText("Interrupt");
                else 
                    jButtonClose.setText("Close");
            }
        });
    }
    
    public byte[] getComputationLog() {
        try {
            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            ObjectOutputStream out = new ObjectOutputStream(bos); 
            out.writeObject(stringLog);
            out.writeObject(isStdOutLog);
            return bos.toByteArray();
        }
        catch (IOException e) {
            return null;
        }
    }
    public void setComputationLog(byte[] serializedLog) {
        try {
            ByteArrayInputStream bis = new ByteArrayInputStream(serializedLog);
            ObjectInputStream ois = new ObjectInputStream(bis);
            @SuppressWarnings("unchecked")
            List<String> serializedStringLog = (List<String>)ois.readObject();
            @SuppressWarnings("unchecked")
            List<Boolean> serializedIsStdOutLog = (List<Boolean>)ois.readObject();
            ListIterator<String> i1 = serializedStringLog.listIterator();
            ListIterator<Boolean> i2 = serializedIsStdOutLog.listIterator();
            while (i1.hasNext())
                append(i1.next(), i2.next());
        }
        catch (IOException | ClassNotFoundException e) {
            Main.logException(e, true);
            textPane.setText("");
        }
    }

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jPanelCenter = new javax.swing.JPanel();
        scrollPane = new javax.swing.JScrollPane();
        jPanelBottom = new javax.swing.JPanel();
        jButtonClose = new javax.swing.JButton();

        jPanelCenter.setBorder(javax.swing.BorderFactory.createTitledBorder("Solver output"));
        jPanelCenter.setLayout(new java.awt.GridLayout(1, 0));
        jPanelCenter.add(scrollPane);

        getContentPane().add(jPanelCenter, java.awt.BorderLayout.CENTER);

        jButtonClose.setText("Close");
        jButtonClose.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButtonCloseActionPerformed(evt);
            }
        });
        jPanelBottom.add(jButtonClose);

        getContentPane().add(jPanelBottom, java.awt.BorderLayout.SOUTH);

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void jButtonCloseActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButtonCloseActionPerformed
        if (interruptListener != null) {
            // Interrupt the solver thread
            interruptListener.interruptSolution();
        }
        else {
            // Close the window
            SwingUtilities.invokeLater(new Runnable() {
                @Override
                public void run() {
                    setVisible(false);
                    dispose();
                }
            });
        }
    }//GEN-LAST:event_jButtonCloseActionPerformed

    public static void main(String[] args) throws InterruptedException {
        final JFrame frame = new JFrame();
        frame.setBounds(0, 0, 800, 600);
        final SolverDialog dlg = new SolverDialog(frame, true);
        SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
                frame.setVisible(true);
                dlg.setVisible(true);
            }
        });
        
        Thread.sleep(1000);
        try (BufferedReader br = new BufferedReader(new FileReader("/Users/elvio/log.txt"))) {
            String line;
            while ((line = br.readLine()) != null) {
//                System.out.println("reading line");
                dlg.append(line+"\n", true);
                Thread.sleep(250);
            }
        }
        catch (IOException e) {
            System.out.println("io exception");
        }
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton jButtonClose;
    private javax.swing.JPanel jPanelBottom;
    private javax.swing.JPanel jPanelCenter;
    private javax.swing.JScrollPane scrollPane;
    // End of variables declaration//GEN-END:variables
}
