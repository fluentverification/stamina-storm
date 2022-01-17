#ifndef STAMINA_NEXT_STATE_GENERATOR_H
#define STAMINA_NEXT_STATE_GENERATOR_H

#include "storm/generator/PrismNextStateGenerator.h"

namespace stamina {
	using namespace storm::generator;

	template<typename ValueType, typename StateType = uint32_t>
	class StaminaNextStateGenerator : public PrismNextStateGenerator<ValueType, StateType> {
	public:
		/**
		 * Overrides the expand() method in storm::generator::PrismNextStateGenerator.
		 * Creates a StateBehavior for the currently loaded state
		 *
		 * @param stateToIdCallback The callback to the ModelBuilder's enqueue method
		 * @return A list of the behaviors/choices/states in the next states
		 * */
		virtual StateBehavior<ValueType, StateType> expand(
			StateToIdCallback const& stateToIdCallback
		) override;
	private:
		/**
		 * Retrieves all choices that are definitely asynchronous, possible fromm the given state.
		 * This is a de-facto "override" of the corresponding private method in PrismNextStateGenerator
		 * that we can't actually override or use.
		 *
		 * @param state The state for which to retrieve the unlabeled choices.
		 * @param stateToIdCallback The callback to the ModelBuilder's enqueue method
		 * @param commandFilter A filter for which command to use
		 *
		 * @return A vector of choices which are asynchronous
		 * */
		std::vector<Choice<ValueType>> getAsynchronousChoices(
			CompressedState const& state
			, StateToIdCallback stateToIdCallback
			, CommandFilter const& commandFilter = CommandFilter::All
		);
		/**
		 * Retrieves all (potentially) synchronous choices possible from the given state.
		 * Note that these may include choices that run asynchronously for this state.
		 * This is a de-facto "override" of the corresponding private method in PrismNextStateGenerator
		 * that we can't actually override or use.
		 *
		 * @param choices The new choices inserted into this vector
		 * @param state The state for which to retrieve the unlabeled choices.
		 * @param stateToIdCallback The callback to the ModelBuilder's enqueue method
		 * @param commandFilter A filter for which command to use
		 * */
		void addSynchronousChoices(
			std::vector<Choice<ValueType>>& choices
			, CompressedState const& state
			, StateToIdCallback stateToIdCallback
			, CommandFilter const& commandFilter = CommandFilter::All
		);
		/**
		 * Recursive helper function to get a synchronized distribution
		 *
		 * @param state
		 * */
		void generateSynchronizedDistribution(
			storm::storage::BitVector const& state
			, ValueType const& probability
			, uint64_t position
			, std::vector<std::vector<std::reference_wrapper<storm::prism::Command const>>::const_iterator> const& iteratorList
			, storm::builder::jit::Distribution<StateType, ValueType>& distribution
			, StateToIdCallback stateToIdCallback
		);
		/**
		 * Applies an update to the state currently loaded into the evaluator and applies the resulting values to
		 * the given compressed state.
		 *
		 * @params state The state to which to apply the new values.
		 * @params update The update to apply.
		 * @return The resulting state.
		 * */
		CompressedState applyUpdate(
			CompressedState const& state
			, storm::prism::Update const& update
		);
	};
} // namespace stamina

#endif // STAMINA_NEXT_STATE_GENERATOR_H
