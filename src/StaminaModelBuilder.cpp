/**
 * Implementation for StaminaModelBuilder methods.
 * 
 * Created by Josh Jeppson on 8/17/2021
 * */
#include "StaminaModelBuilder.h"

using namespace stamina;

template <typename ValueType, typename RewardModelType, typename StateType>
StaminaModelBuilder<ValueType, RewardModelType, StateType>::StaminaModelBuilder(
    std::shared_ptr<storm::generator::NextStateGenerator<ValueType, StateType>> const& generator
    , Options * options
    , std::function<void(std::string)> err
    , std::function<void(std::string)> warn
    , std::function<void(std::string)> info
    , std::function<void(std::string)> good
) : generator(generator)
    , stateStorage(generator->getStateSize())
    , options(options)
    , err(err)
    , warn(warn)
    , info(info)
    , good(good)
{
    // Intentionally left empty
}

template <typename ValueType, typename RewardModelType, typename StateType>
StaminaModelBuilder<ValueType, RewardModelType, StateType>::StaminaModelBuilder(
    storm::prism::Program const& program
    , storm::generator::NextStateGeneratorOptions const& generatorOptions = storm::generator::NextStateGeneratorOptions()
    , Options * options
    , std::function<void(std::string)> err
    , std::function<void(std::string)> warn
    , std::function<void(std::string)> info
    , std::function<void(std::string)> good
) : StaminaModelBuilder( // Invoke other constructor
    std::make_shared<storm::generator::PrismNextStateGenerator<ValueType, StateType>>(program, generatorOptions)
    , options
    , err
    , warn
    , info
    , good
)
{
    // Intentionally left empty
}

template <typename ValueType, typename RewardModelType, typename StateType>
StaminaModelBuilder<ValueType, RewardModelType, StateType>::StaminaModelBuilder(
    storm::jani::Model const& model
    , storm::generator::NextStateGeneratorOptions const& generatorOptions = storm::generator::NextStateGeneratorOptions()
    , Options * options
    , std::function<void(std::string)> err
    , std::function<void(std::string)> warn
    , std::function<void(std::string)> info
    , std::function<void(std::string)> good
) : StaminaModelBuilder( // Invoke other constructor
    std::make_shared<storm::generator::JaniNextStateGenerator<ValueType, StateType>>(model, generatorOptions)
    , options
    , err
    , warn
    , info
    , good
)
{
    // Intentionally left empty
}

std::shared_ptr<storm::models::sparse::Model<ValueType, RewardModelType>> 
StaminaModelBuilder<ValueType, RewardModelType, StateType>::build() {
    switch (generator->getModelType()) {
        // Only supports CTMC models.
        case storm::generator::ModelType::CTMC:
            // TODO: truncate state space and return model
            err("Model building has not been implemented yet");
            break;
        case storm::generator::ModelType::DTMC:
        case storm::generator::ModelType::MDP:
        case storm::generator::ModelType::POMDP:
        case storm::generator::ModelType::MA:
        default:
            err("This model type is not supported!");
    }
}


// Explicitly instantiate the class.
template class StaminaModelBuilder<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;
// template class StaminaStateLookup<uint32_t>;

#ifdef STORM_HAVE_CARL
template class StaminaModelBuilder<RationalNumber, storm::models::sparse::StandardRewardModel<RationalNumber>, uint32_t>;
template class StaminaModelBuilder<RationalFunction, storm::models::sparse::StandardRewardModel<RationalFunction>, uint32_t>;
template class StaminaModelBuilder<double, storm::models::sparse::StandardRewardModel<storm::Interval>, uint32_t>;
#endif