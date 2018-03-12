/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

/** A property that can be edited with a text-based editor.
 *
 * @author elvio
 */
public interface EditableValue {
    public boolean isEditable();
    public Object getValue();
    public boolean isCurrentValueValid();
    public boolean isValueValid(ProjectData proj, ProjectPage page, Object value);
    public void setValue(ProjectData project, ProjectPage page, Object value);
}
