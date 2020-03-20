//
// Created by Riley Layne Roberts on 2/6/20.
//
#include "options.h"

double Options::reachabilityThreshold = 1e-6;


// Kappa reduction factor
double Options::kappaReductionFactor = 1000.0;

// max number of refinement count
int Options::maxApproxCount = 10;

// termination Error window
double Options::probErrorWindow = 1.0e-3;

// Use property based refinement
bool Options::noPropRefine = false;

bool Options::rankTransitions = false;
