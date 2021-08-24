/**
 * Stamina Model Builder Class
 * Created by Josh Jeppson on 8/17/2021
 * 
 * If you look closely, you'll see this is fairly similar to storm::builder::ExplicitModelBuilder
 * */
#ifndef STAMINAMODELBUILDER_H
#define STAMINAMODELBUILDER_H

#include <memory>
#include <utility>
#include <vector>
#include <deque>
#include <cstdint>
#include <functional>

#include "Options.h"
#include "ProbState.h"

#include <boost/functional/hash.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/variant.hpp>

#include "storm/builder/ExplicitModelBuilder.h"
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


namespace stamina {

    using namespace storm::builder;
    using namespace storm::utility::prism;
    using namespace storm::generator;

    template<typename ValueType, typename RewardModelType = storm::models::sparse::StandardRewardModel<ValueType>, typename StateType = uint32_t>
    class StaminaModelBuilder  {
    public:
        /**
         * Constructs a StaminaModelBuilder with a given storm::generator::NextStateGenerator
         * 
         * @param options A pointer to the main stamina::Options
         * @param err Lambda to error function
         * @param warm Lambda to warning function
         * @param info Lambda to info function
         * @param good Lambda to good function
         * @param generator The generator we are going to use.
         * */
        StaminaModelBuilder(
            Options * options
            , std::function<void(std::string)> err
            , std::function<void(std::string)> warn
            , std::function<void(std::string)> info
            , std::function<void(std::string)> good
            , std::shared_ptr<storm::generator::NextStateGenerator<ValueType, StateType>> const& generator
        );
        /**
         * Constructs a StaminaModelBuilder with a PRISM program and generatorOptions
         * 
         * @param options A pointer to the main stamina::Options
         * @param err Lambda to error function
         * @param warm Lambda to warning function
         * @param info Lambda to info function
         * @param good Lambda to good function
         * @param program The PRISM program we are going to use to build the model with.
         * @param generatorOptions Options for the storm::generator::NextStateGenerator we are going to use.
         * */
        StaminaModelBuilder(
            Options * options
            , std::function<void(std::string)> err
            , std::function<void(std::string)> warn
            , std::function<void(std::string)> info
            , std::function<void(std::string)> good
            , storm::prism::Program const& program
            , storm::generator::NextStateGeneratorOptions const& generatorOptions = storm::generator::NextStateGeneratorOptions()
        );
        /**
         * Constructs a StaminaModelBuilder with a JANI model.
         * 
         * @param options A pointer to the main stamina::Options
         * @param err Lambda to error function
         * @param warm Lambda to warning function
         * @param info Lambda to info function
         * @param good Lambda to good function
         * @param model The JANI model we're going to use.
         * @param generatorOptions Options for the storm::generator::NextStateGenerator we are going to use.
         * */
        StaminaModelBuilder(
            Options * options
            , std::function<void(std::string)> err
            , std::function<void(std::string)> warn
            , std::function<void(std::string)> info
            , std::function<void(std::string)> good
            , storm::jani::Model const& model
            , storm::generator::NextStateGeneratorOptions const& generatorOptions = storm::generator::NextStateGeneratorOptions()
        );
        /**
         * Creates a model with a truncated state space for the program provided during construction. State space
         * is truncated during this method using the STAMINA II truncation method described by Riley Roberts and Zhen
         * Zhang, and corresponding to the same algorithm used in the Java version of STAMINA.
         * 
         * @return The truncated model.
         * */
        std::shared_ptr<storm::models::sparse::Model<ValueType, RewardModelType>> build();
    protected:
        /**
         * Gets the state ID of a current state, or adds it to the internal state storage.
         * 
         * @param state Pointer to the state we are looking it
         * @return A pair with the state id and whether or not it was already discovered
         * */
        StateType getOrAddStateIndex(CompressedState const& state);
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
            , ChoiceInformationBuilder& choiceInformationBuilder
            , boost::optional<storm::storage::BitVector>& markovianChoices
            , boost::optional<storm::storage::sparse::StateValuationsBuilder>& stateValuationsBuilder
        );
        /**
         * Explores state space and truncates the model
         * 
         * @return The components of the truncated model
         * */
        storm::storage::sparse::ModelComponents<ValueType, RewardModelType> buildModelComponents();
        /**
         * Builds state labeling for our program
         * 
         * @return State labeling for our program
         * */
        storm::models::sparse::StateLabeling buildStateLabeling();
        /**
         * Performs reachability analysis
         * */
        void doReachabilityAnalysis();
        /**
         * Sets our reachability threshold
         * 
         * @param threshold The new reachability threshold
         * */
        void setReachabilityThreshold(double threshold);
    private:
        /* Data Members */
        std::function<void(std::string)> err;
        std::function<void(std::string)> warn;
        std::function<void(std::string)> info;
        std::function<void(std::string)> good;
        Options * options;
        storm::storage::sparse::StateStorage<StateType> stateStorage;
        std::shared_ptr<storm::generator::NextStateGenerator<ValueType, StateType>> generator;
        std::deque<std::pair<CompressedState, StateType>> statesToExplore;
        boost::optional<std::vector<uint_fast64_t>> stateRemapping;
        std::unordered_map<StateType, ProbState> stateMap;
        double reachabilityThreshold;
    };
}
#endif // STAMINAMODELBUILDER_H