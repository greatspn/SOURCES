/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor;

import common.LogWindow;
import common.Util;
import editor.domain.ListRenderable;
import editor.gui.AppWindow;
import java.awt.AWTEvent;
import java.awt.Component;
import java.awt.Desktop;
import java.awt.EventQueue;
import java.awt.Image;
import java.awt.Toolkit;
import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.net.URISyntaxException;
import java.net.URL;
import java.net.URLConnection;
import java.net.URLDecoder;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;
import latex.LatexProviderImpl;

/**
 *
 * @author Elvio
 */
public class Main {
    
    // Version number
    public static final int VERSION_NUMBER = 100;
    public static final String VERSION_URL = "http://www.di.unito.it/~amparore/mc4cslta/VERSION";
    public static final String APP_NAME = "New GreatSPN Editor";
    public static final String PREF_ROOT_KEY = "New Editor";
    public static int JAVA_VERSION_REQUIRED_MAJOR = 11;

    // The main application instance
    static AppWindow wnd;
    
    // Should we send exceptions to the GUI?
    public static boolean useGuiLogWindowForExceptions = true;
    
    /**
     * @param args the command line arguments
     */
    public static void main(final String[] args) {
        requireJavaVersion(JAVA_VERSION_REQUIRED_MAJOR);
        Util.initApplication(PREF_ROOT_KEY, "/org/unito/mainprefs");
        isAppImageDistribution();
        
        // For debug purposes
        installCustomEventQueue();
        
        SwingUtilities.invokeLater(new Runnable() {
            @Override public void run() {
                // Open the communication channel with the LaTeX provider
                LatexProviderImpl.initializeProvider();
                // Create the main window and start the application
                wnd = new AppWindow();
                wnd.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
//                wnd.addWindowListener(new WindowAdapter() {
//                    @Override
//                    public void windowClosed(WindowEvent we) {
//                        LatexProvider.getProvider().quit();
//                    }
//                });
                // Loads the custom fonts
                //Util.loadFont("/editor/gui/icons/cmr10.ttf");
                
                wnd.setVisible(true);                
                
                // Open the files passed on the command line
                for (String arg : args) {
                    try {
                        // Get the canonical name of the file
                        File file = new File(arg).getCanonicalFile();
                        if (file.isFile() && file.canRead()) {
                            System.out.println("Opening "+file.getAbsolutePath());
                            wnd.openFile(file);
                        }
                    }
                    catch (IOException e) { }
                }
            }
        });
    }
    
    public static enum UiSize implements ListRenderable {
        NORMAL("Normal UI size", "brick16.png", 1.0f),
        LARGE("Large UI size (150 %)", "brick24.png", 1.5f),
        LARGER("Very large UI size (200 %)", "brick32.png", 2.0f);
        
        String description;
        Icon icon;
        float fontSizeMult;

        private UiSize(String description, String iconName, float fontSizeMult) {
            this.description = description;
            this.icon = loadImageIcon(iconName);
            this.fontSizeMult = fontSizeMult;
        }

        @Override
        public String getDescription() { return description; }
        @Override
        public Icon getIcon16() { return icon;  }
        @Override
        public int getTextSize() {
            return (int)(Util.origDefaultUiFontSize * fontSizeMult);
        }
        public float getScaleMultiplier() { return fontSizeMult; }
    }
    private static  UiSize theUiSize = null;
    public static UiSize getUiSize() {
        if (theUiSize == null)
            theUiSize = getStartupUiSize();
        return theUiSize;
    }
    
    // Loading images from the application jar
    public static Image loadImage(String imageName) {
        return Util.loadImage("/editor/gui/icons/" + imageName);
    }
    
