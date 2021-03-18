/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import editor.Main;
import editor.domain.ProjectSnapshot.SerializedPage;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.UUID;
import java.util.zip.DataFormatException;
import java.util.zip.Deflater;
import java.util.zip.Inflater;

/** The current data of a project. 
 * The current data consists in the project name, the set of project pages, 
 * the currently selected page (if any), and the linked resources.
 * 
 * @author elvio
 */
public class ProjectData {
    // Name of this project
    public String projName;
    
    // Active page in the tree visualization, -1 means no selected page
    private int activePageNum = -1;
    
    // Pages of this project
    private final ArrayList<ProjectPage> pages;
    
    // The table of heavy, unmodifiable objects (resources)
    private final Map<UUID, ProjectResource> resourceTable = new HashMap<>();
    
    // The unique version ID
    private UniqueVersionID versionId;
    
    // Create a new project from a serialized project snapshot
    public ProjectData(ProjectSnapshot ps) {
        projName = ps.getProjName();
        activePageNum = ps.getActivePageNum();
        pages = new ArrayList<>(ps.getSerializedPages().length);
        for (SerializedPage serializedPage : ps.getSerializedPages()) {
            ProjectPage page = deserialize(decompress(serializedPage.data));
            page.setVersionId(serializedPage.pageVersionId);
            pages.add(page);
        }
        Map<UUID, ProjectResource> resTab = ps.getResourceTable();
        Iterator<Entry<UUID, ProjectResource>> resIt = resTab.entrySet().iterator();
        while (resIt.hasNext()) {
            Entry<UUID,ProjectResource> entry = resIt.next();
            resourceTable.put(entry.getKey(), entry.getValue());
        }
        for (ProjectPage page : pages) {
            page.relinkTransientResources(resourceTable);
        }
        this.versionId = ps.getDataVersionID();
    }
    
    static int seq = 0;
    
    public void changeVersionId() {
        versionId = new UniqueVersionID();
    }
    
    // Create a project snapshot from this project
    public ProjectSnapshot createSnapshot(ProjectSnapshot anotherSnapshot) {
        SerializedPage[] serializedPages = new SerializedPage[pages.size()];
        boolean[] changedPages = new boolean[pages.size()];
        int totBytes = 0, sharedBytes = 0;
        // Compress the pages
        for (int p=0; p<pages.size(); p++) {
            byte[] pageData = serialize(pages.get(p));
            byte[] md5Hash = makeMd5Hash(pageData);
            SerializedPage serPage = null;
//            SerializedPage data = new SerializedPage(compress(serPg), md5Hash);
            if (anotherSnapshot != null) {
                for (SerializedPage alreadySerializedPage : anotherSnapshot.getSerializedPages()) {
                    if (Arrays.equals(alreadySerializedPage.md5hash, md5Hash)) 
                    {
                        // We can share the same byte array !
                        serPage = alreadySerializedPage;
//                        totBytes -= data.length;
                        sharedBytes += serPage.data.length;
                        changedPages[p] = false;
                        break;
                    }
                }
            }
            if (serPage == null) {
                serPage = new SerializedPage(compress(pageData), md5Hash, new UniqueVersionID());
                totBytes += serPage.data.length;
//                System.out.println("Page "+pages.get(p).getPageName()+" has changed. "+anotherSnapshot);
                changedPages[p] = true;
                
//                try {
//                    if (pages.get(p).getPageName().equals("ReaderWriter")) {
//                        FileOutputStream fos1 = new FileOutputStream("/Users/elvio/Desktop/readerwriter"+seq+".txt");
//                        seq++;
//                        XStream xstream = new XStream(new StaxDriver());
//                        String xml = xstream.toXML(pages.get(p));
//                        fos1.write(xml.getBytes());
////                        java.beans.XMLEncoder xe1 = new java.beans.XMLEncoder(fos1);
////                        xe1.writeObject(pages.get(p));
////                        xe1.flush();
////                        xe1.close();
//                        fos1.close();
//                        System.out.println("wrote readerwriter"+seq+".txt");
//                    }
//                }
//                catch (IOException e) { e.printStackTrace(); }
            }
            serializedPages[p] = serPage;
        }
        
//        System.out.println("Snapshot: "+versionId);
//        for (int p=0; p<pages.size(); p++) {
//            System.out.println("  "+pages.get(p).getPageName()+"   "+serializedPages[p].pageVersionId);
//        }     
//        System.out.println("");
        
        // Garbage collect the unused resources
        garbageCollectResources();
        
        // Copy the resource table
        Map<UUID, ProjectResource> resTabCopy = new HashMap<>();
        Iterator<Entry<UUID,ProjectResource>> resIt = resourceTable.entrySet().iterator();
        while (resIt.hasNext()) {
            Entry<UUID,ProjectResource> entry = resIt.next();
            resTabCopy.put(entry.getKey(), entry.getValue());
        }
        
//        System.out.println("new snapshot of "+totBytes+" bytes plus "+sharedBytes+" shared bytes.");
        return new ProjectSnapshot(projName, activePageNum, serializedPages, changedPages, 
                                   resTabCopy, versionId, null, totBytes, sharedBytes);
    }

