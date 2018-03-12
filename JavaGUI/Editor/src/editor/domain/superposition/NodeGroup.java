/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.superposition;

/** Base interface of node groups.
 *  Note that simple nodes are by themself groups with a single node.
 *
 * @author elvio
 */
public interface NodeGroup {
    
    public int numSynchNodes();
    public NodeGroup getSynchNode(int i);
    public int getSynchNodeNetIndex(int i);
    
    public int numTags();
    public String getTag(int i);
    public boolean hasTag(String tag);
    
    // The grouping class. Only object belonging 
    // to the same group class can be grouped together.
    public GroupClass getGroupClass();
}
