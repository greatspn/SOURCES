/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.play;

import java.util.Random;

/**
 *
 * @author elvio
 */
public class StatisticalDistributions {
    
    private static final Random randomGenerator = new Random();
    
    
    // Generate a random sample between [0, 1]
    public static double uniform01() {
        return randomGenerator.nextDouble();
    }
    
    // Generate a random integer between [0, n)
    public static int randIntN(int n) {
        return randomGenerator.nextInt(n);
    }
    
    // Generate a random sample from the negative exponential distribution
    public static double negExp(double mean) {
        return -Math.log(randomGenerator.nextDouble()) * mean;
    }
    
    // Generate the firingTime of an exponential transition with a given rate
    public static double randomExpFiringTime(double rate) {
        return negExp(1.0 / rate);
    }
    
    // Generate a random sample between [a, b]
    public static double uniformAB(double a, double b) {
        return a + uniform01() * (b - a);
    }
    
    // Generate a random sample from an Erlang(rate, stages) distribution
    public static double randomErlangFiringTime(double rate, int stages) {
        double firingTime = 0.0;
        for (int i=0; i<stages; i++)
            firingTime += randomExpFiringTime(rate);
        return firingTime;
    }
    
}
