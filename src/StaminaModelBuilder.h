/**
 * Stamina Model Builder Class
 * Created by Josh Jeppson on 8/17/2021
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
#include "storm/generator/JaniNextStateGenerator.h"
#include "storm/generator/PrismNextStateGenerator.h"

#include "storm/builder/ExplicitModelBuilder.h"

using namespace storm::builder;
using namespace storm::utility::prism;
using namespace storm::generator;

namespace stamina {
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

    private:
        /* Data Members */
        std::function<void(std::string)> err;
        std::function<void(std::string)> warn;
        std::function<void(std::string)> info;
        std::function<void(std::string)> good;
        Options * options;
        storm::storage::sparse::StateStorage<StateType> stateStorage;
        std::shared_ptr<storm::generator::NextStateGenerator<ValueType, StateType>> generator;

    };
}
#endif // STAMINAMODELBUILDER_H