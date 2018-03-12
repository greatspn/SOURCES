/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain;

import java.io.Serializable;
import java.util.Map;
import java.util.UUID;

/**
 *
 * @author elvio
 * @param <RES> the type of resource pointed
 */
public class ProjectResourceRef<RES extends ProjectResource> implements Serializable, ResourceHolder {
    transient RES  resRef;   // The resource reference
    UUID resID;              // The unique ID of the resource
    
    public ProjectResourceRef() {
        resID = null;
        resRef = null;
    }
    
    public boolean isNull() {
        return resID == null;
    }
    
    @Override
    public void retrieveLinkedResources(Map<UUID, ProjectResource> resourceTable) {
        if (resID != null) {
            if (!resourceTable.containsKey(resID)) {
                assert resRef != null;
                resourceTable.put(resID, resRef);
            }
        }
    }

    @Override
    public void relinkTransientResources(Map<UUID, ProjectResource> resourceTable) {
        if (resID != null) {
            @SuppressWarnings("unchecked")
            RES r = (RES)resourceTable.get(resID);
            assert r != null;
            resRef = (RES)r;
            assert resRef != null;
        }
    }
    
    public RES resRef() {
        assert resID != null && resRef != null;
        return resRef;
    }
    
    public void setRef(RES res) {
        resRef = res;
        resID = (resRef == null) ? null : resRef.getResourceID();
    }
    
    public void setID(UUID id) {
        resRef = null;
        resID = id;
    }

    public UUID getID() {
        return resID;
    }
    
}
