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
public class SingleElementIterator <T> implements Iterator<T> {
    
    private final T item;
    private boolean hasItem = true;

    public SingleElementIterator(T item) {
        this.item = item;
    }

    @Override
    public boolean hasNext() {
        return hasItem;
    }

    @Override
    public T next() {
        if (!hasNext())
            throw new NoSuchElementException();
        hasItem = false;
        return item;
    }

    @Override
    public void remove() {
        throw new UnsupportedOperationException();
    }
}
