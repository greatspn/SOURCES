/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package latex;

/**
 *
 * @author elvio
 */
public class DummyLatexProvider extends LatexProvider {
    
    public static void initializeProvider() {
        DummyLatexProvider lp = new DummyLatexProvider();
        LatexProvider.initialize(lp);
        
        lp.cmd = new MetaCommands("x", 1.0f, 1, 1, 1, 1, 1);
    }
    
    MetaCommands cmd;

    @Override
    MetaCommands typesetLatexFormula(String latex, float size) {
        return cmd;
    }

    @Override
    public void quit() {
    }
    
}
