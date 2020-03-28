//
// Created by Riley Layne Roberts on 3/27/20.
//

#ifndef STAMINA_PROBSTATE_H
#define STAMINA_PROBSTATE_H
#include "storm/storage/BitVector.h"
#include <unordered_map>

typedef storm::storage::BitVector 	CompressedState;
class ProbState : public CompressedState{

	private:
	double curReachabilityProb;
	double nextReachabilityProb;

	bool stateIsTerminal;
	bool stateIsAbsorbing;

	/**
	 * This maps stores transition rate for each outgoing transition.
	 */
	std::unordered_map<ProbState, double> predecessorPropMap;


	public:
	 ProbState(CompressedState s) : CompressedState(s) {

		curReachabilityProb = 0.0;
		nextReachabilityProb = 0.0;

		stateIsTerminal = true;
		stateIsAbsorbing = false;

		predecessorPropMap = std::unordered_map<ProbState, double>();
	}


	bool isStateTerminal(){
		return stateIsTerminal;
	}

	void setStateTerminal(bool flag) {
		isStateTerminal = flag;
	}

	bool isStateAbsorbing(){
		return isStateAbsorbing;
	}

	void setStateAbsorbing(bool flag) {
		isStateAbsorbing = flag;
	}


	/* Probabilistic search */
	double getCurReachabilityProb() {

		return curReachabilityProb;

	}

	void setCurReachabilityProb(double reachProb) {
		curReachabilityProb = reachProb;
	}

    double getNextReachabilityProb() {

		return nextReachabilityProb;

	}

	void setNextReachabilityProbToCurrent() {
		curReachabilityProb = nextReachabilityProb;
	}

	void computeNextReachabilityProb() {

		nextReachabilityProb = 0.0;

		for(ProbState pre: predecessorPropMap.keySet()) {
			nextReachabilityProb += pre.getCurReachabilityProb() * predecessorPropMap.get(pre);
		}

		if (nextReachabilityProb > 1.0) {
			throw new RuntimeException("Path Probability greater than 1.0");
		}
	}

	void updatePredecessorProbMap(ProbState state, double tranProb) {
		predecessorPropMap.put(state, tranProb);
	}


	/**
	 * Get string representation, e.g. "(0,true,5)".
	 */
	@Override
	public String toString()
	{
		int i, n;
		String s = "(";
		n = varValues.length;
		for (i = 0; i < n; i++) {
			if (i > 0)
				s += ",";
			s += varValues[i];
		}
		s += "): ";

		s += curReachabilityProb;
		return s;
	}


}
#endif //STAMINA_PROBSTATE_H
