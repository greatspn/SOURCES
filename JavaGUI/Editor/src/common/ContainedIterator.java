/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package common;

import java.util.Iterator;
import java.util.NoSuchElementException;

/** Iterates over a sub-element of an array
 *
 * @author elvio
 */
public abstract class ContainedIterator<T, C> implements Iterator<T>
{
   private Iterator<C> iter; 

   public ContainedIterator (Iterator<C> iter)
   {
      this.iter = iter;
   }

   //-----------------------------------------------------------------
   //  Returns true if this iterator has at least one more element
   //  to deliver in the iteraion.
   //-----------------------------------------------------------------
   @Override
   public boolean hasNext()
   {
      return iter.hasNext();
   }

   //-----------------------------------------------------------------
   //  Returns the next element in the iteration. If there are no
   //  more elements in this itertion, a NoSuchElementException is
   //  thrown.
   //-----------------------------------------------------------------
   @Override
   public T next()
   {
      return getContained(iter.next()); 
   }
   
   protected abstract T getContained(C elem);

   @Override
   public void remove() throws UnsupportedOperationException
   {
      throw new UnsupportedOperationException();
   }
}
