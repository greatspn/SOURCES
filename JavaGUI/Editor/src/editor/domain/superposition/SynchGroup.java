/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.superposition;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

/** A synchronization group, with the vector of synchronized elements, the
 *  set of synchronization tags, and the node class.
 *
 * @author elvio
 */
public class SynchGroup implements NodeGroup {
    // Vector of nodes that are synchronized in the MultiNetPage.
    private final NodeGroup[] synchNodes;
    // Indexes of the nets where the synchNodes belong
    private final int[] netIndexes;
    // Set of exposed tags (union of tags of the synchronized nodes)
    private final String[] tags;
    // The node class
    private final GroupClass groupClass;
    
    public SynchGroup(NodeGroup[] synchNodes, int[] netIndexes, GroupClass groupClass) {
        assert synchNodes.length > 1; // Avoid trivial synchronization groups
        this.synchNodes = synchNodes;
        this.netIndexes = netIndexes;
        this.groupClass = groupClass;
        this.tags = buildTags(synchNodes);
    }
    
    @Override
    public int numSynchNodes() { return synchNodes.length; }
    @Override
    public NodeGroup getSynchNode(int i) { return synchNodes[i]; }
    @Override
    public int getSynchNodeNetIndex(int i) { return netIndexes[i]; }
    

    @Override
    public int numTags() { return tags.length; }
    @Override
    public String getTag(int i) { return tags[i]; }
    @Override
    public boolean hasTag(String tag) {
        for (String t: tags)
            if (tag.equals(t))
                return true;
        return false;
    }

    @Override
    public GroupClass getGroupClass() {
        return groupClass;
    }
    
    private static String[] buildTags(NodeGroup[] groups) {
        Set<String> tags = new HashSet<>();
        for (NodeGroup grp : groups)
            for (int t=0; t<grp.numTags(); t++)
                tags.add(grp.getTag(t));
        return (String[])tags.toArray();
    }

    // Equality and hash are written in order to test, in an HashSet, if the
    // SynchGroup with the same values has already been added.
    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof SynchGroup))
            return false;
        SynchGroup sg = (SynchGroup)obj;
        return Arrays.equals(synchNodes, sg.synchNodes) &&
               Arrays.equals(netIndexes, netIndexes);
    }

    @Override
    public int hashCode() {
        return Arrays.hashCode(synchNodes) ^ Arrays.hashCode(netIndexes);
    }
}
