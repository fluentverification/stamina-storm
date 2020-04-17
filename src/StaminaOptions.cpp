//
// Created by Riley Layne Roberts on 2/6/20.
//
#include "StaminaOptions.h"

double StaminaOptions::reachabilityThreshold = 0;


// Kappa reduction factor
double StaminaOptions::kappaReductionFactor = 1000.0;

// max number of refinement count
int StaminaOptions::maxApproxCount = 10;

// termination Error window
double StaminaOptions::probErrorWindow = 1.0e-3;

// Use property based refinement
bool StaminaOptions::noPropRefine = false;

bool StaminaOptions::rankTransitions = false;
