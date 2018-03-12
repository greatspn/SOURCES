/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import java.awt.geom.Point2D;

/**
 *
 * @author elvio
 */
public class PointHandlePosition implements HandlePosition {
    public Point2D point;

    public PointHandlePosition(Point2D point) {
        this.point = point;
    }

    public PointHandlePosition(double x, double y) {
        this.point = new Point2D.Double(x, y);
    }

    @Override
    public double getRefX() {
        return point.getX();
    }

    @Override
    public double getRefY() {
        return point.getY();
    }

    @Override
    public Point2D getRefPoint() {
        return new Point2D.Double(point.getX(), point.getY());
    }
    
}
