#include "StaminaSparseMatrixBuilder.h"

namespace stamina {
	namespace util {
		template <typename ValueType>
		StaminaSparseMatrixBuilder<ValueType>::StaminaSparseMatrixBuilder(
			index_type rows = 0
			, index_type columns = 0
			, index_type entries = 0
			, bool forceDimensions = true
			, bool hasCustomRowGrouping = false
			, index_type rowGroups = 0
		) : // Invoke superclass constructor
			storm::storage::SparseMatrixBuilder<ValueType>(
				rows
				, columns
				, entries
				, forceDimensions
				, hasCustomRowGrouping
				, rowGroups
			)
		{
			// Intentionally left empty
		}

		template <typename ValueType>
		void
		StaminaMatrixBuilder<ValueType>::addNextValue(
			index_type row
			, index_type column
			, value_type const& value
		) {
			// Check if a diagonal entry shall be inserted before
			if (pendingDiagonalEntry) {
				index_type diagColumn = hasCustomRowGrouping ? currentRowGroupCount - 1 : lastRow;
				if (row > lastRow || column >= diagColumn) {
					ValueType diagValue = std::move(pendingDiagonalEntry.get());
					pendingDiagonalEntry = boost::none;
					// Add the pending diagonal value now
					if (row == lastRow && column == diagColumn) {
						// The currently added value coincides with the diagonal entry!
						// We add up the values and repeat this call.
						addNextValue(row, column, diagValue + value);
						// We return here because the above call already did all the work.
						return;
					}
					else {
						addNextValue(lastRow, diagColumn, diagValue);
					}
				}
			}

			// If the element is in the same row, but was not inserted in the correct order, we need to fix the row after
			// the insertion.
			bool fixCurrentRow = row == lastRow && column < lastColumn;
			// If the element is in the same row and column as the previous entry, we add them up...
			// unless there is no entry in this row yet, which might happen either for the very first entry or when only a diagonal value has been added
			if (row == lastRow && column == lastColumn && rowIndications.back() < currentEntryCount) {
				columnsAndValues.back().setValue(columnsAndValues.back().getValue() + value);
			}
			else {
				// If we switched to another row, we have to adjust the missing entries in the row indices vector.
				if (row != lastRow) {
					// Otherwise, we need to push the correct values to the vectors, which might trigger reallocations.
					assert(rowIndications.size() == lastRow + 1);
					rowIndications.resize(row + 1, currentEntryCount);
					lastRow = row;
				}

				lastColumn = column;

				// Finally, set the element and increase the current size.
				columnsAndValues.emplace_back(column, value);
				highestColumn = std::max(highestColumn, column);
				++currentEntryCount;

				// If we need to fix the row, do so now.
				if (fixCurrentRow) {
					// First, we sort according to columns.
					std::sort(columnsAndValues.begin() + rowIndications.back(), columnsAndValues.end(),
							  [](storm::storage::MatrixEntry<index_type, ValueType> const& a, storm::storage::MatrixEntry<index_type, ValueType> const& b) {
								  return a.getColumn() < b.getColumn();
							  });

					auto insertIt = columnsAndValues.begin() + rowIndications.back();
					uint64_t elementsToRemove = 0;
					for (auto it = insertIt + 1; it != columnsAndValues.end(); ++it) {
						// Iterate over all entries in this last row and detect duplicates.
						if (it->getColumn() == insertIt->getColumn()) {
							// This entry is a duplicate of the column. Update the previous entry.
							insertIt->setValue(insertIt->getValue() + it->getValue());
							elementsToRemove++;
						}
						else {
							insertIt = it;
						}
					}
					// Then, we eliminate those duplicate entries.
					std::unique(columnsAndValues.begin() + rowIndications.back(), columnsAndValues.end(),
								[](storm::storage::MatrixEntry<index_type, ValueType> const& a, storm::storage::MatrixEntry<index_type, ValueType> const& b) {
									return a.getColumn() == b.getColumn();
								});

					if (elementsToRemove > 0) {
						StaminaMessages::warning("Unordered insertion into matrix builder caused duplicate entries.");
						currentEntryCount -= elementsToRemove;
						columnsAndValues.resize(columnsAndValues.size() - elementsToRemove);
					}
					lastColumn = columnsAndValues.back().getColumn();
				}
			}
		}
	}
}
