#ifndef STAMINA_RARE_SUBSPACE_H
#define STAMINA_RARE_SUBSPACE_H

#include <memory>

namespace stamina {
	namespace rare {
		class Subspace {
		public static:
			void initializeStatic(uint16_t speciesCount);
			void initializeScaling(std::shared_ptr<EigenMatrixXd> scaling);

		protected static:
			EigenVectorXd zero;
			std::shared_ptr<EigenMatrixXd> scaling = nullptr;
			std::unique_ptr<EigenMatrixXd> scalingInv = nullptr;
			bool initialized = false;

		public:
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

			double distance(Eigen::VectorXd vec) override;
		}
	} // namespace rare
} // namespace stamina

#endif // STAMINA_RARE_SUBSPACE_H