///*
// * To change this license header, choose License Headers in Project Properties.
// * To change this template file, choose Tools | Templates
// * and open the template in the editor.
// */
//
//package editor.domain.elements;
//
//import editor.domain.Decor;
//import editor.domain.DrawHelper;
//import editor.domain.EditableCell;
//import editor.domain.EditableValue;
//import editor.domain.LabelDecor;
//import editor.domain.Node;
//import editor.domain.play.ActivityState;
//import java.awt.Color;
//import java.awt.Graphics2D;
//import java.awt.Shape;
//import java.awt.geom.AffineTransform;
//import java.awt.geom.Point2D;
//
///** A shadow reference to another node.
// *
// * @author elvio
// */
//public class NodeRef extends Node {
//
//    public NodeRef() {
//    }
//
//    public NodeRef(Node refNode, String refName, Point2D pos) {
//        initializeNode(pos, refName);
//        this.refNode = refNode;
//    }
//    
//    private Node refNode;
//    
//    @Override
//    public Node getReferencedNode() { return refNode.getReferencedNode(); }
//    
//    @Override
//    public boolean isReference() { return true; }
//
//    // -------------------- Delegate implementation ---------------------
//    
//    @Override
//    public Class getSuperpositionClass() {
//        return refNode.getSuperpositionClass();
//    }
//    
//    @Override
//    public String getSuperPosTags() { return refNode.getSuperPosTags(); }
//    @Override
//    public void setSuperPosTags(String superPosTags) { refNode.setSuperPosTags(superPosTags); }
//    @Override
//    public LabelDecor getSuperPosTagsDecor() { return refNode.getSuperPosTagsDecor(); }
//    
//    
//    @Override
//    public double getWidth() {
//        return refNode.getWidth();
//    }
//
//    @Override
//    public double getHeight() {
//        return refNode.getHeight();
//    }
//
//    @Override
//    public double getDefaultWidth() {
//        return refNode.getDefaultWidth();
//    }
//
//    @Override
//    public double getDefaultHeight() {
//        return refNode.getDefaultHeight();
//    }
//
//    @Override
//    public boolean mayRotate() {
//        return refNode.mayRotate();
//    }
//
//    @Override
//    public void setRotation(double rotation) {
//        refNode.setRotation(rotation);
//    }
//    
//    @Override
//    public boolean mayResizeWidth() {
//        return refNode.mayResizeWidth();
//    }
//
//    @Override
//    public boolean mayResizeHeight() {
//        return refNode.mayResizeHeight();
//    }
//
//    @Override
//    public void setWidth(double newWidth) {
//        refNode.setWidth(newWidth);
//    }
//
//    @Override
//    public void setHeight(double newHeight) {
//        refNode.setHeight(newHeight);
//    }
//
//    @Override
//    public double getRotation() {
//        return refNode.getRotation();
//    }
//
//    @Override
//    public double getRoundX() {
//        return refNode.getRoundX();
//    }
//
//    @Override
//    public double getRoundY() {
//        return refNode.getRoundY();
//    }
//
//    @Override
//    public boolean hasShadow() {
//        return refNode.hasShadow();
//    }
//
//    @Override
//    public ShapeType getShapeType() {
//        return refNode.getShapeType();
//    }
//
//    @Override
//    public Color getBorderColor() {
//        return refNode.getBorderColor();
//    }
//
//    @Override
//    public Color getBorderColorGrayed() {
//        return refNode.getBorderColorGrayed();
//    }
//
//    @Override
//    public Color getBorderColorError() {
//        return refNode.getBorderColorError();
//    }
//
//    @Override
//    public Color getFillColor(ActivityState activity) {
//        return refNode.getFillColor(activity);
//    }
//
//    @Override
//    public Color getFillColorGrayed() {
//        return refNode.getFillColorGrayed();
//    }
//
//    @Override
//    public Color getFillColorError() {
//        return refNode.getFillColorError();
//    }
//
//    @Override
//    public LabelDecor getUniqueNameDecor() {
//        return refNode.getUniqueNameDecor();
//    }
//    
//    @Override
//    public double getActivityValue(ActivityState activity) {
//        return refNode.getActivityValue(activity);
//    }
//
//    @Override
//    public EditableValue getUniqueNameEditable() {
//        return refNode.getUniqueNameEditable();
//    }
//
//    @Override
//    public boolean hasSuperPosTags() {
//        return refNode.hasSuperPosTags();
//    }
//
//    @Override
//    public Point2D getSuperPosTagsDefaultPos() {
//        return refNode.getSuperPosTagsDefaultPos();
//    }
//    
//    // From ref coordinate to node coordinate
//    private double translateX(double X) {
//        return X - getX() + refNode.getX();
//    }
//    private double translateY(double Y) {
//        return Y - getY() + refNode.getY();
//    }
//
//    @Override
//    public EditableCell getCentralEditable(double px, double py) {
//        return refNode.getCentralEditable(translateX(px), translateY(py));
//    }
//
//    @Override
//    protected void paintNodeInterior(Graphics2D g, DrawHelper dh, Color borderColor, 
//                                     boolean errorFlag, Shape nodeShape) 
//    {
//        throw new UnsupportedOperationException("Should not be called");
//    }
//
//    @Override
//    public int getNumDecors() {
//        return refNode.getNumDecors();
//    }
//
//    @Override
//    public Decor getDecor(int i) {
//        return refNode.getDecor(i);
//    }
//
//    @Override
//    public void paintNode(Graphics2D g, DrawHelper dh, boolean errorFlag) {
//        AffineTransform tr = g.getTransform();
//        g.translate(getX()-refNode.getX(), getY()-refNode.getY());
//        boolean selected = refNode.isSelected();
//        refNode.setSelected(isSelected());
//        refNode.paintNode(g, dh, errorFlag); 
//        
//        refNode.setSelected(selected);
//        g.setTransform(tr);
//    }
//}
