#include "Subspace.h"

namespace stamina {
namespace rare {

// Static methods
void
Subspace::initializeStatic(uint16_t speciesCount) {
	Subspace::zero = EigenVectorXd(speciesCount);
}

void
Subspace::initializeScaling(std::shared_ptr<EigenMatrixXd> scaling) {
	assert(Subspace::initialized);
	if (scaling.rows() != Subspace::zero.rows()) {
		throw InvalidSolutionSpaceException();
	}
	Subspace::scaling = scaling;
	// Create inverse matrix
	Subspace::scalingInv = std::make_unique<EigenMatrixXd>(scaling.inverse());
}

// Cpnstructors

Subspace::Subspace(std::vector<Eigen::VectorXd> combinationVectors, Eigen::VectorXd translation) {

}

// Nonstatic methods

uint16_t
Subspace::numSpecies() const {
	return this->translation.rows();
}

uint16_t
Subspace::dimension() const {

}

const Eigen::MatrixXd &
Subspace::projectionMatrix() {

}

double
Subspace::distance(Eigen::VectorXd vec) {

}


OrthSubspace::OrthSubspace(std::vector<std::pair<uint16_t, double>> speciesAndValues) {

}

double
OrthSubspace::distance(Eigen::VectorXd vec) {

}

} // namespace rare
} // namespace stamina
