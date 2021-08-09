/*
Created by Joshua Jeppson on 8/9/2021
*/

#ifndef STAMINA_EXPLICIT_MODEL_BUILDER_H
#define STAMINA_EXPLICIT_MODEL_BUILDER_H

#include <memory>
#include <utility>
#include <vector>
#include <deque>
#include <cstdint>
#include <boost/functional/hash.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/variant.hpp>
#include "storm/models/sparse/StandardRewardModel.h"

#include "storm/storage/prism/Program.h"
#include "storm/storage/expressions/ExpressionEvaluator.h"
#include "storm/storage/BitVectorHashMap.h"
#include "storm/logic/Formulas.h"
#include "storm/models/sparse/Model.h"
#include "storm/models/sparse/StateLabeling.h"
#include "storm/models/sparse/ChoiceLabeling.h"
#include "storm/storage/SparseMatrix.h"
#include "storm/storage/sparse/ModelComponents.h"
#include "storm/storage/sparse/StateStorage.h"
#include "storm/settings/SettingsManager.h"

#include "storm/utility/prism.h"

#include "storm/builder/ExplorationOrder.h"

#include "storm/generator/NextStateGenerator.h"
#include "storm/generator/CompressedState.h"
#include "storm/generator/VariableInformation.h"

#include "storm/builder/ExplicitModelBuilder.h"

namespace storm {
    namespace builder {

        using namespace storm::utility::prism;
        using namespace storm::generator;

        // Forward-declare classes.
        template <typename ValueType> class RewardModelBuilder;
        class StateAndChoiceInformationBuilder;

        template<typename ValueType, typename RewardModelType = storm::models::sparse::StandardRewardModel<ValueType>, typename StateType = uint32_t>
        class StaminaExplicitModelBuilder : ExplicitModelBuilder<ValueType, RewardModelType, StateType> {
        public:
            struct Options {
                /*!
                 * Creates an object representing the default building options.
                 */
                Options();
                
                // The order in which to explore the model.
                ExplorationOrder explorationOrder;
            };
            /* The two constructors I think we need from ExplicitModelBuilder */
            /*!
             * Creates an explicit model builder that uses the provided generator.
             *
             * @param generator The generator to use.
             */
            StaminaExplicitModelBuilder(std::shared_ptr<storm::generator::NextStateGenerator<ValueType, StateType>> const& generator, Options const& options = Options());

            /*!
             * Creates an explicit model builder for the given PRISM program.
             *
             * @param program The program for which to build the model.
             */
            StaminaExplicitModelBuilder(storm::prism::Program const& program, storm::generator::NextStateGeneratorOptions const& generatorOptions = storm::generator::NextStateGeneratorOptions(), Options const& builderOptions = Options());
            /*!
             * Creates an explicit model builder for the given JANI model.
             *
             * @param model The JANI model for which to build the model.
             */
            StaminaExplicitModelBuilder(storm::jani::Model const& model, storm::generator::NextStateGeneratorOptions const& generatorOptions = storm::generator::NextStateGeneratorOptions(), Options const& builderOptions = Options());
            
            /*!
             * Convert the program given at construction time to an abstract model. The type of the model is the one
             * specified in the program. The given reward model name selects the rewards that the model will contain.
             * This method overrides the superclass's
             *
             * @return The explicit model that was given by the probabilistic program as well as additional
             *         information (if requested).
             */
            std::shared_ptr<storm::models::sparse::Model<ValueType, RewardModelType>> build();
        private:
            /*!
             * Gets the state index as an integer.
             *
             * @param state A pointer to a state for which to retrieve the index. This must not be used after the call.
             * @return State index as integer
             */
            StateType getOrAddStateIndex(CompressedState const& state);
            /*!
             * Overrides the storm::builder::ExplicitModelBuilder::buildMatrices so we can bind to proper getOrAddStateIndex
             * 
             * @param transitionMatrixBuilder The builder of the transition matrix.
             * @param rewardModelBuilders The builders for the selected reward models.
             * @param stateAndChoiceInformationBuilder The builder for the requested information of the individual states and choices
             */
            void buildMatrices(storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder, std::vector<RewardModelBuilder<typename RewardModelType::ValueType>>& rewardModelBuilders, StateAndChoiceInformationBuilder& stateAndChoiceInformationBuilder);
            /*!
             * Explores the state space of the given program and returns the components of the model as a result.
             *
             * @return A structure containing the components of the resulting model.
             */
            storm::storage::sparse::ModelComponents<ValueType, RewardModelType> buildModelComponents();
            /*!
             * Builds the state labeling for the given program.
             *
             * @return The state labeling of the given program.
             */
            storm::models::sparse::StateLabeling buildStateLabeling();
             /// The generator to use for the building process.
            std::shared_ptr<storm::generator::NextStateGenerator<ValueType, StateType>> generator;

            /// The options to be used for the building process.
            Options options;

            /// Internal information about the states that were explored.
            storm::storage::sparse::StateStorage<StateType> stateStorage;

            /// A set of states that still need to be explored.
            std::deque<std::pair<CompressedState, StateType>> statesToExplore;

            /// An optional mapping from state indices to the row groups in which they actually reside. This needs to be
            /// built in case the exploration order is not BFS.
            boost::optional<std::vector<uint_fast64_t>> stateRemapping;
        };
    }
}

#endif // STAMINA_EXPLICIT_MODEL_BUILDER_H