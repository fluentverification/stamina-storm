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
- `buildMatrices()` should be where state-space exploration occurs, and it should:
	+ Create a callback to `StateType getOrAddStateIndex(CompressedState const& state)` which it gives to the `nextStateGenerator` instance. Inherited classes may reimplement `getOrAddStateIndex` if needed, although a base implementation is provided.
	+ During exploration, it should *load* the current state into the next state generator, and then call `expand()` to get the successors. `nextStateGenerator` (part of `storm::generator::` and an instance of `PrismNextStateGenerator`) calls `getOrAddStateIndex`.
	+ `getOrAddStateIndex` *should enqueue in your exploration queue!*
- `flushToTransitionMatrix()` should be called at the end of `buildMatrices()`
- If using the absorbing state, `setUpAbsorbingState()` should be called at the beginning of running, since the index of the absorbing state should be `0`.
- `StaminaModelBuilder` and inherited classes are templated. They use the following template types:
	+ `ValueType`: Generally `double`, the type in the sparse matrices
	+ `RewardModelType`: Kept for Storm compatibility. Not used generally by STAMINA. Used for reward models
	+ `StateType`: The type (usually a type of unsigned integer) for state indecies.

Caveats:

- Since `StaminaModelBuilder` is forward-declared in `BaseThread.h` the default types for `ValueType` (no default), `RewardModelType` (`storm::models::sparse::StandardRewardModel<ValueType>`), and `StateType` (`uint32_t`) are defined in that file, *not* in `StaminaModelBuilder.h`.
