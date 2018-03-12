/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.gui;

import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.UnsupportedFlavorException;
import java.util.ArrayList;
import java.util.List;
import javax.swing.JComponent;
import javax.swing.JTree;
import javax.swing.TransferHandler;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreeNode;
import javax.swing.tree.TreePath;

/**
 *
 * @author elvio
 */
public class ProjPageTreeTransferHandler extends TransferHandler {
    
    private DefaultMutableTreeNode[] nodesToRemove;
    
    private final AppWindow appWnd;

    public ProjPageTreeTransferHandler(AppWindow appWnd) {
        this.appWnd = appWnd;
    }
    
    @Override
    public boolean canImport(TransferHandler.TransferSupport support) {
        if(!support.isDrop()) {
            return false;
        }
        support.setShowDropLocation(true);
        if(!support.isDataFlavorSupported(pageDataFlavour)) {
            return false;
        }
//        // Do not allow a drop on the drag source selections.
//        JTree.DropLocation dl =
//                (JTree.DropLocation)support.getDropLocation();
//        JTree tree = (JTree)support.getComponent();
//        int dropRow = tree.getRowForPath(dl.getPath());
//        int[] selRows = tree.getSelectionRows();
//        for(int i = 0; i < selRows.length; i++) {
//            if(selRows[i] == dropRow) {
//                return false;
//            }
//        }
        // Do not allow MOVE-action drops if a non-leaf node is
        // selected unless all of its children are also selected.
//        int action = support.getDropAction();
//        if(action == MOVE) {
//            return haveCompleteNode(tree);
//        }
        // Do not allow a non-leaf node to be copied to a level
        // which is less than its source level.
//        TreePath dest = dl.getPath();
//        DefaultMutableTreeNode target =
//            (DefaultMutableTreeNode)dest.getLastPathComponent();
//        TreePath path = tree.getPathForRow(selRows[0]);
//        DefaultMutableTreeNode firstNode =
//            (DefaultMutableTreeNode)path.getLastPathComponent();
//        if(firstNode.getChildCount() > 0 &&
//               target.getLevel() < firstNode.getLevel()) {
//            return false;
//        }
        return true;
    }
    
//    private boolean haveCompleteNode(JTree tree) {
//        int[] selRows = tree.getSelectionRows();
//        TreePath path = tree.getPathForRow(selRows[0]);
//        DefaultMutableTreeNode first =
//            (DefaultMutableTreeNode)path.getLastPathComponent();
//        int childCount = first.getChildCount();
//        // first has children and no children are selected.
//        if(childCount > 0 && selRows.length == 1)
//            return false;
//        // first may have children.
//        for(int i = 1; i < selRows.length; i++) {
//            path = tree.getPathForRow(selRows[i]);
//            DefaultMutableTreeNode next =
//                (DefaultMutableTreeNode)path.getLastPathComponent();
//            if(first.isNodeChild(next)) {
//                // Found a child of first.
//                if(childCount > selRows.length-1) {
//                    // Not all children of first are selected.
//                    return false;
//                }
//            }
//        }
//        return true;
//    }
    
    @Override
    protected Transferable createTransferable(JComponent c) {
        JTree tree = (JTree)c;
        TreePath[] paths = tree.getSelectionPaths();
        if(paths != null) {
            // Make up a node array of copies for transfer and
            // another for/of the nodes that will be removed in
            // exportDone after a successful drop.
            List<DefaultMutableTreeNode> copies =
                new ArrayList<>();
            List<DefaultMutableTreeNode> toRemove =
                new ArrayList<>();
            DefaultMutableTreeNode node =
                (DefaultMutableTreeNode)paths[0].getLastPathComponent();
            DefaultMutableTreeNode copy = copy(node);
            copies.add(copy);
            toRemove.add(node);
            for(int i = 1; i < paths.length; i++) {
                DefaultMutableTreeNode next =
                    (DefaultMutableTreeNode)paths[i].getLastPathComponent();
                // Do not allow higher level nodes to be added to list.
                if(next.getLevel() < node.getLevel()) {
                    break;
                } else if(next.getLevel() > node.getLevel()) {  // child node
                    copy.add(copy(next));
                    // node already contains child
                } else {                                        // sibling
                    copies.add(copy(next));
                    toRemove.add(next);
                }
            }
            DefaultMutableTreeNode[] nodes =
                copies.toArray(new DefaultMutableTreeNode[copies.size()]);
            nodesToRemove =
                toRemove.toArray(new DefaultMutableTreeNode[toRemove.size()]);
            
            appWnd.setStatus("Drag the selected project page. Press ALT to copy the dragged page.", true);
            
            return new NodesTransferable(nodes);
        }
        return null;
    }

