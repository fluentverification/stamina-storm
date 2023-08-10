# Model Builders

All model builders must inherit from `StaminaModelBuilder` and must re-implement the following methods:

```cpp
void buildMatrices(
	storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder
	, std::vector<RewardModelBuilder<typename RewardModelType::ValueType>>& rewardModelBuilders
	, StateAndChoiceInformationBuilder& choiceInformationBuilder
	, boost::optional<storm::storage::BitVector>& markovianChoices
	, boost::optional<storm::storage::sparse::StateValuationsBuilder>& stateValuationsBuilder
);
```

and

```cpp
storm::storage::sparse::ModelComponents<ValueType, RewardModelType> buildModelComponents()
```

Generally:

- `buildModelComponents()` calls `buildMatrices()`, and then builds components from the resulting sparse matrices.
