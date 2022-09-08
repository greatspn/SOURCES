/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package editor.domain.io;

import editor.domain.Node;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import editor.domain.elements.Transition;
import editor.domain.grammar.ExpressionLanguage;
import editor.domain.grammar.ParserContext;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;

/**
 *
 * @author Irene
 */
public class CppFormat {

    public static String export(File file, GspnPage gspn, ParserContext context)
            throws Exception {

        Set<Double> constantList = new HashSet<Double>();
        Set<Place> placeList = new HashSet<Place>();

        ArrayList<String> log = new ArrayList<>();
        PrintWriter out = new PrintWriter(new BufferedOutputStream(new FileOutputStream(file)));

        for (Node nodePlace : gspn.nodes) {
            if (nodePlace instanceof Place) {
                Place plc = (Place) nodePlace;
                placeList.add(plc);
                out.print("static double " + plc.getUniqueName() + ";\n");
            }
        }

        out.println("#include <math.h>");
        out.println("\n");

        for (Node node : gspn.nodes) {
            if (node instanceof Transition) {
                Transition trn = (Transition) node;
                if (trn.isGeneral()) {

                    out.println("double " + trn.getUniqueName() + "(double *Value,\n"
                            + "                         map <string,int>& NumTrans,\n"
                            + "                         map <string,int>& NumPlaces,\n"
                            + "                         const vector<string> & NameTrans,\n"
                            + "                         const struct InfTr* Trans,\n"
                            + "                         const int T,\n"
                            + "                         const double& time) {\n");
                    
                    

                    out.println("double const = read_constant(\"./" + trn.getUniqueName() + "\", a);\n");

                    String cppDelayExpr = trn.convertDelayLang(context, null, ExpressionLanguage.CPP);
                    
                    Set<Place> placeListCopy =  new HashSet<Place>();
                    placeListCopy.addAll(placeList);
                    for(Place place: placeListCopy ){
                        boolean isFound = cppDelayExpr.contains(place.getUniqueName());
                        if(isFound){
                            //out.println("#define " + place.getUniqueName() + " Value[NumPlaces.find(\"" + place.getUniqueName() + "\")->second]\n");
                            out.println(place.getUniqueName() + " = Value[NumPlaces.find(\"" + place.getUniqueName() + "\")->second];\n");
                            placeList.remove(place);
                        }
                    }
                             
                    
                    System.out.println(trn.getUniqueName() + " " + cppDelayExpr);
                    out.println("   double rate = " + cppDelayExpr + ";");

                    // come faccio a definire una macro per ogni posto? senza avere la
                    //struttura dati degli inidici che è dentro il metodo?
                    /* for (Node nodePlace : gspn.nodes) {
                        if (nodePlace instanceof Place) {
                            Place plc = (Place) nodePlace;
                            out.println("   #ifndef " + plc.getUniqueName());
                            out.println("   Value[NumPlaces.find(\"" + plc.getUniqueName() + "\")->second]");
                            out.println("   #endif\n");
                        }
                    }*/
                }

                out.println("   return rate;");
                out.println("}\n");
            }
        }

        //al log aggiungo messaggi se trovo degli errori durante la stampa.
        //se sarà vuoto tutto bene, se conterrà delle frasi allora ci saranno
        //stati errori e farò return con relativo errore
        return reportLog(log, out);

    }

    //chiude lo stream e ritorna problemi o nulla
    private static String reportLog(ArrayList<String> log, PrintWriter pw) {
        pw.close();
        if (log.isEmpty()) {
            return null; // Everything went ok
        } else {
            String message = "Detected problems while exporting the NetLogo model.\n\n";
            for (String s : log) {
                message += s + "\n";
            }
            return message;
        }
    }

}
