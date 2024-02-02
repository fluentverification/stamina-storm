#include "Subspace.h"

namespace stamina {
namespace rare {

Subspace::Subspace(std::vector<Eigen::VectorXd> combinationVectors, Eigen::VectorXd translation) {

}

uint16_t
Subspace::numSpecies() const {

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