    // Create a new empty project.
    public ProjectData(String projName, ArrayList<ProjectPage> pages) {
        this.projName = projName;
        this.pages = pages;
        this.versionId = new UniqueVersionID();
    }
    
    // Enumerate project pages
    public final int getPageCount() { return pages.size(); }   
    public final ProjectPage getPageAt(int pos) {
        return pages.get(pos);
    }
    public int findPagePosition(ProjectPage page) {
        for (int i=0; i<pages.size(); i++)
            if (page == pages.get(i))
                return i;
        return -1;
    }
    public ProjectPage findPageByName(String name) {
        for (ProjectPage page : pages)
            if (page.getPageName().equals(name))
                return page;
        return null;
    }

    // Active project page
    public ProjectPage getActivePage() { 
        if (activePageNum == -1)
            return null;
        return pages.get(activePageNum);
    }
    public void setActivePage(ProjectPage activePage) {
        activePageNum = -1;
        // Verify that activePage is either null or a real project page
        if (activePage != null) {
            for (int i=0; i< getPageCount(); i++) {
                if (getPageAt(i) == activePage) {
                    activePageNum = i;
                    return;
                }
            }
            throw new RuntimeException("Internal error: the activePage is not "
                                       + "a project page.");
        }
//        else activePageNum = -1;
    }
    
    public void movePage(int actualpos, int newpos) {
        assert actualpos >= 0 && actualpos < pages.size();
        ProjectPage e = pages.get(actualpos);
        pages.remove(actualpos);
        pages.add(newpos, e);
        if (activePageNum == actualpos)
            activePageNum = newpos;
    }
    
    public void deletePage(int pos) {
        assert pos >=0 && pos < pages.size();
        pages.remove(pos);
        if (activePageNum == pos)
            activePageNum--;
    }
    
    public void addPage(ProjectPage newPage) {
        addPageAt(newPage, pages.size());
    }
    
    public void addPageAt(ProjectPage newPage, int index) {
        newPage.preparePageCheck();
        newPage.checkPage(this, null, newPage, null);
        newPage.retrieveLinkedResources(resourceTable);
        newPage.relinkTransientResources(resourceTable);
        pages.add(index, newPage);
    }
    
    public boolean isProjectCorrect() { 
        boolean correct = true; 
        for (ProjectPage page : pages) {
            correct = correct && page.isPageCorrect();
        }
        return correct;
    }
    
    public void checkProjectCorrectness(Set<ProjectPage> changedPages) {
        for (ProjectPage page : pages)
            page.preparePageCheck();
        
        for (ProjectPage page : pages) 
            page.checkPage(this, changedPages, page, null);
        
        for (ProjectPage page : pages)
            page.retrieveLinkedResources(resourceTable);
    }
    
    public String generateUniquePageName(String prefix) {
        for (int i=0; ; i++) {
            String candidate = prefix + (i==0 ? "" : " " + i);
            boolean found = false;
            for (int j=0; j<getPageCount(); j++)
                if (getPageAt(j).getPageName().equals(candidate)) {
                    found = true;
                    break;
                }
            if (!found)
                return candidate; // Unique name
        }        
    }
    
