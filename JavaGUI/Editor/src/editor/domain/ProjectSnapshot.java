/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain;

import java.util.Map;
import java.util.UUID;

/** A serialized project in the undo/redo chain.
 *  NOTE: the content of ProjectData must be kept in sync 
 *  with the snapshot class ProjectSnapshot
 * @author elvio
 */
public class ProjectSnapshot {
    // Serialization of a ProjectData class
    // Name of the project
    private final String projName;
    // Index of the active page
    private final int activePageNum;
    // Serialized and compressed pages
    private final SerializedPage[] serializedPages;
    // Which pages has been changed (this is useful for automatic dependancy management)
    private final boolean[] changedPages;
    // A copy of the resource table
    private final Map<UUID, ProjectResource> resourceTable;
    
    
    // Total number of bytes allocated for this snapshot
    private final int totalBytes;
    // Bytes used by this snapshot that are shared with another snapshot
    private final int sharedBytes;
    // Name of the action that has generated this snapshot
    // This is the only field that can be changed (by the undo manager)
    private String actionName;
    // File version that this snapshot refers to
    private final UniqueVersionID dataVersionID;
//    // Unique ID of this snapshot (generated from incrID)
//    private final int uniqueSnapshotID;
//
//    // generator of the unique version IDs of the snapshots.
//    private static int incrID = 100;
    
    public static class SerializedPage {
        byte[] data;
        byte[] md5hash;
        UniqueVersionID pageVersionId;

        public SerializedPage(byte[] data, byte[] md5hash, UniqueVersionID versionId) {
            this.data = data;
            this.md5hash = md5hash;
            this.pageVersionId = versionId;
        }
    }

    public ProjectSnapshot(String projName, int activePageNum, SerializedPage[] serializedPages, 
                           boolean[] changedPages, Map<UUID, ProjectResource> resourceTable, 
                           UniqueVersionID dataVersionID, String actionName, int totalBytes, int sharedBytes) 
    {
        this.projName = projName;
        this.activePageNum = activePageNum;
        this.serializedPages = serializedPages;
        this.changedPages = changedPages;
        this.resourceTable = resourceTable;
        this.actionName = actionName;
        this.totalBytes = totalBytes;
        this.sharedBytes = sharedBytes;
        this.dataVersionID = dataVersionID;
    }

    public String getProjName() {
        return projName;
    }

    public int getActivePageNum() {
        return activePageNum;
    }

    public SerializedPage[] getSerializedPages() {
        return serializedPages;
    }
    
    public boolean isPageChanged(int pageNum) {
        return changedPages[pageNum];
    }

    public int getTotalBytes() {
        return totalBytes;
    }

    public int getSharedBytes() {
        return sharedBytes;
    }

    public String getActionName() {
        return actionName;
    }

    public void setActionName(String actionName) {
        this.actionName = actionName;
    }

    public UniqueVersionID getDataVersionID() {
        return dataVersionID;
    }    

    public Map<UUID, ProjectResource> getResourceTable() {
        return resourceTable;
    }
}
