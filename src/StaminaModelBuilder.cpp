#include "StaminaModelBuilder.h"

using namespace stamina;

StaminaModelBuilder::StaminaModelBuilder() {

}


// // Explicitly instantiate the class.
// template class ExplicitModelBuilder<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;
// template class ExplicitStateLookup<uint32_t>;

// #ifdef STORM_HAVE_CARL
// template class ExplicitModelBuilder<RationalNumber, storm::models::sparse::StandardRewardModel<RationalNumber>, uint32_t>;
// template class ExplicitModelBuilder<RationalFunction, storm::models::sparse::StandardRewardModel<RationalFunction>, uint32_t>;
// template class ExplicitModelBuilder<double, storm::models::sparse::StandardRewardModel<storm::Interval>, uint32_t>;
// #endif