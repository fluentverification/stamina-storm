#ifndef STAMINA_RARE_EXCEPTION_H
#define STAMINA_RARE_EXCEPTION_H

#include <exception>

// Exceptions available to be thrown by this namespace

class SpeciesCountMismatchException : public Exception {
	virtual const char[] what() const throw() {
		return "A state vector was passed in which either had too few or too many elements";
	}
};

class InvalidSolutionSpaceException : public Exception {
	// ISR requires solution spaces be orthogonal to one or more species,
	// SOP allows any set which is a closed subspace as a solution set.
	virtual const char[] what() const throw() {
		return "The model's solution set either does not comprise a closed subspace, or that subspace is not applicable to the current search method used.";
	}
};

#endif // STAMINA_RARE_EXCEPTION_H
