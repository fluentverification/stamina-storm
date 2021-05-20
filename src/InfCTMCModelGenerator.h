

#ifndef STORM_BUILDER_INFCTMCGENERATOR_H
#define STORM_BUILDER_INFCTMCGENERATOR_H
 
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

#include "ProbState.h"

template<typename ValueType> class ConstantsComparator;

using namespace storm::utility::prism;
using namespace storm::generator;


         
// Forward-declare classes.
namespace storm{
    namespace builder {
        template <typename ValueType> class RewardModelBuilder;
        class ChoiceInformationBuilder;
    }
}
             
         
template<typename ValueType, typename RewardModelType = storm::models::sparse::StandardRewardModel<ValueType>, typename StateType = uint32_t>
class InfCTMCModelGenerator {
    public:
             
        struct Options {
            Options();
            
            // The order in which to explore the model.
            storm::builder::ExplorationOrder explorationOrder;
        };
        
        InfCTMCModelGenerator(
            std::shared_ptr<storm::generator::NextStateGenerator<ValueType, StateType>> const& generator
            , Options const& options = Options()
        );

        InfCTMCModelGenerator(
            storm::prism::Program const& program
            , storm::generator::NextStateGeneratorOptions const& generatorOptions = storm::generator::NextStateGeneratorOptions()
            , Options const& builderOptions = Options()
        );

        InfCTMCModelGenerator(
            storm::jani::Model const& model
            , storm::generator::NextStateGeneratorOptions const& generatorOptions = storm::generator::NextStateGeneratorOptions()
            , Options const& builderOptions = Options()
        );
        
        std::shared_ptr<storm::models::sparse::Model<ValueType, RewardModelType>> build(storm::generator::VariableInformation const& variableInformation);
             
    private:
        storm::generator::VariableInformation variableInformation;
        std::unordered_map<int, ProbState*> stateMap;
        double currentStateReachability;
        StateType getOrAddStateIndex(CompressedState const& state);
        StateType getAbsorbingStateIndex(CompressedState const& state);

        void buildMatrices(
            storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
            , std::vector<storm::builder::RewardModelBuilder<typename RewardModelType::ValueType>>& RewardModelBuilders
            , storm::builder::ChoiceInformationBuilder& ChoiceInformationBuilder
            , boost::optional<storm::storage::BitVector>& markovianChoices
        );
        
        storm::storage::sparse::ModelComponents<ValueType, RewardModelType> buildModelComponents();
        
        storm::models::sparse::StateLabeling buildStateLabeling();
        
        std::shared_ptr<storm::generator::NextStateGenerator<ValueType, StateType>> generator;
        
        Options options;

        storm::storage::sparse::StateStorage<StateType> stateStorage;
        
        std::deque<std::pair<CompressedState, StateType>> statesToExplore;
        
        boost::optional<std::vector<uint_fast64_t>> stateRemapping;

};

#endif  /* STORM_BUILDER_INFCTMCGENERATOR_H */