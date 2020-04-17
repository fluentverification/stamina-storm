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


	bool isStateTerminal(){
		return stateIsTerminal;
	}

	void setStateTerminal(bool flag) {
		stateIsTerminal = flag;
	}

	bool isStateAbsorbing(){
		return stateIsAbsorbing;
	}

	void setStateAbsorbing(bool flag) {
		stateIsAbsorbing = flag;
	}


	/* Probabilistic search */
	double getCurReachabilityProb() const {

		return curReachabilityProb;

	}

	void setCurReachabilityProb(double reachProb) {
		curReachabilityProb = reachProb;
	}

    double getNextReachabilityProb() const {

		return nextReachabilityProb;

	}

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
