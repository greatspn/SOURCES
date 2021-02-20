/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package common;

import editor.Main;
import editor.gui.AppWindow;
import editor.gui.MainWindowInterface;
import java.awt.Desktop;
import java.awt.Image;
import java.awt.Taskbar;
import java.awt.desktop.QuitEvent;
import java.awt.desktop.QuitHandler;
import java.awt.desktop.QuitResponse;
import java.awt.desktop.QuitStrategy;

/**
 *
 * @author elvio
 */
public class OSXIntegration {
    
    public static void completeOSXIntegration(final AppWindow wnd, String iconName) {
        if (Util.isOSX()) {
            // MacOS integration
            Taskbar taskbar = Taskbar.getTaskbar();
            taskbar.setIconImage(Main.loadImage(iconName));
        
            java.awt.Desktop desktop = Desktop.getDesktop();
            desktop.setAboutHandler(e -> wnd.showAboutDialog());
            desktop.setOpenFileHandler(e -> wnd.openFileHandler(e.getSearchTerm()));
            desktop.setOpenURIHandler(e -> wnd.openFileHandler(e.getURI().getPath()));
            desktop.setQuitHandler(new QuitHandler() {
                @Override
                public void handleQuitRequestWith(QuitEvent e, QuitResponse response) {
                    wnd.quitHandler();
                }
            });
            desktop.setPreferencesHandler(e -> wnd.openPreferences());
            desktop.setQuitStrategy(QuitStrategy.NORMAL_EXIT);
        }
    }
}
 