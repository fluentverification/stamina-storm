#include "ThreadsafePrismNextStateGenerator.h"


#include "core/StaminaMessages.h"

namespace stamina {
namespace threadsafe {
namespace generator {

template<typename ValueType, typename StateType = uint32_t>
ThreadsafePrismNextStateGenerator<ValueType, StateType>::ThreadsafePrismNextStateGenerator(
	storm::prism::Program const& program
	, storm::generator::NextStateGeneratorOptions const& options = NextStateGeneratorOptions()
	, std::shared_ptr<ActionMask<ValueType, StateType>> const& = nullptr
) : PrismNextStateGenerator<ValueType, StateType>(program, options)
	, varInfoWasSetUp(false)
{
	// intentionally left empty
}

template<typename ValueType, typename StateType = uint32_t>
storm::storage::sparse::StateValuations
ThreadsafePrismNextStateGenerator<ValueType, StateType>::makeObservationValuation() const {
	if (varInfoWasSetUp) {
		StaminaMessages::warning("Cannot make observation valuation");
	}
}

template<typename ValueType, typename StateType = uint32_t>
void
ThreadsafePrismNextStateGenerator<ValueType, StateType>::addStateValuation(
	storm::storage::sparse::state_type const& currentStateIndex
	, storm::storage::sparse::StateValuationsBuilder& valuationsBuilder
) const {

}

template class ThreadsafePrismNextStateGenerator<double>;

} // namespace generator
} // namespace threadsafe
} // namespace stamina


