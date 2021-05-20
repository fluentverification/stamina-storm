//
// Created by Riley Layne Roberts on 3/27/20.
//

#ifndef STAMINA_PROBSTATE_H
#define STAMINA_PROBSTATE_H
#include "storm/storage/BitVector.h"
#include <string>
#include <utility>
#include <unordered_map>
typedef storm::storage::BitVector CompressedState;
class ProbState{

	private:
		int stateId;
		double curReachabilityProb;
		double nextReachabilityProb;

		bool stateIsTerminal;
		bool stateIsAbsorbing;
	
	public:
		ProbState(uint32_t stateId) {

			this->stateId = (int) stateId;
			curReachabilityProb = 0.0;
			nextReachabilityProb = 0.0;

			stateIsTerminal = true;
			stateIsAbsorbing = false;

		}

		/**
		* This maps stores transition rate for each outgoing transition.
		*/
		std::unordered_map<int, double> predecessorPropMap;

		/**
		 * Wheter or not this state is terminal.
		 * 
		 * @return If the state is terminal or not.
		 * */
		bool isStateTerminal(){
			return stateIsTerminal;
		}
		/**
		 * Sets the state as either terminal or not depending on the flag.
		 * 
		 * @param flag bool Whether or not the state is to be terminal.
		 * */
		void setStateTerminal(bool flag) {
			stateIsTerminal = flag;
		}
		/**
		 * Wheter or not this state is absorbing.
		 * 
		 * @return If the state is absorbing or not.
		 * */
		bool isStateAbsorbing(){
			return stateIsAbsorbing;
		}
		/**
		 * Sets the state as either absorbing or not depending on the flag.
		 * 
		 * @param flag bool Whether or not the state is to be absorbing.
		 * */
		void setStateAbsorbing(bool flag) {
			stateIsAbsorbing = flag;
		}


		/* Probabilistic search */
		/**
		 * Gets the current reachability probability.
		 * 
		 * @return The current reachability probability.
		 * */
		double getCurReachabilityProb() const {

			return curReachabilityProb;

		}
		/**
		 * Sets the current reachability probability.
		 * 
		 * @param reachProb double The new reachability probability to set.
		 * */
		void setCurReachabilityProb(double reachProb) {
			curReachabilityProb = reachProb;
		}
		/**
		 * Gets the next reachability probability for this state.
		 * 
		 * @return the next reachability probability.
		 * */
		double getNextReachabilityProb() const {

			return nextReachabilityProb;

		}
		/**
		 * Sets the next reachability probability to the current one.
		 * */
		void setNextReachabilityProbToCurrent() {
			curReachabilityProb = nextReachabilityProb;
		}

		/*void computeNextReachabilityProb() {

			nextReachabilityProb = 0.0;

			for(auto element : predecessorPropMap) {
				nextReachabilityProb += element.first.getCurReachabilityProb() * element.second;
			}

			if (nextReachabilityProb > 1.0) {
				//throw new stormException("Path Probability greater than 1.0");
			}
		}*/
		/**
		 * Inserts new element into the Predecessor Probability map.
		 * 
		 * @param index int Index of predecessor.
		 * @param tranProb double The transition probability. 
		 * */
		void updatePredecessorProbMap(int index, double tranProb) {
			predecessorPropMap.insert(std::make_pair(index, tranProb));
		}


		/**
		 * Get string representation, e.g. "(0,true,5)".
		 */
		std::string toString()
		{
			int i, n;
			std::string s = "(";
			//n = varValues.length;
			for (i = 0; i < n; i++) {
				if (i > 0)
					s += ",";
				//s += varValues[i];
			}
			s += "): ";

			s += curReachabilityProb;
			return s;
		}
};
#endif //STAMINA_PROBSTATE_H