    /** Defensive copy used in createTransferable. */
    private DefaultMutableTreeNode copy(TreeNode node) {
        return (DefaultMutableTreeNode)node;
//        return new ProjPageTreeNode((ProjPageTreeNode)node);
    }

    @Override
    protected void exportDone(JComponent source, Transferable data, int action) {
//        System.out.println("exportDone!");
        if((action & MOVE) == MOVE) {
            JTree tree = (JTree)source;
            DefaultTreeModel model = (DefaultTreeModel)tree.getModel();
            for (DefaultMutableTreeNode node : nodesToRemove) {
//                model.removeNodeFromParent(node);
//                appWnd.dropRemoveProjectPage((ProjPageTreeNode)node);
            }
        }
    }
    
    @Override
    public int getSourceActions(JComponent c) {
        return COPY_OR_MOVE;
    }

    @Override
    public boolean importData(TransferHandler.TransferSupport support) {
        if(!canImport(support)) {
            return false;
        }
        // Extract transfer data.
        DefaultMutableTreeNode[] nodes = null;
        try {
            Transferable t = support.getTransferable();
            nodes = (DefaultMutableTreeNode[])t.getTransferData(pageDataFlavour);
        } catch(UnsupportedFlavorException ufe) {
            System.out.println("UnsupportedFlavor: " + ufe.getMessage());
        } catch(java.io.IOException ioe) {
            System.out.println("I/O error: " + ioe.getMessage());
        }
        // Get drop location info.
        JTree.DropLocation dl =
                (JTree.DropLocation)support.getDropLocation();
        int childIndex = dl.getChildIndex();
        TreePath dest = dl.getPath();
        DefaultMutableTreeNode parent =
            (DefaultMutableTreeNode)dest.getLastPathComponent();
        JTree tree = (JTree)support.getComponent();
        DefaultTreeModel model = (DefaultTreeModel)tree.getModel();
        // Configure for drop mode.
        int index = childIndex;    // DropMode.INSERT
        if(childIndex == -1) {     // DropMode.ON
            index = parent.getChildCount();
        }
        for (DefaultMutableTreeNode node : nodes) {
            appWnd.dragDropProjectPage((ProjPageTreeNode)node, (ProjPageTreeNode)parent, 
                                   index, 0 != (support.getDropAction() & COPY));
//            model.insertNodeInto(node, parent, index++);
            index++;
        }
        return true;
    }

    public static final DataFlavor pageDataFlavour;
    public static final DataFlavor[] allPageDataFlavours = {null};
    
    static {
        DataFlavor df = null;
        try {
            String mimeType = DataFlavor.javaJVMLocalObjectMimeType +
                              ";class=\"" +
                                ProjPageTreeNode[].class.getName() +
                              "\"";
            df = new DataFlavor(mimeType);
        } catch(ClassNotFoundException e) {
            System.out.println("ClassNotFound: " + e.getMessage());
        }
        pageDataFlavour = df;
        allPageDataFlavours[0] = pageDataFlavour;
    }
    
    public static class NodesTransferable implements Transferable {
        DefaultMutableTreeNode[] nodes;

        public NodesTransferable(DefaultMutableTreeNode[] nodes) {
            this.nodes = nodes;
         }

        @Override
        public Object getTransferData(DataFlavor flavor)
                                 throws UnsupportedFlavorException {
            if(!isDataFlavorSupported(flavor))
                throw new UnsupportedFlavorException(flavor);
            return nodes;
        }

        @Override
        public DataFlavor[] getTransferDataFlavors() {
            return allPageDataFlavours;
        }

        @Override
        public boolean isDataFlavorSupported(DataFlavor flavor) {
            return pageDataFlavour.equals(flavor);
        }
    }
}
