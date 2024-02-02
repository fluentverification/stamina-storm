#ifndef STAMINA_RARE_SUBSPACE_H
#define STAMINA_RARE_SUBSPACE_H

namespace stamina {
	namespace rare {
		class Subspace {
		public:
			Subspace(std::vector<Eigen::VectorXd> combinationVectors);

			uint16_t numSpecies() const;
			uint16_t dimension() const;
			const Eigen::MatrixXd & projectionMatrix();
		private:
			std::vector<Eigen::VectorXd> combinationVectors;
			Eigen::MatrixXd pMatrix;
		};
	} // namespace rare
} // namespace stamina

#endif // STAMINA_RARE_SUBSPACE_H
