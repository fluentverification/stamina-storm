#ifdef STAMINASPARSEMATRIXBUILDER_H
#define STAMINASPARSEMATRIXBUILDER_H
/**
 * Basic Sparse Matrix Builder class inheriting from STORM's SparseMatrixBuilder that,
 * in contrast to its STORM equivalent, allows for nonlinear row insertion.
 *
 * Created by Josh Jeppson on Jun 2, 2022
 * */

#include <storm/storage/SparseMatrix.h>

#include "StaminaMessages.h"

namespace stamina {
	namespace util {
		template <typename ValueType>
		class StaminaSparseMatrixBuilder : public storm::storage::SparseMatrixBuilder<ValueType> {
		public:
			// Typedefs to work well with storm
			typedef SparseMatrixIndexType index_type;
			typedef ValueType value_type;
			/**
			 * Invokes the superclass constructor within the STORM API to create a StaminaSparseMatrixBuilder.
			 * Note that these param notes are taken from STORM itself:
			 *
			 * @param rows The number of rows of the resulting matrix.
			 * @param columns The number of columns of the resulting matrix.
			 * @param entries The number of entries of the resulting matrix.
			 * @param forceDimensions If this flag is set, the matrix is expected to have exactly the given number of
			 * rows, columns and entries for all of these entities that are set to a nonzero value.
			 * @param hasCustomRowGrouping A flag indicating whether the builder is used to create a non-canonical
			 * grouping of rows for this matrix.
			 * @param rowGroups The number of row groups of the resulting matrix. This is only relevant if the matrix
			 * has a custom row grouping.
			 * */
			StaminaSparseMatrixBuilder(
				index_type rows = 0
				, index_type columns = 0
				, index_type entries = 0
				, bool forceDimensions = true
				, bool hasCustomRowGrouping = false
				, index_type rowGroups = 0
			);
			/**
			 * The only method we need to override from STORM's SparseMatrixBuilder class.
			 * This version of SparseMatrixBuilder allows for insertion out of order (nonlinear)
			 * in contrast to the STORM equivalent, which requires linear insertion. Nonlinear
			 * insertion is required for STAMINA because of how we explore the state space of a
			 * model.
			 * */
			void addNextValue(index_type row, index_type column, value_type const& value) override;
		};
	}
}

#endif // STAMINASPARSEMATRIXBUILDER_H
