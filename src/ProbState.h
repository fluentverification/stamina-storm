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

    class hashFunction {
    public:

        // Use sum of lengths of first and last names
        // as hash function.
        size_t operator()(const ProbState& state) const
        {
            std::hash<int> int_hash;
            return int_hash(state.stateId);
        }
    };

    /**
    * This maps stores transition rate for each outgoing transition.
    */
    std::unordered_map<ProbState, double, hashFunction> predecessorPropMap;


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

	void computeNextReachabilityProb() {

		nextReachabilityProb = 0.0;

		for(auto element : predecessorPropMap) {
			nextReachabilityProb += element.first.getCurReachabilityProb() * element.second;
		}

		if (nextReachabilityProb > 1.0) {
			//throw new stormException("Path Probability greater than 1.0");
		}
	}

	void updatePredecessorProbMap(ProbState* state, double tranProb) {
		predecessorPropMap.insert(std::make_pair(state->stateId, tranProb));
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

    bool operator==(ProbState i) const{
        if ( i.curReachabilityProb==this->curReachabilityProb && i.nextReachabilityProb==this->nextReachabilityProb && i.stateIsAbsorbing==this->stateIsAbsorbing && i.stateIsTerminal==this->stateIsAbsorbing && i.predecessorPropMap==this->predecessorPropMap && i.stateId == this->stateId) {
            return true;
        } else {
            return false;
        }
    }




};
#endif //STAMINA_PROBSTATE_H
