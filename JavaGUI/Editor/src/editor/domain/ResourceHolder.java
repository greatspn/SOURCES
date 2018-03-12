/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain;

import java.util.Map;
import java.util.UUID;

/** An object that contains ProjectResourceRef(s) or other sub-objects with such refs.
 *
 * @author elvio
 */
public interface ResourceHolder {
    
    // write in @resourceTable the resources used by this object and all its subobjects
    public void retrieveLinkedResources(Map<UUID, ProjectResource> resourceTable);
    
    // relink the transient resource pointers using the reference in @resourceTable
    public void relinkTransientResources(Map<UUID, ProjectResource> resourceTable);
}
