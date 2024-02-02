#ifndef STAMINA_RARE_DEPENDENCY_GRAPH_H
#define STAMINA_RARE_DEPENDENCY_GRAPH_H

#include "Crn.h"
#include "Subspace.h"

namespace stamina {
	namespace rare {
		template <typename StateType>
		class DependencyGraph<StateType> {
		public:
			/**
			 * Constructor which creates a dependency graph from a RAGTIMER
			 * file, which means the satisfyiability region is a valid subspace
			 * and orthogonal to one or more species dimensions.
			 *
			 * @param ragtimer_filename the (relative) filename of the ragtimer file to parse
			 * */
			DependencyGraph(std::string ragtimer_filename);
			/**
			 * Builds a dependency graph from a built CRN
			 *
			 * @param crn The built CRN
			 * */
			DependencyGraph(const std::shared_ptr<Crn> crn);

			const std::shared_ptr<Crn> getCrn() const;
			void setCrn(const std::shared_ptr<Crn> crn);

			std::vector<std::vector<Crn::Reaction &>> getCycles();
			std::vector<std::shared_ptr<Subspace>> buildSubspaces();

		private:
			std::shared_ptr<Crn> crn = nullptr;

		};
	} // namespace rare
} // namespace stamina

#endif // STAMINA_RARE_DEPENDENCY_GRAPH_H