    public static ImageIcon loadImageIcon(String imageName) {
        return new ImageIcon(loadImage(imageName));
    }
    
//    public enum IconSize {
//        SMALL_16_24_32(16, 24, 32),
//        MEDIUM_24_32_48(24, 32, 48),
//        LARGE_32_48_64(32, 48, 64);
//
//        private IconSize(int size1, int size2, int size3) {
//            this.size1 = size1;
//            this.size2 = size2;
//            this.size3 = size3;
//        }
//        final int size1, size2, size3;
//    }
//    
//    public static ImageIcon loadImageIconOfSize(String imageName, IconSize size) {
//        String load, test1, test2;
//        switch (getUiSize()) {
//            case NORMAL:  
//                load = imageName + size.size1+".png";
//                test1 = imageName + size.size2 + ".png";
//                test2 = imageName + size.size3 + ".png";
//                break;
//            case LARGE:
//                test1 = imageName + size.size1+".png";
//                load = imageName + size.size2 + ".png";
//                test2 = imageName + size.size3 + ".png";
//                break;
//            case LARGER:
//                test1 = imageName + size.size1+".png";
//                test2 = imageName + size.size2 + ".png";
//                load = imageName + size.size3 + ".png";
//                break;
//            default: throw new UnsupportedOperationException();
//        }
//        if (Main.class.getResource("/editor/gui/icons/" + test1) == null) {
//            System.out.println("WARNING: missing icon /editor/gui/icons/" + test1);
//        }
//        if (Main.class.getResource("/editor/gui/icons/" + test2) == null) {
//            System.out.println("WARNING: missing icon /editor/gui/icons/" + test2);
//        }
//        return new ImageIcon(loadImage(load));
//    }
//    
//    public static ImageIcon loadImageIcon16(String imageName) {
//        return loadImageIconOfSize(imageName, IconSize.SMALL_16_24_32);
//    }
//    public static ImageIcon loadImageIcon24(String imageName) {
//        return loadImageIconOfSize(imageName, IconSize.MEDIUM_24_32_48);
//    }
//    public static ImageIcon loadImageIcon32(String imageName) {
//        return loadImageIconOfSize(imageName, IconSize.LARGE_32_48_64);
//    }
    
    private static void installCustomEventQueue() {
        Toolkit.getDefaultToolkit().getSystemEventQueue().push(new EventQueue() {
            @Override protected void dispatchEvent(AWTEvent newEvent) {
                try {
                    super.dispatchEvent(newEvent);
                } catch (Throwable t) {
                    logException(t, true);
                }
            }
        });
    }
    
    // Check on the internet whether a new version of this 
    // application is available for download
    public static void verifyNewVersion(final Component component, final boolean showAlways) {
        Thread checkVersionThread = new Thread(new Runnable() {
            @Override public void run() {
                try {
                    URL website = new URL(Main.VERSION_URL);
                    URLConnection urlConn = website.openConnection();
                    urlConn.setReadTimeout(300);
                    urlConn.setConnectTimeout(300);
                    InputStream input = urlConn.getInputStream();
                    BufferedReader reader = new BufferedReader(new InputStreamReader(input));
                    String versionLine = reader.readLine();
                    final String downloadUrlLine = reader.readLine();                    
                    final int currentVersion = Integer.parseInt(versionLine);
                    
                    SwingUtilities.invokeLater(new Runnable() {
                        @Override
                        public void run() {
                            if (currentVersion > Main.VERSION_NUMBER) {
                                showNewVersionDialog(component, downloadUrlLine);
                            }
                            else {
                                System.out.println("Running the latest version of the application...");
                                if (showAlways)
                                    showNewVersionDialog(component, null);
                            }
                        }
                    });
                }
                catch (Exception e) { /* no check is done... */ }
            }
        });
        checkVersionThread.start();
    }
    
    private static void showNewVersionDialog(Component component, String downloadUrl) {
        if (downloadUrl == null) {
            JOptionPane.showMessageDialog(component, "The application is up to date.", 
                                          "No new updates.", JOptionPane.INFORMATION_MESSAGE);
            return;
        }
        
        int r = JOptionPane.showConfirmDialog(component, 
                                              "A new version of the application is available to download.\n"
                                              + "Do you want to download it now?", "New version alert", 
                                              JOptionPane.YES_NO_OPTION);
        if (r == JOptionPane.YES_OPTION) {
            try {
                Desktop.getDesktop().browse(new URL(downloadUrl).toURI());
            } catch (IOException e) {
                logException(e, true);
            } catch (URISyntaxException e) {
                logException(e, true);
            }
        }
    }
    
    public static String reportFileName = "Editor.errorlog";

    public static synchronized void logException(final Throwable e, boolean showToUser) {
        e.printStackTrace(System.err);
        if (!showToUser)
            return;
        // From the GUI thread
        if (useGuiLogWindowForExceptions) {
            if (SwingUtilities.isEventDispatchThread())
                LogWindow.writeLog(e);
            else {
                // Relaunch the exception in the GUI thread
                SwingUtilities.invokeLater(new Runnable() {
                    @Override
                    public void run() {
                        LogWindow.writeLog(e);
                    }
                });
            }
        }
//        try {
//            e.printStackTrace(System.err);
//            PrintWriter out = new PrintWriter(new BufferedWriter(new FileWriter(reportFileName, true)));
//            out.print("-------------------------------------------\n");
//            e.printStackTrace(out);
//            out.close();
//        }
//        catch (IOException e2) { 
//            e2.printStackTrace(System.err);
//        }
    }
    
