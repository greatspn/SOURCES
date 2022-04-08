/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package common;

import editor.Main;
import editor.gui.AppWindow;
import java.awt.Desktop;
import java.awt.Taskbar;
import java.awt.desktop.OpenFilesEvent;
import java.awt.desktop.OpenURIEvent;
import java.awt.desktop.QuitEvent;
import java.awt.desktop.QuitResponse;
import java.awt.desktop.QuitStrategy;
import java.io.File;
import javax.swing.SwingUtilities;

/**
 *
 * @author elvio
 */
public class OSXIntegration implements GuiIntegration {
    
    @Override
    public void completeOSXIntegration(final AppWindow wnd, String iconName) {
        if (Util.isOSX()) {
            // MacOS integration
            Taskbar taskbar = Taskbar.getTaskbar();
            taskbar.setIconImage(Main.loadImage(iconName));
        
            java.awt.Desktop desktop = Desktop.getDesktop();
            desktop.setAboutHandler(e -> wnd.showAboutDialog());
            desktop.setOpenFileHandler((OpenFilesEvent e) -> {
                SwingUtilities.invokeLater(() -> {
                    for (File file : e.getFiles())
                        wnd.openFileHandler(file.getAbsolutePath());
                });
            });
            desktop.setOpenURIHandler((OpenURIEvent e) -> {
                SwingUtilities.invokeLater(() -> {
                    wnd.openFileHandler(e.getURI().getPath());
                });
            });
            desktop.setPreferencesHandler(e -> wnd.openPreferences());
            desktop.setQuitHandler((QuitEvent e, QuitResponse response) -> {
                wnd.quitHandler();
            });
            desktop.setQuitStrategy(QuitStrategy.NORMAL_EXIT);
        }
    }
}
 