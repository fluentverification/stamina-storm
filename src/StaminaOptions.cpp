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

double StaminaOptions::mispredictionFactor = 2;

std::string StaminaOptions::cuddMemoryLimit = "1g";

// Saving variables
bool StaminaOptions::exportModel = false;

// Saving filenames
std::string StaminaOptions::exportFileName = nullptr;

bool StaminaOptions::exportPerimeterStates = false;
std::string StaminaOptions::exportPerimeterFilename = nullptr;
// Import variables
bool StaminaOptions::importModel = false;

// Import filenames
std::string StaminaOptions::importFileName = nullptr;

// Specific Property
bool StaminaOptions::specificProperty = false;
std::string StaminaOptions::property = nullptr;