    public static void requireJavaVersion(int reqMajor) {
        String currVer = System.getProperty("java.version");
        try {
            String[] verNum = currVer.split("\\.");
            int major = Integer.parseInt(verNum[0]);
//            System.out.println("currVer="+currVer+" major="+major+" reqMajor="+reqMajor);
            if (reqMajor <= major)
                return;
        }
        catch (Exception e) { }
        
        String err = "Java version "+reqMajor+"+ is required, currently running on version "+currVer+".\n"
                + "If you have already installed Java version "+reqMajor+" or greater, please verify\n"
                + "that it is correctly set as the default Java version.";
        System.err.println(err);
        JOptionPane.showMessageDialog(null, err, "Java Version Error", JOptionPane.ERROR_MESSAGE);
    }
    
    // Open a PDF file with an external viewer
    public static void viewPDF(File pdfFile) {
        if (Desktop.isDesktopSupported()) {
            try {
                // Fix GNOME 40 bug
                if (Util.isLinux()) {
                    Runtime.getRuntime().exec(new String[]{"xdg-open", pdfFile.getAbsolutePath()});
                }
                else {
                    Desktop.getDesktop().open(pdfFile);
                }
                return;
            } catch (IOException ex) {
                // no application registered for PDFs
            }
        }
        JOptionPane.showMessageDialog(null, "Could not find an application to open PDF file:\n"+
                pdfFile.getAbsolutePath(), 
                "Could not open PDF", JOptionPane.ERROR_MESSAGE);
    }
    
    //-------------------------------------------------------------------------
    
    private static File appImageGreatSPN_dir = null;
    private static boolean testedAppImageDistrib = false;
    
    // is an app-image with the portable_greatspn binaries?
    public static boolean isAppImageDistribution() {
        if (!testedAppImageDistrib) {
            testedAppImageDistrib = true;
            try {
                String path = Main.class.getProtectionDomain().getCodeSource().getLocation().getPath();
                String decodedPath = URLDecoder.decode(path, "UTF-8");
//                System.out.println("decodedPath="+decodedPath);
                File file = new File(decodedPath);
                if (file.isFile() && file.exists() && decodedPath.endsWith(".jar")) {
                    // Running from JAR
                    File portableDir = new File(file.getParent() + File.separator + "portable_greatspn");
//                    System.out.println(portableDir);
                    if (portableDir.isDirectory()) {
                        appImageGreatSPN_dir = portableDir;
                        System.out.println("PORTABLE "+portableDir);
                    }
                }
                else if (file.isDirectory() && file.exists() && 
                        decodedPath.endsWith("JavaGUI/Editor/build/classes/")) 
                {
                    // Running from class sources
                }
                else {
                    // Unknown
                }
            }
            catch (UnsupportedEncodingException e) { }
        }
        return appImageGreatSPN_dir != null;
    }
    
    public static File getAppImageGreatSPN_dir() {
        if (isAppImageDistribution())
            return appImageGreatSPN_dir;
        return null;
    }
    
    //==========================================================================
    // GreatSPN tool installation directory finder
    //==========================================================================

    private static final String GREATSPN_DIR = "greatspn_dir";
    public static void setPathToGreatSPN(String path) {
        Util.getPreferences().put(GREATSPN_DIR, path);
    }
    public static String getPathToGreatSPN() {
        String path = Util.getPreferences().get(GREATSPN_DIR, "/usr/local/GreatSPN");
        return path;
    }
    
    private static final String USE_APPIMAGE_GREATSPN_DISTRIB_KEY = "use_appimage_greatspn_distrib";
    public static boolean getUseAppImageGreatSPN_Distrib() {
        return Util.getPreferences().getBoolean(USE_APPIMAGE_GREATSPN_DISTRIB_KEY, true);
    }
    public static void setUseAppImageGreatSPN_Distrib(boolean use) {
        Util.getPreferences().putBoolean(USE_APPIMAGE_GREATSPN_DISTRIB_KEY, use);
    }
    
    // Should call the portable GreatSPN distribution inside the AppImage 
    public static boolean useAppImage() {
        return getUseAppImageGreatSPN_Distrib() && Main.isAppImageDistribution();
    }
    
    //==========================================================================
    // Support for Windows Subsystem for Linux
    //==========================================================================
    
    public static boolean useWSL() {
        return Util.isWindows() && !useAppImage();
    }

    // Verify that we can actually call bash from the command line
    public static boolean checkWSL() {
        try {
            Process p = Runtime.getRuntime().exec(new String[]{
                //"bash" ,"-c" ,"exit 25"
                "wsl", "exit", "25"
            });
            int exitcode = p.waitFor();
//            JOptionPane.showMessageDialog(null, "exit = "+exitcode);
            return exitcode == 25;
        }
        catch (IOException | InterruptedException e) {
//            JOptionPane.showMessageDialog(null, "exception "+e);
            return false;
        }
    }
    
