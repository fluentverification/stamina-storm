#include "ProbState.h"

using namespace stamina;

ProbState::ProbState() 
    : stateId(0)
    , state(0)
    , curReachabilityProb(0)
    , nextReachabilityProb(0)
    , stateIsTerminal(true)
    , stateIsAbsorbing(false)
{
    // Intentionally left empty
}

ProbState::ProbState(uint32_t id) 
    : stateId(id)
    , state(id)
    , curReachabilityProb(0)
    , nextReachabilityProb(0)
    , stateIsTerminal(true)
    , stateIsAbsorbing(false)
{
    // Intentionally left empty
}


void
ProbState::setCurReachabilityProb(double reachProb) {
    curReachabilityProb = reachProb;
}

void
ProbState::addToReachability(double newReach) {
    curReachabilityProb += newReach;
    if(curReachabilityProb >= 1.0) {
        curReachabilityProb = 1.0;
    }
}

void
ProbState::subtractFromReachability(double minusReach) {
    curReachabilityProb -= minusReach;
    if(curReachabilityProb <= 0.0) {
        curReachabilityProb = 0.0;
    }
}

double
ProbState::getNextReachabilityProb() const {
    return nextReachabilityProb;
}

bool
ProbState::isStateTerminal(){
    return stateIsTerminal;
}

void
ProbState::setStateTerminal(bool flag) {
    stateIsTerminal = flag;
}

bool
ProbState::isStateAbsorbing(){
    return stateIsAbsorbing;
}

void
ProbState::setStateAbsorbing(bool flag) {
    stateIsAbsorbing = flag;
}

double
ProbState::getCurReachabilityProb() const {

    return curReachabilityProb;

}

void
ProbState::updatePredecessorProbMap(uint32_t index, double tranProb) {
    predecessorPropMap.insert(std::make_pair(index, tranProb));
}