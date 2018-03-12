/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package common;

import java.util.Iterator;
import java.util.NoSuchElementException;

/**
 *
 * @author elvio
 */
public class EmptyIterator<T> implements Iterator<T> {

    public EmptyIterator() {
    }
    
    public static final EmptyIterator INSTANCE = new EmptyIterator();

    @Override
    public boolean hasNext() {
        return false;
    }

    @Override
    public T next() {
        throw new NoSuchElementException();
    }

    @Override
    public void remove() {
        throw new UnsupportedOperationException();
    }
}
