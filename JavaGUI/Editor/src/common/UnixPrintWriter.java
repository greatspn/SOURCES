/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package common;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InterruptedIOException;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.io.Writer;

/** A modified PrintWriter that outputs line endings in Unix format (CR only, no CRLF).
 *
 * @author Elvio
 */
public class UnixPrintWriter extends PrintWriter {

    public UnixPrintWriter(OutputStream out) {
        super(out);
    }

    public UnixPrintWriter(Writer out) {
        super(out);
    }

    public UnixPrintWriter(File file) throws FileNotFoundException {
        super(file);
    }
    
    
    protected void ensureOpen() throws IOException {
        if (out == null) {
            throw new IOException("Stream closed");
        }
    }

    @Override
    public void println() {
        // Method body taken from java.io.PrintWriter.println();
        try {
            synchronized (lock) {
                ensureOpen();

                out.write('\n');

//                if (autoFlush) {
//                    out.flush();
//                }
            }
        } catch (InterruptedIOException e) {
            Thread.currentThread().interrupt();
        } catch (IOException e) {
            setError();
        }
    }
    
}
