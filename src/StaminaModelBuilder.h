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
#include <boost/functional/hash.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/variant.hpp>

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

#include "storm/builder/ExplicitModelBuilder.h"

using namespace storm::builder;
using namespace storm::utility::prism;
using namespace storm::generator;

namespace stamina {
    // template<typename ValueType, typename RewardModelType = storm::models::sparse::StandardRewardModel<ValueType>, typename StateType = uint32_t>
    class StaminaModelBuilder  {
    public:
        StaminaModelBuilder();
    protected:

    private:

    };
}
#endif // STAMINAMODELBUILDER_H