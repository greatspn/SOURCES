///*
// * To change this license header, choose License Headers in Project Properties.
// * To change this template file, choose Tools | Templates
// * and open the template in the editor.
// */
//
//package editor.domain.superposition;
//
///** A group of nodes that share a set of tags and are mergeable between them.
// *
// * @author elvio
// */
//public class MultiNodeGroup implements NodeGroup {
//    // Nodes or subgroups in this group
//    private final NodeGroup[] subGroups;
//    // Set of common tags shared by all the nodes in the group.
//    private final String[] tags;
//    // The class of this group
//    private final GroupClass groupClass;
//    
//    // Temporary flag for the composition algorithm
//    private boolean composedFlag = false;
//
//    public MultiNodeGroup(NodeGroup[] subGroups) {
//        this.subGroups = subGroups;
//        this.tags = buildCommonTags(subGroups);
//        this.groupClass = findGroupClass();
//    }
//
//    public MultiNodeGroup(NodeGroup g1, NodeGroup g2) {
//        assert haveCommonTags(g1, g2) && g1.getGroupClass() == g2.getGroupClass();
//        this.subGroups = new NodeGroup[g1.numNodes() + g2.numNodes()];
//        for (int i=0; i<g1.numNodes(); i++)
//            this.subGroups[i] = g1.getGroupNode(i);
//        for (int i=0; i<g2.numNodes(); i++)
//            this.subGroups[i + g1.numNodes()] = g2.getGroupNode(i);
//        this.tags = buildCommonTags(subGroups);
//        this.groupClass = findGroupClass();
//    }
//    
//    @Override
//    public int numNodes() { return subGroups.length; }
//    @Override
//    public NodeGroup getGroupNode(int i) { return subGroups[i]; }
//
//    @Override
//    public int numTags() { return tags.length; }
//    @Override
//    public String getCommonTag(int i) { return tags[i]; }
//    
//    @Override
//    public void setComposed(boolean c) { composedFlag = c; }
//    @Override 
//    public boolean isComposed() { return composedFlag = false; }
//    
//
//    @Override
//    public boolean hasCommonTag(String tag) {
//        for (String t: tags)
//            if (tag.equals(t))
//                return true;
//        return false;
//    }
//
//    private static boolean haveCommonTags(NodeGroup g1, NodeGroup g2) {
//        for (int t1=0; t1<g1.numTags(); t1++)
//            for (int t2=0; t2<g2.numTags(); t2++)
//                if (g1.getCommonTag(t1).equals(g2.getCommonTag(t2)))
//                    return true;
//        return false;
//    }
//
//    private static String[] buildCommonTags(NodeGroup[] subGroups) {
//        // Array of candidate common tags
//        String[] ctags = new String[subGroups[0].numTags()];
//        for (int t=0; t<ctags.length; t++)
//            ctags[t] = subGroups[0].getCommonTag(t);
//        
//        for (int i=1; i<subGroups.length; i++) {
//            for (int j=0; j<ctags.length; j++) {
//                if (ctags[j] != null) {
//                    boolean found = false;
//                    for (int t=0; t<subGroups[i].numTags(); t++) {
//                        if (ctags[j].equals(subGroups[i].getCommonTag(t))) {
//                            found = true;
//                            break; // found
//                        }
//                    }
//                    if (!found)
//                        ctags[j] = null;
//                }
//            }
//        }
//        int notNulls = 0;
//        for (String tag : ctags)
//            if (tag != null)
//                notNulls++;
//        assert notNulls > 0;
//        String[] tags = new String[notNulls];
//        notNulls = 0;
//        for (String tag : ctags)
//            if (tag != null)
//                tags[notNulls++] = tag;
//        return tags;
//    }
//
//    @Override
//    public GroupClass getGroupClass() {
//        return groupClass;
//    }
//    
//    private GroupClass findGroupClass() {
//        GroupClass sc = subGroups[0].getGroupClass();
//        for (int i=1; i<subGroups.length; i++) {
//            if (subGroups[i].getGroupClass() != sc)
//                throw new IllegalStateException("Group contains nodes of mltiple classes.");
//        }
//        return sc;
//    }
//}
