/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package models;

import java.io.File;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;

/**
 *
 * @author elvio
 */
public class BuiltinModels {
    public static String[][] models = {
        {
            // Basic P/T models
            "Cycle of Four Seasons",
            "FlexibleManufacturingSystem",
            "Multiprocessor",
            "Philosophers4",
            "2PhaseLockViolation",
            "ReaderWriter",
            "Glycolysis",
            "SimpleCSLTA",
        },
        {
            // Basic CPN/SWN models
            "SimpleCPN",
            "DatabaseCPN",
            "PhilosophersCPN",
            "Polling System",
            "Fat Tree",
            "Erathostenes",
            "n-queens",
        },
        {
            // Compositional models
            "CCS-like Composition",
            "CSP-like Composition",
            "ModularClientsAndServers",
            "PhilosophersUnfolding",
        }
    };
    
    public static InputStream getModelStream(String model) {
        return BuiltinModels.class.getResourceAsStream("/models/" + model + ".PNPRO");
    }
    
    public static void main(String[] args) {
        try {
            String path = BuiltinModels.class.getProtectionDomain().getCodeSource().getLocation().getPath();
            String decodedPath = URLDecoder.decode(path, "UTF-8");
            System.out.println("decodedPath="+decodedPath);
            File file = new File(decodedPath);
            if (file.isFile() && file.exists() && decodedPath.endsWith(".jar")) {
                // Running from JAR
                System.out.println(file.getParent());
            }
            else if (file.isDirectory() && file.exists() && 
                    decodedPath.endsWith("JavaGUI/Editor/build/classes/")) 
            {
                // Running from class sources
            }
            else {
                // Unknown
            }
        }
        catch (UnsupportedEncodingException e) {
            
        }
    }
}
