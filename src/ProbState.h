//
// Created by Riley Layne Roberts on 3/27/20.
//

#ifndef STAMINA_PROBSTATE_H
#define STAMINA_PROBSTATE_H
#include "storm/storage/BitVector.h" // /storm/generator/CompressedState.h
#include <string>
#include <utility>
#include <unordered_map>
typedef storm::storage::BitVector CompressedState; // <-- Get rid of this (defined in CompressedState.h). Then use unpackStateIntoValuation() to get Values

class ProbState {
public:
	ProbState() 
		: stateId(0),
		state(0),
		curReachabilityProb(0),
		nextReachabilityProb(0),
		stateIsTerminal(true),
		stateIsAbsorbing(false)
	{}
	ProbState(uint32_t id) 
		: stateId(id),
		state(id),
		curReachabilityProb(0),
		nextReachabilityProb(0),
		stateIsTerminal(true),
		stateIsAbsorbing(false)
	{}


	void setCurReachabilityProb(double reachProb) {
		curReachabilityProb = reachProb;
	}

	void addToReachability(double newReach) {
		curReachabilityProb += newReach;
		if(curReachabilityProb >= 1.0) {
			curReachabilityProb = 1.0;
		}
	}

	void subtractFromReachability(double minusReach) {
		curReachabilityProb -= minusReach;
		if(curReachabilityProb <= 0.0) {
			curReachabilityProb = 0.0;
		}
	}

	double getNextReachabilityProb() const {
		return nextReachabilityProb;
	}
  	uint32_t stateId;
	CompressedState state;
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

	void updatePredecessorProbMap(uint32_t index, double tranProb) {
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
	double curReachabilityProb;
	double nextReachabilityProb;

	bool stateIsTerminal;
	bool stateIsAbsorbing;
};

namespace std
{
		template<> 
		struct hash<ProbState> {
			std::size_t operator()(ProbState const& p) const noexcept
			{
					std::size_t h = std::hash<uint32_t>{}(p.stateId);
					return h;
			}
		};
}	// namespace std

#endif //STAMINA_PROBSTATE_H
