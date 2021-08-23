/**
 * Probability State Class
 * 
 * (Re-)created by Josh Jeppson on 8/23/2021
 * */
#ifndef PROBSTATE_H
#define PROBSTATE_H

#include "storm/storage/BitVector.h"
#include <string>
#include <utility>
#include <unordered_map>
typedef storm::storage::BitVector CompressedState;

namespace stamina {
    class ProbState {
    public:
        /**
         * Constructor for ProbState class.
         * */
        ProbState();
        /**
         * Alternate constructor for the ProbState class with ID.
         * 
         * @param id ID to set.
         * */
        ProbState(uint32_t id);
        /**
         * Sets the current reachability probability of our state
         * 
         * @param reachProb Our state's reachability probability
         * */
        void setCurReachabilityProb(double reachProb);
        /**
         * Adds value to our reachability probability
         * 
         * @param newReach Reachability to add
         * */
        void addToReachability(double newReach);
        /**
         * Subtracts value from our reachability probability
         * 
         * @param minusReach Reachability to subtract
         * */
        void subtractFromReachability(double minusReach);
        /**
         * Gets what our current reachability probability is.
         * 
         * @return Current reachability probability.
         * */
        double getNextReachabilityProb() const;
        /**
         * Equality operator for ProbState
         * 
         * @param rhs The other ProbState to check.
         * */
    	bool operator==(const ProbState &rhs) const {
            return stateId == rhs.stateId;
        }
        /**
         * Copy operator for ProbState
         * 
         * @param rhs The ProbState to copy from
         * */
        void operator=(const ProbState &rhs) {
            stateId = rhs.stateId;
            state = rhs.state;
            curReachabilityProb = rhs.curReachabilityProb;
            nextReachabilityProb = rhs.nextReachabilityProb;
            stateIsTerminal = rhs.stateIsTerminal;
            stateIsAbsorbing = rhs.stateIsAbsorbing;
        }
        		/**
		 * Wheter or not this state is terminal.
		 * 
		 * @return If the state is terminal or not.
		 * */
		bool isStateTerminal();
		/**
		 * Sets the state as either terminal or not depending on the flag.
		 * 
		 * @param flag bool Whether or not the state is to be terminal.
		 * */
		void setStateTerminal(bool flag);
		/**
		 * Wheter or not this state is absorbing.
		 * 
		 * @return If the state is absorbing or not.
		 * */
		bool isStateAbsorbing();
		/**
		 * Sets the state as either absorbing or not depending on the flag.
		 * 
		 * @param flag bool Whether or not the state is to be absorbing.
		 * */
		void setStateAbsorbing(bool flag);
		/* Probabilistic search */
		/**
		 * Gets the current reachability probability.
		 * 
		 * @return The current reachability probability.
		 * */
		double getCurReachabilityProb() const;
        /**
         * 
         * */
	    void updatePredecessorProbMap(uint32_t index, double tranProb);
        /* Data members */
        uint32_t stateId;
	    CompressedState state;
        // This maps stores transition rate for each outgoing transition.
		std::unordered_map<int, double> predecessorPropMap;
    private:
        /* Data members */
        double curReachabilityProb;
        double nextReachabilityProb;
        bool stateIsTerminal;
        bool stateIsAbsorbing;
    };
}
/* Overrite hashing with stateId */
namespace std {
    template<> 
    struct hash<ProbState> {
        std::size_t operator()(ProbState const& p) const noexcept {
            std::size_t h = std::hash<uint32_t>{}(p.stateId);
            return h;
        }
    };
}

#endif // PROBSTATE_H