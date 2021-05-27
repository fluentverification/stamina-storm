//
// Created by Riley Layne Roberts on 2/6/20.
//

#ifndef STAMINA_STAMINAOPTIONS_H
#define STAMINA_STAMINAOPTIONS_H

#include <string>

class StaminaOptions {

    private:   
        // Probabilistic state search termination value : Defined by kappa in command line argument
        static double reachabilityThreshold;

        // Kappa reduction factor
        static double kappaReductionFactor;

        // Misprediction factor
        static double mispredictionFactor;

        // max number of refinement count
        static int maxApproxCount;

        // termination Error window
        static double probErrorWindow;

        // Do not use property based refinement
        static bool noPropRefine;

        // Whether or not to use ranktransitions.
        static bool rankTransitions;

        static std::string cuddMemoryLimit;

        // Saving variables
        static bool exportModel;

        // Saving filenames
        static std::string exportFileName;

        static bool exportPerimeterStates;
        static std::string exportPerimeterFilename;
        // Import variables
        static bool importModel;

        // Import filenames
        static std::string importFileName;

        // Specific Property
        static bool specificProperty;
        static std::string property;


    public:
        /**
         * Gets the reachability threshold (&kappa;)
         * 
         * @return The reachability threshold.
         * */
        static double getReachabilityThreshold() {
            return reachabilityThreshold;
        }
        /**
         * Sets the reachability threshold (&kappa;)
         * 
         * @param reach New kappa to set.
         * */
        static void setReachabilityThreshold(double reach) {
            reachabilityThreshold = reach;
        }
        /**
         * Gets the kappa reduction factor (r<sub>&kappa;</sub>)
         * 
         * @return The kappa reduction factor.
         * */
        static double getKappaReductionFactor() {
            return kappaReductionFactor;
        }
        /**
         * Sets the kappa reduction factor.
         * 
         * @param fac  New kappa reduction factor to set.
         * */
        static void setKappaReductionFactor(double fac) {
            kappaReductionFactor = fac;
        }
        /**
         * Gets the maximum count of refinement iterations.
         * 
         * @return The maximum approximate refinement count.
         * */
        static int getMaxApproxCount() {
            return maxApproxCount;
        }
        /**
         * Sets the maximum count of refinement iterations.
         * 
         * @param rc The new max refinement count.
         * */
        static void setMaxRefinementCount(int rc) {
            maxApproxCount = rc;
        }
        /**
         * Gets the maximum allowed difference between P<sub>min</sub> and P<sub>max</sub>.
         * 
         * @return Probability error window.
         */
        static double getProbErrorWindow() {
            return probErrorWindow;
        }
        /**
         * Sets the maximum allowed difference between P<sub>min</sub> and P<sub>max</sub>.
         * @param w The window to be set.
         */
        static void setProbErrorWindow(double w) {
            probErrorWindow = w;
        }
        /**
         * Sets whether or not we are using property based refinement.
         * 
         * @param o Whether or not to use property based refinement.
         */
        static void setNoPropRefine(bool o) {
            noPropRefine = o;
        }
        /**
         * Gets whether or not we are using property based refinement.
         * 
         * @return Whether or not we are using property based refinement.
         */
        static bool getNoPropRefine() {
            return noPropRefine;
        }
        /**
         * Sets whether or not we are using rank transitions.
         * 
         * @param o Rank transitions are used or not.
         */
        static void setRankTransitions(bool o) {
            rankTransitions = o;
        }
        /**
         * Gets whether or not we are using rank transitions.
         * 
         * @return Whether rank transitions are used or not.
         */
        static bool getRankTransitions() {
            return rankTransitions;
        }
};

#endif //STAMINA_STAMINAOPTIONS_H
