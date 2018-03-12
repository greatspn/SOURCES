/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor;

/**
 *
 * @author elvio
 */
public class ProvaCloseable {
    
    static class DisposeException extends Exception { }
    
    static interface AutoDisposable extends AutoCloseable {
        @Override
        void close() throws DisposeException;
    }
    
    static class AnyValue implements AutoDisposable {

        public AnyValue(ValueFactory factory) {
            this.factory = factory;
        }
        
        public int i;
        public double d;
        
        ValueFactory factory;
        AnyValue next;

        @Override
        public void close() throws DisposeException {
            factory.disposeValue(this);
        }
    }
    
    static class ValueFactory {
        private AnyValue  freeList;
        
        public AnyValue getValue() throws DisposeException {
            AnyValue av = freeList;
            if (av == null) {
                av = new AnyValue(this);
            }
            else {
                freeList = freeList.next;
                av.next = null;
            }
            System.out.println("getValue "+av);
            
            return av;
        }
        
        public void disposeValue(AnyValue av) {
            System.out.println("disposeValue "+av);
            assert av.next == null;
            av.next = freeList;
            freeList = av;
        }
    }
    
    public static void main(String[] args)  {
        ValueFactory f = new ValueFactory();
        try (AnyValue a = f.getValue();
            AnyValue b = f.getValue()) 
        {
            
        }
        catch (DisposeException e) { }
        
        try (AnyValue a = f.getValue();
            AnyValue b = f.getValue()) 
        {
//            a = null;
        }
        catch (DisposeException e) { }
        
        //AnyValue a2 = f.getValue();
    }
    
}
