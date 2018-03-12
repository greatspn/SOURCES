package editor.gui;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.EventQueue;
import java.awt.FlowLayout;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JTextArea;
import javax.swing.JToolBar;
import javax.swing.UIManager;

/**
 * @see http://stackoverflow.com/a/16694524/230513
 */
public class UnifiedMain {

    private void display() {
        JFrame f = new JFrame("Main");
        f.getRootPane().putClientProperty("apple.awt.brushMetalLook", true);
        f.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        f.add(createToolBar(f), BorderLayout.NORTH);
        f.add(new JTextArea(5, 16));
        f.pack();
        f.setLocationRelativeTo(null);
        f.setVisible(true);
    }

    private JToolBar createToolBar(final Component parent) {
        JToolBar bar = new JToolBar("Toolbar");
        bar.setFloatable(false);
        bar.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        bar.add(createButton("FileChooser.detailsViewIcon"));
        bar.add(createButton("FileChooser.homeFolderIcon"));
        bar.add(createButton("FileChooser.newFolderIcon"));
        return bar;
    }

    private JButton createButton(String s) {
        JButton b = new JButton(UIManager.getIcon(s));
        b.setHorizontalTextPosition(JButton.CENTER);
        b.setVerticalTextPosition(JButton.CENTER);
        return b;
    }

    public static void main(String[] args) {
        EventQueue.invokeLater(new Runnable() {
            @Override
            public void run() {
                new UnifiedMain().display();
            }
        });
    }
}