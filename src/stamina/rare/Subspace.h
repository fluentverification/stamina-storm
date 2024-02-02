#ifndef STAMINA_RARE_SUBSPACE_H
#define STAMINA_RARE_SUBSPACE_H

namespace stamina {
	namespace rare {
		class Subspace {
		public:
			static EigenVectorXd zero;
			Subspace(std::vector<Eigen::VectorXd> combinationVectors, Eigen::VectorXd translation = Subspace::zero);

			uint16_t numSpecies() const;
			uint16_t dimension() const;
			const Eigen::MatrixXd & projectionMatrix();
			virtual double distance(Eigen::VectorXd vec);
		private:
			std::vector<Eigen::VectorXd> combinationVectors;
			Eigen::MatrixXd pMatrix;
			Eigen::VectorXd & translationVector;
		};

		/**
		 * A version of subspace that is orthogonal to one or more
		 * species dimensions and can short circuit distance calculations.
		 * Used for the solution space in ragtimer models
		 * */
		class OrthSubspace : public Subspace {
		public:
			/**
			 * Overriden constructor taking the species index and desired values for that species in a pair
			 * */
			OrthSubspace(std::vector<std::pair<uint16_t, double>> speciesAndValues);
			OrthSubspace(std::vector<Eigen::VectorXd> combinationVectors, Eigen::VectorXd translation) = delete;
		}
	} // namespace rare
} // namespace stamina

#endif // STAMINA_RARE_SUBSPACE_H
