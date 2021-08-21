/**
 * Implementation for StaminaModelBuilder methods.
 * 
 * Created by Josh Jeppson on 8/17/2021
 * */
#include "StaminaModelBuilder.h"

using namespace stamina;

template <typename ValueType, typename RewardModelType, typename StateType>
StaminaModelBuilder<ValueType, RewardModelType, StateType>::StaminaModelBuilder(
    Options * options
    , std::function<void(std::string)> err
    , std::function<void(std::string)> warn
    , std::function<void(std::string)> info
    , std::function<void(std::string)> good
    , std::shared_ptr<storm::generator::NextStateGenerator<ValueType, StateType>> const& generator
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
    Options * options
    , std::function<void(std::string)> err
    , std::function<void(std::string)> warn
    , std::function<void(std::string)> info
    , std::function<void(std::string)> good
    , storm::prism::Program const& program
    , storm::generator::NextStateGeneratorOptions const& generatorOptions
) : StaminaModelBuilder( // Invoke other constructor
    options
    , err
    , warn
    , info
    , good
    , std::make_shared<storm::generator::PrismNextStateGenerator<ValueType, StateType>>(program, generatorOptions)
)
{
    // Intentionally left empty
}

template <typename ValueType, typename RewardModelType, typename StateType>
StaminaModelBuilder<ValueType, RewardModelType, StateType>::StaminaModelBuilder(
    Options * options
    , std::function<void(std::string)> err
    , std::function<void(std::string)> warn
    , std::function<void(std::string)> info
    , std::function<void(std::string)> good
    , storm::jani::Model const& model
    , storm::generator::NextStateGeneratorOptions const& generatorOptions
) : StaminaModelBuilder( // Invoke other constructor
    options
    , err
    , warn
    , info
    , good
    , std::make_shared<storm::generator::JaniNextStateGenerator<ValueType, StateType>>(model, generatorOptions)
)
{
    // Intentionally left empty
}

template <typename ValueType, typename RewardModelType, typename StateType>
std::shared_ptr<storm::models::sparse::Model<ValueType, RewardModelType>> 
StaminaModelBuilder<ValueType, RewardModelType, StateType>::build() {
    switch (generator->getModelType()) {
        // Only supports CTMC models.
        case storm::generator::ModelType::CTMC:
            return storm::utility::builder::buildModelFromComponents(storm::models::ModelType::Ctmc, buildModelComponents());
        case storm::generator::ModelType::DTMC:
        case storm::generator::ModelType::MDP:
        case storm::generator::ModelType::POMDP:
        case storm::generator::ModelType::MA:
        default:
            err("This model type is not supported!");
    }
    return nullptr;
}

template <typename ValueType, typename RewardModelType, typename StateType>
StateType 
StaminaModelBuilder<ValueType, RewardModelType, StateType>::getOrAddStateIndex(CompressedState const& state) {

}

template <typename ValueType, typename RewardModelType, typename StateType>
void 
StaminaModelBuilder<ValueType, RewardModelType, StateType>::buildMatrices(
    storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
    , std::vector<RewardModelBuilder<typename RewardModelType::ValueType>>& rewardModelBuilders
    , ChoiceInformationBuilder& choiceInformationBuilder
    , boost::optional<storm::storage::BitVector>& markovianChoices
    , boost::optional<storm::storage::sparse::StateValuationsBuilder>& stateValuationsBuilder
) {

}

template <typename ValueType, typename RewardModelType, typename StateType>
storm::storage::sparse::ModelComponents<ValueType, RewardModelType>
StaminaModelBuilder<ValueType, RewardModelType, StateType>::buildModelComponents() {
    
}

template <typename ValueType, typename RewardModelType, typename StateType>
storm::models::sparse::StateLabeling
StaminaModelBuilder<ValueType, RewardModelType, StateType>::buildStateLabeling() {

}


// Explicitly instantiate the class.
template class StaminaModelBuilder<double, storm::models::sparse::StandardRewardModel<double>, uint32_t>;

#ifdef STORM_HAVE_CARL
template class StaminaModelBuilder<storm::RationalNumber, storm::models::sparse::StandardRewardModel<storm::RationalNumber>, uint32_t>;
template class StaminaModelBuilder<storm::RationalFunction, storm::models::sparse::StandardRewardModel<storm::RationalFunction>, uint32_t>;
template class StaminaModelBuilder<double, storm::models::sparse::StandardRewardModel<storm::Interval>, uint32_t>;
#endif