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

		private:
			Eigen::MatrixXd reactMatrix;
			std::vector<Reaction> reactions;
			Subspace & solutionSpace;
			Subspace & reachableSpace;
		};
	}
}

#endif // STAMINA_RARE_CRN_H
