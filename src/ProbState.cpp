#include "ProbState.h"

template<typename ValueType, typename StateType>
ProbState::ProbState() 
    : stateId(0),
      state(0),
      curReachabilityProb(0),
      nextReachabilityProb(0),
      stateIsTerminal(true),
      stateIsAbsorbing(false)
{}