    //-------------------------------------------------------------------------

    public ProjectResource findResourceByID(UUID id) {
        return resourceTable.get(id);
    }
    
    public ProjectResource findDuplicateResource(ProjectResource res) {
        for (ProjectResource r : resourceTable.values())
            if (r.equals(res))
                return r;
        return null;
    }
    
    public void addResource(ProjectResource res) {
        assert !resourceTable.containsKey(res.getResourceID());
        resourceTable.put(res.getResourceID(), res);
    }
    
    // Remove all the project resources that are not marked as 'in use'
    public void garbageCollectResources() {
        Map<UUID, ProjectResource> usedResources = new HashMap<>();
        for (ProjectPage page : pages)
            page.retrieveLinkedResources(usedResources);
                
        Iterator<UUID> resIt = resourceTable.keySet().iterator();
        while (resIt.hasNext()) {
            UUID resID = resIt.next();
            if (!usedResources.containsKey(resID))
                resIt.remove();
        }
        
        // Add missing resources
        resIt = usedResources.keySet().iterator();
        while (resIt.hasNext()) {
            UUID resID = resIt.next();
            if (!resourceTable.containsKey(resID))
                resourceTable.put(resID, usedResources.get(resID));
        }
        
        // Avoid duplication of the same resources
        relinkPageResources();
    }
    
    public void relinkPageResources() {
        for (ProjectPage page : pages) {
            page.relinkTransientResources(resourceTable);
        }
    }
    
    public Iterator<Entry<UUID, ProjectResource>> getResourceIterator() {
        return resourceTable.entrySet().iterator();
    }

    //-------------------------------------------------------------------------
    private static byte[] serialize(ProjectPage page) {
        try {
            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            ObjectOutputStream oos = new ObjectOutputStream(bos);
            oos.writeObject(page);
            oos.flush();
            oos.close();
            bos.close();
            return bos.toByteArray();
        }
        catch (IOException ex) {
            Main.logException(ex, true);
        }        
        return null;
    }
    
    private static ProjectPage deserialize(byte[] byteData) {
        ProjectPage page = null;
        try {
            ByteArrayInputStream bais = new ByteArrayInputStream(byteData);
            page = (ProjectPage)new ObjectInputStream(bais).readObject();
        }
        catch (IOException | ClassNotFoundException ex) {
            Main.logException(ex, true);
        }
        return page;
    }
    
    private static boolean COMPRESS_DATA = true;
    private static byte[] makeMd5Hash(byte[] data) {
        if (!COMPRESS_DATA)
            return data; // no need to make an hash
        try {
            java.security.MessageDigest md = java.security.MessageDigest.getInstance("MD5");
            return md.digest(data);
        }
        catch (NoSuchAlgorithmException nsae) {
            Main.logException(nsae, true);
        }
        return null;
    }
    
    private static byte[] buffer = new byte[4096];
    private static byte[] compress(byte[] input) {
        if (!COMPRESS_DATA)
            return input;
        byte[] output = null;
        try {
            Deflater deflater = new Deflater();  
            deflater.setInput(input);

            ByteArrayOutputStream outputStream = new ByteArrayOutputStream(input.length);

            deflater.finish();
            while (!deflater.finished()) {
                int count = deflater.deflate(buffer);  
                outputStream.write(buffer, 0, count);
            }
            outputStream.close();
            output = outputStream.toByteArray();
//            System.out.println("Original: "+byteData.length+" compressed: "+output.length);
        }
        catch (IOException ex) {
            Main.logException(ex, true);
//            System.exit(-1);
        }        
        return output;
    }
    
    private static byte[] decompress(byte[] input) {
        if (!COMPRESS_DATA)
            return input;
        try {
            Inflater inflater = new Inflater();
            inflater.setInput(input);

            ByteArrayOutputStream outputStream = new ByteArrayOutputStream(input.length);
            while (!inflater.finished()) {
                int count = inflater.inflate(buffer);
                outputStream.write(buffer, 0, count);
            }
            outputStream.close();
            return outputStream.toByteArray();
        }
        catch (IOException | DataFormatException ex) {
            Main.logException(ex, true);
        }
        return null;
    }
}
