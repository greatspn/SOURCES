/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui;

import common.Util;
import java.io.File;
import java.util.ArrayList;
import java.util.prefs.Preferences;

/** Manages the list of recently opened files.
 *
 * @author elvio
 */
public class RecentFilesListLRU {
    
    int maxFiles;
    ArrayList<File> filenames = new ArrayList<>();
    
    public void loadList() {
        Preferences prefs = Util.getPreferences();
        maxFiles = prefs.getInt("max-recent-files", 10);
        for (int i=0; i<maxFiles; i++) {
            String fn = prefs.get("recent-file-"+i, null);
            if (fn == null)
                break;
            filenames.add(new File(fn));
        }
    }
    
    public void saveList() {
        Preferences prefs = Util.getPreferences();
        for (int i=0; i<filenames.size(); i++)
            prefs.put("recent-file-"+i, filenames.get(i).getAbsolutePath());
        for (int i=filenames.size(); i<=maxFiles; i++)
            prefs.remove("recent-file-"+i);
    }
    
    public int size() {
        return filenames.size();
    }
    
    public File get(int i) {
        return filenames.get(i);
    }
    
    public void open(File f) {
        // Check if the file is already in the LRU list
        for (int i=0; i<filenames.size(); i++) {
            if (filenames.get(i).getAbsolutePath().equals(f.getAbsolutePath())) {
                // Move on top
                filenames.remove(i);
                filenames.add(0, f);
                return;
            }
        }
        // Add on top
        filenames.add(0, f);
        while (filenames.size() > maxFiles)
            filenames.remove(filenames.size() - 1);
    }
    
    public void save(File f) {
        // Same logic as open(f)
        open(f);
    }
    
    public void remove(File f) {
        for (int i=0; i<filenames.size(); i++) {
            if (filenames.get(i).getAbsolutePath().equals(f.getAbsolutePath())) {
                filenames.remove(i);
            }
        }
    }
}