    // Verify the existance of a file in the WSL subsystem
    public static boolean checkWSLcanExecute(String file) {
        try {
            Process p = Runtime.getRuntime().exec(new String[]{
//                "bash", "-c", "test -x \""+file+"\""
                "wsl", "test", "-x", file
            });
            int exitcode = p.waitFor();
            return exitcode == 0; // 0 means it has the x flag, otherwise test returns 1
        }
        catch (IOException | InterruptedException e) {
            return false;
        }
    }

    //-------------------------------------------------------------------------
    // Application preferences
    //-------------------------------------------------------------------------
    
    private static final String AUTOMATIC_CHECK_KEY = "automatic_check_update";
    public static void setAutomaticCheckForUpdates(boolean ok) {
        Util.getPreferences().putBoolean(AUTOMATIC_CHECK_KEY, ok);
    }
    public static boolean isCheckForUpdatesAutomatic() {
        return Util.getPreferences().getBoolean(AUTOMATIC_CHECK_KEY, false);
    }
    
    private static final String REOPEN_FILES_AT_STARTUP = "repen_files_at_startup";
    public static void setReopenPrevOpenFilesAtStartup(boolean ok) {
        Util.getPreferences().putBoolean(REOPEN_FILES_AT_STARTUP, ok);
    }
    public static boolean isReopenPrevOpenFilesAtStartup() {
        return Util.getPreferences().getBoolean(REOPEN_FILES_AT_STARTUP, false);
    }

    private static final String MAX_UNDO_KEY = "max_undo2";
    public static void setMaxUndo(int max) {
        Util.getPreferences().putInt(MAX_UNDO_KEY, max);
    }
    public static int getMaxUndo() {
        return Util.getPreferences().getInt(MAX_UNDO_KEY, 75);
    }
    
    private static final String MAX_LATEX_CACHE_KEY = "max_latex_cache";
    public static void setMaxLatexCacheEntries(int max) {
        Util.getPreferences().putInt(MAX_LATEX_CACHE_KEY, max);
    }
    public static int getMaxLatexCacheEntries() {
        return Util.getPreferences().getInt(MAX_LATEX_CACHE_KEY, 200);
    }
    
    private static final String UI_SIZE_CACHE_KEY = "ui_size";
    public static UiSize fixedUiSize = null;
    public static void setStartupUiSize(UiSize sz) {
        Util.getPreferences().put(UI_SIZE_CACHE_KEY, sz.name());
    }
    public static UiSize getStartupUiSize() {
        if (fixedUiSize != null)
            return fixedUiSize;
        try {
            return UiSize.valueOf(Util.getPreferences().get(UI_SIZE_CACHE_KEY, UiSize.NORMAL.name()));
        }
        catch (Throwable e) { 
            // Note: JavaBeans are not capable of getting the preference size,
            // so this escape is necessary.
            return UiSize.NORMAL;
        }
    }
    
    private static final String ALLOW_GSPN_EXT_KEY = "allow_greatspn_ext";
    public static void setGreatSPNExtAllowed(boolean ok) {
        Util.getPreferences().putBoolean(ALLOW_GSPN_EXT_KEY, ok);
    }
    public static boolean isGreatSPNExtAllowed() {
        return Util.getPreferences().getBoolean(ALLOW_GSPN_EXT_KEY, false);
    }
    
    private static final String ALLOW_GSPN_MDEP_ARCS_KEY = "allow_greatspn_mdep_arcs";
    public static void setGreatSPNMdepArcsAllowed(boolean ok) {
        Util.getPreferences().putBoolean(ALLOW_GSPN_MDEP_ARCS_KEY, ok);
    }
    public static boolean areGreatSPNMdepArcsAllowed() {
        return Util.getPreferences().getBoolean(ALLOW_GSPN_MDEP_ARCS_KEY, false);
    }

//    private static final String MODEL_LIBRARY_DIR = "model_library_dir";
//    public static String getModelLibraryDirectory() {
//        return Util.getPreferences().get(MODEL_LIBRARY_DIR, "/usr/local/GreatSPN/models");
//    }
//    public static void setModelLibraryDirectory(String dir) {
//        Util.getPreferences().put(MODEL_LIBRARY_DIR, dir);
//    }

    //-------------------------------------------------------------------------
    // Developer switch
    //-------------------------------------------------------------------------
    
    private static final String DEVELOPER_SWITCH = "developer_switch";
    public static void setDeveloperMachine(boolean ok) {
        Util.getPreferences().putBoolean(DEVELOPER_SWITCH, ok);
    }
    public static boolean isDeveloperMachine() {
//        setDeveloperMachine(true);
        return Util.getPreferences().getBoolean(DEVELOPER_SWITCH, false);
    }
}
