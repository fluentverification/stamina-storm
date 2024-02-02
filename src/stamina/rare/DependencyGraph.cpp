#include "DependencyGraph.h"

#define VERY_LARGE_NUMBER 100000

namespace stamina {
namespace rare {

DependencyGraph::DependencyGraph(std::string ragtimerFilename) {

}

DependencyGraph::DependencyGraph(const std::shared_ptr<Crn> crn) {

}

const std::shared_ptr<Crn>
DependencyGraph::getCrn() {

}

void
DependencyGraph::setCrn(const std::shared_ptr<Crn> crn) {

}

std::vector<std::vector<Crn::Reaction &>>
DependencyGraph::getCycles() {

}

std::vector<std::shared_ptr<Subspace>>
DependencyGraph::buildSubspaces() {

}

DependencyGraph::Node::Node(const Crn::Reaction & reaction, std::vector<std::shared_ptr<Node>> successors) {

}

void
DependencyGraph::Node::computeMld() {
	if (successors.length() == 0) {
		this->mld = 0;
		return;
	}
	uint16_t minNext = VERY_LARGE_NUMBER;
	for (auto & successorNode : successors) {
		if (successorNode.mld < minNext) {
			minNext = successorNode.mld;
		}
	}
	this->mld = minNext + 1;
}

} // namespace rare
} // namespace stamina
