//
// Created by Riley Layne Roberts on 2/6/20.
//

#ifndef STAMINA_OPTIONS_H
#define STAMINA_OPTIONS_H

class Options {

private:    //Probabilistic state search termination value : Defined by kappa in command line argument
    static double reachabilityThreshold;


    // Kappa reduction factor
    static double kappaReductionFactor;

    // max number of refinement count
    static int maxApproxCount;

    // termination Error window
    static double probErrorWindow;

    // Use property based refinement
    static bool noPropRefine;

    static bool rankTransitions;


public:
    static double getReachabilityThreshold() {
        return reachabilityThreshold;
    }

    static void setReachabilityThreshold(double reach) {
        reachabilityThreshold = reach;
    }

    static double getKappaReductionFactor() {
        return kappaReductionFactor;
    }

    static void setKappaReductionFactor(double fac) {
        kappaReductionFactor = fac;
    }

    static int getMaxApproxCount() {
        return maxApproxCount;
    }

    static void setMaxRefinementCount(int rc) {
        maxApproxCount = rc;
    }

    static double getProbErrorWindow() {
        return probErrorWindow;
    }

    static void setProbErrorWindow(double w) {
        probErrorWindow = w;
    }

    static void setNoPropRefine(bool o) {
        noPropRefine = o;
    }

    static bool getNoPropRefine() {
        return noPropRefine;
    }

    static void setRankTransitions(bool o) {
        rankTransitions = o;
    }

    static bool getRankTransitions() {
        return rankTransitions;
    }
};



#endif //STAMINA_OPTIONS_H
