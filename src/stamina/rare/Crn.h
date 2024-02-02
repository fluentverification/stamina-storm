#ifndef STAMINA_RARE_CRN_H
#define STAMINA_RARE_CRN_H

#include <Eigen/Dense>

#include "Subspace.h"

namespace stamina {
	namespace rare {
		class Crn {
		public:
			class Reaction {
			public:
				Reaction(std::string ragtimerLine, const Crn & parent);

				Eigen::VectorXd updateVector;
				const Crn & parent;
			};
			Crn(std::string ragtimerFilename);

			uint16_t numberSpecies() const;
		private:
			Eigen::MatrixXd reactMatrix;
			std::vector<Reaction> reactions;
			Subspace & solutionSpace;
			Subspace & reachableSpace;
			std::map<std::string, uint16_t> nameToIdx;
			std::string[] idxToName;
			uint16_t numSpecies;
		};
	}
}

#endif // STAMINA_RARE_CRN_H
