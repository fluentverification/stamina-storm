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

template<typename ValueType, typename StateType = uint32_t>
class ProbState {
public:
	typedef typename ProbState<ValueType, StateType> ProbState;
	ProbState() 
		: stateId(0),
		state(0),
		curReachabilityProb(0),
		nextReachabilityProb(0),
		stateIsTerminal(true),
		stateIsAbsorbing(false)
	{}
	ProbState(StateType id) 
		: stateId(id),
		state(id),
		curReachabilityProb(0),
		nextReachabilityProb(0),
		stateIsTerminal(true),
		stateIsAbsorbing(false)
	{}
	

	/**
	* This maps stores transition rate for each outgoing transition.
	*/
	std::unordered_map<StateType, ValueType> predecessorPropMap;
	
  StateType stateId;
	CompressedState state;


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
	ValueType getCurReachabilityProb() const {

		return curReachabilityProb;

	}

	void setCurReachabilityProb(ValueType reachProb) {
		curReachabilityProb = reachProb;
	}

	void addToReachability(ValueType newReach) {
		curReachabilityProb += newReach;
		if(curReachabilityProb >= 1.0) {
			curReachabilityProb = 1.0;
		}
	}

	void subtractFromReachability(ValueType minusReach) {
		curReachabilityProb -= minusReach;
		if(curReachabilityProb <= 0.0) {
			curReachabilityProb = 0.0;
		}
	}

	ValueType getNextReachabilityProb() const {
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

	void updatePredecessorProbMap(StateType index, ValueType tranProb) {
		predecessorPropMap.insert(std::make_pair(index, tranProb));
	}


	/**
	 * Get string representation, e.g. "(0,true,5)".
	 */

	// std::string toString()
	// {
	// 	int i, n;
	// 	std::string s = "(";
	// 	n = varValues.length;
	// 	for (i = 0; i < n; i++) {
	// 		if(i > 0) s += ",";
	// 		s += varValues[i];
	// 	}
	// 	s += "): ";

	// 	s += curReachabilityProb;
	// 	return s;
	// }

	bool operator==(const ProbState &rhs) const { return stateId == rhs.stateId; }
	void operator=(const ProbState &rhs) {
		stateId = rhs.stateId;
		state = rhs.state;
		curReachabilityProb = rhs.curReachabilityProb;
		nextReachabilityProb = rhs.nextReachabilityProb;
		stateIsTerminal = rhs.stateIsTerminal;
		stateIsAbsorbing = rhs.stateIsAbsorbing;
	}

private:
	ValueType curReachabilityProb;
	ValueType nextReachabilityProb;

	bool stateIsTerminal;
	bool stateIsAbsorbing;
};

namespace std
{
		template<typename ValueType, typename StateType> 
		struct hash<ProbState<ValueType, StateType>> {
			std::size_t operator()(ProbState const& p) const noexcept
			{
					std::size_t h = std::hash<StateType>{}(p.stateId);
					return h;
			}
		};
}	// namespace std

#endif //STAMINA_PROBSTATE_H
