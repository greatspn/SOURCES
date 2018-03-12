/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.play;

/** An event of the play engine that asks to let time elapse, without any
 * transition firing or DTA edge firing.
 *
 * @author elvio
 */
public class TimeElapse implements Firable {
    
    public final static TimeElapse SINGLETON = new TimeElapse();

    @Override
    public boolean isIdentifiedAs(String name) {
        // There is no specific identifier for the time elapse
        return false;
    }

    @Override
    public boolean isFiringInstantaneous() {
        return true;
    }

    @Override
    public boolean isFiringFlow() {
        return false;
    }
}
