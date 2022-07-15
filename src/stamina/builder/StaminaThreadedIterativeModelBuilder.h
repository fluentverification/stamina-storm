/**
 * There are some benefits if only using one thread to using just the StaminaModelBuilder
 * class or StaminaIterativeModelBuilder class. This class is used if and only if the threadcount
 * is greater than 1.
 *
 * Simply including threading makes it so that there is some nontrivial overhead in exploration,
 * and we want to provide the users the option without having them have to have a control thread
 * idling and waiting for the exploration thread.
 *
 * Created by Josh Jeppson on Jul 15, 2022
 * */
#ifndef STAMINA_BUILDER_STAMINATHREADEDITERATIVEMODELBUILDER_H
#define STAMINA_BUILDER_STAMINATHREADEDITERATIVEMODELBUILDER_H

#include "StaminaModelBuilder.h"

namespace stamina {
	namespace builder {
		template<typename ValueType, typename RewardModelType = storm::models::sparse::StandardRewardModel<ValueType>, typename StateType = uint32_t>
		class StaminaThreadedIterativeModelBuilder : public StaminaIterativeModelBuilder<ValueType, RewardModelType, StateType> {
		public:
			/**
			* Constructs a StaminaThreadedIterativeModelBuilder with a given storm::generator::PrismNextStateGenerator. Invokes super's constructor
			*
			* @param generator The generator we are going to use.
			* */
			StaminaThreadedIterativeModelBuilder(
				std::shared_ptr<storm::generator::PrismNextStateGenerator<ValueType, StateType>> const& generator
				, storm::prism::Program const& modulesFile
				, storm::generator::NextStateGeneratorOptions const & options
			);
			/**
			* Constructs a StaminaThreadedIterativeModelBuilder with a PRISM program and generatorOptions. Invokes super's constructor
			*
			* @param program The PRISM program we are going to use to build the model with.
			* @param generatorOptions Options for the storm::generator::PrismNextStateGenerator we are going to use.
			* */
			StaminaThreadedIterativeModelBuilder(
				storm::prism::Program const& program
				, storm::generator::NextStateGeneratorOptions const& generatorOptions = storm::generator::NextStateGeneratorOptions()
			);
						/**
			* Builds transition matrix of truncated state space for the given program.
			*
			* @param transitionMatrixBuilder The builder of the transition matrix.
			* @param rewardModelBuilders The builders for the selected reward models.
			* @param choiceInformationBuilder The builder for the requested information of the choices
			* @param markovianChoices is set to a bit vector storing whether a choice is Markovian (is only set if the model type requires this information).
			* @param stateValuationsBuilder if not boost::none, we insert valuations for the corresponding states
			* */
			void buildMatrices(
				storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
				, std::vector<RewardModelBuilder<typename RewardModelType::ValueType>>& rewardModelBuilders
				, StateAndChoiceInformationBuilder& choiceInformationBuilder
				, boost::optional<storm::storage::BitVector>& markovianChoices
				, boost::optional<storm::storage::sparse::StateValuationsBuilder>& stateValuationsBuilder
			) override;
		private:
		};
		// "Custom" deleter (which actually is not custom) to allow for polymorphic shared pointers
		template<typename ValueType, typename RewardModelType = storm::models::sparse::StandardRewardModel<ValueType>, typename StateType = uint32_t>
		void __delete_stamina_iterative_model_builder(StaminaThreadedIterativeModelBuilder<ValueType, RewardModelType, StateType> * t) { delete t; }
	}
}

#endif // STAMINA_BUILDER_STAMINATHREADEDITERATIVEMODELBUILDER_H
