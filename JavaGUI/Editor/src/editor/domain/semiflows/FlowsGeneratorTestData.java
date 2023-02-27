/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package editor.domain.semiflows;

/**
 *
 * @author elvio
 */
public class FlowsGeneratorTestData {
    
    static class FlowProblem {
        int[][] input, solution;
        PTFlows.Type probType;

        public FlowProblem(int[][] input, int[][] solution, PTFlows.Type probType) {
            this.input = input;
            this.solution = solution;
            this.probType = probType;
        }
    }
    
    static final FlowProblem probAnisimov = new FlowProblem(
        new int[][] {
            {2, 0},
            {0, -1},
            {-1, 1},
            {-1, 0},
        },
        new int[][] {
            {1, 0, 0, 2},
            {1, 2, 2, 0},
        },
        PTFlows.Type.PLACE_SEMIFLOWS
    );
    
    static final FlowProblem probCSP = new FlowProblem(
        new int[][] {
            {1, 1, 1},
            {1, 0, 1},
            {0, 1, 0},
            {0, 0, 2},
            {0, 1, 1},
        },
        new int[][] {
            {-1, 1, 1, 0, 0},
            {0, 0, 2, 1, -2},
            {2, -2, 0, 1, -2},
        },
        PTFlows.Type.PLACE_FLOWS
    );
    
    static final FlowProblem probComp = new FlowProblem(
        new int[][] {
            {1, 1, 1},
            {-1, 0, 1},
            {0, -1, 0},
            {0, 0, -2},
            {0, -1, -1},
        },
        new int[][] {
            {1, 1, 1, 1, 0},
            {2, 2, 0, 1, 2},
        },
        PTFlows.Type.PLACE_SEMIFLOWS
    );
    
    static final FlowProblem prob1 = new FlowProblem(
        new int[][] {
            {1, 0, 0, 0, -1, 0, 0, 0, 0, 0},
            {0, 0, 0, -1, 1, 0, 0, 0, 0, 0},
            {1, 0, 0, 0, 0, -1, 0, 0, 0, 0},
            {-1, 0, 0, 1, 0, 0, 0, 0, 0, 0},
            {-1, 0, 0, 0, 0, 0, 1, 0, 0, 0},
            {0, 1, 0, 0, 0, 0, -1, 0, 0, 0},
            {0, -1, 0, 0, 0, 0, 0, 0, 1, 0},
            {0, 1, 0, 0, 0, 0, 0, -1, 0, 0},
            {0, 0, -1, 0, 0, 0, 0, 1, 0, 0},
            {0, 0, 1, 0, 0, 0, 0, 0, -1, 0},
            {0, 0, 1, 0, 0, 0, 0, 0, 0, -1},
            {0, 0, 0, 0, 0, 0, 0, 0, -1, 1},
            {0, 0, 0, -1, 0, 0, 0, 0, 0, 1},
            {0, -1, 0, 0, 0, 1, 0, 0, 0, 0},
        },
        new int[][] {
            {1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
            {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
            {0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1},
            {0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0},
        },
        PTFlows.Type.PLACE_SEMIFLOWS
    );
    
    static final FlowProblem probSiphonBasis = new FlowProblem(
        new int[][] {
            {-1, 1, 0, 0, 0, 0},
            {1, -1, -1, 1, -1, -1},
            {0, -1, -1, 0, 1, 0},
            {0, 0, 0, -1, 0, 1},
            {0, 0, 1, 0, -1, -1},
            {1, 0, 0, 0, 0, 0},
            {0, 1, 0, 0, 0, 0},
            {0, 0, 1, 0, 0, 0},
            {0, 0, 0, 1, 0, 0},
            {0, 0, 0, 0, 1, 0},
            {0, 0, 0, 0, 0, 1},
        },
        new int[][] {
            {1, 1, 0, 1, 0},
            {0, 0, 1, 0, 1},
        },
        PTFlows.Type.SIPHONS
    );
}
