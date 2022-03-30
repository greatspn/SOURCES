/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package models;

import editor.Main;
import java.io.InputStream;

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
        System.out.println(Main.getAppImageGreatSPN_dir());
    }
}
