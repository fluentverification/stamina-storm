/**
 * Threadsafe drop-in replacement to storm::generator::PrismNextStateGenerator
 * Overrides the following methods:
 *   storm::storage::sparse::StateValuations NextStateGenerator<ValueType, StateType>::makeObservationValuation() const
 *   void NextStateGenerator<ValueType, StateType>::addStateValuation(storm::storage::sparse::state_type const& currentStateIndex, storm::storage::sparse::StateValuationsBuilder& valuationsBuilder) const
 * */
#ifndef STAMINA_THREADSAFE_GENERATOR_THREADSAFEPRISMNEXTSTATEGENERATOR_H
#define STAMINA_THREADSAFE_GENERATOR_THREADSAFEPRISMNEXTSTATEGENERATOR_H

#include <mutex>

#include "storm/generator/PrismNextStateGenerator.h"

namespace stamina {
	namespace threadsafe {
		namespace generator {

			template<typename ValueType, typename StateType = uint32_t>
			class ThreadsafePrismNextStateGenerator : public storm::generator::PrismNextStateGenerator<ValueType, StateType> {
			public:
				ThreadsafePrismNextStateGenerator(
					storm::prism::Program const& program
					, storm::generator::NextStateGeneratorOptions const& options = NextStateGeneratorOptions()
					, std::shared_ptr<ActionMask<ValueType, StateType>> const& = nullptr
				);
				storm::storage::sparse::StateValuations makeObservationValuation() const override;
				void addStateValuation(
					storm::storage::sparse::state_type const& currentStateIndex
					, storm::storage::sparse::StateValuationsBuilder& valuationsBuilder
				) const override;
			private:
				std::mutex variableInformationMutex;
				// Variable information should be set up *before* calls to expand()
				bool varInfoWasSetUp;
			};
		} // namespace generator
	} // namespace threadsafe
} // namespace stamina

#endif // STAMINA_THREADSAFE_GENERATOR_THREADSAFEPRISMNEXTSTATEGENERATOR_H

